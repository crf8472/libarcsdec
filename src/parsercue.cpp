/**
 * \file
 *
 * \brief Implements libcue-based parser for CUESheets.
 */

#ifndef __LIBARCSDEC_PARSERCUE_HPP__
#include "parsercue.hpp"
#endif
#ifndef __LIBARCSDEC_PARSERCUE_DETAILS_HPP__
#include "parsercue_details.hpp"
#endif

extern "C" {
#include <libcue/libcue.h>
}

#include <cstdio>    // for fopen, fclose, FILE
#include <sstream>   // for ostringstream
#include <stdexcept> // for invalid_argument
#include <string>    // from .hpp
#include <vector>    // from .hpp

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>  // for TOC, make_toc, InvalidMetadataException
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include "metaparser.hpp"
#endif


// Note: This project requires libcue >= 2.0 but the code compiles fine with
// libcue 1.4. However, it will not work as expected since libcue 2.0
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
namespace details
{
namespace libcue
{

using arcstk::TOC;
using arcstk::make_toc;
using arcstk::InvalidMetadataException;


// FreeCd


void FreeCd::operator()(::Cd* cd) const
{
	if (cd)
	{
		::cd_delete(cd);
		cd = nullptr;
	}
}


// CueOpenFile


CueOpenFile::CueOpenFile(CueOpenFile &&file) noexcept = default;


CueOpenFile& CueOpenFile::operator = (CueOpenFile &&file) noexcept = default;


CueOpenFile::CueOpenFile(const std::string &filename)
	: cd_info_(nullptr)
{
	::Cd* cd_info = nullptr;

	{ // begin scope of FILE f
#ifdef MSC_SAFECODE
		FILE* f = 0;
		fopen_s(&f, filename.c_str(), "r");
#else
		FILE* f = std::fopen(filename.c_str(), "r");
#endif

		// Parse file using libcue

		if (!f)
		{
			std::ostringstream message;
			message << "Failed to open CUEsheet file: " << filename;

			throw FileReadException(message.str());
		}

		ARCS_LOG(DEBUG1) << "Start reading CUEsheet file with libcue";

		cd_info = ::cue_parse_file(f);

		// Close file

		if (std::fclose(f)) // fclose returns 0 on success and EOF on error
		{
			::cd_delete(cd_info);
			cd_info = nullptr;

			std::ostringstream message;
			message << "Failed to close CUEsheet file after reading: "
				<< filename;

			throw FileReadException(message.str());
		}
	} // scope of FILE f

	if (!cd_info)
	{
		std::ostringstream message;
		message << "Failed to parse CUEsheet file: " << filename;

		throw MetadataParseException(message.str());
	}

	cd_info_ = CdPtr(cd_info);

	ARCS_LOG(DEBUG1) << "CUEsheet file successfully read";
}


CueInfo CueOpenFile::parse_info()
{
	auto cd_info = cd_info_.get();

	const int track_count = ::cd_get_ntrack(cd_info);

	if (track_count < 0 or track_count > 99) // FIXME Use CDDA constants
	{
		std::ostringstream ss;
		ss << "Invalid number of tracks: " << track_count;

		throw MetadataParseException(ss.str());
	}

	std::vector<lba_type>    offsets;
	std::vector<lba_type>    lengths;
	std::vector<std::string> filenames;

	using offsets_sz   = decltype( offsets )::size_type;
	using lengths_sz   = decltype( lengths )::size_type;
	using filenames_sz = decltype( filenames )::size_type;

	offsets.reserve(static_cast<offsets_sz>(track_count));
	lengths.reserve(static_cast<lengths_sz>(track_count));
	filenames.reserve(static_cast<filenames_sz>(track_count));

	// types according to libcue-API
	long trk_offset = 0;
	long trk_length = 0;
	::Track* trk = nullptr;

	// Read offset, length + filename for each track in CUE file

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
			std::ostringstream msg;
			msg << "Offset for track " << i
				<< " is not expected to be negative: " << trk_offset;
			throw InvalidMetadataException(msg.str());
		}

		trk_length = ::track_get_length(trk);

		// Length of last track is allowed to be -1.
		if (i < track_count and trk_length < 0)
		{
			std::ostringstream msg;
			msg << "Length for track " << i
				<< " is not expected to be negative: " << trk_length;
			throw InvalidMetadataException(msg.str());
		}

		std::string audiofilename(::track_get_filename(trk));

		// Log the contents

		ARCS_LOG(DEBUG1) << "CUE Track "
			<< std::right
			<< std::setw(2)
			<< i
			<< ": offset: "
			<< std::setw(6)
			<< trk_offset
			<< ", length: "
			<< std::setw(6)
			<< trk_length
			<< ", file: " << audiofilename;

		// NOTE that the length the last track cannot be calculated from
		// the CUE which only contains the start offsets. To get the length
		// of the last track, you would have to subtract its offset from the
		// offset of the non-existent following track.

		try
		{
			offsets.emplace_back(cast_or_throw<lba_type>(trk_offset));
			lengths.emplace_back(cast_or_throw<lba_type>(trk_length));
		} catch (const std::invalid_argument &e)
		{
			std::ostringstream msg;
			msg << "Track " << i << ": ";
			msg << e.what();

			throw InvalidMetadataException(msg.str());
		}

		filenames.emplace_back(audiofilename);
	}

	return std::make_tuple(track_count, std::move(offsets),
			std::move(lengths), std::move(filenames));
}


// CueParserImpl


CueInfo CueParserImpl::parse_worker(const std::string &filename)
{
	auto cue_file = CueOpenFile { filename };
	return cue_file.parse_info();
}


std::unique_ptr<TOC> CueParserImpl::do_parse(const std::string &filename)
{
	auto cue_info = this->parse_worker(filename);

	return make_toc(
			std::get<0>(cue_info),  // track count
			std::get<1>(cue_info),  // offsets
			std::get<2>(cue_info),  // lengths
			std::get<3>(cue_info)); // filenames
}


std::unique_ptr<FileReaderDescriptor> CueParserImpl::do_descriptor() const
{
	return std::make_unique<DescriptorCUE>();
}


} // namespace libcue
} // namespace details


// DescriptorCUE


DescriptorCUE::~DescriptorCUE() noexcept = default;


std::string DescriptorCUE::do_name() const
{
	return "CUESheet";
}


LibInfo DescriptorCUE::do_libraries() const
{
	using details::find_lib;
	using details::libarcsdec_libs;

	return { { "libcue", find_lib(libarcsdec_libs(), "libcue") } };
}


bool DescriptorCUE::do_accepts_bytes(
		const std::vector<unsigned char> & /* bytes */,
		const uint64_t & /* offset */) const
{
	return true;
}


std::unique_ptr<FileReader> DescriptorCUE::do_create_reader() const
{
	auto impl = std::make_unique<details::libcue::CueParserImpl>();
	return std::make_unique<MetadataParser>(std::move(impl));
}


bool DescriptorCUE::do_accepts(Format format) const
{
	return format == Format::CUE;
}


std::set<Format> DescriptorCUE::do_formats() const
{
	return { Format::CUE };
}


std::unique_ptr<FileReaderDescriptor> DescriptorCUE::do_clone() const
{
	return std::make_unique<DescriptorCUE>();
}


// Add this descriptor to the metadata descriptor registry

namespace {

const auto d = RegisterMetadataDescriptor<DescriptorCUE>{};

} // namespace

} // namespace v_1_0_0
} // namespace arcsdec

