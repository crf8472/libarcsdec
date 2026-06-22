#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for calculators.hpp.
 */

#ifndef LIBARCSDEC_CALCULATORS_HPP_
#include "calculators.hpp"              // TO BE TESTED
#endif

#ifndef LIBARCSDEC_AUDIOREADER_HPP_
#include "audioreader.hpp"              // for AudioReader
#endif
#ifndef LIBARCSDEC_METAPARSER_HPP_
#include "metaparser.hpp"               // for MetadataParser
#endif
#ifndef LIBARCSDEC_SELECTION_HPP_
#include "selection.hpp"                // for FileReaderRegistry
#endif


using arcsdec::select::ReaderAndFormatHolder;

/**
 * \brief Mock for ReaderAndFormatHolder.
 *
 * ReaderAndFormatHolder is an abstract class by its destructor. The mock is
 * intended to add no functionality but just access the functions in its base
 * class.
 */
class Mock_ReaderAndFormatHolder : public ReaderAndFormatHolder
{
	// empty
};


TEST_CASE ( "SelectionPerformer", "[selectionperformer]")
{
	using arcsdec::read::AudioReader;
	using arcsdec::read::MetadataParser;
	using arcsdec::select::SelectionPerformer;

	const auto h = Mock_ReaderAndFormatHolder{};
	const auto p = SelectionPerformer<MetadataParser>{};
	const auto a = SelectionPerformer<AudioReader>{};

	SECTION ( "Create reader for CueSheet correctly" )
	{
		auto reader = p.file_reader("data/ok01.cue", &h);

		CHECK ( reader != nullptr );
	}

	SECTION ( "Create reader for RIFFWAV/PCM correctly" )
	{
		auto reader = a.file_reader("data/test01.wav", &h);

		CHECK ( reader != nullptr );
	}
}


TEST_CASE ( "AudioInfo", "[calculators]")
{
	using arcsdec::calc::AudioInfo;
	using arcsdec::select::FileReaderRegistry;

	const auto i = AudioInfo{};

	// SECTION ("Initial set of FileReaders is present and complete")
	// {
	// 	CHECK ( i.readers() == FileReaderRegistry::readers() );
	// 	CHECK ( not i.readers()->empty() );
	// 	CHECK ( 5 <= i.readers()->size() ); // cue, wavpcm, ffmpeg, flac, wvpk
	// 	CHECK ( 8 >= i.readers()->size() ); // + toc, libcue, sndfile
	// }

	SECTION( "Get size of wav file correctly" )
	{
		const auto size { i.size("data/test01.wav") };

		CHECK ( size.bytes()   == 4100 );
		CHECK ( size.samples() == 1025 ); // == 4100 bytes / 4 bytes/sample
		CHECK ( size.frames()  == 1 );    // == 1025 samples / 588 samples/frame
	}
}


TEST_CASE ( "ToCParser", "[calculators]" )
{
	using arcsdec::select::FileReaderRegistry;
	using arcsdec::calc::ToCParser;

	const auto p = ToCParser{};

	// SECTION ("Initial set of FileReaders is present and complete")
	// {
	// 	CHECK ( p.readers() == FileReaderRegistry::readers() );
	// 	CHECK ( not p.readers()->empty() );
	// 	CHECK ( 5 <= p.readers()->size() ); // cue, wavpcm, ffmpeg, flac, wvpk
	// 	CHECK ( 8 >= p.readers()->size() ); // + toc, libcue, sndfile
	// }

	SECTION( "Parse CueSheet file correctly" )
	{
		const auto toc { p.parse("data/ok01.cue") };

		CHECK ( toc.total_tracks() == 2 );
		CHECK ( toc.offsets().at(0).frames() ==   150 );
		CHECK ( toc.offsets().at(1).frames() == 25072 );
	}
}

