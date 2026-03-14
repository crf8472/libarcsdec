#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for readerffmpeg_details.hpp.
 */

#include <cstdarg>
#include <string>

#ifndef LIBARCSDEC_READERFFMPEG_HPP__
#define LIBARCSDEC_READERFFMPEG_HPP__   // allow readerffmpeg_details.hpp
#endif
#ifndef LIBARCSDEC_READERFFMPEG_DETAILS_HPP__
#include "readerffmpeg_details.hpp"     // TO BE TESTED
#endif


namespace test
{
std::string format_string_wrapper(const char* fmt,...);

std::string format_string_wrapper(const char* fmt,...)
{
	auto s = std::string {};
	std::va_list args;

	va_start(args, fmt);
	s = arcsdec::read::details::ffmpeg::v_format_string(fmt, args);
    va_end(args);

	return s;
}
} // namespace test


TEST_CASE ( "format_string", "[format_string]" )
{
	using test::format_string_wrapper;

	const auto too_long_string = std::string(259, 'x');


	SECTION ("Works for input less long than default buffer size")
	{
		CHECK ( "TEST123 |= 86" ==
				format_string_wrapper("%s |= %d", "TEST123", 86) );

		CHECK ( "2xfoobar171717687;" ==
				format_string_wrapper("%dx%s%d;", 2, "foobar", 171717687) );

		const auto text = std::string { "foobarquux" };
		const auto ref = format_string_wrapper("%s", text.c_str());

		CHECK ( "foobarquux" == ref );
	}

	SECTION ("Works for input longer than default buffer size")
	{
		// format a string with more than 256 chars

		const auto ref = std::string { "!-" + too_long_string + "-?" };

		/* longer than 256 chars, the default char buffer size of function */
		REQUIRE ( ref.size() > 256 );

		const auto text = format_string_wrapper("!-%s-?",
				too_long_string.c_str());

		CHECK ( text == ref );
	}
}


TEST_CASE ( "FrameQueue", "[framequeue]" )
{
	using arcsdec::read::details::ffmpeg::AVFormatContextPtr;
	using arcsdec::read::details::ffmpeg::AVCodecContextPtr;
	using arcsdec::read::details::ffmpeg::FrameQueue;
	using arcsdec::read::details::ffmpeg::av_err2str;

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

		using arcsdec::read::details::ffmpeg::AVFramePtr;
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

		using arcsdec::read::details::ffmpeg::AVFramePtr;
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

