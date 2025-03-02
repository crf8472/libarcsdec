#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for parsertoc.hpp.
 */

#ifndef __LIBARCSDEC_PARSERTOC_HPP__
#include "parsertoc.hpp"                // TO BE TESTED
#endif

#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"                // for FileReaderSelection
#endif


TEST_CASE ("DescriptorToc", "[parsertoc]" )
{
	using arcsdec::DescriptorToc;
	using arcsdec::Format;
	using arcsdec::Codec;

	auto d = DescriptorToc{};

	SECTION ("Returns own name correctly")
	{
		CHECK ( "CDRDAO" == d.name() );
	}

	SECTION ("Returns linked libraries correctly")
	{
		const auto libs = d.libraries();

		CHECK ( libs.size() == 1 );
		CHECK ( libs.front().first  == "libcdio" );
		// TODO Test for libcdio++.so
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
		CHECK ( d.accepts(Format::CDRDAO) );
	}

	SECTION ("Does not match any formats not accepted by this descriptor")
	{
		CHECK ( !d.accepts(Format::UNKNOWN) );
		CHECK ( !d.accepts(Format::CUE)  );
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
		CHECK ( d.formats() == std::set<Format>{ Format::CDRDAO } );
	}

	// TODO Does create_reader() really create a TocParserImpl?
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
		CHECK ( nullptr != arcsdec::FileReaderRegistry::reader("cdrdaotoc") );
	}
}

//TEST_CASE ("FormatToc", "[parsertoc]" )
//{
//	auto f = arcsdec::FormatToc{};
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
//		CHECK ( f.filename("foo.toc") );
//		CHECK ( f.filename("bar.TOC") );
//		CHECK ( f.filename("bar.TOc") );
//
//		CHECK ( !f.filename("bar.rtoc") );
//		CHECK ( !f.filename("bar.PTOc") );
//
//		CHECK ( !f.filename("bar.tocf") );
//		CHECK ( !f.filename("bar.TOCl") );
//	}
//}

