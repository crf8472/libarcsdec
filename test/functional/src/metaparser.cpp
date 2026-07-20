#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Testcases for metaparser.hpp
 */


#ifndef LIBARCSDEC_METAPARSER_HPP_
#define LIBARCSDEC_METAPARSER_HPP_
#endif
#ifndef LIBARCSDEC_METAPARSER_DETAILS_HPP_
#include "metaparser_details.hpp"           // TO BE TESTED
#endif


TEST_CASE ("file_content", "[metaparser]" )
{
	using arcsdec::read::details::file_content;

	SECTION ("file_content loads file")
	{
		const auto cue_toc_05 = file_content("data/ok01.cue", 524);

		CHECK ( cue_toc_05 );

		auto s = cue_toc_05.value();

		CHECK ( s.size() == 524 );

		CHECK ( s[0]   == 'C' );
		CHECK ( s[1]   == 'A' );
		CHECK ( s[2]   == 'T' );
		CHECK ( s[3]   == 'A' );
		CHECK ( s[4]   == 'L' );

		CHECK ( s[519] == 'D' );
		CHECK ( s[520] == 'E' );
		CHECK ( s[521] == 'X' );
		CHECK ( s[522] == '\r' );
		CHECK ( s[523] == '\n' );
	}
}

