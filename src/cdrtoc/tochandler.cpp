#ifndef __LIBARCSDEC_CDRDAOTOC_TOCHANDLER_HPP__
#include "tochandler.hpp"
#endif

#include <memory>   // for unique_ptr

#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include "metaparser.hpp"      // msf_to_frames
#endif

namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{
namespace cdrtoc
{

//ToCHandlerState


void ToCHandlerState::set_track(const int t)
{
	track_ = t;
}

void ToCHandlerState::set_pregap(const int32_t frames)
{
	pregap_ = frames;
}

void ToCHandlerState::append_offset(const int32_t frames)
{
	offsets_.emplace_back(frames);
}

void ToCHandlerState::append_length(const int32_t frames)
{

	lengths_.emplace_back(frames);
}

void ToCHandlerState::append_filename(const std::string& filename)
{
	filenames_.emplace_back(filename);
}

int ToCHandlerState::track() const
{
	return track_;
}

int32_t ToCHandlerState::pregap() const
{
	return pregap_;
}

int32_t ToCHandlerState::prev_offset() const
{
	return offsets_.empty() ? -1 /* TODO magic number */ : offsets_.back();
}

std::vector<int32_t> ToCHandlerState::offsets() const
{
	return offsets_;
}

std::vector<int32_t> ToCHandlerState::lengths() const
{
	return lengths_;
}

std::vector<std::string> ToCHandlerState::filenames() const
{
	return filenames_;
}


// ToCHandler


ToCHandler::ToCHandler()
	: state_ {}
{
	// empty
}


void ToCHandler::do_end_input()
{
	state_.append_length(-1/* TODO magic number */);
	// Normalize number of lengths to total tracks
}


void ToCHandler::do_catalog(const std::string& /*mcn*/)
{
	// empty
}


void ToCHandler::do_cdtextfile(const std::string& /*name*/)
{
	// empty
}


void ToCHandler::do_file(const std::string& name, const FILE_FORMAT& /*t*/)
{
	state_.append_filename(name);
}


void ToCHandler::do_track_flags(const std::vector<TRACK_FLAG>& /*flags*/)
{
	// empty
}


void ToCHandler::do_index(const int i, const int m, const int s, const int f)
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


void ToCHandler::do_isrc(const std::string& /*name*/)
{
	// empty
}


void ToCHandler::do_performer(const std::string& /*name*/)
{
	// empty
}


void ToCHandler::do_postgap(const int /*m*/, const int /*s*/, const int /*f*/)
{
	// empty
}


void ToCHandler::do_pregap(const int m, const int s, const int f)
{
	using details::msf_to_frames;
	state_.set_pregap(msf_to_frames(m, s, f));
}


// rem


void ToCHandler::do_songwriter(const std::string& /*name*/)
{
	// empty
}


void ToCHandler::do_title(const std::string& /*title*/)
{
	// empty
}


void ToCHandler::do_track(const int i, const TRACK_MODE& /*m*/)
{
	state_.set_track(i);
}


int ToCHandler::total_tracks() const
{
	return state_.offsets().size();
}


std::vector<int32_t> ToCHandler::offsets() const
{
	return state_.offsets();
}


std::vector<int32_t> ToCHandler::lengths() const
{
	return state_.lengths();
}


std::vector<std::string> ToCHandler::filenames() const
{
	return state_.filenames();
}

} // namespace cdrtoc
} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

