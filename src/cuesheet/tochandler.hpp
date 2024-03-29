#ifndef __LIBARCSDEC_CUESHEET_TOCHANDLER_HPP__
#define __LIBARCSDEC_CUESHEET_TOCHANDLER_HPP__

/**
 * \file
 *
 * \brief Public header for a handler that constructs a TOC.
 */

#ifndef __LIBARCSDEC_CUESHEET_HANDLER_HPP__
#include "handler.hpp"
#endif

#include <string>
#include <vector>


namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{
namespace cuesheet
{

/**
 * \brief State of a TOCHandler
 */
class TOCHandlerState
{
	int track_ = 0;

	int32_t pregap_ = 0;

	std::vector<int32_t> offsets_{};

	std::vector<int32_t> lengths_{};

public:

	void set_track(const int t);

	void set_pregap(const int32_t frames);

	void append_offset(const int32_t frames);

	void append_length(const int32_t frames);

	int track() const;

	int32_t pregap() const;

	int32_t prev_offset() const;

	std::vector<int32_t> offsets() const;

	std::vector<int32_t> lengths() const;
};


/**
 * \brief Handler that collects data required for building a TOC.
 */
class TOCHandler final : public Handler
{
	TOCHandlerState state_;

	// Handler

	void do_end_input() final;

	void do_catalog(const std::string& mcn) final;

	void do_cdtextfile(const std::string& name) final;

	void do_file(const std::string& name, const FILE_TYPE& t) final;

	// flags

	void do_index(const int i, const int m, const int s, const int f) final;

	void do_isrc(const std::string& name) final;

	// performer

	void do_postgap(const int m, const int s, const int f) final;

	void do_pregap(const int m, const int s, const int f) final;

	// rem
	// songwriter

	void do_title(const std::string& title) final;

	void do_track(const int i, const TRACK_MODE& m) final;

public:

	TOCHandler();

	int total_tracks() const;

	std::vector<int32_t> offsets() const;

	std::vector<int32_t> lengths() const;
};

} // namespace cuesheet
} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

#endif

