/**
 * \file
 *
 * \brief
 */

#ifndef LIBARCSDEC_PARSERCUE_HPP_
#define LIBARCSDEC_PARSERCUE_HPP_      // allow parsercue_details.hpp
#endif
#ifndef LIBARCSDEC_PARSERCUE_DETAILS_HPP_
#include "parsercue_details.hpp"        // TO BE TESTED
#endif
#ifndef LIBARCSDEC_CUESHEET_DRIVER_HPP_
#include "cuesheet/driver.hpp"          // for Driver
#endif
#ifndef LIBARCSDEC_TOCHANDLER_HPP_
#include "tochandler.hpp"               // for ParserToCHandler
#endif

#include <string>      // for vector
#include <vector>      // for string

#include <cstdlib>    // for EXIT_SUCCESS
#include <iostream>   // for cout, cerr

// generated yy parts

using arcsdec::read::details::cuesheet::yycuesheet::location;
using arcsdec::read::details::cuesheet::yycuesheet::position;
using arcsdec::read::details::cuesheet::yycuesheet::Lexer;
using arcsdec::read::details::cuesheet::yycuesheet::Parser;

// libarcsdec parts

using arcsdec::read::details::cuesheet::Driver;

using arcsdec::read::details::DefaultLexerHandler;
using arcsdec::read::details::ParserToCHandler;
using arcsdec::read::details::TokenLocation;

int main(int argc, char** argv)
{
	std::cout << "sizeof(Lexer): " << sizeof(Lexer) << '\n';
	std::cout << "alignof(Lexer): " << alignof(Lexer) << '\n';

	std::cout << "sizeof(Parser): " << sizeof(Parser) << '\n';
	std::cout << "alignof(Parser): " << alignof(Parser) << '\n';

	auto loc = TokenLocation<position, location> {};

	auto lh = DefaultLexerHandler {};
	auto ph = ParserToCHandler {};

	auto lexer = Lexer { &loc, &lh };
	auto parser = Parser { &loc, &lexer, &ph };
}

