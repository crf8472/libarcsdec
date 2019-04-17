/**
 * \file parsercue.cpp Implements parser for CUESheets
 *
 */


#ifndef __LIBARCSDEC_PARSERCUE_HPP__
#include "parsercue.hpp"
#endif

extern "C" {
#include <libcue/libcue.h>
}

#include <cstdio>    // for fopen, fclose, FILE
#include <iomanip>   // for debug
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

namespace
{

using arcs::InvalidMetadataException;
using arcs::TOC;
using arcs::make_toc;


/**
 * \cond IMPL_ONLY
 *
 * \internal \defgroup parserCueImpl Implementation details of CUESheet parsing
 *
 * \ingroup parsercue
 *
 * @{
 */


// forward declaration (for use in CueOpenFile)
class CueInfo;


// CueOpenFile


/**
 * Represents an opened CUEsheet file. This class is the only symbol that
 * depends on libcue API as an implementation detail.
 */
class CueOpenFile
{

public:

	/**
	 * Open CUEsheet with the given name. This class does RAII and does
	 * not require you to manually close the instance.
	 *
	 * \param[in] filename The CUEsheet file to read
	 *
	 * \throw FileReadException      If the CUESheet file could not be read
	 * \throw MetadataParseException If the CUE data could not be parsed
	 */
	explicit CueOpenFile(const std::string &filename);

	/**
	 * Destructor
	 */
	virtual ~CueOpenFile() noexcept;

	/**
	 * Returns all TOC information from the file.
	 *
	 * \return CueInfo representing the TOC information
	 */
	CueInfo parse_info();


protected:

	/**
	 * Service method: Convert a signed long to uint32_t, clipping negative
	 * values and larger values to 0.
	 *
	 * \param[in] value The value to convert
	 *
	 * \return The converted value
	 */
	uint32_t signed_long_to_uint32(const signed long &value) const;


private:

	/**
	 * Internal libcue-based representation
	 */
	::Cd* cd_info_;


	// make class non-copyable (1/2)
	CueOpenFile(const CueOpenFile &file) = delete;

	// make class non-copyable (2/2)
	CueOpenFile& operator = (CueOpenFile &file) = delete;
};


/**
 * Represents information on track offsets, track lengths and audio filenames
 * from the CUE sheet.
 *
 * Since CueInfos are exclusively constructed by a builder, they have no setter
 * methods. For convenience, there is a parameterless default constructor that
 * enables to construct an empty CueInfo instance.
 */
class CueInfo
{

	// CueOpenFile::parse_info() is a friend method of CueInfo since it
	// constructs CueInfos exclusively
	friend CueInfo CueOpenFile::parse_info();


public:

	/**
	 * Default destructor
	 */
	virtual ~CueInfo() noexcept;

	/**
	 * Number of tracks specified in the CUE file
	 *
	 * \return Number of tracks according to the CUEsheet
	 */
	int track_count() const;

	/**
	 * Return the frame offsets specified in the CUE file
	 *
	 * \return The list of offsets, index corresponds to the track number
	 */
	std::vector<int32_t> offsets() const;

	/**
	 * Return the track lengths (in frames) specified in the CUE file
	 *
	 * \return The list of lengths, index corresponds to the track number
	 */
	std::vector<int32_t> lengths() const;

	/**
	 * Return the track audiofile names
	 *
	 * \return The list of audio filenames, index corresponds to the track
	 * number
	 */
	std::vector<std::string> audiofilenames() const;


private:

	/**
	 * Default constructor.
	 */
	CueInfo();

	/**
	 * Append next track to instance.
	 *
	 * \param[in] offset Track offset
	 * \param[in] length Track length
	 * \param[in] audiofilename Audiofile containing the track
	 */
	void append_track(
			const int32_t &offset,
			const int32_t &length,
			const std::string &audiofilename);

	/**
	 * Number of tracks specified in the CUE file
	 *
	 * Represented as unsigned integer to make comparing with sizes easier.
	 */
	uint16_t track_count_;

	/**
	 * Frame offsets specified in the CUE file
	 */
	std::vector<int32_t> offsets_;

	/**
	 * Track lenghts specified in the CUE file
	 */
	std::vector<int32_t> lengths_;

	/**
	 * Name of the audio file for the respective track
	 */
	std::vector<std::string> audiofilenames_;
};


/// @}
/// \endcond IMPL_ONLY


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
			std::stringstream message;
			message << "Failed to open CUEsheet file: " << filename;

