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


TEST_CASE ("DescriptorWavPCM", "[readerwav]" )
{
	using arcsdec::DescriptorWavPCM;
	using arcsdec::Format;
	using arcsdec::Codec;
	using arcsdec::details::wave::RIFFWAV_PCM_CDDA_t;

	auto d = DescriptorWavPCM {};

	SECTION ("Returns own name correctly")
	{
		CHECK ( "RIFF/WAV(PCM)" == d.name() );
	}

	SECTION ("Returns linked libraries correctly")
	{
		const auto libs = d.libraries();

		CHECK ( libs.size() == 1 );
		CHECK ( libs.front().first  == "-genuine-" );
		CHECK ( libs.front().second.find("libarcsdec") != std::string::npos );
	}

	SECTION ("Matches accepted bytes correctly")
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

	SECTION ("Matches accepted codecs correctly")
	{
		CHECK ( d.accepts(Codec::PCM_S16BE) );
		CHECK ( d.accepts(Codec::PCM_S16BE_PLANAR) );
		CHECK ( d.accepts(Codec::PCM_S16LE) );
		CHECK ( d.accepts(Codec::PCM_S16LE_PLANAR) );
		CHECK ( d.accepts(Codec::PCM_S32BE) );
		CHECK ( d.accepts(Codec::PCM_S32BE_PLANAR) );
		CHECK ( d.accepts(Codec::PCM_S32LE) );
		CHECK ( d.accepts(Codec::PCM_S32LE_PLANAR) );
	}

	SECTION ("Does not match codecs not accepted by this descriptor")
	{
		CHECK ( !d.accepts(Codec::UNKNOWN) );
		CHECK ( !d.accepts(Codec::FLAC) );
		CHECK ( !d.accepts(Codec::WAVPACK) );
		CHECK ( !d.accepts(Codec::MONKEY) );
		CHECK ( !d.accepts(Codec::ALAC) );
	}

	SECTION ("Returns accepted codecs correctly")
	{
		auto codecs = d.codecs();

		CHECK ( codecs == std::set<Codec>{ Codec::PCM_S16BE,
			Codec::PCM_S16BE_PLANAR,
			Codec::PCM_S16LE,
			Codec::PCM_S16LE_PLANAR,
			Codec::PCM_S32BE,
			Codec::PCM_S32BE_PLANAR,
			Codec::PCM_S32LE,
			Codec::PCM_S32LE_PLANAR} );
	}

	SECTION ("Returns no codecs that are not accepted")
	{
		CHECK ( d.codecs().size() == 8 );
	}

	SECTION ("Matches accepted formats correctly")
	{
		CHECK ( d.accepts(Format::WAV) );
	}

	SECTION ("Does not match any formats not accepted by this descriptor")
	{
		CHECK ( !d.accepts(Format::UNKNOWN) );
		CHECK ( !d.accepts(Format::CDRDAO)  );
		CHECK ( !d.accepts(Format::WV)      );
		CHECK ( !d.accepts(Format::FLAC)    );
		CHECK ( !d.accepts(Format::APE)     );
		CHECK ( !d.accepts(Format::CAF)     );
		CHECK ( !d.accepts(Format::M4A)     );
		CHECK ( !d.accepts(Format::OGG)     );
		CHECK ( !d.accepts(Format::AIFF)    );
	}

	SECTION ("Returns accepted formats correctly")
	{
		CHECK ( d.formats() == std::set<Format>{ Format::WAV } );
	}

	SECTION ("Matches accepted filenames correctly")
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
}


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

