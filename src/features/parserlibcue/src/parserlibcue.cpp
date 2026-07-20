/**
 * \file
 *
 * \brief Implements libcue-based parser for CueSheets.
 */

#ifndef LIBARCSDEC_PARSERLIBCUE_HPP_
#include "parserlibcue.hpp"
#endif
#ifndef LIBARCSDEC_PARSERLIBCUE_DETAILS_HPP_
#include "parserlibcue_details.hpp"  // for LibcueParserImpl
#endif

extern "C" {
#include <libcue/libcue.h>
}

#include <cstdint>		// for uintmax_t
#include <iomanip>		// for setw
#include <ios>			// for right
#include <memory>		// for unique_ptr
#include <set>          // for set
#include <sstream>      // for ostringstream
#include <stdexcept>    // for invalid_argument
#include <string>       // for string
#include <utility>      // for move

#ifndef LIBARCSTK_METADATA_HPP_
#include <arcstk/metadata.hpp>    // for ToC, make_toc
#endif
#ifndef LIBARCSTK_LOGLEVEL_HPP_
#include <arcstk/loglevel.hpp>
#endif
#ifndef LIBARCSTK_LOGGING_HPP_
#include <arcstk/logging.hpp>
#endif

#ifndef LIBARCSDEC_DESCRIPTOR_HPP_
#include "descriptor.hpp"         // for Codec, Format
#endif
#ifndef LIBARCSDEC_METAPARSER_HPP_
#include "metaparser.hpp"         // for MetadataParseException
#endif
#ifndef LIBARCSDEC_METAPARSER_DETAILS_HPP_
#include "metaparser_details.hpp" // for cast_or_throw, file_content
#endif
#ifndef LIBARCSDEC_TOCHANDLER_HPP_
#include "tochandler.hpp"         // for ParserToCHandler
#endif
#ifndef LIBARCSDEC_SELECTION_HPP_
#include "selection.hpp"          // for RegisterDescriptor
#endif


// Note: This project requires libcue >= 2.0 but the code compiles fine with
// libcue 1.4. However, it will not work as expected with 1.4 since libcue 2.0
// introduced an API change in respect of handling track bounds:
//
// See:
// https://github.com/lipnitsk/libcue/commit/8855ccdb4b37908263a01751b81a7233498e08ab
//
// The computation of ARCSs requires that trailing gaps are appended to the
// previous track, as is documented here:
//
// https://wiki.hydrogenaud.io/index.php?title=AccurateRip#Checksum_calculation

// Note: This project requires libcue >= 2.3 but the code compiles fine with
// libcue 2.{0-2}*. Note that in those versions, regular cuesheets that
// have trailing newlines will erroneously cause errors, although they are
// syntactically correct.
//
// https://github.com/lipnitsk/libcue/issues/52

// Note: This project requires libcue > 2.3 but the code compiles fine with
// libcue 2.3. Note that libcue 2.3 has memory handling errors that cause memory
// leaks on some types of parsing errors.
//
// https://github.com/lipnitsk/libcue/issues/78

