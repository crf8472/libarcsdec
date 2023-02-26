#include "catch2/catch_test_macros.hpp"

#ifndef __LIBARCSDEC_READERWAV_HPP__
#include "readerwvpk.hpp"
#endif
#ifndef __LIBARCSDEC_READERWAV_DETAILS_HPP__
#include "readerwvpk_details.hpp"
#endif

#ifndef __LIBARCSDEC_READERMOCKS_HPP__
#include "readermocks.hpp"
#endif

/**
 * \file
 *
 * Tests for classes in readerwvpk.cpp
 */


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
	using arcsdec::DescriptorWavpack;

	auto d = WavpackAudioReaderImpl{}.descriptor();

	SECTION ("Parser implementation returns correct descriptor type")
	{
		CHECK ( d );
		auto p = d.get();

		CHECK ( dynamic_cast<const DescriptorWavpack*>(p) != nullptr );
	}

	SECTION ("Parses a syntactically intact input correctly")
	{
		using arcsdec::details::wavpack::WavpackAudioReaderImpl;

		auto r { WavpackAudioReaderImpl{} };
		auto proc = SampleProcessorMock{};
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

