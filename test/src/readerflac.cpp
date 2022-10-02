#include "catch2/catch_test_macros.hpp"

#ifndef __LIBARCSDEC_READERFLAC_HPP__
#include "readerflac.hpp"
#endif
#ifndef __LIBARCSDEC_READERFLAC_DETAILS_HPP__
#include "readerflac_details.hpp"
#endif


/**
 * \file
 *
 * Tests for classes in readerflac.cpp
 */


TEST_CASE ("DescriptorFlac", "[readerflac]" )
{
	using arcsdec::DescriptorFlac;

	auto d = DescriptorFlac {};

	SECTION ("Matches names correctly")
	{
		CHECK ( d.accepts_name("foo.flac") );
		CHECK ( d.accepts_name("bar.FLAC") );
		CHECK ( d.accepts_name("bar.FlAc") );

		CHECK ( !d.accepts_name("bar.rflac") );
		CHECK ( !d.accepts_name("bar.PFLac") );

		CHECK ( !d.accepts_name("bar.flacr") );
		CHECK ( !d.accepts_name("bar.FLACD") );
	}
}

