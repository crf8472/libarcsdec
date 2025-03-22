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


void ParserHandler::start_input()
{
	do_start_input();
}


void ParserHandler::end_input()
{
	do_end_input();
}


// remove_quotes


std::string strip_quotes(const std::string& s)
{
	if (s.length()  < 2) { return s; };
	if (s.length() == 2) { return std::string{}; };

	return s.substr(1, s.length() - 2);
}

} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

