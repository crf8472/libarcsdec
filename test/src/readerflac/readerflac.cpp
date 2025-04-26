#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for readerflac.hpp.
 */

#ifndef __LIBARCSDEC_READERFLAC_HPP__
#include "readerflac.hpp"               // TO BE TESTED
#endif

#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"                // for FileReaderSelection
#endif


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


TEST_CASE ("FileReaderSelection", "[filereaderselection]")
{
	using arcsdec::FileReaderSelection;
	using arcsdec::FileReaderRegistry;
	using arcsdec::Format;
	using arcsdec::Codec;
	using arcsdec::DescriptorFlac;

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


//TEST_CASE ("FormatFlac", "[readerflac]" )
//{
//	auto f = arcsdec::FormatFlac{};
//
//	SECTION ("Matches accepted bytes correctly")
//	{
//		CHECK ( f.bytes({ 0x66, 0x4C, 0x61, 0x43 }, 0) );
//	}
//
//	SECTION ("Matches names correctly")
//	{
//		CHECK ( f.filename("foo.flac") );
//		CHECK ( f.filename("bar.FLAC") );
//		CHECK ( f.filename("bar.FlAc") );
//
//		CHECK ( !f.filename("bar.rflac") );
//		CHECK ( !f.filename("bar.PFLac") );
//
//		CHECK ( !f.filename("bar.flacr") );
//		CHECK ( !f.filename("bar.FLACD") );
//	}
//}

