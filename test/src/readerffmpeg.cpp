#include "catch2/catch_test_macros.hpp"

#include <stdexcept>
#include <string>

#ifndef __LIBARCSDEC_READERFFMPEG_HPP__
#include "readerffmpeg.hpp"
#endif
#ifndef __LIBARCSDEC_READERFFMPEG_DETAILS_HPP__
#include "readerffmpeg_details.hpp"
#endif
#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"
#endif


/**
 * \file
 *
 * Tests for classes in readerffmpeg.cpp
 */


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
		int total_frames = 0;
		while (queue.enqueue_frame())
		{
			++total_frames;
		}

		CHECK ( total_frames == 2 );
		CHECK ( queue.size() == 2 );

		// Further enqueueing yields only false but no exception

		CHECK ( not queue.enqueue_frame() );
	}


	SECTION ( "enqueue_frame()/dequeue_frame() loop traverses all samples" )
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
		CHECK ( total_samples == 1025 );
		CHECK ( queue.size()  == 0 );

		// Further enqueueing yields only false but no exception

		CHECK ( not queue.enqueue_frame() );
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
		CHECK ( nullptr != arcsdec::FileReaderRegistry::reader("ffmpeg") );
	}


	SECTION ( "Default settings select ffmpeg for OGG/FLAC" )
	{
		auto reader = default_selection->get(Format::OGG, Codec::FLAC,
				*default_readers );

		CHECK ( "ffmpeg" == reader->id() );
	}

	SECTION ( "Default settings select ffmpeg for OGG/UNKNOWN" )
	{
		auto reader = default_selection->get(Format::OGG, Codec::UNKNOWN,
				*default_readers );

		CHECK ( "ffmpeg" == reader->id() );
	}


	SECTION ( "Default settings select ffmpeg for CAF/ALAC" )
	{
		auto reader = default_selection->get(Format::CAF, Codec::ALAC,
				*default_readers );

		CHECK ( "ffmpeg" == reader->id() );
	}

	SECTION ( "Default settings select ffmpeg for CAF/UNKNOWN" )
	{
		auto reader = default_selection->get(Format::CAF, Codec::UNKNOWN,
				*default_readers );

		CHECK ( "ffmpeg" == reader->id() );
	}


	SECTION ( "Default settings select ffmpeg for M4A/ALAC" )
	{
		auto reader = default_selection->get(Format::M4A, Codec::ALAC,
				*default_readers );

		CHECK ( "ffmpeg" == reader->id() );
	}

	SECTION ( "Default settings select ffmpeg for M4A/UNKNOWN" )
	{
		auto reader = default_selection->get(Format::M4A, Codec::UNKNOWN,
				*default_readers );

		CHECK ( "ffmpeg" == reader->id() );
	}


	SECTION ( "Default settings select ffmpeg for APE/MONKEY" )
	{
		auto reader = default_selection->get(Format::APE, Codec::MONKEY,
				*default_readers );

		CHECK ( "ffmpeg" == reader->id() );
	}

	SECTION ( "Default settings select ffmpeg for APE/UNKNOWN" )
	{
		auto reader = default_selection->get(Format::APE, Codec::UNKNOWN,
				*default_readers );

		CHECK ( "ffmpeg" == reader->id() );
	}
}

