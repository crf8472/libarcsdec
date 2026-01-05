#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for readerwvpk_details.hpp.
 */

#ifndef __LIBARCSDEC_READERWVPK_HPP__
#define __LIBARCSDEC_READERWVPK_HPP__   // allow readerwvpk_details.hpp
#endif
#ifndef __LIBARCSDEC_READERWVPK_DETAILS_HPP__
#include "readerwvpk_details.hpp"       // TO BE TESTED
#endif

#ifndef __LIBARCSDEC_READERMOCKS_HPP__
#include "readermocks.hpp"              // for Mock_SampleProcessor
#endif


TEST_CASE ( "WAVPACK_CDDA_t constants are correct", "[readerwvpk]" )
{
	arcsdec::details::wavpack::WAVPACK_CDDA_t w;

	CHECK(  w.lossless()              );
	CHECK(  w.wav_format_only()       );
	CHECK( !w.floats_ok()             );
	CHECK(  w.at_least_version() == 1 );
	CHECK(  w.at_most_version()  == 5 );
}


TEST_CASE ("WavpackAudioReaderImpl", "[readerwvpk]" )
{
	using arcsdec::details::wavpack::WavpackAudioReaderImpl;
	//using arcsdec::DescriptorWavpack;

	auto d = WavpackAudioReaderImpl{}.descriptor();

	// SECTION ("Parser implementation returns correct descriptor type")
	// {
	// 	CHECK ( d );
	// 	auto p = d.get();
	//
	// 	CHECK ( dynamic_cast<const DescriptorWavpack*>(p) != nullptr );
	// }

	SECTION ("Parses a syntactically intact input correctly")
	{
		using arcsdec::details::wavpack::WavpackAudioReaderImpl;

		auto r { WavpackAudioReaderImpl{} };
		auto proc = Mock_SampleProcessor{};
		r.attach_processor(proc);

		r.process_file("test01.wv");
		// TODO What the mock sees in its callbacks has to be tested
	}
}


TEST_CASE ("WavpackOpenFile", "[readerwvpk]" )
{
	using arcsdec::details::wavpack::WavpackOpenFile;

	WavpackOpenFile f {"test01.wv"};

	SECTION ("Provides format specific metadata correctly")
	{
		CHECK ( f.is_lossless() );
		CHECK ( f.has_wav_format() );
		CHECK ( !f.has_float_samples() );
		CHECK ( f.version() == 5 );
	}

	SECTION ("Provides CDDA relevant properties correctly")
	{
		CHECK ( f.bits_per_sample() == 16 );
		CHECK ( f.samples_per_second() == 44100 );
		CHECK ( f.num_channels() == 2 );
	}

	SECTION ("Provides size info correctly")
	{
		CHECK ( f.total_pcm_samples() == 1025 );
	}

	SECTION ("Provides channel order info correctly")
	{
		CHECK ( f.channel_order() );
		CHECK ( f.channel_mask() == 3 ); // stereo
		CHECK ( !f.needs_channel_reorder() );
	}

	SECTION ("Reads PCM samples correctly")
	{
		std::vector<int32_t> buffer(128);
		REQUIRE ( buffer.size() == 128 );

		CHECK ( 64 == f.read_pcm_samples(64, buffer) );
		CHECK ( buffer.size() == 128 );
	}
}


TEST_CASE ("WavpackValidatingHandler", "[readerwvpk]" )
{
	using arcsdec::details::wavpack::WavpackValidatingHandler;
	using arcsdec::details::wavpack::WavpackOpenFile;
	using arcsdec::details::wavpack::WAVPACK_CDDA_t;

	auto valid = std::make_unique<WAVPACK_CDDA_t>();
	auto h = std::make_unique<WavpackValidatingHandler>(std::move(valid));

	WavpackOpenFile file {"test01.wv"};

	SECTION ("Validates for lossless compression correctly")
	{
		CHECK ( h->validate_mode(file) );
	}

	SECTION ("Validates WAV file format correctly")
	{
		CHECK ( h->validate_format(file) );
	}

	SECTION ("Validates Wavpack version correctly")
	{
		CHECK ( h->validate_version(file) );
	}

	SECTION ("Validates CDDA conformity correctly")
	{
		CHECK ( h->validate_cdda(file) );
	}
}

