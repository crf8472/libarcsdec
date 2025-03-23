#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for parsercue_details.hpp.
 */

#ifndef __LIBARCSDEC_PARSERCUE_HPP__
#define __LIBARCSDEC_PARSERCUE_HPP__ // allow parsercue_details.hpp
#endif
#ifndef __LIBARCSDEC_PARSERCUE_DETAILS_HPP__
#include "parsercue_details.hpp"        // TO BE TESTED
#endif

#ifndef __LIBARCSDEC_CUESHEET_DRIVER_HPP__
#include "cuesheet/driver.hpp"          // for Driver
#endif
#ifndef __LIBARCSDEC_TOCHANDLER_HPP__
#include "tochandler.hpp"               // for ParserToCHandler
#endif

#include <fstream> // for ifstream


TEST_CASE ("cuesheet", "[yycuesheet]" )
{
	using arcsdec::details::DefaultLexerHandler;
	using arcsdec::details::ParserToCHandler;
	using arcsdec::details::cuesheet::Driver;

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
			file.open("cuesheet/ok01.cue", std::ifstream::in);
		}
		catch (const std::ifstream::failure& f)
		{
			throw std::runtime_error(
				std::string { "Failed to open file "
					"'cuesheet/ok01.cue', got: " }
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
			file.open("cuesheet/ok02.cue", std::ifstream::in);
		}
		catch (const std::ifstream::failure& f)
		{
			throw std::runtime_error(
				std::string { "Failed to open file "
					"'cuesheet/ok02.cue', got: " }
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
			file.open("cuesheet/ok03.cue", std::ifstream::in);
		}
		catch (const std::ifstream::failure& f)
		{
			throw std::runtime_error(
				std::string { "Failed to open file "
					"'cuesheet/ok03.cue', got: " }
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
			file.open("cuesheet/error01.cue", std::ifstream::in);
		}
		catch (const std::ifstream::failure& f)
		{
			throw std::runtime_error(
				std::string { "Failed to open file "
					"'cuesheet/error01.cue', got: " }
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
			file.open("cuesheet/error02.cue", std::ifstream::in);
		}
		catch (const std::ifstream::failure& f)
		{
			throw std::runtime_error(
				std::string { "Failed to open file "
					"'cuesheet/error02.cue', got: " }
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
			file.open("cuesheet/error03.cue", std::ifstream::in);
		}
		catch (const std::ifstream::failure& f)
		{
			throw std::runtime_error(
				std::string { "Failed to open file "
					"'cuesheet/error03.cue', got: " }
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
			file.open("cuesheet/error04.cue", std::ifstream::in);
		}
		catch (const std::ifstream::failure& f)
		{
			throw std::runtime_error(
				std::string { "Failed to open file "
					"'cuesheet/error04.cue', got: " }
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
			file.open("cuesheet/error05.cue", std::ifstream::in);
		}
		catch (const std::ifstream::failure& f)
		{
			throw std::runtime_error(
				std::string { "Failed to open file "
					"'cuesheet/error05.cue', got: " }
				+ typeid(f).name()
				+ ", message: " + f.what());
		}

		driver.set_input(file);
		const int result { driver.parse() };

		CHECK ( result > 0 );
	}
}

