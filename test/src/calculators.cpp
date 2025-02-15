#include "catch2/catch_test_macros.hpp"

#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"
#endif
#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include "metaparser.hpp"
#endif
#ifndef __LIBARCSDEC_CALCULATORS_HPP__
#include "calculators.hpp"
#endif


/**
 * \file
 *
 * Tests for all API classes exported by calculators.hpp
 */


using arcsdec::ReaderAndFormatHolder;

// ReaderAndFormatHolder is abstract by its destructor
class TestHolder : public ReaderAndFormatHolder
{
	// empty
};


TEST_CASE ( "ReaderAndFormatHolder", "[readerandformatholder]")
{
	using arcsdec::FileReaderRegistry;

	TestHolder h;

	SECTION ( "Create Holder for Readers and Formats with defaults" )
	{
		CHECK ( h.formats() == FileReaderRegistry::formats() );
		CHECK ( h.readers() == FileReaderRegistry::readers() );
	}

	SECTION ( "set_formats() works as expected" )
	{
		CHECK ( h.formats() == FileReaderRegistry::formats() );

		h.set_formats(nullptr);

		CHECK ( h.formats() == nullptr );
	}

	SECTION ( "set_readers() works as expected" )
	{
		CHECK ( h.readers() == FileReaderRegistry::readers() );

		h.set_readers(nullptr);

		CHECK ( h.readers() == nullptr );
	}
}


TEST_CASE ( "SelectionPerformer", "[selectionperformer]")
{
	using arcsdec::SelectionPerformer;

	using arcsdec::MetadataParser;
	using arcsdec::AudioReader;

	TestHolder h;
	SelectionPerformer<MetadataParser> p;
	SelectionPerformer<AudioReader> a;

	SECTION ( "Create reader for CueSheet correctly" )
	{
		auto reader = p.file_reader("cuesheet/ok01.cue", &h);

		CHECK ( reader != nullptr );
	}

	SECTION ( "Create reader for RIFFWAV/PCM correctly" )
	{
		auto reader = a.file_reader("test01.wav", &h);

		CHECK ( reader != nullptr );
	}
}


TEST_CASE ( "AudioInfo", "[calculators]")
{
	using arcsdec::AudioInfo;
	using arcsdec::FileReaderRegistry;

	AudioInfo i;

	SECTION ("Initial set of FileReaders is present and complete")
	{
		CHECK ( i.readers() == FileReaderRegistry::readers() );
		CHECK ( not i.readers()->empty() );
		CHECK ( 5 <= i.readers()->size() ); // cue, wavpcm, ffmpeg, flac, wvpk
		CHECK ( 8 >= i.readers()->size() ); // + toc, libcue, sndfile
	}

	SECTION( "Get size of wav file correctly" )
	{
		const auto leadout { i.size("test01.wav")->samples() };

		CHECK ( leadout == 1025 );
	}
}


TEST_CASE ( "ToCParser", "[calculators]" )
{
	using arcsdec::ToCParser;
	using arcsdec::FileReaderRegistry;

	ToCParser p;

	SECTION ("Initial set of FileReaders is present and complete")
	{
		CHECK ( p.readers() == FileReaderRegistry::readers() );
		CHECK ( not p.readers()->empty() );
		CHECK ( 5 <= p.readers()->size() ); // cue, wavpcm, ffmpeg, flac, wvpk
		CHECK ( 8 >= p.readers()->size() ); // + toc, libcue, sndfile
	}

	SECTION( "Parse CueSheet file correctly" )
	{
		const auto toc { p.parse("cuesheet/ok01.cue") };

		CHECK ( toc->total_tracks() == 2 );
		CHECK ( toc->offsets().at(0).frames() ==   150 );
		CHECK ( toc->offsets().at(1).frames() == 25072 );
	}
}


TEST_CASE ( "ARCSCalculator", "[calculators]" )
{
	using arcsdec::ARCSCalculator;

	ARCSCalculator c;

	SECTION ("Initial DescriptorSet is present and complete")
	{
		CHECK ( 8 >= c.readers()->size() );
		CHECK ( not c.readers()->empty() );
	}

	SECTION( "Read wav file correctly" )
	{
		const auto checksums = c.calculate("test01.wav", true, true);

		CHECK ( checksums.empty() );
	}

	// TODO Check whether flac is compiled in before testing
	//
	//SECTION( "Read flac file correctly" )
	//{
	//	const auto checksums = c.calculate("test01.flac", true, true);
	//
	//	CHECK ( checksums.empty() );
	//}
}


TEST_CASE ( "ARIdCalculator", "[calculators]" )
{
	using arcsdec::ARIdCalculator;
	using arcsdec::FileReaderRegistry;

	ARIdCalculator c;

	SECTION ("Initial set of FileReaders is present and complete")
	{
		CHECK ( c.readers() == FileReaderRegistry::readers() );
		CHECK ( not c.readers()->empty() );
		CHECK ( 5 <= c.readers()->size() ); // cue, wavpcm, ffmpeg, flac, wvpk
		CHECK ( 8 >= c.readers()->size() ); // + toc, libcue, sndfile
	}

	// TODO Provide test files with realistic results
	//
	//SECTION( "Get ARId from cue+wav file correctly" )
	//{
	//	const auto id = c.calculate("test01.wav", "test01_ok.cue");
	//
	//	CHECK ( id->to_string() == "foo" );
	//}
}

