#include "catch2/catch_test_macros.hpp"

#ifndef __LIBARCSDEC_READERFLAC_HPP__
#include "readerflac.hpp"
#endif
#ifndef __LIBARCSDEC_READERFLAC_DETAILS_HPP__
#include "readerflac_details.hpp"
#endif
#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"
#endif

#ifndef __LIBARCSDEC_READERMOCKS_HPP__
#include "readermocks.hpp"
#endif

#include <set>  // for set


/**
 * \file
 *
 * Tests for classes in readerflac.cpp
 */


TEST_CASE ("DescriptorFlac", "[readerflac]" )
{
	using arcsdec::DescriptorFlac;
	using arcsdec::Format;
	using arcsdec::Codec;

	auto d = DescriptorFlac {};

	SECTION ("Returns own name correctly")
	{
		CHECK ( "Flac" == d.name() );
	}

	SECTION ("Returns linked libraries correctly")
	{
		const auto libs = d.libraries();

		CHECK ( libs.size() == 2 );

		auto l = libs.begin();

		CHECK ( l->first  == "libFLAC++" );
		//CHECK ( l->second.find("libFLAC") != std::string::npos );

		++l;

		CHECK ( l->first  == "libFLAC" );
		CHECK ( l->second.find("libFLAC") != std::string::npos );
	}

	SECTION ("Matches accepted codecs correctly")
	{
		CHECK ( d.accepts(Codec::FLAC) );
	}

	SECTION ("Does not match codecs not accepted by this descriptor")
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
		CHECK ( !d.accepts(Codec::WAVPACK) );
		CHECK ( !d.accepts(Codec::MONKEY) );
		CHECK ( !d.accepts(Codec::ALAC) );
	}

	SECTION ("Returns accepted codecs correctly")
	{
		CHECK ( d.codecs() == std::set<Codec>{ Codec::FLAC } );
	}

	SECTION ("Returns no codecs that are not accepted")
	{
		CHECK ( d.codecs().size() == 1 );
	}

	SECTION ("Matches accepted formats correctly")
	{
		CHECK ( d.accepts(Format::FLAC) );
	}

	SECTION ("Does not match any formats not accepted by this descriptor")
	{
		CHECK ( !d.accepts(Format::UNKNOWN) );
		CHECK ( !d.accepts(Format::CDRDAO)  );
		CHECK ( !d.accepts(Format::WAV)     );
		CHECK ( !d.accepts(Format::WV)      );
		CHECK ( !d.accepts(Format::APE)     );
		CHECK ( !d.accepts(Format::CAF)     );
		CHECK ( !d.accepts(Format::M4A)     );
		CHECK ( !d.accepts(Format::OGG)     );
		CHECK ( !d.accepts(Format::AIFF)    );
	}

	SECTION ("Returns accepted formats correctly")
	{
		CHECK ( d.formats() == std::set<Format>{ Format::FLAC } );
	}
}


TEST_CASE ("FlacMetadataHandler", "[readerflac]" )
{
	using arcsdec::details::flac::FlacMetadataHandler;
	using arcsdec::Codec;

	FlacMetadataHandler h;

	SECTION ("Accepted set of codecs is only FLAC")
	{
		CHECK ( h.codecs() == std::set<Codec>{ Codec::FLAC } );
	}
}


TEST_CASE ("FlacAudioReaderImpl", "[readerflac]" )
{
	using arcsdec::details::flac::FlacAudioReaderImpl;
	using arcsdec::DescriptorFlac;

	FlacAudioReaderImpl r;
	auto proc = SampleProcessorMock{};
	r.attach_processor(proc);

	auto d = r.descriptor();

	SECTION ("Parser implementation returns correct descriptor type")
	{
		CHECK ( d );
		auto p = d.get();

		CHECK ( dynamic_cast<const DescriptorFlac*>(p) != nullptr );
	}

	SECTION ("Parses a syntactically intact input correctly")
	{
		r.process_file("test01.flac");
		// TODO What the mock sees in its callbacks has to be tested
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
		CHECK ( nullptr != arcsdec::FileReaderRegistry::reader("flac") );
	}

	SECTION ( "Default settings select flac for FLAC/FLAC" )
	{
		auto reader = default_selection->get(Format::FLAC, Codec::FLAC,
				*default_readers );

		CHECK ( "flac" == reader->id() );
	}

	SECTION ( "Default settings select flac for FLAC/UNKNOWN" )
	{
		auto reader = default_selection->get(Format::FLAC, Codec::UNKNOWN,
				*default_readers );

		CHECK ( "flac" == reader->id() );
	}
}

