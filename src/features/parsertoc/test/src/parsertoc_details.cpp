#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for parsertoc_details.hpp.
 */

#ifndef LIBARCSDEC_PARSERTOC_HPP_
#define LIBARCSDEC_PARSERTOC_HPP_        // allow parsertoc_details.hpp
#endif
#ifndef LIBARCSDEC_PARSERTOC_DETAILS_HPP_
#include "parsertoc_details.hpp"         // TO BE TESTED
#endif

#include <fstream>                       // for ifstream

#ifndef LIBARCSDEC_CUESHEET_DRIVER_HPP_
#include "cdrtoc/driver.hpp"             // for Driver
#endif
#ifndef LIBARCSDEC_TOCHANDLER_HPP_
#include "tochandler.hpp"                // for ParserToCHandler
#endif


TEST_CASE ("cdrtoc", "[yycdrtoc]" )
{
	using arcsdec::read::details::DefaultLexerHandler;
	using arcsdec::read::details::ParserToCHandler;
	using arcsdec::read::details::cdrtoc::Driver;

	auto lexer_handler  = DefaultLexerHandler {};
	auto parser_handler = ParserToCHandler {};
	auto driver = Driver { &lexer_handler, &parser_handler };

	// TODO
}

