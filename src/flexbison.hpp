#ifndef __LIBARCSDEC_FLEXBISON_HPP__
#define __LIBARCSDEC_FLEXBISON_HPP__
/**
 * \file
 *
 * \brief Tools for flex scanners and bison parsers.
 */

#ifndef __LIBARCSTK_METADATA_HPP__
#include <arcstk/metadata.hpp>    // for AudioSize, ToCData, ToC
#endif

#ifndef __LIBARCSDEC_FLEXBISONDRIVER_HPP__
#include "flexbisondriver.hpp"
#endif

#include <cstdint>  // for uint32_t, int32_t
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
 * \brief Convert MSF time to CDDA frames.
 *
 * \param[in] m Minutes
 * \param[in] s Seconds
 * \param[in] f Frames
 *
 * \return Number of frames
 */
long to_frames(const int m, const int s, const int f);

/**
 * \brief Convert CDDA frames to MSF frames.
 *
 * \param[in]  frames Input frames
 * \param[out] m      Minutes
 * \param[out] s      Seconds
 * \param[out] f      Frames
 */
//void frames_to_msf(long frames, int* m, int* s, int* f);

/**
 * \brief Interface: parser handler defines reaction on grammar symbols.
 */
class ParserToCHandler final : public ParserHandler
{
	arcstk::ToCData tocdata_;

	std::vector<std::string> filenames_;

	std::vector<std::string> isrcs_;

	std::size_t total_tracks_;

	std::string mcn_;

	std::string disc_id_;

public:

	ParserToCHandler();

	~ParserToCHandler() noexcept;

	void set_offset(const std::size_t t, const uint64_t& frames);

	AudioSize offset(const std::size_t t) const;

	void set_filename(const std::size_t t, const std::string& filename);

	std::string filename(const std::size_t t) const;

	void inc_current_track();

	std::size_t current_track() const;

	ToC get_toc() const;

	void set_isrc(const std::size_t t, const std::string& isrc);

	std::string isrc(const std::size_t t) const;

	void set_mcn(const std::string& mcn);

	void set_disc_id(const std::string& disc_id);
};

} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

#endif

