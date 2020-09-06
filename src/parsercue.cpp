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
#include <iomanip>   // for debug
#include <limits>    // for numeric_limits
#include <sstream>   // for debug
#include <stdexcept>
#include <string>    // from .h
#include <vector>    // from .h

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>
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

using arcstk::InvalidMetadataException;
using arcstk::TOC;
using arcstk::make_toc;


// cast_or_throw


int32_t cast_or_throw(const signed long value, const std::string &name)
{
	if (value > std::numeric_limits<int32_t>::max())
	{
		std::ostringstream msg;
		msg << "Value '" << name << "': " << value << " too big for int32_t";

		throw InvalidMetadataException(msg.str());
	}

	return static_cast<int32_t>(value);
}


// CueOpenFile


CueOpenFile::CueOpenFile(const std::string &filename)
	: cd_info_(nullptr)
{
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

			ARCS_LOG_ERROR << message.str();
			throw FileReadException(message.str());
		}

		ARCS_LOG(DEBUG1) << "Start reading CUEsheet file with libcue";

		cd_info_ = ::cue_parse_file(f);

		// Close file

		if (std::fclose(f)) // fclose returns 0 on success and EOF on error
		{
			::cd_delete(cd_info_);
			cd_info_ = nullptr;

			std::ostringstream message;
			message << "Failed to close CUEsheet file after reading: "
				<< filename;

			ARCS_LOG_ERROR << message.str();
			throw FileReadException(message.str());
		}
	} // scope of FILE f

	if (!cd_info_)
	{
		std::ostringstream message;
		message << "Failed to parse CUEsheet file: " << filename;

		ARCS_LOG_ERROR << message.str();
		throw MetadataParseException(message.str());
	}

	ARCS_LOG(DEBUG1) << "CUEsheet file successfully read";
}


CueOpenFile::~CueOpenFile() noexcept
{
	if (cd_info_)
	{
		::cd_delete(cd_info_);
	}
}


CueInfo CueOpenFile::parse_info()
{
	int track_count = ::cd_get_ntrack(cd_info_);

	if (track_count < 0 or track_count > 99)
	{
		std::ostringstream ss;
		ss << "Invalid number of tracks: " << track_count;

		ARCS_LOG_ERROR << ss.str();

		throw MetadataParseException(ss.str());
	}

	CueInfo cue_info;

	// return types according to libcue-API
	long trk_offset = 0;
	long trk_length = 0;

	Track* trk = nullptr;

	// Read offset, length + filename for each track in CUE file

	for (int i = 1; i <= track_count; ++i)
	{
		trk = ::cd_get_track(cd_info_, i);

		if (!trk)
		{
			ARCS_LOG_ERROR << "Could not retrieve track " << i;

			cue_info.append_track(0, 0, std::string());

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

		cue_info.append_track(
				cast_or_throw(trk_offset, "track offset"),
				cast_or_throw(trk_length, "track length"),
				audiofilename);
	}

	// Basic verification

	if (static_cast<uint16_t>(track_count) != cue_info.track_count())
	{
		ARCS_LOG_WARNING << "Expected " << track_count << " tracks, but parsed "
			<< cue_info.track_count() << " tracks ";
	}

	return cue_info;
}


// CueInfo


CueInfo::CueInfo()
	: track_count_(0)
	, offsets_()
	, lengths_()
	, audiofilenames_()
{
	// empty
}


uint16_t CueInfo::track_count() const
{
	return track_count_;
}


std::vector<int32_t> CueInfo::offsets() const
{
	return offsets_;
}


std::vector<int32_t> CueInfo::lengths() const
{
	return lengths_;
}


std::vector<std::string> CueInfo::audiofilenames() const
{
	return audiofilenames_;
}


void CueInfo::append_track(
		const int32_t &offset,
		const int32_t &length,
		const std::string &audiofilename)
{
	offsets_.push_back(offset);
	lengths_.push_back(length);
	audiofilenames_.push_back(audiofilename);

	++track_count_;
}


// CueParserImpl


CueInfo CueParserImpl::read(const std::string &filename)
{
	CueOpenFile cue_file(filename);
	return cue_file.parse_info();
}


std::unique_ptr<TOC> CueParserImpl::do_parse(const std::string &filename)
{
	CueInfo cue_info = this->read(filename);

	return make_toc(
			cue_info.track_count(),
			cue_info.offsets(),
			cue_info.lengths(),
			cue_info.audiofilenames());
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

