#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for parsercue_details.hpp.
 */

#ifndef LIBARCSDEC_PARSERCUE_HPP_
#define LIBARCSDEC_PARSERCUE_HPP_      // allow parsercue_details.hpp
#endif
#ifndef LIBARCSDEC_PARSERCUE_DETAILS_HPP_
#include "parsercue_details.hpp"        // TO BE TESTED
#endif

#include <fstream>                      // for ifstream

#ifndef LIBARCSDEC_CUESHEET_DRIVER_HPP_
#include "cuesheet/driver.hpp"          // for Driver
#endif
#ifndef LIBARCSDEC_TOCHANDLER_HPP_
#include "tochandler.hpp"               // for ParserToCHandler
#endif


TEST_CASE ("cuesheet", "[yycuesheet]" )
{
	using arcsdec::read::details::DefaultLexerHandler;
	using arcsdec::read::details::ParserToCHandler;
	using arcsdec::read::details::cuesheet::Driver;

	auto l_hdler = DefaultLexerHandler { /* default */ } ;
	auto handler = ParserToCHandler {};
	auto driver  = Driver{&l_hdler, &handler};

	/* Activate debugging for flex-generated scanner */
	//driver.set_lexer_debug_level(0);
	// Note: use --debug when running flex on cuesheet.l to have debug support

	/* Activate debugging for bison-generated parser */
	//driver.set_parser_debug_level(0);
	// Note: use --debug when running bison on cuesheet.y to have debug support

	SECTION ("Cuesheet without syntax errors and trailing newline is OK")
	{
		std::ifstream file;
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			file.open("data/ok01.cue", std::ifstream::in);
		}
		catch (const std::ifstream::failure& f)
		{
			throw std::runtime_error(
				std::string { "Failed to open file "
					"'data/ok01.cue', got: " }
				+ typeid(f).name()
				+ ", message: " + f.what());
		}

		driver.set_input(file);
		const int result { driver.parse() };

		CHECK ( result == 0 );
	}

	SECTION ("Cuesheet without syntax errors and no newline is OK")
	{
		std::ifstream file;
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			file.open("data/ok02.cue", std::ifstream::in);
		}
		catch (const std::ifstream::failure& f)
		{
			throw std::runtime_error(
				std::string { "Failed to open file "
					"'data/ok02.cue', got: " }
				+ typeid(f).name()
				+ ", message: " + f.what());
		}

		driver.set_input(file);
		const int result { driver.parse() };

		CHECK ( result == 0 );
	}

	SECTION ("Cuesheet without syntax errors and no newline is OK")
	{
		std::ifstream file;
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			file.open("data/ok03.cue", std::ifstream::in);
		}
		catch (const std::ifstream::failure& f)
		{
			throw std::runtime_error(
				std::string { "Failed to open file "
					"'data/ok03.cue', got: " }
				+ typeid(f).name()
				+ ", message: " + f.what());
		}

		driver.set_input(file);
		const int result { driver.parse() };

		const auto toc { handler.get_toc() };

		CHECK ( result == 0 );
		CHECK ( handler.current_track() == 16 );

		// CHECK ( toc->offsets() == std::vector<int32_t>{
		// 			  33,
		// 			5225,
		// 			7390,
		// 		   23380,
		// 		   35608,
		// 		   49820,
		// 		   69508,
		// 		   87733,
		// 		  106333,
		// 		  139495,
		// 		  157863,
		// 		  198495,
		// 		  213368,
		// 		  225320,
		// 		  234103
		// 		});
		// CHECK ( handler.lengths().size() == 15 );
		// CHECK ( handler.lengths() == std::vector<int32_t>{
		// 			5192,
		// 			2165,
		// 		   15990,
		// 		   12228,
		// 		   14212,
		// 		   19688,
		// 		   18225,
		// 		   18600,
		// 		   33162,
		// 		   18368,
		// 		   40632,
		// 		   14873,
		// 		   11952,
		// 			8783,
		// 		      -1 //18935 // not from Cuesheet
		// 		});
	}

	SECTION ("Cuesheet with trailing chars in FILE statement fails")
	{
		std::ifstream file;
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			file.open("data/error01.cue", std::ifstream::in);
		}
		catch (const std::ifstream::failure& f)
		{
			throw std::runtime_error(
				std::string { "Failed to open file "
					"'data/error01.cue', got: " }
				+ typeid(f).name()
				+ ", message: " + f.what());
		}

		driver.set_input(file);
		const int result { driver.parse() };

		CHECK ( result > 0 );
	}

	SECTION ("Cuesheet with trailing chars in TRACK statement fails")
	{
		std::ifstream file;
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			file.open("data/error02.cue", std::ifstream::in);
		}
		catch (const std::ifstream::failure& f)
		{
			throw std::runtime_error(
				std::string { "Failed to open file "
					"'data/error02.cue', got: " }
				+ typeid(f).name()
				+ ", message: " + f.what());
		}

		driver.set_input(file);
		const int result { driver.parse() };

		CHECK ( result > 0 );
	}

	SECTION ("Cuesheet with trailing chars in INDEX statement fails")
	{
		std::ifstream file;
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			file.open("data/error03.cue", std::ifstream::in);
		}
		catch (const std::ifstream::failure& f)
		{
			throw std::runtime_error(
				std::string { "Failed to open file "
					"'data/error03.cue', got: " }
				+ typeid(f).name()
				+ ", message: " + f.what());
		}

		driver.set_input(file);
		const int result { driver.parse() };

		CHECK ( result > 0 );
	}

	SECTION ("Cuesheet erroneous leading characters in CDTEXTFILE fails")
	{
		std::ifstream file;
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			file.open("data/error04.cue", std::ifstream::in);
		}
		catch (const std::ifstream::failure& f)
		{
			throw std::runtime_error(
				std::string { "Failed to open file "
					"'data/error04.cue', got: " }
				+ typeid(f).name()
				+ ", message: " + f.what());
		}

		driver.set_input(file);
		const int result { driver.parse() };

		CHECK ( result > 0 );
	}

	SECTION ("Cuesheet with unknown global statement tag fails")
	{
		std::ifstream file;
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			file.open("data/error05.cue", std::ifstream::in);
		}
		catch (const std::ifstream::failure& f)
		{
			throw std::runtime_error(
				std::string { "Failed to open file "
					"'data/error05.cue', got: " }
				+ typeid(f).name()
				+ ", message: " + f.what());
		}

		driver.set_input(file);
		const int result { driver.parse() };

		CHECK ( result > 0 );
	}
}

