/**
 * \file
 *
 * \brief Implementations to tochandler.hpp.
 */

#ifndef __LIBARCSDEC_TOCHANDLER_HPP__
#include "tochandler.hpp"
#endif

#ifndef __LIBARCSTK_METADATA_HPP__
#include <arcstk/metadata.hpp>    // for ToC, AudioSize, UNIT
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#include <cctype>      // for isalnum, isdigit
#include <iomanip>     // for setw
#include <string>      // for vector
#include <vector>      // for string

namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{

using arcstk::ToC;
using arcstk::ToCData;


// to_sframes


int32_t to_sframes(const int32_t m, const int32_t s, const int32_t f)
{
	if (m < 0 || m > 99 || s < 0 || s >= 60 || f < 0 || f >= 75)
	{
		using std::to_string;
		throw std::runtime_error(std::string{"Values '"}
					+ to_string(m) + ":"
					+ to_string(s) + ":"
					+ to_string(f) + ":"
					+ "' are not a valid msf timestamp");
	}

	return (m * 60 + s) * 75 + f;
}


// to_uframes


uint64_t to_uframes(const uint64_t m, const uint64_t s, const uint64_t f)
{
	if (m > std::numeric_limits<int32_t>::max())
	{
		throw std::runtime_error(
				std::string{"Value "} + std::to_string(m) + " exceeds int32_t");
	}
	if (s > std::numeric_limits<int32_t>::max())
	{
		throw std::runtime_error(
				std::string{"Value "} + std::to_string(s) + " exceeds int32_t");
	}
	if (f > std::numeric_limits<int32_t>::max())
	{
		throw std::runtime_error(
				std::string{"Value "} + std::to_string(f) + " exceeds int32_t");
	}

	const auto frames = to_sframes(static_cast<int32_t>(m),
			static_cast<int32_t>(s),
			static_cast<int32_t>(f));

	if (frames < 0)
	{
		throw std::runtime_error(
				std::string{"Frame value "} + std::to_string(frames)
					+ " cannot be safely converted to uint64_t");
	}

	return static_cast<uint64_t>(frames);
}


// validate_mcn


void validate_mcn(const std::string& mcn)
{
	// MCN ::= [0-9]{13}

	if (mcn.length() != 13)
	{
		using std::to_string;
		throw std::runtime_error(std::string{"MCN validation:"}
				+ "String has wrong length ("
				+ to_string(mcn.length())
				+ ") instead of 13.");
	}

	using std::cbegin;
	using std::cend;

	if (!std::all_of(cbegin(mcn), cend(mcn), ::isdigit))
	{
		throw std::runtime_error(std::string{"MCN validation:"}
				+ "String contains chars that "
				+ "are not digits.");
	}
}


// validate_isrc


void validate_isrc(const std::string& isrc)
{
	// isrc has to be a length of 12 and format 'CCOOOYYSSSSS'
	//C: country code (upper case letters or digits)
	//O: owner code (upper case letters or digits)
	//Y: year (digits)
	//S: serial number (digits)
	// therefore [0-9A-Z]{5}[0-9]{7}

	using std::cbegin;
	using std::cend;

	if (!std::all_of(cbegin(isrc), cbegin(isrc) + 5, ::isalnum))
	{
		throw std::runtime_error(std::string{"ISRC validation:"}
				+ "Country and owner code parts contain "
				+ "chars that are not alphanumeric.");
	}

	if (!std::all_of(cbegin(isrc) + 5, cend(isrc), ::isdigit))
	{
		throw std::runtime_error(std::string{"ISRC validation:"}
				+ "Year and serial number parts contain "
				+ "chars that are not digits.");
	}
}


// validate_disc_id


void validate_disc_id(const std::string& disc_id)
{
	if (disc_id.length() > 8)
	{
		using std::to_string;
		throw std::runtime_error(std::string{"Disc id validation:"}
				+ "Id is too long ("
				+ to_string(disc_id.length())
				+ "chars) instead of 8 chars.");
	}

	using std::cbegin;
	using std::cend;

	if (!std::all_of(cbegin(disc_id), cend(disc_id), ::isalnum))
	{
		throw std::runtime_error(std::string{"Disc id validation:"}
				+ "Id is not a hash (contains non-alphanumeric chars).");
	}
}


// ParserTocHandler


ParserToCHandler::ParserToCHandler()
	: offsets_       { /* empty */ }
	, filenames_     { /* empty */ }
	, isrcs_         { /* empty */ }
	, current_track_ { 0 }
	, leadout_       { 0 }
	, mcn_           { /* empty */ }
	, disc_id_       { /* empty */ }
{
	// empty
}


ParserToCHandler::~ParserToCHandler() noexcept = default;


std::size_t ParserToCHandler::to_index(const std::size_t track) const
{
	return track > 0 ? track - 1 : 0;
}


void ParserToCHandler::dump_log() const
{
	const auto no_value = std::string {"(none)"};

	ARCS_LOG(DEBUG2) << "MCN: " << (mcn_.empty() ? no_value : mcn_);

	ARCS_LOG(DEBUG2) << "Disc Id (cddb): "
		<< (disc_id_.empty() ? no_value : disc_id_);

	const auto total_tracks = current_track_ - 1;

	ARCS_LOG(DEBUG2) << "Total tracks: " <<  total_tracks;

	auto t = int { 0 };
	auto isrc = std::string {/* empty */};

	for (const auto& o : offsets_)
	{
		try
		{
			isrc = this->isrc(static_cast<std::size_t>(t));
		} catch (const std::exception& e)
		{
			// do nothing
		}

		if (isrc.empty())
		{
			ARCS_LOG(DEBUG2) << "Offset " << std::setw(2) << ++t
				<< ": " << std::setw(6) << o;
		} else
		{
			ARCS_LOG(DEBUG2) << "Offset " << std::setw(2) << ++t
				<< ": " << std::setw(6) << o
				<< ", ISRC: " << isrc;
		}
	}
}


void ParserToCHandler::do_start_input()
{
	current_track_ = 1;
}


void ParserToCHandler::do_end_input()
{
	// TODO Check loglevel and call dump_log only if it would print sth
	this->dump_log();
}


void ParserToCHandler::append_offset(const uint64_t& frames)
{
	offsets_.push_back(frames);
}


void ParserToCHandler::set_offset(const std::size_t t, const uint64_t& frames)
{
	offsets_[to_index(t)] = frames;
}


int32_t ParserToCHandler::offset(const std::size_t t) const
{
	return offsets_.at(to_index(t));
}


void ParserToCHandler::append_filename(const std::string& filename)
{
	filenames_.push_back(filename);
}


std::string ParserToCHandler::filename(const std::size_t t) const
{
	return filenames_.at(to_index(t));
}


void ParserToCHandler::inc_current_track()
{
	++current_track_;
}


std::size_t ParserToCHandler::current_track() const
{
	return current_track_;
}


std::unique_ptr<ToC> ParserToCHandler::get_toc() const
{
	return arcstk::make_toc(offsets_, filenames_);
}


void ParserToCHandler::append_isrc(const std::string& isrc)
{
	isrcs_.push_back(isrc);
}


std::string ParserToCHandler::isrc(const std::size_t t) const
{
	return isrcs_.at(to_index(t));
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

