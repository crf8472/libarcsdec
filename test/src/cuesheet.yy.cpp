#include "catch2/catch_test_macros.hpp"

#include <fstream>

/**
 * \file
 *
 * Fixtures for testing generated parser/lexer on Cuesheet input
 */

#include "cuesheet/driver.hpp"

TEST_CASE ("cuesheet", "[yycuesheet]" )
{
	using cuesheet::Driver;

	Driver driver;

	//cuesheet::DefaultHandler handler;
	//driver.set_handler(&handler);

	/* Activate debugging for flex-generated scanner */
	driver.set_lexer_debug_level(3);
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
}

