#include "catch2/catch.hpp"

#ifndef __LIBARCSDEC_READERWAV_HPP__
#include "readerwvpk.hpp"
#endif
#ifndef __LIBARCSDEC_READERWAV_DETAILS_HPP__
#include "readerwvpk_details.hpp"
#endif


/**
 * \file
 *
 * Tests for all API classes exported by readerwvpk.hpp
 */


TEST_CASE ( "WAVPACK_WAV_PCM_CDDA_t constants", "[readerwvpk]" )
{
	arcsdec::details::wavpack::WAVPACK_WAV_PCM_CDDA_t w;

	CHECK(  w.wav_format_only() );
	CHECK(  w.lossless()        );
	CHECK( !w.floats_ok()       );

	CHECK(  w.at_least_version() == 1 );
	CHECK(  w.at_most_version()  == 5 );
	CHECK(  w.bytes_per_sample() == 2 );
}


TEST_CASE ("DescriptorWavpack", "[readerwvpk]" )
{
	using arcsdec::DescriptorWavpack;

	auto d = DescriptorWavpack {};

	SECTION ("Matches names correctly")
	{
		CHECK ( d.accepts_name("foo.wv") );
		CHECK ( d.accepts_name("bar.WV") );

		CHECK ( !d.accepts_name("bar.WAV") );
		CHECK ( !d.accepts_name("bar.wav") );

		CHECK ( !d.accepts_name("bar.rwv") );
		CHECK ( !d.accepts_name("bar.RWV") );

		CHECK ( !d.accepts_name("bar.wvx") );
		CHECK ( !d.accepts_name("bar.WVX") );
	}
}