namespace arcsdec
{
inline namespace v_1_0_0
{
namespace read
{
namespace details::libcue
{

using arcstk::ToC;


// LibcueParserImpl


ToC LibcueParserImpl::convert(const CdPtr& cd) const
{
	// Signed integral type for amounts of lba frames.
	using lba_type = int32_t;

	const auto* cd_info   = cd.get();
	const int track_count = ::cd_get_ntrack(cd_info);

	// Types according to libcue-API
	auto trk_offset = long { 0 }; // NOLINT(google-runtime-int)
	using cstring = const char*;
	auto filename = cstring { nullptr };
	const ::Track* trk = nullptr; // non-owning, destroyed with cd
	// TODO MCN

	// Read offset, length + filename for each track in Cue file

	for (int i = 1; i <= track_count; ++i)
	{
		handler()->inc_current_track();
		// TODO ISRC

		trk = ::cd_get_track(cd_info, i);

		if (!trk)
		{
			ARCS_LOG_ERROR << "Could not retrieve track " << i;
			continue;
		}

		trk_offset = ::track_get_start(trk);

		if (trk_offset < 0)
		{
			ARCS_LOG_WARNING  << "Offset for track "   << i
				<< " is not expected to be negative: " << trk_offset;
		}

		if (trk_offset > arcstk::CDDA::MAX_BLOCK_ADDRESS)
		{
			ARCS_LOG_WARNING  << "Offset for track "   << i
				<< " exceeds maximal block address: " << trk_offset;
		}

		filename = ::track_get_filename(trk);

		// Log the contents

		ARCS_LOG(DEBUG1) << "Cuesheet: track "
			<< std::right
			<< std::setw(2)
			<< i
			<< ": offset: "
			<< std::setw(6)
			<< trk_offset
			<< ", file: " << (filename ? filename : "<none>");

		try
		{
			handler()->append_offset(trk_offset);

			if (filename)
			{
				handler()->append_filename(filename);
			}

		} catch (const std::invalid_argument& e)
		{
			auto msg = std::ostringstream{};
			msg << "Track " << i << ": ";
			msg << e.what();
		}
	}

	return handler()->get_toc();
}


ToC LibcueParserImpl::parse_worker(const std::string& filename) const
{
	handler()->start_input();

	ARCS_LOG(DEBUG1) << "Start reading Cuesheet file with libcue: "
		<< filename;

	const auto MAX_CUESHEET_SIZE = std::uintmax_t { 10240 }; // bytes
	const auto cuesheet = file_content(filename, MAX_CUESHEET_SIZE);

	if (!cuesheet)
	{
		auto message = std::ostringstream{};
		message << "Failed to load file: " << filename;

		throw MetadataParseException(message.str());
	}

	ARCS_LOG(DEBUG1) << "Cuesheet file successfully read";

	auto toc_ptr = CdPtr { ::cue_parse_string(cuesheet.value().data()) };

	if (!toc_ptr)
	{
		auto message = std::ostringstream{};
		message << "Failed to parse Cuesheet file: " << filename;

		throw MetadataParseException(message.str());
	}

	auto toc = convert(toc_ptr);

	handler()->end_input();

	ARCS_LOG(DEBUG1) << "Cuesheet file successfully parsed";

	return toc;
}


ToC LibcueParserImpl::do_parse(const std::string& filename)
{
	return parse_worker(filename);
}


std::unique_ptr<FileReaderDescriptor> LibcueParserImpl::do_descriptor() const
{
	return std::make_unique<DescriptorLibcue>();
}

} // namespace details::libcue


// DescriptorLibcue


DescriptorLibcue::~DescriptorLibcue() noexcept = default;


std::string DescriptorLibcue::do_id() const noexcept
{
	return "libcue";
}


std::string DescriptorLibcue::do_name() const
{
	return "Libcue";
}


InputType DescriptorLibcue::do_input_type() const
{
	return InputType::TOC;
}


bool DescriptorLibcue::do_accepts_codec(Codec codec) const
{
	ARCS_LOG(DEBUG1) << "Is Codec NONE?";
	return codec == Codec::NONE;
}


std::set<Format> DescriptorLibcue::define_formats() const
{
	return { Format::CUE };
}


LibInfo DescriptorLibcue::do_libraries() const
{
	return { libinfo_entry_filepath("libcue") };
}


std::unique_ptr<FileReader> DescriptorLibcue::do_create_reader() const
{
	auto impl = std::make_unique<details::libcue::LibcueParserImpl>();
	return std::make_unique<MetadataParser>(std::move(impl));
}


std::unique_ptr<FileReaderDescriptor> DescriptorLibcue::do_clone() const
{
	return std::make_unique<DescriptorLibcue>();
}

} // namespace read

// Add this descriptor to the metadata descriptor registry

namespace {

using select::RegisterDescriptor;
using read::DescriptorLibcue;

const auto d = RegisterDescriptor<DescriptorLibcue>{};

} // namespace

} // namespace v_1_0_0
} // namespace arcsdec

