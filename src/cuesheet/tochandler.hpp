#ifndef __LIBARCSDEC_CUESHEET_TOCHANDLER_HPP__
#define __LIBARCSDEC_CUESHEET_TOCHANDLER_HPP__

/**
 * \file
 *
 * \brief Public header for a handler that constructs a ToC.
 */

#ifndef __LIBARCSDEC_CUESHEET_HANDLER_HPP__
#include "handler.hpp"
#endif

#include <string>  // for string
#include <vector>  // for vector


namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{
namespace cuesheet
{

/**
 * \brief State of a ToCHandler
 */
class ToCHandlerState
{
	int track_ = 0;

	int32_t pregap_ = 0;

	std::vector<int32_t> offsets_{};

	std::vector<int32_t> lengths_{};

	std::vector<std::string> filenames_{};

public:

	void set_track(const int t);

	void set_pregap(const int32_t frames);

	void append_offset(const int32_t frames);

	void append_length(const int32_t frames);

	void append_filename(const std::string& filename);

	int track() const;

	int32_t pregap() const;

	int32_t prev_offset() const;

	std::vector<int32_t> offsets() const;

	std::vector<int32_t> lengths() const;

	std::vector<std::string> filenames() const;
};


/**
 * \brief Handler that collects data required for building a ToC.
 */
class ToCHandler final : public Handler
{
	ToCHandlerState state_;

	// Handler

	void do_end_input() final;

	void do_catalog(const std::string& mcn) final;

	void do_cdtextfile(const std::string& name) final;

	void do_file(const std::string& name, const FILE_FORMAT& t) final;

	void do_track_flags(const std::vector<TRACK_FLAG>& flags) final;

	void do_index(const int i, const int m, const int s, const int f) final;

	void do_isrc(const std::string& name) final;

	void do_performer(const std::string& name) final;

	void do_postgap(const int m, const int s, const int f) final;

	void do_pregap(const int m, const int s, const int f) final;

	// rem

	void do_songwriter(const std::string& name) final;

	void do_title(const std::string& title) final;

	void do_track(const int i, const TRACK_MODE& m) final;

public:

	ToCHandler();

	int total_tracks() const;

	std::vector<int32_t> offsets() const;

	std::vector<int32_t> lengths() const;

	std::vector<std::string> filenames() const;
};

} // namespace cuesheet
} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

#endif