			ARCS_LOG_ERROR << message.str();
			throw FileReadException(message.str());
		}

		ARCS_LOG_INFO << "Start reading CUE file";

		cd_info_ = ::cue_parse_file(f);

		// Close file

		if (std::fclose(f)) // fclose returns 0 on success and EOF on error
		{
			::cd_delete(cd_info_);
			cd_info_ = nullptr;

			std::stringstream message;
			message << "Failed to close CUEsheet file after reading: "
				<< filename;

			ARCS_LOG_ERROR << message.str();
			throw FileReadException(message.str());
		}
	} // scope of FILE f

	if (!cd_info_)
	{
		std::stringstream message;
		message << "Failed to parse CUEsheet file: " << filename;

		ARCS_LOG_ERROR << message.str();
		throw MetadataParseException(message.str());
	}

	ARCS_LOG_INFO << "CUE file read";
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
	// return types according to libcue-API
	long trk_offset = 0;
	long trk_length = 0;
	uint16_t track_count = ::cd_get_ntrack(cd_info_);

	if (track_count <= 0)
	{
		std::stringstream ss;
		ss << "Invalid number of tracks: " << track_count;

		ARCS_LOG_ERROR << ss.str();

		throw MetadataParseException(ss.str());
	}

	Track* trk = nullptr;
	CueInfo cue_info;

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
		trk_length = ::track_get_length(trk);
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
			signed_long_to_uint32(trk_offset),
			signed_long_to_uint32(trk_length),
			audiofilename);
	}

	// Basic verification

	if (track_count != cue_info.offsets().size())
	{
		ARCS_LOG_WARNING << "Expected " << track_count << " tracks, but parsed "
			<< cue_info.offsets().size() << " offsets "
			<< "(" << std::abs(static_cast<int32_t>(
					track_count - cue_info.offsets().size()))
			<< (track_count < cue_info.offsets().size() ? " more" : " less")
			<< ")";
	}
	if (track_count != cue_info.lengths().size())
	{
		ARCS_LOG_WARNING << "Expected " << track_count << " tracks, but parsed "
			<< cue_info.lengths().size() << " lengths "
			<< "(" << std::abs(static_cast<int32_t>(
					track_count - cue_info.lengths().size()))
			<< (track_count < cue_info.lengths().size() ? " more" : " less")
			<< ")";
	}

	return cue_info;
}


uint32_t CueOpenFile::signed_long_to_uint32(const long &value) const
{
	// Maximal value for uint32_t (== 0xFFFFFFFF);
	static const uint32_t UINT32T_MAX = static_cast<uint32_t>(-1);

	if (value < 0)
	{
		ARCS_LOG(DEBUG1) << "Value from CUE sheet is negative, clip to 0";
		return 0;
	}

	if (value < static_cast<int64_t>(UINT32T_MAX) + 1)
	{
		return static_cast<uint32_t>(value);
	}

	ARCS_LOG_ERROR << "Value from CUE sheet is larger than uint32 maximum";
	return 0;
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


CueInfo::~CueInfo() noexcept = default;


int CueInfo::track_count() const
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


/// \cond IMPL_ONLY
/// \internal \addtogroup parserCueImpl
/// @{


/**
 * Reads a CUE file and exposes the relevant information as CueInfo. Note that
 * CueParser exclusively populates CueInfo objects.
 */
class CueParserImpl : public MetadataParserImpl
{

public:

	/**
	 * Default constructor
	 */
	CueParserImpl();

	/**
	 * Default destructor
	 */
	virtual ~CueParserImpl() noexcept;

	/**
	 * Return CUE data.
	 *
	 * \return The CueInfo of the parsed CUEsheet
	 *
	 * \throw FileReadException If the file could not be read
	 */
	CueInfo read(const std::string &filename);

	/**
	 * Return name of the CUE file
	 *
	 * \return The filename of the audio file described by the CUEsheet
	 */
	std::string filename() const;


private:

	std::unique_ptr<TOC> do_parse(const std::string &filename) override;

	/**
	 * Name of the last parsed CUE file
	 */
	std::string filename_;
};


/// @}
/// \endcond IMPL_ONLY


// CueParserImpl


CueParserImpl::CueParserImpl()
	:filename_()
{
	//empty
}


CueParserImpl::~CueParserImpl() noexcept = default;


CueInfo CueParserImpl::read(const std::string &filename)
{
	filename_ = filename;

	CueOpenFile cue_file(this->filename());
	return cue_file.parse_info();
}


std::string CueParserImpl::filename() const
{
	return filename_;
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


} // namespace


// DescriptorCUE


DescriptorCUE::~DescriptorCUE() noexcept = default;


std::string DescriptorCUE::do_name() const
{
	return "CUESheet";
}


bool DescriptorCUE::do_accepts_bytes(const std::vector<char> & /* bytes */,
		const uint64_t & /* offset */) const
{
	return true;
}


bool DescriptorCUE::do_accepts_suffix(const std::string &suffix) const
{
	char letter = suffix.at(0);

	if (not (letter == 'C' or letter == 'c'))
	{
		return false;
	}

	letter = suffix.at(1);

	if (not (letter == 'U' or letter == 'u'))
	{
		return false;
	}

	letter = suffix.at(2);

	if (not (letter == 'E' or letter == 'e'))
	{
		return false;
	}

	return true;
}


std::unique_ptr<FileReader> DescriptorCUE::do_create_reader() const
{
	auto impl = std::make_unique<CueParserImpl>();

	return std::make_unique<MetadataParser>(std::move(impl));
}


std::unique_ptr<FileReaderDescriptor> DescriptorCUE::do_clone() const
{
	return std::make_unique<DescriptorCUE>();
}

} // namespace v_1_0_0

} // namespace arcsdec

