/**
 * \file
 *
 * \brief Implements libcue-based parser for CueSheets.
 */

#ifndef __LIBARCSDEC_PARSERLIBCUE_HPP__
#include "parserlibcue.hpp"
#endif
#ifndef __LIBARCSDEC_PARSERLIBCUE_DETAILS_HPP__
#include "parserlibcue_details.hpp"  // for CueParserImpl, CueOpenFile
#endif

#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include "metaparser.hpp"         // for MetadataParseException
#endif
#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"          // for RegisterDescriptor
#endif

#ifndef __LIBARCSTK_METADATA_HPP__
#include <arcstk/metadata.hpp>    // for ToC, make_toc
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

extern "C" {
#include <libcue/libcue.h>
}

#include <cstdio>    // for fopen, fclose, FILE
#include <iomanip>   // for setw
#include <memory>    // for unique_ptr
#include <set>       // for set
#include <sstream>   // for ostringstream
#include <stdexcept> // for invalid_argument
#include <string>    // for string
#include <vector>    // for vector


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
namespace details
{
namespace libcue
{

using arcstk::ToC;
using arcstk::make_toc;
// using arcstk::InvalidMetadataException;


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


CdPtr Make_CdPtr::operator()(const std::string& filename) const
{
	CdPtr cd_ptr;

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
			message << "Failed to open Cuesheet file: " << filename;

			throw FileReadException(message.str());
		}

		ARCS_LOG(DEBUG1) << "Start reading Cuesheet file with libcue";

		cd_ptr = CdPtr(::cue_parse_file(f));

		// Close file

		if (std::fclose(f)) // fclose returns 0 on success and EOF on error
		{
			std::ostringstream message;
			message << "Failed to close Cuesheet file after reading: "
				<< filename;

			throw FileReadException(message.str());
		}
	} // scope of FILE f

	if (!cd_ptr.get())
	{
		std::ostringstream message;
		message << "Failed to parse Cuesheet file: " << filename;

		throw MetadataParseException(message.str());
	}

	ARCS_LOG(DEBUG1) << "Cuesheet file successfully read";

	return cd_ptr;
}


// CueOpenFile


CueOpenFile::CueOpenFile(CueOpenFile&& file) noexcept = default;


CueOpenFile& CueOpenFile::operator = (CueOpenFile&& file) noexcept = default;


CueOpenFile::CueOpenFile(const std::string& filename)
	: cd_info_ { nullptr }
{
	static const Make_CdPtr make_cd;

	cd_info_ = std::move(make_cd(filename));
}


CueInfo CueOpenFile::info() const
{
	const auto cd_info = cd_info_.get();

	const int track_count = ::cd_get_ntrack(cd_info);

	if (track_count < 0 or track_count > arcstk::CDDA::MAX_TRACKCOUNT)
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
			std::ostringstream msg;
			msg << "Offset for track " << i
				<< " is not expected to be negative: " << trk_offset;
			//throw InvalidMetadataException(msg.str());
			throw std::invalid_argument(msg.str());
		}

		trk_length = ::track_get_length(trk);

		// Length of last track is allowed to be -1.
		if (i < track_count and trk_length < 0)
		{
			std::ostringstream msg;
			msg << "Length for track " << i
				<< " is not expected to be negative: " << trk_length;
			//throw InvalidMetadataException(msg.str());
			throw std::invalid_argument(msg.str());
		}

		std::string audiofilename(::track_get_filename(trk));

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
			<< ", file: " << audiofilename;

		// NOTE that the length the last track cannot be calculated from
		// the Cue which only contains the start offsets. To get the length
		// of the last track, you would have to subtract its offset from the
		// offset of the non-existent following track.

		try
		{
			offsets.emplace_back(cast_or_throw<lba_type>(trk_offset));
			lengths.emplace_back(cast_or_throw<lba_type>(trk_length));
		} catch (const std::invalid_argument& e)
		{
			std::ostringstream msg;
			msg << "Track " << i << ": ";
			msg << e.what();

			//throw InvalidMetadataException(msg.str());
			throw std::invalid_argument(msg.str());
		}

		filenames.emplace_back(audiofilename);
	}

	return std::make_tuple(track_count, std::move(offsets),
			std::move(lengths), std::move(filenames));
}


// CueParserImpl


CueInfo CueParserImpl::parse_worker(const std::string& filename) const
{
	return CueOpenFile { filename }.info();
}


std::unique_ptr<ToC> CueParserImpl::do_parse(const std::string& filename)
{
	const auto cue_info = this->parse_worker(filename);

	/*
	return make_toc(
			std::get<0>(cue_info),  // track count
			std::get<1>(cue_info),  // offsets
			std::get<2>(cue_info),  // lengths
			std::get<3>(cue_info)); // filenames
	*/
	return make_toc(std::get<1>(cue_info),  // offsets
					std::get<3>(cue_info)); // filenames
}


std::unique_ptr<FileReaderDescriptor> CueParserImpl::do_descriptor() const
{
	return std::make_unique<DescriptorCue>();
}


} // namespace libcue
} // namespace details


// DescriptorCue


DescriptorCue::~DescriptorCue() noexcept = default;


std::string DescriptorCue::do_id() const
{
	return "libcue";
}


std::string DescriptorCue::do_name() const
{
	return "Libcue";
}


InputType DescriptorCue::do_input_type() const
{
	return InputType::TOC;
}


bool DescriptorCue::do_accepts_codec(Codec codec) const
{
	ARCS_LOG(DEBUG1) << "Is Codec NONE?";
	return codec == Codec::NONE;
}


std::set<Format> DescriptorCue::define_formats() const
{
	return { Format::CUE };
}


LibInfo DescriptorCue::do_libraries() const
{
	return { libinfo_entry_filepath("libcue") };
}


std::unique_ptr<FileReader> DescriptorCue::do_create_reader() const
{
	auto impl = std::make_unique<details::libcue::CueParserImpl>();
	return std::make_unique<MetadataParser>(std::move(impl));
}


std::unique_ptr<FileReaderDescriptor> DescriptorCue::do_clone() const
{
	return std::make_unique<DescriptorCue>();
}


// Add this descriptor to the metadata descriptor registry

namespace {

const auto d = RegisterDescriptor<DescriptorCue>{};

} // namespace

} // namespace v_1_0_0
} // namespace arcsdec

