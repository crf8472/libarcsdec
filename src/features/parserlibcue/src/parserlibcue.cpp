/**
 * \file
 *
 * \brief Implements libcue-based parser for CueSheets.
 */

#ifndef LIBARCSDEC_PARSERLIBCUE_HPP_
#include "parserlibcue.hpp"
#endif
#ifndef LIBARCSDEC_PARSERLIBCUE_DETAILS_HPP_
#include "parserlibcue_details.hpp"  // for LibcueParserImpl, CueOpenFile
#endif

extern "C" {
#include <libcue/libcue.h>
}

#include <cstdio>    // for fopen, fclose, FILE
#include <iomanip>   // for setw
#include <ios>       // for right
#include <filesystem>// for file_size
#include <fstream>   // for ifstream
#include <memory>    // for unique_ptr
#include <optional>  // for optional
#include <set>       // for set
#include <sstream>   // for ostringstream
#include <stdexcept> // for invalid_argument
#include <string>    // for string
#include <tuple>     // for make_tuple
#include <vector>    // for vector
#include <utility>   // for move

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
#include "metaparser_details.hpp" // for cast_or_throw
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


namespace arcsdec
{
inline namespace v_1_0_0
{
namespace read
{
namespace details::libcue
{

using arcstk::ToC;
using arcstk::make_toc;

// FreeCd


void Free_Cd::operator()(::Cd* cd) const
{
	if (cd)
	{
		::cd_delete(cd);
		cd = nullptr;
	}
}


// Make_CdPtr


// CdPtr Make_CdPtr::operator()(const std::string& filename) const
// {
// 	// Parse file using libcue
//
// 	ARCS_LOG(DEBUG1) << "Start reading Cuesheet file with libcue: "
// 		<< filename;
//
// 	// Commented out, kept for reference
// 	//auto f = safe_open_for_read(filename);
// 	//auto cd = ::cue_parse_file(f.get());
//
// 	const auto MAX_CUESHEET_SIZE = std::uintmax_t { 10240 }; // bytes
//
// 	const auto cuesheet = file_content(filename, MAX_CUESHEET_SIZE);
//
// 	if (!cuesheet)
// 	{
// 		auto message = std::ostringstream{};
// 		message << "Failed to load file: " << filename;
//
// 		throw MetadataParseException(message.str());
// 	}
//
// 	auto cd = CdPtr { ::cue_parse_string(cuesheet.value().data()) };
//
// 	if (!cd)
// 	{
// 		auto message = std::ostringstream{};
// 		message << "Failed to parse Cuesheet file: " << filename;
//
// 		throw MetadataParseException(message.str());
// 	}
//
// 	// cue_parse_file() must complete synchronously and read the entire file
//     // before returning. The FILE* is closed immediately after this call.
//
// 	ARCS_LOG(DEBUG1) << "Cuesheet file successfully read";
//
// 	return cd;
// }


// Close_FILEPtr (Commented out since unused, kept for reference)


// void Close_FILEPtr::operator()(FILE* f) const
// {
// 	if (f)
// 	{
// 		// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
// 		if (std::fclose(f)) // fclose returns 0 on success & EOF on error
// 		{
// 			ARCS_LOG_ERROR << "Failed to close Cuesheet file";
// 		}
// 		f = nullptr;
// 	}
// }


// safe_open_for_read() (Commented out since unused, kept for reference)


// FILEPtr safe_open_for_read(const std::string& filename)
// {
// #ifdef MSC_SAFECODE
// 	FILE* temp = nullptr; // NOLINT(cppcoreguidelines-owning-memory)
// 	const int err = fopen_s(&temp, filename.c_str(), "r");
// 	auto f = FILEPtr { temp };
// 	const bool has_failed { err != 0 || f == nullptr };
// #else
// 	auto f = FILEPtr { std::fopen(filename.c_str(), "r") };
// 	const bool has_failed { f == nullptr };
// #endif
//
// 	if (has_failed)
// 	{
// 		auto message = std::ostringstream{};
// 		message << "Failed to open Cuesheet file: " << filename;
// 		throw FileReadException(message.str());
// 	}
//
// 	return f;
// }


// convert


ToC convert(const CdPtr& cd)
{
	if (!cd)
	{
		// TODO
		return ToC{};
	}


	// Signed integral type for amounts of lba frames.
	using lba_type = int32_t;

	const auto* cd_info = cd.get();

	// track count

	const int track_count = ::cd_get_ntrack(cd_info);

	// if (track_count < 0 or track_count > arcstk::CDDA::MAX_TRACKCOUNT)
	// {
	// 	auto msg = std::ostringstream{};
	// 	msg << "Invalid number of tracks: " << track_count;
	//
	// 	throw MetadataParseException(msg.str());
	// }

	// offset, lengths, filenames

	auto offsets   = std::vector<lba_type>{};
	auto lengths   = std::vector<lba_type>{};
	auto filenames = std::vector<std::string>{};

	using offsets_sz   = decltype( offsets )::size_type;
	using lengths_sz   = decltype( lengths )::size_type;
	using filenames_sz = decltype( filenames )::size_type;

	offsets.reserve(static_cast<offsets_sz>(track_count));
	lengths.reserve(static_cast<lengths_sz>(track_count));
	filenames.reserve(static_cast<filenames_sz>(track_count));

	// Types according to libcue-API
	auto trk_offset = long { 0 }; // NOLINT(google-runtime-int)
	auto trk_length = long { 0 }; // NOLINT(google-runtime-int)
	using cstring = const char*;
	auto filename  = cstring { nullptr };
	const ::Track* trk = nullptr;

	// Read offset, length + filename for each track in Cue file

	for (int i = 1; i <= track_count; ++i)
	{
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

		trk_length = ::track_get_length(trk);

		// Length of last track is allowed to be -1.
		if (i < track_count and trk_length < 0)
		{
			ARCS_LOG_WARNING << "Length for track "    << i
				<< " is not expected to be negative: " << trk_length;
		}

		filename = ::track_get_filename(trk);

		// Log the contents

		ARCS_LOG(DEBUG1) << "Cue Track "
			<< std::right
			<< std::setw(2)
			<< i
			<< ": offset: "
			<< std::setw(6)
			<< trk_offset
			<< ", length: "
			<< std::setw(6)
			<< trk_length
			<< ", file: " << (filename ? filename : "<null>");

		// NOTE that the length the last track cannot be calculated from
		// the Cuesheet which only contains the start offsets. To get the length
		// of the last track, you would have to subtract its offset from the
		// offset of the non-existent following track.

		try
		{
			offsets.emplace_back(cast_or_throw<lba_type>(trk_offset));
			//lengths.emplace_back(cast_or_throw<lba_type>(trk_length));

			if (filename)
			{
				filenames.emplace_back(filename);
			}

		} catch (const std::invalid_argument& e)
		{
			auto msg = std::ostringstream{};
			msg << "Track " << i << ": ";
			msg << e.what();

			//throw std::invalid_argument(msg.str());
		}
	}

	return make_toc(offsets, filenames);
}


std::uintmax_t file_size_or_throw(const std::string &filepath)
{
	namespace fs = std::filesystem;

	// Check existence

	if (!fs::exists(filepath))
	{
        throw std::runtime_error("File not found");
    }

	// Check file size

	std::error_code rc;
    const auto file_size { fs::file_size(filepath, rc) };

	if (rc)
	{
		auto msg = std::ostringstream{};

		msg << "Unable to determine file size for file '"
			<< filepath
			<< "'";/* + "', error was: " + rc */

		throw std::runtime_error(msg.str());
	}

	return file_size;
}


std::optional<std::vector<char>> file_content(const std::string &filepath,
		const std::uintmax_t max_size)
{
	// Get file size

	auto file_size = file_size_or_throw(filepath);

	if (file_size == 0)
	{
		return std::nullopt;
	}

	if (file_size > max_size)
	{
		auto msg = std::ostringstream{};

		msg << "File too large, more than maximum of "
			<< max_size
			<< " bytes";

		throw std::runtime_error(msg.str());
	}

	// Check before casting to signed type when passing it to ifstream::read()
	if (file_size > static_cast<std::uintmax_t>(
				std::numeric_limits<std::streamsize>::max()))
	{
		throw std::runtime_error(
				"File too large, is not readable in a single read operation");
	}

	// Open file

    auto input = std::ifstream { filepath };

    if (!input)
	{
		auto msg = std::ostringstream{};

		msg << "Unable to correctly open file '"
			<< filepath
			<< "'";

		throw std::runtime_error(msg.str());
    }

	input.exceptions(std::ios::failbit | std::ios::badbit);

	// Load file content into vector

	auto chars = std::vector<char>(file_size + 1); // parenthesis
	input.read(chars.data(), static_cast<std::streamsize>(file_size));
	chars.back() = '\0';

    return chars;
}


// LibcueParserImpl


ToC LibcueParserImpl::parse_worker(const std::string& filename) const
{
	// Parse file using libcue

	ARCS_LOG(DEBUG1) << "Start reading Cuesheet file with libcue: "
		<< filename;

	// Commented out, kept for reference
	//auto f = safe_open_for_read(filename);
	//auto cd = ::cue_parse_file(f.get());

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

	ARCS_LOG(DEBUG1) << "Cuesheet file successfully parsed";

	return convert(toc_ptr);
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

