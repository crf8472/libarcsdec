#ifndef __LIBARCSDEC_CUESHEET_TOCHANDLER_HPP__
#include "tochandler.hpp"
#endif

#include <memory>   // for unique_ptr

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>  // for TOC, make_toc
#endif

#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include "metaparser.hpp"
#endif

namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{
namespace cuesheet
{

//TOCHandlerState


void TOCHandlerState::set_track(const int t)
{
	track_ = t;
}

void TOCHandlerState::set_pregap(const int32_t frames)
{
	pregap_ = frames;
}

void TOCHandlerState::append_offset(const int32_t frames)
{
	offsets_.emplace_back(frames);
}

void TOCHandlerState::append_length(const int32_t frames)
{

	lengths_.emplace_back(frames);
}

int TOCHandlerState::track() const
{
	return track_;
}

int32_t TOCHandlerState::pregap() const
{
	return pregap_;
}

int32_t TOCHandlerState::prev_offset() const
{
	return offsets_.empty() ? -1 /* TODO magic number */ : offsets_.back();
}

std::vector<int32_t> TOCHandlerState::offsets() const
{
	return offsets_;
}

std::vector<int32_t> TOCHandlerState::lengths() const
{
	return lengths_;
}


// TOCHandler


TOCHandler::TOCHandler()
	: state_ {}
{
	// empty
}


void TOCHandler::do_end_input()
{
	state_.append_length(-1/* TODO magic number */);
	// Normalize number of lengths to total tracks
}


void TOCHandler::do_catalog(const std::string& /*mcn*/)
{
	// empty
}


void TOCHandler::do_cdtextfile(const std::string& /*name*/)
{
	// empty
}


void TOCHandler::do_file(const std::string& /*name*/, const FILE_FORMAT& /*t*/)
{
	// empty
}


void TOCHandler::do_track_flags(const std::vector<TRACK_FLAG>& /*flags*/)
{
	// empty
}


void TOCHandler::do_index(const int i, const int m, const int s, const int f)
{
	using details::msf_to_frames;

	if (1 == i)
	{
		// Add first length only when parsing second track
		if (state_.prev_offset() > -1)
		{
			state_.append_length(msf_to_frames(m, s, f) - state_.prev_offset());
			// length of previous track
		}

		state_.append_offset(msf_to_frames(m, s, f)); // current track
	} else
	if (0 == i)
	{
		state_.set_pregap(msf_to_frames(m, s, f));
	}
}


void TOCHandler::do_isrc(const std::string& /*name*/)
{
	// empty
}


void TOCHandler::do_performer(const std::string& /*name*/)
{
	// empty
}


void TOCHandler::do_postgap(const int /*m*/, const int /*s*/, const int /*f*/)
{
	// empty
}


void TOCHandler::do_pregap(const int m, const int s, const int f)
{
	using details::msf_to_frames;
	state_.set_pregap(msf_to_frames(m, s, f));
}


// rem


void TOCHandler::do_songwriter(const std::string& /*name*/)
{
	// empty
}


void TOCHandler::do_title(const std::string& /*title*/)
{
	// empty
}


void TOCHandler::do_track(const int i, const TRACK_MODE& /*m*/)
{
	state_.set_track(i);
}


int TOCHandler::total_tracks() const
{
	return state_.offsets().size();
}


std::vector<int32_t> TOCHandler::offsets() const
{
	return state_.offsets();
}


std::vector<int32_t> TOCHandler::lengths() const
{
	return state_.lengths();
}

} // namespace cuesheet
} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

