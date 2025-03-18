/**
 * \file
 *
 * \brief Implementations for flexbisondriver.hpp.
 */

#ifndef __LIBARCSDEC_FLEXBISONDRIVER_HPP__
#include "flexbisondriver.hpp"
#endif

#include <fstream>     // for ifstream
#include <stdexcept>   // for invalid_argument
#include <string>      // for vector
#include <vector>      // for string

namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{


// LexerHandler


LexerHandler::~LexerHandler() noexcept = default;


void LexerHandler::notify(const std::string& token_name,
		const std::string& chars)
{
	do_notify(token_name, chars);
}


// DefaultLexerHandler


void DefaultLexerHandler::do_notify(const std::string& /* token_name */,
		const std::string& /* chars */)
{
	// empty
}


// ParserHandler


ParserHandler::~ParserHandler() noexcept = default;


std::vector<int32_t> ParserHandler::offsets() const
{
	return do_offsets();
}


std::vector<std::string> ParserHandler::filenames() const
{
	return do_filenames();
}


// DefaultParserHandler


DefaultParserHandler::DefaultParserHandler()
	: offsets_   (/* empty */)
	, filenames_ (/* empty */)
{
	// empty
}


std::vector<int32_t> DefaultParserHandler::do_offsets() const
{
	return offsets_;
}


std::vector<std::string> DefaultParserHandler::do_filenames() const
{
	return filenames_;
}


// remove_quotes


std::string strip_quotes(const std::string& s)
{
	if (s.length()  < 2) { return s; };
	if (s.length() == 2) { return std::string{}; };

	return s.substr(1, s.length() - 2);
}


// msf_to_frames


long to_frames(const int m, const int s, const int f)
{
	if (m < 0 || m > 99 || s < 0 || s >= 60 || f < 0 || f >= 75) {
		return -1;
	}

	return (m * 60 + s) * 75 + f;
}

} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

