#ifndef __LIBARCSDEC_CUESHEET_HANDLER_HPP__
#define __LIBARCSDEC_CUESHEET_HANDLER_HPP__

/**
 * \file
 *
 * \brief Public header for representing a Cuesheet node.
 */

#include <string>

#include "version.hpp"

namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{

namespace cuesheet
{


/**
 * \brief FILE types in cuesheets.
 */
enum class FILE_TYPE
{
	BINARY,
	MOTOROLA,
	AIFF,
	WAVE,
	MP3,
	FLAC
};


/**
 * \brief TRACK flags in cuesheets
 */
enum class TRACK_FLAGS
{
	PRE,
	DCP,
	FOUR_CH,
	SCMS
};


/**
 * \brief TRACK modes in cuesheets.
 */
enum class TRACK_MODE
{
	AUDIO,
	MODE1_2048,
	MODE1_2352,
	MODE2_2048,
	MODE2_2324,
	MODE2_2332,
	MODE2_2336,
	MODE2_2342,
	MODE2_2352
};


/**
 * \brief Cuesheet parser callbacks.
 */
class Handler
{
public:

	/**
	 * \brief Constructor.
	 */
	Handler();

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~Handler() noexcept;

	/**
	 * \brief Resets the Handler to its initial state.
	 *
	 * All content is discarded.
	 */
	void reset();

	/**
	 * \brief Start Cuesheet input.
	 *
	 * Called when input starts.
	 */
	void start_input();

	/**
	 * \brief End Cuesheet input.
	 *
	 * Called after EOF has occurred.
	 */
	void end_input();

	void catalog(const std::string& mcn);
	void cdtextfile(const std::string& name);
	void file(const std::string& name, const FILE_TYPE& t);
	// flags
	void index(const int i, const int m, const int s, const int f);
	void isrc(const std::string& name);
	// performer
	void postgap(const int m, const int s, const int f);
	void pregap(const int m, const int s, const int f);
	// rem
	// songwriter
	void title(const std::string& title);
	void track(const int i, const TRACK_MODE& m);

private:

	virtual void do_reset()
	= 0;

	virtual void do_start_input();

	virtual void do_end_input();

	virtual void do_catalog(const std::string& mcn)
	= 0;

	virtual void do_cdtextfile(const std::string& name)
	= 0;

	virtual void do_file(const std::string& name, const FILE_TYPE& t)
	= 0;

	// flags

	virtual void do_index(const int i, const int m, const int s, const int f)
	= 0;

	virtual void do_isrc(const std::string& name)
	= 0;

	// performer

	virtual void do_postgap(const int m, const int s, const int f)
	= 0;

	virtual void do_pregap(const int m, const int s, const int f)
	= 0;

	// rem
	// songwriter

	virtual void do_title(const std::string& title)
	= 0;

	virtual void do_track(const int i, const TRACK_MODE& m)
	= 0;
};

} // namespace cuesheet
} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

#endif // __LIBARCSDEC_CUESHEET_HANDLER_HPP__

