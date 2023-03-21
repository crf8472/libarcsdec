#include "catch2/catch_test_macros.hpp"

#ifndef __LIBARCSDEC_READERWAV_HPP__
#include "readerwav.hpp"
#endif
#ifndef __LIBARCSDEC_READERWAV_DETAILS_HPP__
#include "readerwav_details.hpp"
#endif
#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"
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


TEST_CASE ("FileReaderSelection", "[filereaderselection]")
{
	using arcsdec::FileReaderSelection;
	using arcsdec::FileReaderRegistry;
	using arcsdec::Format;
	using arcsdec::Codec;

	const auto default_selection {
		FileReaderRegistry::default_audio_selection() };

	REQUIRE ( default_selection );

	const auto default_readers { FileReaderRegistry::readers() };

	REQUIRE ( default_readers );


	SECTION ( "Descriptor is registered" )
	{
		CHECK ( nullptr != arcsdec::FileReaderRegistry::reader("wavpcm") );
	}

	SECTION ( "Default settings select wavpcm for RIFFWAVE/PCM16LE" )
	{
		auto reader = default_selection->get(Format::WAV, Codec::PCM_S16LE,
				*default_readers );

		CHECK ( "wavpcm" == reader->id() );
	}

	SECTION ( "Default settings select wavpcm for RIFFWAVE/UNKNOWN" )
	{
		auto reader = default_selection->get(Format::WAV, Codec::UNKNOWN,
				*default_readers );

		CHECK ( "wavpcm" == reader->id() );
	}
}

