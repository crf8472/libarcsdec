#include "catch2/catch.hpp"

#ifndef __LIBARCSDEC_READERWAV_HPP__
#include "readerwvpk.hpp"
#endif
#ifndef __LIBARCSDEC_READERWAV_DETAILS_HPP__
#include "readerwvpk_details.hpp"
#endif

/**
 * \file readerwvpk.cpp Tests for all API classes exported by readerwvpk.hpp
 */

TEST_CASE ( "WAVPACK_WAV_PCM_CDDA_t constants", "[readerwvpk]" )
{
	arcsdec::WAVPACK_WAV_PCM_CDDA_t w;

	CHECK(  w.wav_format_only() );
	CHECK(  w.lossless()        );
	CHECK( !w.floats_ok()       );

	CHECK(  w.at_least_version() == 1 );
	CHECK(  w.at_most_version()  == 5 );
	CHECK(  w.bytes_per_sample() == 2 );
}

