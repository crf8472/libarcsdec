#include "catch2/catch.hpp"

#ifndef __LIBARCSDEC_READERWAV_HPP__
#include "readerwav.hpp"
#endif
#ifndef __LIBARCSDEC_READERWAV_DETAILS_HPP__
#include "readerwav_details.hpp"
#endif

/**
 * \file
 *
 * Tests for all API classes exported by readerwav.hpp
 */


TEST_CASE ( "RIFFWAV_PCM_CDDA_t constants", "[readerwav]" )
{
	arcsdec::RIFFWAV_PCM_CDDA_t w;

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


TEST_CASE ( "RIFFWAV_PCM_CDDA_t match()", "[readerwav]" )
{
	arcsdec::RIFFWAV_PCM_CDDA_t w;

	CHECK ( !w.match( {}, 0 ));
	CHECK ( !w.match( {}, 12 ));
	CHECK ( !w.match( {}, 45 ));
	CHECK ( !w.match( {}, 145 ));

	// wav-header (0-11)
	CHECK (  w.match( {'R', 'I', 'F', 'F'}, 0) );

	CHECK (  w.match( {'I', 'F', 'F', ' ', ' ', ' ', ' ', 'W' }, 1) );
	CHECK (  w.match( {'I', 'F', 'F', '1', '2', '3', '4', 'W' }, 1) );
	CHECK ( !w.match( {'I', 'F', 'F', ' ', ' ', ' ', ' ', 'X' }, 1) );

	CHECK (  w.match( {'W', 'A', 'V', 'E'}, 8) );
	CHECK ( !w.match( {'W', 'A', 'V', 'E'}, 9) );

	// 'fmt ' (12-33)
	CHECK (  w.match( {'f', 'm', 't', ' '}, 12) );
	CHECK ( !w.match( {'f', 'm', 't', '_'}, 12) );
	// size == 16, wFormatTag == 1, Channels == 2, dwSamplesPerSec = 44.100
	CHECK (  w.match( { 16, 0, 0, 0, 1, 0, 2, 0, 68, static_cast<char>(172),
				0, 0 }, 16) );
	CHECK (  w.match( { 68, static_cast<char>(172), 0, 0}, 24));
	// dwAvgBytesPerSec == 176400, wBlockAlign  == 4, wBitsPerSample == 16
	CHECK (  w.match( { 16, static_cast<char>(177), 2, 0, 4, 0 }, 28));

	CHECK ( !w.match( { 0, 0, 0, 16, 0, 1, 0, 2, 0, 0 }, 15) );
	CHECK ( !w.match( { 0, 0, 0, 16, 0, 1, 1, 2, 0, 0 }, 16) );
	CHECK ( !w.match( { 16, static_cast<char>(176), 2, 0, 4, 0 }, 28));
	CHECK ( !w.match( { 16, static_cast<char>(176), 2, 0, 5, 0 }, 28));
}


TEST_CASE ("DescriptorWavPCM", "[readerwav]" )
{
	using arcsdec::DescriptorWavPCM;

	auto d = DescriptorWavPCM {};

	SECTION ("Matches names correctly")
	{
		CHECK ( d.accepts_name("foo.wav") );
		CHECK ( d.accepts_name("bar.WAV") );
		CHECK ( d.accepts_name("foo.wave") );
		CHECK ( d.accepts_name("bar.WAVE") );
		CHECK ( d.accepts_name("foo.wAvE") );
		CHECK ( d.accepts_name("bar.Wave") );

		CHECK ( !d.accepts_name("bar.WAVX") );
		CHECK ( !d.accepts_name("bar.wavx") );
		CHECK ( !d.accepts_name("bar.waving") );
		CHECK ( !d.accepts_name("bar.warg") );
		CHECK ( !d.accepts_name("bar.walar") );
		CHECK ( !d.accepts_name("bar.WALINOR") );
		CHECK ( !d.accepts_name("bar.PWAV") );
		CHECK ( !d.accepts_name("bar.pwav") );
		CHECK ( !d.accepts_name("bar.CWAVE") );
		CHECK ( !d.accepts_name("bar.cwave") );
	}
}

