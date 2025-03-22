#ifndef __LIBARCSDEC_TOCHANDLER_HPP__
#define __LIBARCSDEC_TOCHANDLER_HPP__
/**
 * \file
 *
 * \brief Tools for bison parsers for compact disc toc files.
 */

#ifndef __LIBARCSTK_METADATA_HPP__
#include <arcstk/metadata.hpp>    // for AudioSize, ToCData, ToC
#endif

#ifndef __LIBARCSDEC_FLEXBISONDRIVER_HPP__
#include "flexbisondriver.hpp"    // for ParserHandler
#endif

#include <cstdint>  // for uint64_t, int32_t
#include <string>   // for string
#include <vector>   // for vector

namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{

using arcstk::AudioSize;
using arcstk::ToC;
using arcstk::ToCData;

/**
 * \brief Convert MSF time to CDDA frames (signed).
 *
 * \param[in] m Minutes
 * \param[in] s Seconds
 * \param[in] f Frames
 *
 * \return Number of frames
 *
 * \throws runtime_error If m, s, or f are not within their legal range
 */
int32_t to_sframes(const int32_t m, const int32_t s, const int32_t f);

/**
 * \brief Convert MSF time to CDDA frames (unsigned).
 *
 * \param[in] m Minutes
 * \param[in] s Seconds
 * \param[in] f Frames
 *
 * \return Number of frames
 *
 * \throws runtime_error If m, s, or f are not within their legal range
 */
uint64_t to_uframes(const uint64_t m, const uint64_t s, const uint64_t f);

/**
 * \brief Validate input string as MCN.
 *
 * \param[in] mcn String to validate as MCN
 *
 * \throws runtime_error If validation fails
 */
void validate_mcn(const std::string& mcn);

/**
 * \brief Validate input string as ISRC.
 *
 * \param[in] isrc String to validate as ISRC
 *
 * \throws runtime_error If validation fails
 */
void validate_isrc(const std::string& isrc);

/**
 * \brief Validate input string as disc id.
 *
 * \param[in] disc_id String to validate as disc id
 *
 * \throws runtime_error If validation fails
 */
void validate_disc_id(const std::string& disc_id);

/**
 * \brief Interface: parser handler defines reaction on grammar symbols.
 */
class ParserToCHandler final : public ParserHandler
{
	std::vector<int32_t> offsets_;

	std::vector<std::string> filenames_;

	std::vector<std::string> isrcs_;

	std::size_t current_track_;

	int32_t leadout_;

	std::string mcn_;

	std::string disc_id_;

	/**
	 * \brief Convert track number to internal index.
	 *
	 * \param[in] track Track number to convert to index
	 *
	 * \return Array index
	 */
	std::size_t to_index(const std::size_t track) const;

	/**
	 * \brief Dump a log on level DEBUG2.
	 */
	void dump_log() const;


	// ParserHandler

	void do_start_input() final;

	void do_end_input() final;

public:

	/**
	 * \brief Default constructor.
	 */
	ParserToCHandler();

	/**
	 * \brief Destructor.
	 */
	~ParserToCHandler() noexcept;

	/**
	 * \brief Append offset value as offset for current track.
	 */
	void append_offset(const uint64_t& frames);

	/**
	 * \brief Update an existing offset to a new value.
	 */
	void set_offset(const std::size_t t, const uint64_t& frames);

	/**
	 * \brief Offset of specified track.
	 */
	int32_t offset(const std::size_t t) const;

	/**
	 * \brief Append filename.
	 */
	void append_filename(const std::string& filename);

	/**
	 * \brief Filename of specified track.
	 */
	std::string filename(const std::size_t t) const;

	/**
	 * \brief Increment current track by one.
	 */
	void inc_current_track();

	/**
	 * \brief Current 1-based track.
	 */
	std::size_t current_track() const;

	/**
	 * \brief Get ToC of parsed values.
	 */
	std::unique_ptr<ToC> get_toc() const;

	/**
	 * \brief Append a track's ISRC.
	 */
	void append_isrc(const std::string& isrc);

	/**
	 * \brief ISRC of specified track.
	 */
	std::string isrc(const std::size_t t) const;

	/**
	 * \brief Set MCN of parsed medium toc.
	 */
	void set_mcn(const std::string& mcn);

	/**
	 * \brief Set disc id of parsed medium toc.
	 */
	void set_disc_id(const std::string& disc_id);
};

} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

#endif

