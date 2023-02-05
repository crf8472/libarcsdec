#include "catch2/catch_test_macros.hpp"

#ifndef __LIBARCSDEC_READERSNDFILE_HPP__
#include "readersndfile.hpp"
#endif
#ifndef __LIBARCSDEC_READERSNDFILE_DETAILS_HPP__
#include "readersndfile_details.hpp"
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

	SECTION ("Matches accepted bytes correctly")
	{
		//CHECK ( d.accepts_bytes({ 0x77, 0x76, 0x70, 0x6B }, 0) );
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
		CHECK ( d.accepts(Format::CAF)  );
	}

	SECTION ("Does not match any formats not accepted by this descriptor")
	{
		CHECK ( !d.accepts(Format::UNKNOWN) );
		CHECK ( !d.accepts(Format::CUE)     );
		CHECK ( !d.accepts(Format::CDRDAO)  );
		CHECK ( !d.accepts(Format::APE)     );
		CHECK ( !d.accepts(Format::M4A)     );
		CHECK ( !d.accepts(Format::OGG)     );
	}

	SECTION ("Returns accepted formats correctly")
	{
		CHECK ( d.formats() == std::set<Format>{
			Format::WAV,
			Format::FLAC,
			Format::AIFF,
			Format::CAF
		} );
	}

	//SECTION ("Matches accepted filenames correctly")
	//{
	//}
}

