#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for parserlibcue_details.hpp.
 */

#ifndef __LIBARCSDEC_PARSERLIBCUE_HPP__
#define __LIBARCSDEC_PARSERLIBCUE_HPP__ // allow parserlibcue_details.hpp
#endif
#ifndef __LIBARCSDEC_PARSERLIBCUE_DETAILS_HPP__
#include "parserlibcue_details.hpp"     // TO BE TESTED
#endif


TEST_CASE ("CueParserImpl", "[parserlibcue]" )
{
	using arcsdec::details::libcue::CueParserImpl;
	//using arcsdec::DescriptorCue;

	auto d = CueParserImpl{}.descriptor();

	SECTION ("ok01.cue: Parses a syntactically intact input correctly")
	{
		using arcstk::AudioSize;
		using arcstk::UNIT;

		using arcsdec::details::libcue::CueParserImpl;
		auto parser = CueParserImpl{};
		const auto cue = parser.parse("cuesheet/ok01.cue");
		// This Cuesheet is complete and syntactically correct
		const auto filenames { cue->filenames() };
		const auto offsets   { cue->offsets() };

		CHECK ( cue->total_tracks() == 2 );

		CHECK ( filenames[0] == "john_doe_album.wav" );
		CHECK ( filenames[1] == "john_doe_album.wav" );

		CHECK ( offsets[0] == AudioSize { 150,   UNIT::FRAMES } );
		CHECK ( offsets[1] == AudioSize { 25072, UNIT::FRAMES } );

		CHECK ( cue->leadout().zero() ); // since last track (2) has unkown length
		CHECK ( !cue->complete() ); // since leadout is 0
	}

	SECTION ("ok02.cue: Parses a syntactically intact input correctly")
	{
		using arcstk::AudioSize;
		using arcstk::UNIT;

		using arcsdec::details::libcue::CueParserImpl;
		auto parser = CueParserImpl{};
		const auto cue = parser.parse("cuesheet/ok02.cue");
		// This Cuesheet is complete and syntactically correct
		const auto filenames { cue->filenames() };
		const auto offsets   { cue->offsets() };

		CHECK ( cue->total_tracks() == 2 );

		CHECK ( filenames[0] == "john_doe_album.wav" );
		CHECK ( filenames[1] == "john_doe_album.wav" );

		CHECK ( offsets[0] == AudioSize { 150,   UNIT::FRAMES } );
		CHECK ( offsets[1] == AudioSize { 25072, UNIT::FRAMES } );

		CHECK ( cue->leadout().zero() ); // since last track (2) has unkown length
		CHECK ( !cue->complete() ); // since leadout is 0
	}
}

