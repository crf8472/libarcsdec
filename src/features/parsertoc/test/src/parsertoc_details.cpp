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


TEST_CASE ("cdrtoc/Driver", "[yycdrtoc]" )
{
	// generated yy parts

	using arcsdec::read::details::cdrtoc::yycdrtoc::location;
	using arcsdec::read::details::cdrtoc::yycdrtoc::position;
	using arcsdec::read::details::cdrtoc::yycdrtoc::Lexer;
	using arcsdec::read::details::cdrtoc::yycdrtoc::Parser;

	// libarcsdec parts

	using arcsdec::read::details::cdrtoc::Driver;

	using arcsdec::read::details::DefaultLexerHandler;
	using arcsdec::read::details::ParserToCHandler;
	using arcsdec::read::details::TokenLocation;

	auto lexer_handler  = DefaultLexerHandler {};
	auto parser_handler = ParserToCHandler {};
	auto driver = Driver { &lexer_handler, &parser_handler };


	SECTION ("Driver is instantiated correctly")
	{
		if (driver.debug_enabled())
		{
			CHECK (driver.lexer_debug_level()  == 1);
			CHECK (driver.parser_debug_level() == 1);
		} else
		{
			CHECK (driver.lexer_debug_level()  == 0);
			CHECK (driver.parser_debug_level() == 0);
		}

	}

	SECTION ("Lexer instantiates with default TokenLocation and LexerHandler")
	{
		auto lh = DefaultLexerHandler {};

		auto lexer = Lexer { {}, &lh };

		CHECK (lexer.debug() == 0); // always the default
	}

	SECTION ("Parser instantiates with default TokenLocation, Lexer"
			" and ParserHandler")
	{
		auto lh = DefaultLexerHandler {};
		auto ph = ParserToCHandler {};

		auto lexer = Lexer { {}, &lh };
		auto parser = Parser { {}, &lexer, &ph };

		CHECK (&parser);
	}

	SECTION ("Lexer and Parser combine correctly")
	{
		auto loc = TokenLocation<position, location> {};
		auto lh = DefaultLexerHandler {};
		auto ph = ParserToCHandler {};

		auto lexer  = std::make_unique<Lexer>(&loc, &lh);
		auto parser = Parser { &loc, lexer.get(), &ph };

		// This fails for unclear reasons in Debug build:
		//auto parser = std::make_unique<Parser>(&loc, lexer.get(), &ph);

		CHECK (&parser);
	}
}

