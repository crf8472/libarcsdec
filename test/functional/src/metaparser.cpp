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
	SECTION ("file_content loads file")
	{
		using arcsdec::read::details::file_content;

		auto cue_toc_05 = file_content("data/ok01.cue", 524);

		CHECK ( cue_toc_05 );

		auto v = cue_toc_05.value();

		CHECK ( v.size() == 524 );

		CHECK ( v[0]   == 'C' );
		CHECK ( v[1]   == 'A' );

		CHECK ( v[521] == 'X' );
		CHECK ( v[522] == '\r' );
		CHECK ( v[523] == '\n' );
		CHECK ( v[524] == '\0' );
	}
}

