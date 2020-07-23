#include "catch2/catch.hpp"

#ifndef __LIBARCSDEC_READERFLAC_HPP__
#include "readerflac.hpp"
#endif


/**
 * \file
 *
 * Tests for all API classes exported by readerflac.hpp
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

