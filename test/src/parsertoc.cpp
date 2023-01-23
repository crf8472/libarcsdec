#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * Tests for classes in parsertoc.cpp
 */

#ifndef __LIBARCSDEC_PARSERTOC_HPP__
#include "parsertoc.hpp"
#endif
#ifndef __LIBARCSDEC_PARSERTOC_DETAILS_HPP__
#include "parsertoc_details.hpp"
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
		//CHECK ( libs.front().second.find("libcdio++.so") != std::string::npos );
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

	SECTION ("Matches accepted filenames correctly")
	{
		CHECK ( d.accepts_name("foo.toc") );
		CHECK ( d.accepts_name("bar.TOC") );
		CHECK ( d.accepts_name("bar.TOc") );

		CHECK ( !d.accepts_name("bar.rtoc") );
		CHECK ( !d.accepts_name("bar.PTOc") );

		CHECK ( !d.accepts_name("bar.tocf") );
		CHECK ( !d.accepts_name("bar.TOCl") );
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

	SECTION ("Parses a syntactically intact input correctly")
	{
		using arcsdec::details::cdrdao::TocParserImpl;
		auto parser = TocParserImpl{};
		/*
		const auto toc = parser.parse("test01_ok.toc");

		CHECK ( toc->total_tracks() == 2 );

		CHECK ( toc->filename(1) == "john_doe_album.wav" );
		CHECK ( toc->filename(2) == "john_doe_album.wav" );

		CHECK ( toc->offset(1) == 150 );
		CHECK ( toc->offset(2) == 25072 );

		CHECK ( toc->parsed_length(1) == 24922 );
		CHECK ( toc->parsed_length(2) == 0 ); // OK since AudioSize is unknown

		CHECK ( toc->leadout() == 0 ); // since last track (2) has unkown length
		CHECK ( !toc->complete() ); // since leadout is 0
		*/
	}
}

