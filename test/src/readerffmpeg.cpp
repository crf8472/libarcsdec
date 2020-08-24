#include "catch2/catch.hpp"

#ifndef __LIBARCSDEC_READERFFMPEG_HPP__
#include "readerffmpeg.hpp"
#endif
#ifndef __LIBARCSDEC_READERFFMPEG_DETAILS_HPP__
#include "readerffmpeg_details.hpp"
#endif

#include <iostream>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/version.h>
#include <libavutil/avutil.h>
}


/**
 * \file
 *
 * Tests for all API classes exported by readerfffmpeg.hpp
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


TEST_CASE ( "PacketQueue", "[packetqueue]" )
{
	::AVFormatContext* ff_fctx = nullptr;

	auto error_fopen =
		::avformat_open_input(&ff_fctx, "test01.wav", nullptr, nullptr);

	REQUIRE ( error_fopen == 0 );

	arcsdec::AVFormatContextPtr fctx(ff_fctx);

	::AVCodec* codec = nullptr;
	int stream_idx = ::av_find_best_stream(fctx.get(), ::AVMEDIA_TYPE_AUDIO,
			-1, -1, &codec, 0);

	REQUIRE ( stream_idx >= 0 );
	REQUIRE ( codec );

	::AVStream* stream = fctx->streams[stream_idx];

	REQUIRE ( stream );

	::AVCodecContext* ff_cctx = ::avcodec_alloc_context3(nullptr);

	REQUIRE ( ff_cctx );

	arcsdec::AVCodecContextPtr cctx(ff_cctx);

	auto error_pars = ::avcodec_parameters_to_context(cctx.get(),
			stream->codecpar);

	REQUIRE ( error_pars == 0 );

	auto error_copen = ::avcodec_open2(cctx.get(), codec, nullptr);

	REQUIRE ( error_copen == 0 );


	using arcsdec::PacketQueue;

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
		using arcsdec::AVFramePtr;

		auto total_frames  = int32_t { 0 };
		auto total_samples = int32_t { 0 };
		AVFramePtr frame   = nullptr;

		while (queue.enqueue_frame())
		{
			while((frame = queue.dequeue_frame()))
			{
				total_samples += frame->nb_samples * 2;

				std::cout << "+++ Frame counted, size: " << frame->nb_samples
					<< std::endl;

				++total_frames;
			}

			std::cout << "Frame is null" << std::endl;
		}
		total_samples /= 2;

		CHECK ( total_frames  == 2 );
		CHECK ( queue.size()  == 0 );

		CHECK ( total_samples == 1025 );
	}
}

