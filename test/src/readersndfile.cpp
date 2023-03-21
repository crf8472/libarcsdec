#include "catch2/catch_test_macros.hpp"

#ifndef __LIBARCSDEC_READERSNDFILE_HPP__
#include "readersndfile.hpp"
#endif
#ifndef __LIBARCSDEC_READERSNDFILE_DETAILS_HPP__
#include "readersndfile_details.hpp"
#endif
#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"
#endif

/**
 * \file
 *
 * Tests for classes in readersndfile.cpp
 */


TEST_CASE ("DescriptorSndfile", "[readersndfile]" )
{
	using arcsdec::DescriptorSndfile;
	using arcsdec::Format;
	using arcsdec::Codec;

	auto d = DescriptorSndfile {};

	SECTION ("Returns own name correctly")
	{
		CHECK ( "Libsndfile" == d.name() );
	}

	SECTION ("Returns linked libraries correctly")
	{
		const auto libs = d.libraries();

		CHECK ( libs.size() == 1 );
		CHECK ( libs.front().first  == "libsndfile" );
		CHECK ( libs.front().second.find("libsndfile") != std::string::npos );
	}

	SECTION ("Matches accepted codecs correctly")
	{
		CHECK ( d.accepts(Codec::PCM_S16BE) );
		CHECK ( d.accepts(Codec::PCM_S16BE_PLANAR) );
		CHECK ( d.accepts(Codec::PCM_S16LE) );
		CHECK ( d.accepts(Codec::PCM_S16LE_PLANAR) );
		CHECK ( d.accepts(Codec::PCM_S32BE) );
		CHECK ( d.accepts(Codec::PCM_S32BE_PLANAR) );
		CHECK ( d.accepts(Codec::PCM_S32LE) );
		CHECK ( d.accepts(Codec::PCM_S32LE_PLANAR) );
		CHECK ( d.accepts(Codec::FLAC) );
		CHECK ( d.accepts(Codec::ALAC) );
	}

	SECTION ("Does not match codecs not accepted by this descriptor")
	{
		CHECK ( !d.accepts(Codec::UNKNOWN) );
		CHECK ( !d.accepts(Codec::WAVPACK) );
		CHECK ( !d.accepts(Codec::MONKEY)  );
	}

	SECTION ("Returns accepted codecs correctly")
	{
		CHECK ( d.codecs() == std::set<Codec>{
			Codec::PCM_S16BE,
			Codec::PCM_S16BE_PLANAR,
			Codec::PCM_S16LE,
			Codec::PCM_S16LE_PLANAR,
			Codec::PCM_S32BE,
			Codec::PCM_S32BE_PLANAR,
			Codec::PCM_S32LE,
			Codec::PCM_S32LE_PLANAR,
			Codec::FLAC,
			Codec::ALAC
		} );
	}

	SECTION ("Returns no codecs that are not accepted")
	{
		CHECK ( d.codecs().size() == 10 );
	}

	SECTION ("Matches accepted formats correctly")
	{
		CHECK ( d.accepts(Format::WAV)  );
		CHECK ( d.accepts(Format::FLAC) );
		CHECK ( d.accepts(Format::AIFF) );
		//CHECK ( d.accepts(Format::OGG)  );
		//CHECK ( d.accepts(Format::CAF)  ); // TODO Activate when ready
	}

	SECTION ("Does not match any formats not accepted by this descriptor")
	{
		CHECK ( !d.accepts(Format::UNKNOWN) );
		CHECK ( !d.accepts(Format::CUE)     );
		CHECK ( !d.accepts(Format::CDRDAO)  );
		CHECK ( !d.accepts(Format::APE)     );
		CHECK ( !d.accepts(Format::M4A)     );
	}

	SECTION ("Returns accepted formats correctly")
	{
		CHECK ( d.formats() == std::set<Format>{
			Format::WAV,
			Format::FLAC,
			Format::AIFF
			//Format::OGG
			//Format::CAF // TODO Activate when ready
		} );
	}

	// TODO Implement test for filename matching
	//SECTION ("Matches accepted filenames correctly")
	//{
	//}
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
		CHECK ( nullptr != arcsdec::FileReaderRegistry::reader("libsndfile") );
	}

	SECTION ( "Default settings select libsndfile for AIFF/PCM_S16LE" )
	{
		auto reader = default_selection->get(Format::AIFF, Codec::PCM_S16LE,
				*default_readers );

		CHECK ( "libsndfile" == reader->id() );
	}

	SECTION ( "Default settings select libsndfile for AIFF/UNKNOWN" )
	{
		auto reader = default_selection->get(Format::AIFF, Codec::UNKNOWN,
				*default_readers );

		CHECK ( "libsndfile" == reader->id() );
	}
}

