#include "catch2/catch_test_macros.hpp"

#ifndef __LIBARCSDEC_PARSERCUE_HPP__
#include "parsercue.hpp"
#endif
#ifndef __LIBARCSDEC_PARSERCUE_DETAILS_HPP__
#include "parsercue_details.hpp"
#endif


/**
 * \file
 *
 * Tests for classes in parsercue.cpp
 */


TEST_CASE ("DescriptorCue", "[parsercue]" )
{
	using arcsdec::DescriptorCue;

	auto d = DescriptorCue {};

	SECTION ("Matches filenames correctly")
	{
		CHECK ( d.accepts_name("foo.cue") );
		CHECK ( d.accepts_name("bar.CUE") );
		CHECK ( d.accepts_name("bar.CUe") );

		CHECK ( !d.accepts_name("bar.rcue") );
		CHECK ( !d.accepts_name("bar.PCUe") );

		CHECK ( !d.accepts_name("bar.cuef") );
		CHECK ( !d.accepts_name("bar.CUEl") );
	}
}

