#include "catch2/catch_test_macros.hpp"

#include <fstream>

/**
 * \file
 *
 * Fixtures for testing generated parser/lexer on Cuesheet input
 */

#include "cuesheet/driver.hpp"
#include "cuesheet/tochandler.hpp"

TEST_CASE ("cuesheet", "[yycuesheet]" )
{
	using arcsdec::details::cuesheet::Driver;
	using arcsdec::details::cuesheet::TOCHandler;

	Driver driver;
	TOCHandler handler;

	driver.set_handler(handler);

	/* Activate debugging for flex-generated scanner */
	driver.set_lexer_debug_level(0);
	// Note: use --debug when running flex on cuesheet.l to have debug support

	/* Activate debugging for bison-generated parser */
	//driver.set_parser_debug_level(0);
	// Note: use --debug when running bison on cuesheet.y to have debug support

	SECTION ("Cuesheet without syntax errors and trailing newline is OK")
	{
		std::ifstream file;
		file.open("test01_ok.cue", std::ifstream::in);
		driver.set_input(file);
		const int res { driver.parse() };

		CHECK ( res == 0 );
	}

	SECTION ("Cuesheet without syntax errors and no newline is OK")
	{
		std::ifstream file;
		file.open("test01_ok_without_lf.cue", std::ifstream::in);
		driver.set_input(file);
		const int res { driver.parse() };

		CHECK ( res == 0 );
	}

	SECTION ("Cuesheet without syntax errors and no newline is OK")
	{
		std::ifstream file;
		file.open("bach.cue", std::ifstream::in);
		driver.set_input(file);
		const int res { driver.parse() };

		CHECK ( res == 0 );
		CHECK ( handler.offsets().size() == 15 );
		CHECK ( handler.offsets() == std::vector<int32_t>{
					  33,
					5225,
					7390,
				   23380,
				   35608,
				   49820,
				   69508,
				   87733,
				  106333,
				  139495,
				  157863,
				  198495,
				  213368,
				  225320,
				  234103
				});
		CHECK ( handler.lengths().size() == 15 );
		CHECK ( handler.lengths() == std::vector<int32_t>{
				    5192,
					2165,
				   15990,
				   12228,
				   14212,
				   19688,
				   18225,
				   18600,
				   33162,
				   18368,
				   40632,
				   14873,
				   11952,
				    8783,
				      -1 //18935 // not from Cuesheet
				});
	}
}

