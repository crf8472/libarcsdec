#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for parserlibcue.hpp.
 */

#ifndef __LIBARCSDEC_PARSERLIBCUE_HPP__
#include "parserlibcue.hpp"             // TO BE TESTED
#endif

#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"                // for FileReaderSelection
#endif


TEST_CASE ("DescriptorCue", "[parserlibcue]" )
{
	using arcsdec::DescriptorCue;
	using arcsdec::Format;
	using arcsdec::Codec;

	auto d = arcsdec::DescriptorCue{};

	SECTION ("Returns own name correctly")
	{
		CHECK ( "Libcue" == d.name() );
	}

	SECTION ("Returns linked libraries correctly")
	{
		const auto libs = d.libraries();

		CHECK ( libs.size() == 1 );

		auto l = libs.begin();

		CHECK ( l->first  == "libcue" );
		CHECK ( l->second.find("libcue") != std::string::npos );
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

	// TODO Does create_reader() really create a CueParserImpl?
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
		CHECK ( nullptr != arcsdec::FileReaderRegistry::reader("libcue") );
	}
}


//TEST_CASE ("FormatCue", "[parserlibcue]" )
//{
//	auto f = arcsdec::FormatCue{};
//
//	SECTION ("Returns own name correctly")
//	{
//		CHECK ( "cue" == f.name() );
//	}
//
//	SECTION ("Matches accepted bytes correctly")
//	{
//		CHECK ( f.bytes({}, 0) );
//		CHECK ( f.bytes({3, 2, 1}, 2) );
//		CHECK ( f.bytes({0x65, 0x32, 0x88}, 1) );
//		// TODO Check for always true
//	}
//
//	SECTION ("Matches accepted filenames correctly")
//	{
//		CHECK ( f.filename("foo.cue") );
//		CHECK ( f.filename("bar.CUE") );
//		CHECK ( f.filename("bar.CUe") );
//
//		CHECK ( !f.filename("bar.rcue") );
//		CHECK ( !f.filename("bar.PCUe") );
//
//		CHECK ( !f.filename("bar.cuef") );
//		CHECK ( !f.filename("bar.CUEl") );
//	}
//}

