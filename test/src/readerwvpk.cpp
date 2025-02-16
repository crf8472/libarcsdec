#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for readerwvpk.hpp.
 */

#ifndef __LIBARCSDEC_READERWVPK_HPP__
#include "readerwvpk.hpp"               // TO BE TESTED
#endif
#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"                // for FileReaderSelection
#endif


TEST_CASE ("DescriptorWavpack", "[readerwvpk]" )
{
	using arcsdec::DescriptorWavpack;
	using arcsdec::Format;
	using arcsdec::Codec;

	auto d = DescriptorWavpack {};

	SECTION ("Returns own name correctly")
	{
		CHECK ( "Wavpack" == d.name() );
	}

	SECTION ("Returns linked libraries correctly")
	{
		const auto libs = d.libraries();

		CHECK ( libs.size() == 1 );
		CHECK ( libs.front().first  == "libwavpack" );
		CHECK ( libs.front().second.find("libwavpack") != std::string::npos );
	}

	SECTION ("Matches accepted codecs correctly")
	{
		CHECK ( d.accepts(Codec::WAVPACK) );
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
		CHECK ( !d.accepts(Codec::FLAC) );
		CHECK ( !d.accepts(Codec::MONKEY) );
		CHECK ( !d.accepts(Codec::ALAC) );
	}

	SECTION ("Returns accepted codecs correctly")
	{
		CHECK ( d.codecs() == std::set<Codec>{ Codec::WAVPACK } );
	}

	SECTION ("Returns no codecs that are not accepted")
	{
		CHECK ( d.codecs().size() == 1 );
	}

	SECTION ("Matches accepted formats correctly")
	{
		CHECK ( d.accepts(Format::WV) );
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
		CHECK ( !d.accepts(Format::AIFF)    );
	}

	SECTION ("Returns accepted formats correctly")
	{
		CHECK ( d.formats() == std::set<Format>{ Format::WV } );
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
		CHECK ( nullptr != arcsdec::FileReaderRegistry::reader("wavpack") );
	}

	SECTION ( "Default settings select wavpack for WV/Wavpack" )
	{
		auto reader = default_selection->get(Format::WV, Codec::WAVPACK,
				*default_readers );

		CHECK ( "wavpack" == reader->id() );
	}

	SECTION ( "Default settings select wavpack for WV/UNKNOWN" )
	{
		auto reader = default_selection->get(Format::WV, Codec::UNKNOWN,
				*default_readers );

		CHECK ( "wavpack" == reader->id() );
	}
}


//TEST_CASE ("FormatWavpack", "[readerwvpk]" )
//{
//	auto f = arcsdec::FormatWavpack {};
//
//	SECTION ("Matches accepted bytes correctly")
//	{
//		CHECK ( f.bytes({ 0x77, 0x76, 0x70, 0x6B }, 0) );
//	}
//
//	SECTION ("Matches accepted filenames correctly")
//	{
//		CHECK ( f.filename("foo.wv") );
//		CHECK ( f.filename("bar.WV") );
//
//		CHECK ( !f.filename("bar.WAV") );
//		CHECK ( !f.filename("bar.wav") );
//
//		CHECK ( !f.filename("bar.rwv") );
//		CHECK ( !f.filename("bar.RWV") );
//
//		CHECK ( !f.filename("bar.wvx") );
//		CHECK ( !f.filename("bar.WVX") );
//	}
//}

