#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for readerwav_details.hpp.
 */

#ifndef __LIBARCSDEC_READERWAV_HPP__
//#include "readerwav.hpp"
#define __LIBARCSDEC_READERWAV_HPP__
#endif
#ifndef __LIBARCSDEC_READERWAV_DETAILS_HPP__
#include "readerwav_details.hpp"
#endif


TEST_CASE ( "RIFFWAV_PCM_CDDA_t constants", "[readerwav]" )
{
	using arcsdec::details::wave::RIFFWAV_PCM_CDDA_t;

	RIFFWAV_PCM_CDDA_t w;

	// RIFF Chunk
	CHECK( w.chunk_id()          ==  0x52494646 );
	CHECK( w.format()            ==  0x57415645 );
	CHECK( w.fmt_subchunk_id()   ==  0x666D7420 );
	CHECK( w.data_subchunk_id()  ==  0x64617461 );

	// Format Subchunk
	CHECK( w.fmt_subchunk_size() ==  16 );
	CHECK( w.wFormatTag()        ==  1 );
	CHECK( w.wChannels()         ==  2 );
	CHECK( w.dwSamplesPerSec()   ==  44100 );
	CHECK( w.dwAvgBytesPerSec()  ==  176400 );
	CHECK( w.wBlockAlign()       ==  4 );
	CHECK( w.wBitsPerSample()    ==  16 );
}

