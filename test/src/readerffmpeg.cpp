#include "catch2/catch.hpp"

#ifndef __LIBARCSDEC_READERFFMPEG_HPP__
#include "readerffmpeg.hpp"
#endif


/**
 * \file
 *
 * Tests for all API classes exported by readerflac.hpp
 */


TEST_CASE ("DescriptorFFmpeg", "[readerffmpeg]" )
{
	using arcsdec::DescriptorFFmpeg;

	auto d = DescriptorFFmpeg{};

	SECTION ("Matches names correctly")
	{
		CHECK ( d.accepts_name("foo.everything") );
		CHECK ( d.accepts_name("bar.allesmoegliche") );
		CHECK ( d.accepts_name("bar.anystuff") );

		CHECK ( d.accepts_name("bar.auchdashier") );
		CHECK ( d.accepts_name("bar.alsothis") );

		CHECK ( d.accepts_name("bar.andthis") );
		CHECK ( d.accepts_name("bar.thisinparticular") );
	}
}

