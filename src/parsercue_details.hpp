#ifndef __LIBARCSDEC_PARSERCUE_HPP__
#error "Do not include parsercue_details.hpp, include parsercue.hpp instead"
#endif

/**
 * \file
 *
 * \brief Internal APIs for libcue-based CUESheet reader
 */

#ifndef __LIBARCSDEC_PARSERCUE_DETAILS_HPP__
#define __LIBARCSDEC_PARSERCUE_DETAILS_HPP__

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

extern "C" {
#include <libcue/libcue.h>
}

namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{
namespace libcue
{


/**
 * \internal \defgroup parserCueImpl Implementation details of CUESheet parsing
 *
 * \ingroup parsercue
 *
 * @{
 */

/**
 * \brief Functor for freeing Cd* instances.
 */
struct FreeCd final
{
	void operator()(::Cd* cd) const;
};


/**
 * \brief A unique_ptr for Cd using FreeCd as a custom deleter.
 */
using CdPtr = std::unique_ptr<::Cd, FreeCd>;


/**
 * \brief Service method: Convert a long value to int32_t.
 *
 * \param[in] value The value to convert
 * \param[in] name  Name of the value to show in error message
 *
 * \throw InvalidMetadataException If \c value negative or too big
 *
 * \return The converted value
 */
int32_t cast_or_throw(const signed long value, const std::string &name);


// forward declaration (for use in CueOpenFile)
class CueInfo;

/**
 * \brief Represents an opened CUEsheet file.
 *
 * Instances of this class are non-copyable but movable.
 */
class CueOpenFile final
{
public:

	/**
	 * \brief Open CUEsheet with the given name.
	 *
	 * \param[in] filename The CUEsheet file to read
	 *
	 * \throw FileReadException      If the CUESheet file could not be read
	 * \throw MetadataParseException If the CUE data could not be parsed
	 */
	explicit CueOpenFile(const std::string &filename);

	// make class non-copyable
	CueOpenFile(const CueOpenFile &file) = delete;
	CueOpenFile& operator = (CueOpenFile &file) = delete;

	CueOpenFile(CueOpenFile &&file) noexcept = default;
	CueOpenFile& operator = (CueOpenFile &&file) noexcept = default;

	/**
	 * \brief Returns all TOC information from the file.
	 *
	 * \return CueInfo representing the TOC information
	 */
	CueInfo parse_info();

private:

	/**
	 * \brief Internal libcue-based representation.
	 */
	CdPtr cd_info_;
};


/**
 * \brief Represents track offsets, track lengths and audio filenames.
 */
class CueInfo final
{
	// CueOpenFile::parse_info() is a friend method of CueInfo since it
	// constructs CueInfos exclusively
	friend CueInfo CueOpenFile::parse_info();

public:

	/**
	 * \brief Number of tracks specified in the CUE file.
	 *
	 * \return Number of tracks according to the CUEsheet
	 */
	uint16_t track_count() const;

	/**
	 * \brief Return the frame offsets specified in the CUE file.
	 *
	 * \return The list of offsets, index corresponds to the track number
	 */
	std::vector<int32_t> offsets() const;

	/**
	 * \brief Return the track lengths (in frames) specified in the CUE file.
	 *
	 * \return The list of lengths, index corresponds to the track number
	 */
	std::vector<int32_t> lengths() const;

	/**
	 * \brief Return the track audiofile names.
	 *
	 * \return The list of audio filenames, index corresponds to the track
	 * number
	 */
	std::vector<std::string> audiofilenames() const;

private:

	/**
	 * \brief Default constructor.
	 */
	CueInfo();

	/**
	 * \brief Append next track to instance.
	 *
	 * \param[in] offset        Track offset
	 * \param[in] length        Track length
	 * \param[in] audiofilename Audiofile containing the track
	 */
	void append_track(
			const int32_t &offset,
			const int32_t &length,
			const std::string &audiofilename);

	/**
	 * \brief Number of tracks specified in the CUE file.
	 *
	 * Represented as unsigned integer to make comparing with sizes easier.
	 */
	uint16_t track_count_;

	/**
	 * \brief Frame offsets specified in the CUE file.
	 */
	std::vector<int32_t> offsets_;

	/**
	 * \brief Track lengths specified in the CUE file.
	 */
	std::vector<int32_t> lengths_;

	/**
	 * \brief Names of the audio files specified in the CUE file.
	 */
	std::vector<std::string> audiofilenames_;
};


/**
 * \brief Implementation for libcue-based reading of CUESheets.
 */
class CueParserImpl final : public MetadataParserImpl
{
public:

	/**
	 * \brief Return CUE data.
	 *
	 * \return The CueInfo of the parsed CUEsheet
	 *
	 * \throw FileReadException If the file could not be read
	 */
	CueInfo read(const std::string &filename);

private:

	std::unique_ptr<TOC> do_parse(const std::string &filename) override;

	std::unique_ptr<FileReaderDescriptor> do_descriptor() const override;
};

/// @}


} // namespace libcue
} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

#endif

