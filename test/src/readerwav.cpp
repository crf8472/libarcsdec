#include "catch2/catch.hpp"

#ifndef __LIBARCSDEC_READERWAV_HPP__
#include "readerwav.hpp"
#endif
#ifndef __LIBARCSDEC_READERWAV_DETAILS_HPP__
#include "readerwav_details.hpp"
#endif

/**
 * \file readerwav.cpp Tests for all API classes exported by readerwav.hpp
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

	CHECK (  w.match( {'R', 'I', 'F', 'F'}, 0) );
	CHECK (  w.match( {'I', 'F', 'F', ' ', ' ', ' ', ' ', 'W' }, 1) );
	CHECK (  w.match( {'I', 'F', 'F', '1', '2', '3', '4', 'W' }, 1) );
	CHECK ( !w.match( {'I', 'F', 'F', ' ', ' ', ' ', ' ', 'X' }, 1) );

	CHECK (  w.match( {'W', 'A', 'V', 'E'}, 8) );
	CHECK ( !w.match( {'W', 'A', 'V', 'E'}, 9) );

	CHECK (  w.match( { 16, 0, 0, 0, 1, 0, 2, 0, 68, static_cast<char>(172) }, 16) );
	CHECK (  w.match( { 68, static_cast<char>(172), 0, 0}, 24));
	CHECK (  w.match( { 16, static_cast<char>(177), 2, 0}, 28));
	CHECK ( !w.match( { 0, 0, 0, 16, 0, 1, 0, 2, 0, 0 }, 15) );
	CHECK ( !w.match( { 0, 0, 0, 16, 0, 1, 1, 2, 0, 0 }, 16) );
}

