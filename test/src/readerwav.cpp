#include "catch2/catch_test_macros.hpp"

#ifndef __LIBARCSDEC_READERWAV_HPP__
#include "readerwav.hpp"
#endif
#ifndef __LIBARCSDEC_READERWAV_DETAILS_HPP__
#include "readerwav_details.hpp"
#endif

/**
 * \file
 *
 * Tests for classes in readerwav.cpp
 */


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


TEST_CASE ("DescriptorWavPCM", "[readerwav]" )
{
	using arcsdec::DescriptorWavPCM;
	using arcsdec::details::wave::RIFFWAV_PCM_CDDA_t;

	auto d = DescriptorWavPCM {};

	SECTION ("Matches names correctly")
	{
		CHECK ( d.accepts_name("foo.wav") );
		CHECK ( d.accepts_name("bar.WAV") );
		CHECK ( d.accepts_name("foo.wave") );
		CHECK ( d.accepts_name("bar.WAVE") );
		CHECK ( d.accepts_name("foo.wAvE") );
		CHECK ( d.accepts_name("bar.Wave") );

		CHECK ( not d.accepts_name("bar.WAVX") );
		CHECK ( not d.accepts_name("bar.wavx") );
		CHECK ( not d.accepts_name("bar.waving") );
		CHECK ( not d.accepts_name("bar.warg") );
		CHECK ( not d.accepts_name("bar.walar") );
		CHECK ( not d.accepts_name("bar.WALINOR") );
		CHECK ( not d.accepts_name("bar.PWAV") );
		CHECK ( not d.accepts_name("bar.pwav") );
		CHECK ( not d.accepts_name("bar.CWAVE") );
		CHECK ( not d.accepts_name("bar.cwave") );
	}

	SECTION ( "accept_bytes()" )
	{
		RIFFWAV_PCM_CDDA_t w;

		CHECK ( not d.accepts_bytes( {}, 0 ));
		CHECK ( not d.accepts_bytes( {}, 12 ));
		CHECK ( not d.accepts_bytes( {}, 45 ));
		CHECK ( not d.accepts_bytes( {}, 145 ));

		// wav-header (0-11)
		CHECK (     d.accepts_bytes( {'R', 'I', 'F', 'F'}, 0) );
		CHECK ( not d.accepts_bytes( {'R', 'I', 'F', 'F'}, 3) );
		CHECK (     d.accepts_bytes( {'I', 'F', 'F'}, 1) );
		CHECK ( not d.accepts_bytes( {'I', 'F', 'F'}, 2) );
		CHECK (     d.accepts_bytes( {'W', 'A', 'V', 'E'}, 8) );
		CHECK ( not d.accepts_bytes( {'W', 'A', 'V', 'E'}, 9) );

		// 'fmt ' (12-33)
		CHECK (     d.accepts_bytes( {'f', 'm', 't', ' '}, 12) );
		CHECK ( not d.accepts_bytes( {'f', 'm', 't', '_'}, 12) );
		// size == 16, wFormatTag == 1, Channels == 2, dwSamplesPerSec = 44.100
		CHECK ( d.accepts_bytes( { 16, 0, 0, 0, 1, 0, 2, 0, 68, 172, 0, 0 },
				16) );
		CHECK ( not d.accepts_bytes( { 16, 1, 0, 0, 1, 1, 2, 1, 68, 173, 0, 0 },
				16) );
		CHECK ( d.accepts_bytes( { 68, 172, 0, 0}, 24));
		// dwAvgBytesPerSec == 176400, wBlockAlign  == 4
		CHECK ( d.accepts_bytes( { 16, 177, 2, 0, 4, 0 }, 28));
		CHECK ( not d.accepts_bytes( { 16, 177, 2, 1, 5, 0 }, 28));
		// wBitsPerSample == 16
		CHECK ( d.accepts_bytes( { 16, 0 }, 34));
		CHECK ( not d.accepts_bytes( { 16, 1 }, 34));
		CHECK ( not d.accepts_bytes( { 17, 0 }, 34));

		CHECK ( not d.accepts_bytes( { 0, 0, 0, 16, 0, 1, 0, 2, 0, 0 }, 15) );
		CHECK ( not d.accepts_bytes( { 0, 0, 0, 16, 0, 1, 1, 2, 0, 0 }, 16) );
		CHECK ( not d.accepts_bytes( { 16, 176, 2, 0, 4, 0 }, 28));
		CHECK ( not d.accepts_bytes( { 16, 176, 2, 0, 5, 0 }, 28));

		// Accepts any declared file size?

		CHECK (     d.accepts_bytes( {' ', ' ', ' ', ' ' }, 4) );
		CHECK (     d.accepts_bytes( {' ', ' ', ' ', ' ', 'W' }, 4) );
		CHECK ( not d.accepts_bytes( {' ', ' ', ' ', ' ', 'T' }, 4) );
		CHECK (     d.accepts_bytes( {'I', 'F', 'F', ' ', ' ', ' ', ' ', 'W' },
					1) );
		CHECK (     d.accepts_bytes( {'I', 'F', 'F', '1', '2', '3', '4', 'W' },
					1) );
		CHECK ( not d.accepts_bytes( {'I', 'F', 'F', ' ', ' ', ' ', ' ', 'X' },
					1) );

		// Accepts any declared data chunk size?

		CHECK (     d.accepts_bytes( {' ', ' ', ' ', ' ' }, 40) );
		CHECK (     d.accepts_bytes( {' ', ' ', ' ', ' ', '%' }, 40) );
		CHECK (     d.accepts_bytes( {'a', 't', 'a', ' ', ' ', ' ', ' ', 'W' },
					37) );
		CHECK (     d.accepts_bytes( {'a', 't', 'a', '1', '2', '3', '4', 'T' },
					37) );
		CHECK ( not d.accepts_bytes( {'a', 't', 'i', ' ', ' ', ' ', ' ', 'X' },
					37) );
		CHECK (     d.accepts_bytes( {'a', 't', 'a', '1', '2', '3', '4' },
					37) );
	}
}

