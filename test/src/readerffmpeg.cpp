#include "catch2/catch.hpp"

#ifndef __LIBARCSDEC_READERFFMPEG_HPP__
#include "readerffmpeg.hpp"
#endif
#ifndef __LIBARCSDEC_READERFFMPEG_DETAILS_HPP__
#include "readerffmpeg_details.hpp"
#endif


/**
 * \file
 *
 * Tests for classes in readerffmpeg.cpp
 */


TEST_CASE ( "DescriptorFFmpeg", "[readerffmpeg]" )
{
	using arcsdec::DescriptorFFmpeg;

	auto d = DescriptorFFmpeg{};

	SECTION ("Matches names correctly")
	{
		CHECK ( d.accepts_name("foo.everything") );
		CHECK ( d.accepts_name("bar.allesmoegliche") );
		CHECK ( d.accepts_name("bar.anystuff") );

		CHECK ( d.accepts_name("bar.auchdashier") );
		CHECK ( d.accepts_name("bar.alsothis") );

		CHECK ( d.accepts_name("bar.andthis") );
		CHECK ( d.accepts_name("bar.thisinparticular") );
	}
}


//TEST_CASE ( "TypeSize" )
//{
//	using arcsdec::details::ffmpeg::TypeSize;
//
//	CHECK ( TypeSize(::AV_SAMPLE_FMT_S16) == 2 );
//	CHECK ( TypeSize(::AV_SAMPLE_FMT_S32) == 4 );
//}


TEST_CASE ( "PacketQueue", "[packetqueue]" )
{
	using arcsdec::details::ffmpeg::AVFormatContextPtr;
	using arcsdec::details::ffmpeg::AVCodecContextPtr;
	using arcsdec::details::ffmpeg::PacketQueue;

	::AVFormatContext* ff_fctx = nullptr;

	auto error_fopen =
		::avformat_open_input(&ff_fctx, "test01.wav", nullptr, nullptr);

	REQUIRE ( error_fopen == 0 );

	AVFormatContextPtr fctx(ff_fctx);

	::AVCodec* codec = nullptr;
	int stream_idx = ::av_find_best_stream(fctx.get(), ::AVMEDIA_TYPE_AUDIO,
			-1, -1, &codec, 0);

	REQUIRE ( stream_idx >= 0 );
	REQUIRE ( codec );

	::AVStream* stream = fctx->streams[stream_idx];

	REQUIRE ( stream );

	::AVCodecContext* ff_cctx = ::avcodec_alloc_context3(nullptr);

	REQUIRE ( ff_cctx );

	AVCodecContextPtr cctx(ff_cctx);

	auto error_pars = ::avcodec_parameters_to_context(cctx.get(),
			stream->codecpar);

	REQUIRE ( error_pars == 0 );

	auto error_copen = ::avcodec_open2(cctx.get(), codec, nullptr);

	REQUIRE ( error_copen == 0 );

	PacketQueue queue;
	queue.set_source(fctx.get(), stream->index);
	queue.set_decoder(cctx.get());

	REQUIRE ( queue.size() == 0 );


	SECTION ( "enqueue_frame() loop stops correctly" )
	{
		int total_frames = 0;
		while (queue.enqueue_frame())
		{
			++total_frames;
		}

		CHECK ( total_frames == 2 );
		CHECK ( queue.size() == 2 );

		// Further enqueueing leads only false but no exception

		CHECK ( not queue.enqueue_frame() );
	}


	SECTION ( "loop traverses all frames" )
	{
		using arcsdec::details::ffmpeg::AVFramePtr;

		auto total_frames  = int32_t { 0 };
		auto total_samples = int32_t { 0 };
		AVFramePtr frame   = nullptr;

		while (queue.enqueue_frame())
		{
			while((frame = queue.dequeue_frame()))
			{
				total_samples += frame->nb_samples;
				++total_frames;
			}
		}

		CHECK ( total_frames  == 2 );
		CHECK ( queue.size()  == 0 );

		CHECK ( total_samples == 1025 );
	}
}

