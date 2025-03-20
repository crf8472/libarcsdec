/**
 * \file
 *
 * \brief Implementations to flexbison.hpp.
 */

#ifndef __LIBARCSDEC_FLEXBISON_HPP__
#include "flexbison.hpp"
#endif

#ifndef __LIBARCSTK_METADATA_HPP__
#include <arcstk/metadata.hpp>    // for ToCData, ToC, AudioSize, UNIT
#endif

#include <string>      // for vector
#include <vector>      // for string

namespace arcsdec
{
namespace v_1_0_0
{
namespace details
{

using arcstk::AudioSize;
using arcstk::ToC;
using arcstk::ToCData;
using arcstk::UNIT;


// msf_to_frames


long to_frames(const int m, const int s, const int f)
{
	if (m < 0 || m > 99 || s < 0 || s >= 60 || f < 0 || f >= 75) {
		return -1;
	}

	return (m * 60 + s) * 75 + f;
}


// ParserTocHandler


ParserToCHandler::ParserToCHandler()
	: tocdata_      { 0 }
	, filenames_    { /* empty */ }
	, isrcs_        { /* empty */ }
	, total_tracks_ { 1 }
	, mcn_          { /* empty */ }
	, disc_id_      { /* empty */ }
{
	// empty
}


ParserToCHandler::~ParserToCHandler() noexcept = default;


void ParserToCHandler::set_offset(const std::size_t t, const uint64_t& frames)
{
	tocdata_[t] = AudioSize { frames, UNIT::FRAMES };
}


AudioSize ParserToCHandler::offset(const std::size_t t) const
{
	return tocdata_[t];
}


void ParserToCHandler::set_filename(const std::size_t t,
		const std::string& filename)
{
	filenames_[t] = filename;
}


std::string ParserToCHandler::filename(const std::size_t t) const
{
	return filenames_[t];
}


void ParserToCHandler::inc_current_track()
{
	++total_tracks_;
}


std::size_t ParserToCHandler::current_track() const
{
	return total_tracks_;
}


ToC ParserToCHandler::get_toc() const
{
	return { tocdata_, filenames_ };
}


void ParserToCHandler::set_isrc(const std::size_t t, const std::string& isrc)
{
	isrcs_[t] = isrc;
}


std::string ParserToCHandler::isrc(const std::size_t t) const
{
	return isrcs_[t];
}


void ParserToCHandler::set_mcn(const std::string& mcn)
{
	mcn_ = mcn;
}


void ParserToCHandler::set_disc_id(const std::string& disc_id)
{
	disc_id_ = disc_id;
}


} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

