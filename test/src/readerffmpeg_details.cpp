#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for readerffmpeg_details.hpp.
 */

#ifndef __LIBARCSDEC_READERFFMPEG_HPP__
#define __LIBARCSDEC_READERFFMPEG_HPP__ // allow readerffmpeg_details.hpp
#endif
#ifndef __LIBARCSDEC_READERFFMPEG_DETAILS_HPP__
#include "readerffmpeg_details.hpp"     // TO BE TESTED
#endif


TEST_CASE ( "FrameQueue", "[framequeue]" )
{
	using arcsdec::details::ffmpeg::AVFormatContextPtr;
	using arcsdec::details::ffmpeg::AVCodecContextPtr;
	using arcsdec::details::ffmpeg::FrameQueue;
	using arcsdec::details::ffmpeg::av_err2str;

#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(58, 9, 100) //  < ffmpeg 4.0
	::av_register_all();
#endif

	::AVFormatContext* ff_fctx = nullptr;

	auto error_open_input =
		::avformat_open_input(&ff_fctx, "test01.wav", nullptr, nullptr);

	if ( error_open_input != 0 )
	{
		throw std::runtime_error("av_open_input: " +
				std::string(av_err2str(error_open_input)));
	}
	REQUIRE ( error_open_input == 0 );

	AVFormatContextPtr fctx(ff_fctx);

#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(59, 16, 100) //  < ffmpeg 5.0
	::AVCodec* codec = nullptr;
#else
	const ::AVCodec* codec = nullptr;
#endif

	int stream_idx = ::av_find_best_stream(fctx.get(), ::AVMEDIA_TYPE_AUDIO,
			-1, -1, &codec, 0);

	if ( stream_idx < 0 )
	{
		throw std::runtime_error("av_find_best_stream: " +
				std::string(av_err2str(stream_idx)));
	}
	REQUIRE ( stream_idx >= 0 );
	if ( !codec )
	{
		throw std::runtime_error("av_find_best_stream did not yield a codec");
	}
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

	if ( error_copen != 0 )
	{
		throw std::runtime_error("avcodec_open2: " +
				std::string(av_err2str(error_copen)));
	}
	REQUIRE ( error_copen == 0 );

	FrameQueue queue;
	queue.set_source(fctx.get(), stream->index);
	queue.set_decoder(cctx.get());

	REQUIRE ( queue.size() == 0 );


	SECTION ( "enqueue_frame() loop enqueues all frames" )
	{
		auto total_frames = int { 0 };
		while (queue.enqueue_frame())
		{
			++total_frames;
		}

		CHECK ( total_frames > 0 );
		CHECK ( queue.size() > 0 );

		// Further enqueueing yields only false but no exception

		using arcsdec::details::ffmpeg::AVFramePtr;
		auto frame = AVFramePtr { nullptr };
		auto total_samples = int32_t { 0 };

		while((frame = queue.dequeue_frame()))
		{
			total_samples += frame->nb_samples;
		}

		CHECK ( queue.size()  == 0 );

		// Did we see all samples? Then we saw all frames
		CHECK ( total_samples == 1025 );
	}


	SECTION ( "enqueue_frame()/dequeue_frame() loop traverses all samples" )
	{
		// TODO This is basically the same test as in the SECTION above!

		using arcsdec::details::ffmpeg::AVFramePtr;
		auto frame = AVFramePtr { nullptr };
		auto total_samples = int32_t { 0 };

		while (queue.enqueue_frame())
		{
			CHECK ( queue.size() > 0 );

			while((frame = queue.dequeue_frame()))
			{
				total_samples += frame->nb_samples;
			}
		}

		CHECK ( queue.size()  == 0 );

		CHECK ( total_samples == 1025 );
		// TODO By accidentally counting 1 frame n-times?
	}
}

