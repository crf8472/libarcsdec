#include "catch2/catch_test_macros.hpp"

#ifndef __LIBARCSDEC_PARSERTOC_HPP__
#include "parsertoc.hpp"
#endif
#ifndef __LIBARCSDEC_PARSERTOC_DETAILS_HPP__
#include "parsertoc_details.hpp"
#endif
#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"
#endif

/**
 * \file
 *
 * Tests for classes in parsertoc.cpp
 */


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


TEST_CASE ("TocParserImpl", "[parsertoc]" )
{
	using arcsdec::details::cdrdao::TocParserImpl;
	using arcsdec::DescriptorToc;

	auto d = TocParserImpl{}.descriptor();

	SECTION ("Parser implementation returns correct descriptor type")
	{
		CHECK ( d );
		auto p = d.get();

		CHECK ( dynamic_cast<const DescriptorToc*>(p) != nullptr );
	}
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

