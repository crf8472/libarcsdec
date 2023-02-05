#include "catch2/catch_test_macros.hpp"

#ifndef __LIBARCSDEC_PARSERCUE_HPP__
#include "parsercue.hpp"
#endif
#ifndef __LIBARCSDEC_PARSERCUE_DETAILS_HPP__
#include "parsercue_details.hpp"
#endif

/**
 * \file
 *
 * Tests for classes in parsercue.cpp
 */


TEST_CASE ("DescriptorCue", "[parsercue]" )
{
	using arcsdec::DescriptorCue;
	using arcsdec::Format;
	using arcsdec::Codec;

	auto d = DescriptorCue{};

	SECTION ("Returns own name correctly")
	{
		CHECK ( "CueSheet" == d.name() );
	}

	SECTION ("Returns linked libraries correctly")
	{
		const auto libs = d.libraries();

		CHECK ( libs.size() == 1 );
		CHECK ( libs.front().first  == "libcue" );
		CHECK ( libs.front().second.find("libcue.so") != std::string::npos );
	}

	SECTION ("Matches accepted bytes correctly")
	{
		CHECK ( d.accepts_bytes({}, 0) );
		CHECK ( d.accepts_bytes({3, 2, 1}, 2) );
		CHECK ( d.accepts_bytes({0x65, 0x32, 0x88}, 1) );
		// TODO Check for always true
	}

	SECTION ("Does not match any codecs not accepted by this descriptor")
	{
		CHECK ( !d.accepts(Codec::UNKNOWN) );
		CHECK ( !d.accepts(Codec::PCM_S16BE) );
		CHECK ( !d.accepts(Codec::PCM_S16BE_PLANAR) );
		CHECK ( !d.accepts(Codec::PCM_S16LE) );
		CHECK ( !d.accepts(Codec::PCM_S16LE_PLANAR) );
		CHECK ( !d.accepts(Codec::PCM_S32BE) );
		CHECK ( !d.accepts(Codec::PCM_S32BE_PLANAR) );
		CHECK ( !d.accepts(Codec::PCM_S32LE) );
		CHECK ( !d.accepts(Codec::PCM_S32LE_PLANAR) );
		CHECK ( !d.accepts(Codec::FLAC) );
		CHECK ( !d.accepts(Codec::WAVPACK) );
		CHECK ( !d.accepts(Codec::MONKEY) );
		CHECK ( !d.accepts(Codec::ALAC) );
	}

	SECTION ("Returns accepted codecs correctly")
	{
		CHECK ( d.codecs() == std::set<Codec>{ } );
	}

	SECTION ("Returns no codecs that are not accepted")
	{
		CHECK ( d.codecs().size() == 0 );
	}

	SECTION ("Matches accepted formats correctly")
	{
		CHECK ( d.accepts(Format::CUE) );
	}

	SECTION ("Does not match any formats not accepted by this descriptor")
	{
		CHECK ( !d.accepts(Format::UNKNOWN) );
		CHECK ( !d.accepts(Format::CDRDAO)  );
		CHECK ( !d.accepts(Format::WAV)     );
		CHECK ( !d.accepts(Format::FLAC)    );
		CHECK ( !d.accepts(Format::APE)     );
		CHECK ( !d.accepts(Format::CAF)     );
		CHECK ( !d.accepts(Format::M4A)     );
		CHECK ( !d.accepts(Format::OGG)     );
		CHECK ( !d.accepts(Format::WV)      );
		CHECK ( !d.accepts(Format::AIFF)    );
	}

	SECTION ("Returns accepted formats correctly")
	{
		CHECK ( d.formats() == std::set<Format>{ Format::CUE } );
	}

	SECTION ("Matches accepted filenames correctly")
	{
		CHECK ( d.accepts_name("foo.cue") );
		CHECK ( d.accepts_name("bar.CUE") );
		CHECK ( d.accepts_name("bar.CUe") );

		CHECK ( !d.accepts_name("bar.rcue") );
		CHECK ( !d.accepts_name("bar.PCUe") );

		CHECK ( !d.accepts_name("bar.cuef") );
		CHECK ( !d.accepts_name("bar.CUEl") );
	}

	// TODO Does create_reader() really create a CueParserImpl?
}


TEST_CASE ("CueParserImpl", "[parsercue]" )
{
	using arcsdec::details::libcue::CueParserImpl;
	using arcsdec::DescriptorCue;

	auto d = CueParserImpl{}.descriptor();

	SECTION ("Parser implementation returns correct descriptor type")
	{
		CHECK ( d );
		auto p = d.get();

		CHECK ( dynamic_cast<const DescriptorCue*>(p) != nullptr );
	}

	SECTION ("Parses a syntactically intact input correctly")
	{
		using arcsdec::details::libcue::CueParserImpl;
		auto parser = CueParserImpl{};
		const auto cue = parser.parse("test01_ok.cue");

		CHECK ( cue->total_tracks() == 2 );

		CHECK ( cue->filename(1) == "john_doe_album.wav" );
		CHECK ( cue->filename(2) == "john_doe_album.wav" );

		CHECK ( cue->offset(1) == 150 );
		CHECK ( cue->offset(2) == 25072 );

		CHECK ( cue->parsed_length(1) == 24922 );
		CHECK ( cue->parsed_length(2) == 0 ); // OK since AudioSize is unknown

		CHECK ( cue->leadout() == 0 ); // since last track (2) has unkown length
		CHECK ( !cue->complete() ); // since leadout is 0
	}
}

