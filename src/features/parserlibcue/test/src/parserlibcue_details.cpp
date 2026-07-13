#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Testcases for parserlibcue_details.hpp.
 */

#ifndef LIBARCSDEC_PARSERLIBCUE_HPP_
#define LIBARCSDEC_PARSERLIBCUE_HPP_    // allow parserlibcue_details.hpp
#endif
#ifndef LIBARCSDEC_PARSERLIBCUE_DETAILS_HPP_
#include "parserlibcue_details.hpp"     // TO BE TESTED
#endif


TEST_CASE ("LibcueParserImpl", "[parserlibcue]" )
{
	using arcsdec::read::details::libcue::LibcueParserImpl;

	SECTION ("file_content loads file")
	{
		using arcsdec::read::details::libcue::file_content;

		auto cue_toc_05 = file_content("data/error05.cue", 534);

		CHECK ( cue_toc_05 );

		auto v = cue_toc_05.value();

		CHECK ( v.size() == 535 ); // +1 for \0

		CHECK ( v[0]   == 'C' );
		CHECK ( v[1]   == 'A' );

		CHECK ( v[532] == '2' );
		CHECK ( v[533] == '\n' );
		CHECK ( v[534] == '\0' );
	}

	SECTION ("ok01.cue: Parses a syntactically intact input correctly")
	{
		using arcstk::AudioSize;
		using arcstk::UNIT;

		using arcsdec::read::details::libcue::LibcueParserImpl;
		auto parser = LibcueParserImpl{};
		const auto cue = parser.parse("data/ok01.cue");
		// This Cuesheet is complete and syntactically correct
		const auto filenames { cue.filenames() };
		const auto offsets   { cue.offsets() };

		CHECK ( cue.total_tracks() == 2 );

		CHECK ( filenames[0] == "john_doe_album.wav" );
		CHECK ( filenames[1] == "john_doe_album.wav" );

		CHECK ( offsets[0] == AudioSize { 150,   UNIT::FRAMES } );
		CHECK ( offsets[1] == AudioSize { 25072, UNIT::FRAMES } );

		CHECK ( cue.leadout().zero() ); // since last track (2) has unkown length
		CHECK ( !cue.complete() ); // since leadout is 0
	}

	// XXX This section succeeds only with libcue >= 2.3 and fails otherwise!
	SECTION ("ok02.cue: Parses a syntactically intact input correctly")
	{
		using arcstk::AudioSize;
		using arcstk::UNIT;

		using arcsdec::read::details::libcue::LibcueParserImpl;
		auto parser = LibcueParserImpl{};
		const auto cue = parser.parse("data/ok02.cue");
		// This Cuesheet is complete and syntactically correct.
		// It has no trailing whitespace and no REM statement after the last
		// statement. That causes libcue <= 2.2 to state "syntax error".
		const auto filenames { cue.filenames() };
		const auto offsets   { cue.offsets() };

		CHECK ( cue.total_tracks() == 2 );

		CHECK ( filenames[0] == "john_doe_album.wav" );
		CHECK ( filenames[1] == "john_doe_album.wav" );

		CHECK ( offsets[0] == AudioSize { 150,   UNIT::FRAMES } );
		CHECK ( offsets[1] == AudioSize { 25072, UNIT::FRAMES } );

		CHECK ( cue.leadout().zero() ); // last track (2) has unknown length
		CHECK ( !cue.complete() ); // since leadout is 0
	}

	// XXX This section fails with libcue 2.3 for a memory leak in libcue.
	SECTION ("Parse with syntax error: unrecognized input/tag")
	{
		using arcsdec::read::details::libcue::LibcueParserImpl;
		auto parser = LibcueParserImpl{};

		auto cue_toc_05 = parser.parse("data/error05.cue");

		CHECK ( cue_toc_05.total_tracks() > 0 );
	}

	SECTION ("Parse with syntax error: leading chars before CDTEXTFILE")
	{
		using arcsdec::read::details::libcue::LibcueParserImpl;
		auto parser = LibcueParserImpl{};

		auto cue_toc_04 = parser.parse("data/error04.cue");

		CHECK ( cue_toc_04.total_tracks() > 0 );
	}

	SECTION ("Parse with syntax error: trailing chars after INDEX statement")
	{
		using arcsdec::read::details::libcue::LibcueParserImpl;
		auto parser = LibcueParserImpl{};

		auto cue_toc_03 = parser.parse("data/error03.cue");

		CHECK ( cue_toc_03.total_tracks() > 0 );
	}

	SECTION ("Parse with syntax error: trailing chars after TRACK statement")
	{
		using arcsdec::read::details::libcue::LibcueParserImpl;
		auto parser = LibcueParserImpl{};

		auto cue_toc_02 = parser.parse("data/error02.cue");

		CHECK ( cue_toc_02.total_tracks() > 0 );
	}

	SECTION ("Parse with syntax error: trailing chars after FILE statement")
	{
		using arcsdec::read::details::libcue::LibcueParserImpl;
		auto parser = LibcueParserImpl{};

		auto cue_toc_01 = parser.parse("data/error01.cue");

		CHECK ( cue_toc_01.total_tracks() > 0 );
	}
}

