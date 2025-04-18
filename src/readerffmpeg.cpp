/**
 * \file
 *
 * \brief Implements FFmpeg-based generic audio reader.
 */

#ifndef __LIBARCSDEC_READERFFMPEG_HPP__
#include "readerffmpeg.hpp"
#endif
#ifndef __LIBARCSDEC_READERFFMPEG_DETAILS_HPP__
#include "readerffmpeg_details.hpp"
#endif

#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"  // for AudioReaderImpl, InvalidAudioException
#endif
#ifndef __LIBARCSDEC_LIBINSPECT_HPP__
#include "libinspect.hpp"   // for first_libname_match
#endif
#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"    // for RegisterDescriptor
#endif

#ifndef __LIBARCSTK_METADATA_HPP__
#include <arcstk/metadata.hpp>  // for AudioSize
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>   // for ARCS_LOG, _ERROR, _WARNING, _INFO, _DEBUG
#endif

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/version.h>   // for LIBAVFORMAT_VERSION_INT
#include <libavutil/avutil.h>
}

#include <algorithm>  // for remove
#include <cerrno>     // for EAGAIN
#include <climits>    // for CHAR_BIT
#include <cstdarg>    // for va_list
#include <cstdlib>    // for size_t, abs
#include <functional> // for function, bind, placeholders
#include <memory>     // for unique_ptr, make_unique
#include <new>        // for bad_alloc
#include <ostream>    // for ostream, endl
#include <set>        // for set
#include <sstream>    // for ostringstream
#include <stdexcept>  // for invalid_argument, runtime_error
#include <string>     // for string
#include <utility>    // for make_pair, move
#include <vector>     // for vector


namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{
namespace ffmpeg
{

using arcstk::AudioSize;

extern "C"
{

void arcs_av_log(void* /*v*/, int lvl, const char* msg, va_list /*l*/)
{
	// Remove newline(s) from message text

	std::string text { msg };

	using std::begin;
	using std::end;

	text.erase(std::remove(begin(text), end(text), '\n'), end(text));

	// Log according to the loglevel
	// (All AV_LOG_* names are macros and have therefore no leading '::'.)

	if (AV_LOG_ERROR == lvl)
	{
		ARCS_LOG_ERROR << "[FFMPEG] " << text;
	} else
	if (AV_LOG_WARNING == lvl)
	{
		ARCS_LOG_WARNING << "[FFMPEG] " << text;
	} else
	if (AV_LOG_INFO == lvl)
	{
		ARCS_LOG_INFO << "[FFMPEG] " << text;
	} else
	if (AV_LOG_DEBUG == lvl)
	{
		// ignore
	} else
	if (AV_LOG_TRACE == lvl)
	{
		// ignore
	} else
	{
		// If level is totally unknown, at least show it when debugging

		ARCS_LOG_DEBUG << "[FFMPEG] " << text;
	}
}

} // extern C


// FFmpegException


FFmpegException::FFmpegException(const int error, const std::string& name)
	: error_ { error }
	, msg_   {/* empty */}
{
	auto msg = std::ostringstream{};
	msg << "FFmpeg: Function " << name  << " returned error '"
		<< av_err2str(error_) << "' ("  << error_ << ")";
	msg_ = msg.str();
}


int FFmpegException::error() const
{
	return error_;
}


char const* FFmpegException::what() const noexcept
{
	return msg_.c_str();
}


// Free_AVFormatContext


void Free_AVFormatContext::operator()(::AVFormatContext* fctx) const
{
	if (fctx)
	{
		::avformat_close_input(&fctx); // Calls ::avformat_free_context
		fctx = nullptr;
	}
}


// Free_AVCodecContext


void Free_AVCodecContext::operator()(::AVCodecContext* cctx) const
{
	if (cctx)
	{
		::avcodec_free_context(&cctx); // Calls ::avcodec_close internally
		cctx = nullptr;
	}
}


// Free_AVPacket


void Free_AVPacket::operator()(::AVPacket* packet) const
{
	::av_packet_free(&packet);
	packet = nullptr;
}


// Make_AVPacketPtr


AVPacketPtr Make_AVPacketPtr::operator()() const
{
	auto packet { AVPacketPtr { ::av_packet_alloc() } };

	if (!packet)
	{
		throw std::bad_alloc();
	}

	packet->data = nullptr;
	packet->size = 0;

	return packet;
}


// Free_AVFrame


void Free_AVFrame::operator()(::AVFrame* frame) const
{
	::av_free(frame);
	frame = nullptr;
}


// Make_AVFramePtr


AVFramePtr Make_AVFramePtr::operator()() const
{
	auto frame { AVFramePtr { ::av_frame_alloc() } };

	if (!frame)
	{
		throw std::bad_alloc();
	}

	return frame;
}


// FrameQueue


FrameQueue::FrameQueue(const std::size_t capacity)
	: frames_         { }
	, current_packet_ { }
	, stream_index_   { 0 }
	, cctx_           { nullptr }
	, fctx_           { nullptr }
	, capacity_       { capacity }
{
	// empty
}


void FrameQueue::set_source(::AVFormatContext* fctx, const int stream_index)
{
	fctx_         = fctx;
	stream_index_ = stream_index;
}


std::pair<const ::AVFormatContext*, const int> FrameQueue::source() const
{
	return { fctx_, stream_index_ };
}


void FrameQueue::set_decoder(::AVCodecContext* cctx)
{
	cctx_ = cctx;
}


const ::AVCodecContext* FrameQueue::decoder() const
{
	return cctx_;
}


std::size_t FrameQueue::fill()
{
	while (size() < capacity())
	{
		if (not enqueue_frame())
		{
			// TODO EOF reached
			break;
		}
	}

	return size();
}


bool FrameQueue::enqueue_frame()
{
	auto error  = int { 0 };
	auto packet { make_packet() };

	while (true)
	{
		error = ::av_read_frame(format_context(), packet.get());

		// For audio, the packet contains:
		// - If the audio frames have a known fixed size (e.g. PCM or ADPCM),
		//   an integer number of frames.
		// - If the audio frames have a variable size (e.g. MPEG audio),
		//   exactly one frame.

		// On error, packet is blank, like if it came from av_packet_alloc()

		if (/*macro*/AVERROR_EOF == error)
		{
			return false;
		}

		if (error < 0)
		{
			throw FFmpegException(error, "av_read_frame");
		}

		if (packet->stream_index == stream_index())
		{
			break; // Packet was successfully read, we are done
		}

		// Respect only packets from the specified stream.
		// Discard other packets and redo.

		::av_packet_unref(packet.get());
	}

	frames_.push(std::move(packet));

	return true;
}


AVFramePtr FrameQueue::dequeue_frame()
{
	auto error          = int  { 0 };
	auto decode_success = bool { false };
	auto frame          { make_frame() };

	while (true)
	{
		error = ::avcodec_receive_frame(decoder(), frame.get());

		if (AVERROR(EAGAIN) == error) // repeating required
		{
			// Decoder requires more input packets before it can provide
			// any frames, so just finish the processing of this packet
			// and provide next packet to decoder.

			if (frames_.empty())
			{
				return nullptr; // enqueue_frame() needs to be called
			}

			current_packet_ = std::move(frames_.front());
			frames_.pop();

			try
			{
				decode_success = decode_packet(current_packet_.get());

			} catch (const FFmpegException& e)
			{
				if (AVERROR(EAGAIN) == e.error())
				{
					return nullptr; // enqueue_frame() needs to be called
				}

				throw;
			}

			if (!decode_success)
			{
				return nullptr; // enqueue_frame() needs to be called
			}

			// ...on success repeat until EAGAIN vanishes

		} else // no repeating required
		{
			if (AVERROR_EOF == error)
			{
				return nullptr;
			}

			if (error < 0) // some error occurred, catchall
			{
				throw FFmpegException(error, "avcodec_receive_frame");

				// Possible errors (other then EAGAIN and EOF) are:
				// AVERROR(EINVAL) : Codec not opened.
			}

			break; // Packet was successfully decoded, we are done
		}
	}

	return frame;
}


bool FrameQueue::decode_packet(::AVPacket* packet)
{
	const auto error { ::avcodec_send_packet(decoder(), packet) };

	if (error < 0)
	{
		throw FFmpegException(error, "avcodec_send_packet");

		// Possible errors are:
		// AVERROR(EAGAIN) : Input is not accepted in the current state
		//                   user must read output with avcodec_receive_frame().
		//                   Once all output is read, the packet should be
		//                   resent.
		// AVERROR(EINVAL) : Codec requires flush.
		// AVERROR(ENOMEM) : Failed to add packet to internal queue
		// AVERROR_EOF     : The decoder has been flushed, and no new packets
		//                   can be sent to it (also returned if more than 1
		//                   flush packet is sent).
	}

	return true;
}


std::size_t FrameQueue::size() const
{
	return frames_.size();
}


std::size_t FrameQueue::capacity() const noexcept
{
	return capacity_;
}


void FrameQueue::set_capacity(const std::size_t capacity)
{
	capacity_ = capacity;
}


bool FrameQueue::empty() const
{
	return frames_.empty();
}


::AVCodecContext* FrameQueue::decoder()
{
	return cctx_;
}


::AVFormatContext* FrameQueue::format_context()
{
	return fctx_;
}


int FrameQueue::stream_index() const
{
	return stream_index_;
}


AVPacketPtr FrameQueue::make_packet()
{
	static const Make_AVPacketPtr new_packet;

	return new_packet();
}


AVFramePtr FrameQueue::make_frame()
{
	static const Make_AVFramePtr new_frame;

	return new_frame();
}


// FFmpegFile


AVFormatContextPtr FFmpegFile::format_context(const std::string& filename)
{
	// Open input stream and read header

	::AVFormatContext* fctx { nullptr };
	::AVInputFormat* detect { nullptr }; // TODO Currently unused
	::AVDictionary* options { nullptr }; // TODO Currently unused

	const auto error_open { ::avformat_open_input(&fctx , filename.c_str(),
			detect, &options) };

	if (error_open != 0)
	{
		throw FFmpegException(error_open, "avformat_open_input");
	}

	// Read some packets to acquire information about the streams
	// (This is useful for formats without a header)

	const auto error_find { ::avformat_find_stream_info(fctx, nullptr) };

	if (error_find < 0)
	{
		::avformat_close_input(&fctx);

		throw FFmpegException(error_find, "avformat_find_stream_info");
	}

	return AVFormatContextPtr(fctx);
}


int FFmpegFile::audio_stream(::AVFormatContext* fctx)
{
	const auto stream_and_codec { identify_stream(fctx, ::AVMEDIA_TYPE_AUDIO) };
	return stream_and_codec.first;
}


AVCodecContextPtr FFmpegFile::audio_decoder(::AVFormatContext* fctx,
		const int stream_idx)
{
	if (!fctx)
	{
		throw std::invalid_argument("AVFormatContext is NULL");
	}

	if (stream_idx < 0)
	{
		throw std::invalid_argument("Stream index is negative");
	}

	// ::AVStream*
	const auto* stream { fctx->streams[stream_idx] };

	if (!stream)
	{
		auto msg = std::ostringstream{};
		msg << "Could not find stream for requested index ";
		msg << stream_idx;

		throw std::invalid_argument(msg.str());
	}

	{ // Inspect side data

		auto skip = bool { false };

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(60, 31, 102)
		const uint8_t* data =
			::av_stream_get_side_data(
					stream, ::AV_PKT_DATA_SKIP_SAMPLES, nullptr);

		skip = data != nullptr;
#else
		for (auto i = int { 0 }; i < stream->codecpar->nb_coded_side_data; ++i)
		{
			//const AVPacketSideData* const
			const auto* const sd_data { &stream->codecpar->coded_side_data[i] };

			if (::AV_PKT_DATA_SKIP_SAMPLES == sd_data->type)
			{
				skip = true;
				break;
			}
		}
#endif

		if (skip)
		{
			ARCS_LOG_WARNING << "Stream side data indicates skipped samples!"
				<< " This is not yet implemented! Verify checksums carefully!";
		}
	}

	const auto stream_params { stream->codecpar };

	if (!stream_params)
	{
		throw std::runtime_error("No stream parameters found");
	}

	if (stream_params->codec_type != ::AVMEDIA_TYPE_AUDIO)
	{
		auto msg = std::ostringstream{};
		msg << "Stream with requested index ";
		msg << stream_idx;
		msg << " is not an audio stream";

		throw std::invalid_argument(msg.str());
	}

	const ::AVCodec* codec { nullptr };

	if (fctx->audio_codec)
	{
		codec = fctx->audio_codec;
	} else
	{
		codec = ::avcodec_find_decoder(stream_params->codec_id);
	}

	if (!codec)
	{
		throw std::runtime_error(
				"avcodec_find_decoder could not determine codec");
	}

	auto ccontext { AVCodecContextPtr { ::avcodec_alloc_context3(codec) } };

	if (!ccontext)
	{
		ARCS_LOG_ERROR << "Could not allocate AVCodecContext for decoding";
		throw std::bad_alloc();
	}

	if (NumberOfChannels(ccontext.get()) > AV_NUM_DATA_POINTERS)/*macro*/
	{
		// We have already ensured 2 channels by validating against CDDA.

		ARCS_LOG_WARNING << "Codec specifies "
			<< NumberOfChannels(ccontext.get())
			<< " but stream provides " << AV_NUM_DATA_POINTERS
			<< " data pointers";

		// TODO Check frame->extended_data
	}

	const auto error_pars {
		::avcodec_parameters_to_context(ccontext.get(), stream_params) };

	if (error_pars < 0) // success: >= 0
	{
		throw FFmpegException(error_pars, "avcodec_parameters_to_context");
	}

	const auto error_open { ::avcodec_open2(ccontext.get(), codec, nullptr) };

	if (error_open < 0) // success: == 0
	{
		throw FFmpegException(error_open, "avcodec_open2");
	}

	return ccontext;
}


std::pair<int, const ::AVCodec*> FFmpegFile::identify_stream(
		::AVFormatContext* fctx, const ::AVMediaType media_type)
{
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(59, 16, 100) //  < ffmpeg 5.0
	::AVCodec* codec { nullptr };
#else
	const ::AVCodec* codec { nullptr };
#endif

	const int stream_index { ::av_find_best_stream(fctx, media_type,
			-1/*no wanted stream*/, -1/*no related stream*/,
			&codec/*request codec*/, 0/*no flags*/) };

	if (stream_index < 0)
	{
		throw FFmpegException(stream_index, "av_find_best_stream");

		// Possible Errors:
		// AVERROR_STREAM_NOT_FOUND if no stream with the requested type could
		//			be found
		// AVERROR_DECODER_NOT_FOUND if streams were found but no decoder
	}

	if (!codec)
	{
		auto msg = std::ostringstream{};
		msg << "No codec found for audio stream " << stream_index;

		ARCS_LOG_ERROR << msg.str();

		// This is not a problem since we will try to identify the codec
		// with create_audio_decoder() and throw a proper exception there.
		// Caller must check codec.
	}

	return std::make_pair(stream_index, codec);
}


int64_t FFmpegFile::total_samples(::AVCodecContext* cctx, ::AVStream* stream)
{
	// Calculate number of samples from duration, which should be accurate
	// if stream metadata is intact

	const double time_base =
		static_cast<double>(stream->time_base.num) /
		static_cast<double>(stream->time_base.den);

	const double duration_secs =
		static_cast<double>(stream->duration) * time_base;

	ARCS_LOG_DEBUG << "Estimate duration:       " << duration_secs << " secs";

	return duration_secs * cctx->sample_rate;
}


// IsSupported


bool IsSupported::format(const ::AVSampleFormat id)
{
	static std::set<::AVSampleFormat> formats =
	{
		::AV_SAMPLE_FMT_S16P,
		::AV_SAMPLE_FMT_S16,
		::AV_SAMPLE_FMT_S32P,
		::AV_SAMPLE_FMT_S32
	};

	return formats.find(id) != formats.end();
}


bool IsSupported::codec(const ::AVCodecID id)
{
	static std::set<::AVCodecID> codecs =
	{
		::AV_CODEC_ID_ALAC,
		::AV_CODEC_ID_APE,
		::AV_CODEC_ID_FLAC,
		::AV_CODEC_ID_PCM_S16BE,
		::AV_CODEC_ID_PCM_S16LE,
		::AV_CODEC_ID_PCM_S16BE_PLANAR, /* TODO untested */
		::AV_CODEC_ID_PCM_S16LE_PLANAR  /* TODO untested */
	};
		// TODO WMALOSSLESS
		// ::AV_CODEC_ID_WAVPACK: // Removed: could not check for lossy

	return codecs.find(id) != codecs.end();
}


// FFmpegValidator


bool FFmpegValidator::cdda(::AVCodecContext* cctx)
{
	bool is_validated { true };

	// TODO Subclass DefaultValidator to use error stack correctly

	if (not CDDAValidator::bits_per_sample(
				::av_get_bytes_per_sample(cctx->sample_fmt) * CHAR_BIT))
	{
		ARCS_LOG_ERROR << "Not CDDA: not 16 bits per sample";
		is_validated = false;
	}

	if (not CDDAValidator::num_channels(NumberOfChannels(cctx)))
	{
		ARCS_LOG_ERROR << "Not CDDA: not stereo";
		is_validated = false;
	}

	if (not CDDAValidator::samples_per_second(cctx->sample_rate))
	{
		ARCS_LOG_ERROR << "Not CDDA: sample rate is not 44100 Hz";
		is_validated = false;
	}

	return is_validated;
}


AudioValidator::codec_set_type FFmpegValidator::do_codecs() const
{
	return { }; // Does not validate any codecs, only CDDA
}


// FFmpegAudioStreamLoader


std::unique_ptr<FFmpegAudioStream> FFmpegAudioStreamLoader::load(
		const std::string& filename) const
{
	ARCS_LOG_DEBUG << "Start to analyze audio file with ffmpeg";

#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(58, 9, 100) //  < ffmpeg 4.0
	static const bool registered = [](){ ::av_register_all(); return true; }();
#endif

	auto fcontext { FFmpegFile::format_context(filename) };
	ARCS_LOG(DEBUG1) << fcontext.get();

	const auto stream_idx { FFmpegFile::audio_stream(fcontext.get()) };
	ARCS_LOG(DEBUG1) << fcontext->streams[stream_idx];

	auto ccontext { FFmpegFile::audio_decoder(fcontext.get(), stream_idx) };
	ARCS_LOG_DEBUG << ccontext.get();


	if (not IsSupported::format(ccontext->sample_fmt))
	{
		auto msg = std::ostringstream{};
		msg << "Sample format not supported: "
			<< ::av_get_sample_fmt_name(ccontext->sample_fmt);
		throw InvalidAudioException(msg.str());
	}

	if (not IsSupported::codec(ccontext->codec->id))
	{
		auto msg = std::ostringstream{};
		msg << "Codec not supported: " << ccontext->codec->long_name;
		throw InvalidAudioException(msg.str());
	}

	if (not FFmpegValidator::cdda(ccontext.get()))
	{
		auto msg = std::ostringstream{};
		msg << "CDDA validation failed";
		throw InvalidAudioException(msg.str());
	}


	const auto total_samples { FFmpegFile::total_samples(ccontext.get(),
			fcontext->streams[stream_idx]) };
	ARCS_LOG_DEBUG << "Expect " << total_samples << " total samples";

	// We have to update the expected AudioSize before the last block of samples
	// is passed.
	//
	// We estimate the number of samples by the duration of the stream. We use
	// this estimation to decide at which point we start to buffer frames till
	// EOF is seen. We assume that the declaration does not yield an estimation
	// smaller than 6 frames less of the physical total samples. So, as soon as
	// the declared total sample amount is only 6 frames ahead, we just start to
	// read until EOF.
	//
	// In most cases, the estimation is correct if the file is intact and the
	// codec does not use padding (priming or remainder frames). The guess is
	// reliable for PCM*, FLAC, WAVPACK, APE. It fails for ALAC. However, we
	// count the samples and correct the estimation before flushing the last
	// relevant block.


	// Configure file object with ffmpeg properties

	auto stream { std::make_unique<FFmpegAudioStream>(FFmpegAudioStream{}) };

	stream->num_planes_ =
		::av_sample_fmt_is_planar(ccontext->sample_fmt) ? 2 : 1;

	stream->channels_swapped_ =
			!ChannelOrder::is_leftright(ccontext.get())  &&
			!ChannelOrder::is_unspecified(ccontext.get());
	// We already have verified to have exactly 2 channels. If they are
	// not FL+FR and not unspecified, they are considered to be swapped.

	stream->total_samples_declared_ = total_samples;
	stream->stream_index_  = stream_idx;
	stream->codecContext_  = std::move(ccontext);
	stream->formatContext_ = std::move(fcontext);

	ARCS_LOG_DEBUG << "Input stream ready";

	return stream;
}


// FFmpegAudioStream


FFmpegAudioStream::FFmpegAudioStream()
	: formatContext_ { nullptr }
	, codecContext_  { nullptr }
	, stream_index_     { 0 }
	, num_planes_       { 0 }
	, channels_swapped_ { false }
	, total_samples_declared_ { 0 }
	, start_input_      { }
	, push_frame_       { }
	, update_audiosize_ { }
	, end_input_        { }
{
	// empty
}


int64_t FFmpegAudioStream::total_samples_declared() const
{
	return total_samples_declared_;
}


AVSampleFormat FFmpegAudioStream::sample_format() const
{
	return codecContext_->sample_fmt;
}


int FFmpegAudioStream::stream_index() const
{
	return stream_index_;
}


bool FFmpegAudioStream::channels_swapped() const
{
	return channels_swapped_;
}


void FFmpegAudioStream::register_start_input(std::function<void()> func)
{
	start_input_ = func;
}


void FFmpegAudioStream::register_push_frame(
		std::function<void(AVFramePtr frame)> func)
{
	push_frame_ = func;
}


void FFmpegAudioStream::register_update_audiosize(
		std::function<void(const AudioSize& size)> func)
{
	update_audiosize_ = func;
}


void FFmpegAudioStream::register_end_input(std::function<void()> func)
{
	end_input_ = func;
}


int FFmpegAudioStream::num_planes() const
{
	return num_planes_;
}


int64_t FFmpegAudioStream::traverse_samples()
{
	auto queue = FrameQueue { 12 };
	queue.set_source(formatContext_.get(), stream_index());
	queue.set_decoder(codecContext_.get());


	// Fill queue to its defined average size

	queue.fill();

	ARCS_LOG_DEBUG << "Loaded " << queue.size() << " encoded packets to queue";
	ARCS_LOG_DEBUG << "Start to manage decoding queue";


	// Manage queue as long as new frames are available

	// Allow 1 packet less than capacity before requesting new packets
	const auto allowed_diff   = decltype( queue )::size_type { 1 };

	const auto queue_capacity { queue.capacity() };

	// current frame
	auto frame = AVFramePtr { nullptr };

	// total samples in queue
	auto total_samples = int32_t { 0 };

	while ((frame = queue.dequeue_frame()))
	{
		total_samples += frame->nb_samples;

		this->push_frame_(std::move(frame));

		// queue too small?
		if ((queue_capacity - queue.size()) > allowed_diff)
		{
			if (not queue.enqueue_frame())
			{
				// TODO EOF reached
				break;
			}
		}
	}

	ARCS_LOG_DEBUG << "Last frame was read, flush queue after "
		<< total_samples << " samples";

	// Flush queue

	auto decoded_frames = std::vector<AVFramePtr>{};
	decoded_frames.reserve(queue.size());
	// TODO Using a vector is not the nicest solution, consider AVAudioFifo
	// https://ffmpeg.org/doxygen/trunk/structAVAudioFifo.html

	auto last_size = decltype( queue )::size_type { 0 };


	// Flush all frames from decoder: we need to know the total number of
	// samples to update the Calculation with the expected number of samples

	flush:

	last_size = decoded_frames.size();

	while ((frame = queue.dequeue_frame()))
	{
		total_samples += frame->nb_samples;

		decoded_frames.push_back(std::move(frame));
	}

	// Respect delayed frames (if any)

	if (decoded_frames.size() > last_size) // Did flushing add samples?
	{
		if (/*macro*/AV_CODEC_CAP_DELAY & codecContext_->codec->capabilities)
		{
			ARCS_LOG_INFO  << "Codec declares to have delay capability";
			ARCS_LOG_DEBUG << "Check for delayed frames";

			// https://ffmpeg.org/doxygen/4.1/group__lavc__encdec.html

			// Enter "draining mode" by sending nullptr as packet
			if (::avcodec_send_packet(codecContext_.get(), nullptr) < 0)
			{
				ARCS_LOG_DEBUG << "Could not get any delayed frames";
				// TODO This is an error, handle it!
			} else
			{
				goto flush; // Flush again in "draining" mode
			}
		}
	} else
	{
		ARCS_LOG_DEBUG << "Seems there are no delayed frames, just proceed";
	}

	// Update audiosize

	AudioSize updated_size;
	updated_size.set_samples(total_samples);

	update_audiosize_(updated_size);

	// Pass last samples

	for (auto& f : decoded_frames)
	{
		this->push_frame_(std::move(f));
	}

	return total_samples;
}


// FFmpegAudioReaderImpl


FFmpegAudioReaderImpl::FFmpegAudioReaderImpl()
	: AudioReaderImpl()
{
	// empty
}


FFmpegAudioReaderImpl::~FFmpegAudioReaderImpl() noexcept = default;


std::unique_ptr<AudioSize> FFmpegAudioReaderImpl::do_acquire_size(
	const std::string& filename)
{
	using arcstk::AudioSize;
	using arcstk::UNIT;

	const auto loader    { FFmpegAudioStreamLoader{} };
	const auto audiofile { loader.load(filename) };

	return std::make_unique<AudioSize>(
				audiofile->total_samples_declared(), UNIT::SAMPLES);
}


void FFmpegAudioReaderImpl::do_process_file(const std::string& filename)
{
	// Redirect ffmpeg logging to arcs logging

	::av_log_set_callback(arcs_av_log);


	// Plug file, buffer and processor together

	const auto loader      { FFmpegAudioStreamLoader{} };
	const auto audiostream { loader.load(filename) };

	if (audiostream->channels_swapped())
	{
		ARCS_LOG_INFO << "FFmpeg says channels are swapped.";
	}

	// Register this AudioReaderImpl instance as the stream's callback provider.
	// This imitates how a SampleProcessor is attached to a SampleProvider.

	audiostream->register_start_input(
		std::bind(&FFmpegAudioReaderImpl::signal_startinput, this));

	audiostream->register_push_frame(
		std::bind(&FFmpegAudioReaderImpl::frame_callback,
			this,
			std::placeholders::_1));

	audiostream->register_update_audiosize(
		std::bind(&FFmpegAudioReaderImpl::signal_updateaudiosize,
			this,
			std::placeholders::_1));

	audiostream->register_end_input(
		std::bind(&FFmpegAudioReaderImpl::signal_endinput, this));


	// Process file

	this->signal_startinput();

	const auto total_samples_expected { audiostream->total_samples_declared() };

	if (total_samples_expected > std::numeric_limits<int32_t>::max())
	{
		using std::to_string;
		throw std::runtime_error("Number of samples too big: " +
				to_string(total_samples_expected));
	}

	using arcstk::UNIT;

	this->signal_updateaudiosize(
			{ static_cast<int32_t>(total_samples_expected), UNIT::SAMPLES });

	const auto total_samples { audiostream->traverse_samples() };

	this->signal_endinput();


	// Do some logging

	ARCS_LOG_DEBUG << "Respected samples: " << total_samples;

	if (total_samples != total_samples_expected)
	{
		ARCS_LOG_INFO << "Expected " << total_samples_expected
					<< " samples but encountered " << total_samples
					<< " ("
					<< std::abs(total_samples_expected - total_samples)
					<< ((total_samples_expected < total_samples)
							? " more"
							: " less")
					<< ")";
	}

	ARCS_LOG_DEBUG << "Finished processing";
}


std::unique_ptr<FileReaderDescriptor> FFmpegAudioReaderImpl::do_descriptor()
	const
{
	return std::make_unique<DescriptorFFmpeg>();
}


void FFmpegAudioReaderImpl::frame_callback(AVFramePtr frame)
{
	this->pass_frame(std::move(frame));

	// Alternatively, pass_frame_to_buffer() could be used for a configurable
	// frame buffer but this is currently significantly slower.
}


void FFmpegAudioReaderImpl::pass_frame(AVFramePtr frame)
{
	const auto format { static_cast<::AVSampleFormat>(frame->format) };

	switch (format)
	{
		case ::AV_SAMPLE_FMT_S16 :/* int16_t, interleaved - e.g. AIFF */
			{
				this->pass_samples<::AV_SAMPLE_FMT_S16>(std::move(frame));
				break;
			}
		case ::AV_SAMPLE_FMT_S16P:/* int16_t, planar - e.g. MONKEY, ALAC */
			{
				this->pass_samples<::AV_SAMPLE_FMT_S16P>(std::move(frame));
				break;
			}
		case ::AV_SAMPLE_FMT_S32 :/* int32_t, interleaved - e.g. FLAC */
			{
				this->pass_samples<::AV_SAMPLE_FMT_S32>(std::move(frame));
				break;
			}
		case ::AV_SAMPLE_FMT_S32P:/* int32_t, planar - e.g. WAVPACK */
			{
				this->pass_samples<::AV_SAMPLE_FMT_S32P>(std::move(frame));
				break;
			}
		default:
			{
				auto msg = std::ostringstream{};
				msg << "Cannot pass sequence with unknown sample format: "
					<< ::av_get_sample_fmt_name(format);

				throw std::invalid_argument(msg.str());
			}
	}// switch
}


template<enum ::AVSampleFormat F>
void FFmpegAudioReaderImpl::pass_samples(AVFramePtr frame)
{
	const auto sequence { sequence_for<F>(frame) };

	using std::cbegin;
	using std::cend;

	this->signal_appendsamples(cbegin(sequence), cend(sequence));
}


void print_dictionary(std::ostream& out, const ::AVDictionary* dict)
{
	::AVDictionaryEntry* e { nullptr };

	while ((e = ::av_dict_get(dict, "", e, /*macro*/AV_DICT_IGNORE_SUFFIX)))
	{
		out << "  Name: " << e->key << "  Value: "    << e->value << std::endl;
	}
}


void operator << (std::ostream& out, const ::AVDictionary* dict)
{
	print_dictionary(out, dict);
}


void print_codec_info(std::ostream& out, const ::AVCodecContext* cctx)
{
	if (!cctx)
	{
		out << "CodecContext information: NULL" << std::endl;
		return;
	}

	out << "CodecContext information:" << std::endl;

	if (!cctx->codec_descriptor)
	{
		out << "  Context has no codec descriptor" << std::endl;

		if (!cctx->codec)
		{
			out << "  Context has neither a codec object" << std::endl;
		} else
		{
			out << "  Codec name:     " << cctx->codec->long_name << std::endl;
			out << "  Short name:     " << cctx->codec->name << std::endl;
		}
	} else
	{
		out << "  Codec name:     " << cctx->codec_descriptor->long_name
			<< std::endl;
		out << "  Short name:     " << cctx->codec_descriptor->name
			<< std::endl;
	}

	out << "  Sample format:  " << ::av_get_sample_fmt_name(cctx->sample_fmt)
		<< std::endl;

	const bool is_planar = ::av_sample_fmt_is_planar(cctx->sample_fmt);
	out << "  Is planar:      " << (is_planar ? "yes" : "no") << std::endl;

	const auto bps = ::av_get_bytes_per_sample(cctx->sample_fmt);
	out << "  Bytes/Sample:   " << bps << " (= " << (bps * CHAR_BIT) << " bit)"
		<< std::endl;

	out << "  Number of channels:           " <<
		NumberOfChannels(cctx) << std::endl;
	out << "  Channel order is_leftright:   " <<
		(ChannelOrder::is_leftright(cctx) ? "yes" : "no") <<
		std::endl;
	out << "  Channel order is_unspecified: " <<
		(ChannelOrder::is_unspecified(cctx) ? "yes" : "no") <<
		std::endl;
	out << "  Samplerate:     " << cctx->sample_rate << " Hz (samples/sec)"
		<< std::endl;
	out << "  skip_bottom:      " << cctx->skip_bottom << std::endl;

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(60, 2, 100) //  < ffmpeg 6.0
	out << "  frame_number:     " << cctx->frame_number << std::endl;
#else
	out << "  frame_num:     "    << cctx->frame_num << std::endl;
#endif

	out << "  frame_size:       " << cctx->frame_size << std::endl;
	out << "  initial_padding:  " << cctx->initial_padding << std::endl;
	out << "  trailing_padding: " << cctx->trailing_padding << std::endl;

	switch(cctx->skip_frame)
	{
		case AVDISCARD_NONE:
			out << "  skip_frame:       AVDISCARD_NONE" << std::endl;
			break;
		case AVDISCARD_DEFAULT:
			out << "  skip_frame:       AVDISCARD_DEFAULT" << std::endl;
			break;
		case AVDISCARD_NONREF:
			out << "  skip_frame:       AVDISCARD_NONREF" << std::endl;
			break;
		case AVDISCARD_BIDIR:
			out << "  skip_frame:       AVDISCARD_BIDIR" << std::endl;
			break;
		case AVDISCARD_NONINTRA:
			out << "  skip_frame:       AVDISCARD_NONINTRA" << std::endl;
			break;
		case AVDISCARD_NONKEY:
			out << "  skip_frame:       AVDISCARD_NONKEY" << std::endl;
			break;
		case AVDISCARD_ALL:
			out << "  skip_frame:       AVDISCARD_ALL" << std::endl;
			break;

		default: ; // do not print anything
	}

	if (!cctx->codec_descriptor)
	{
		out <<
			"  Context has no codec descriptor, cannot print codec properties"
			<< std::endl;
	}

	out << "  --Codec Properties--" << std::endl;
	{
		// Losslessness

		const bool codec_prop_lossless =
			cctx->codec_descriptor->props & AV_CODEC_PROP_LOSSLESS;

		const bool codec_prop_lossy =
			cctx->codec_descriptor->props & AV_CODEC_PROP_LOSSY;

		out << "  PROP_LOSSLESS:  " << (codec_prop_lossless ? "yes" : "no")
			<< std::endl;
		out << "  PROP_LOSSY:     " << (codec_prop_lossy ?    "yes" : "no")
			<< std::endl;

		if (codec_prop_lossy)
		{
			if (not codec_prop_lossless)
			{
				out << "Codec declares itself lossy-only, bail out" << std::endl;
			} else
			{
				out << "Codec declares support for lossy encoding" << std::endl;
				out << "If you know that your file is lossless, proceed"
					<< std::endl;
			}
		}
	}

	if (!cctx->codec)
	{
		out << "No codec object in context, cannot print capabilities"
			<< std::endl;
	}

	out << "  --Codec Capabilities--" << std::endl;
	{
		out << "  Capabilities:            " << cctx->codec->capabilities
			<< std::endl;
		out << "  Capability bits:         " <<
					(sizeof(cctx->codec->capabilities) * 8) << std::endl;

		// Variable frame size ?

		const bool codec_cap_variable_frame_size =
			cctx->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE;

		out << "  CAP_VARIABLE_FRAME_SIZE: "
			<< (codec_cap_variable_frame_size ? "yes" : "no ")
			<< "  (supports variable frame size)" << std::endl;

		// Last frame smaller ?

		const bool codec_cap_small_last_frame =
			cctx->codec->capabilities & AV_CODEC_CAP_SMALL_LAST_FRAME;

		out << "  CAP_SMALL_LAST_FRAME:    "
			<< (codec_cap_small_last_frame ? "yes" : "no ")
			<< "  (supports smaller last frame)" << std::endl;

		// Delay frames/require flush ?

		const bool codec_cap_delay =
			cctx->codec->capabilities & AV_CODEC_CAP_DELAY;

		out << "  CAP_DELAY:               "
			<< (codec_cap_delay ? "yes" : "no ")
			<< "  (may delay frames, decoder requires flushing)" << std::endl;

		// More than 1 frame per packet?

		const bool codec_cap_subframes =
			cctx->codec->capabilities & AV_CODEC_CAP_SUBFRAMES;

		out << "  CAP_SUBFRAMES:           "
			<< (codec_cap_subframes ? "yes" : "no ")
			<< "  (allows more than 1 frame/packet)" << std::endl;

		// May use mulithreading (frame order?)

		const bool codec_cap_frame_threads =
			cctx->codec->capabilities & AV_CODEC_CAP_FRAME_THREADS;
		// applies for flac, alac

		out << "  CAP_FRAME_THREADS:       "
			<< (codec_cap_frame_threads ? "yes" : "no ")
			<< "  (supports frame-level multithreading)" << std::endl;

		// Allows custom allocators?

		const bool codec_cap_dr1 =
			cctx->codec->capabilities & AV_CODEC_CAP_DR1;
		// applies for flac, alac, ape

		out << "  CAP_DR1:                 "
			<< (codec_cap_dr1 ? "yes" : "no ")
			<< "  (uses get_buffer() to allocate, supports custom allocators)";
	}
}


void operator << (std::ostream& out, const ::AVCodecContext* cctx)
{
	print_codec_info(out, cctx);
}


void print_format_info(std::ostream& out, const ::AVFormatContext* fctx)
{
	// Commented out, kept as a note:
	// Output ffmpeg-sytle info
	//::av_dump_format(fctx, 0, filename.c_str(), 0);

	out << "FormatContext information:" << std::endl;

	if (fctx->metadata)
	{
		out << "  Metadata:" << std::endl;
		out << fctx->metadata;
	}

	out << "  packet_size:  " << std::to_string(fctx->packet_size);
}


void operator << (std::ostream& out, const ::AVFormatContext* fctx)
{
	print_format_info(out, fctx);
}


void print_stream_info(std::ostream& out, const ::AVStream* s)
{
	out << "Stream information:" << std::endl;

	if (s->metadata)
	{
		out << "  Metadata:" << std::endl;
		out << s->metadata;
	}

	out << "  initial_padding:  " << s->codecpar->initial_padding << std::endl;
	out << "  trailing_padding: " << s->codecpar->trailing_padding << std::endl;
	out << "  frame_size:       " << s->codecpar->frame_size << std::endl;
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(60, 31, 102)
	out << "  nb_side_data:     " << s->nb_side_data << std::endl;
#else
	out << "  nb_coded_side_data: " << s->codecpar->nb_coded_side_data << std::endl;
#endif
	out << "  nb_frames:        " << s->nb_frames;
}


void operator << (std::ostream& out, const ::AVStream* stream)
{
	print_stream_info(out, stream);
}

} // namespace ffmpeg
} // namespace details


// DescriptorFFmpeg


DescriptorFFmpeg::~DescriptorFFmpeg() noexcept = default;


std::string DescriptorFFmpeg::do_id() const
{
	return "ffmpeg";
}


std::string DescriptorFFmpeg::do_name() const
{
	return "FFmpeg";
}


std::set<Format> DescriptorFFmpeg::define_formats() const
{
	return
	{
		Format::WAV,
		Format::FLAC,
		Format::APE,
		Format::CAF,
		Format::M4A,
		Format::OGG,
		// not WV,
		Format::AIFF
		//TODO Format::WMA
	};
}


std::set<Codec> DescriptorFFmpeg::define_codecs() const
{
	return {
		Codec::PCM_S16BE,
		Codec::PCM_S16BE_PLANAR,
		Codec::PCM_S16LE,
		Codec::PCM_S16LE_PLANAR,
		Codec::PCM_S32BE,
		Codec::PCM_S32BE_PLANAR,
		Codec::PCM_S32LE,
		Codec::PCM_S32LE_PLANAR,
		Codec::FLAC,
		// not WAVEPACK
		Codec::MONKEY,
		Codec::ALAC
		//TODO Codec::WMALOSSLESS
	};
}


LibInfo DescriptorFFmpeg::do_libraries() const
{
	return {
		libinfo_entry_filepath("libavformat"),
		libinfo_entry_filepath("libavcodec"),
		libinfo_entry_filepath("libavutil"),
	};
}


std::unique_ptr<FileReader> DescriptorFFmpeg::do_create_reader() const
{
	auto impl { std::make_unique<details::ffmpeg::FFmpegAudioReaderImpl>() };
	return std::make_unique<AudioReader>(std::move(impl));
}


std::unique_ptr<FileReaderDescriptor> DescriptorFFmpeg::do_clone() const
{
	return std::make_unique<DescriptorFFmpeg>();
}


// Add this descriptor to the audio descriptor registry

namespace {

const auto d = RegisterDescriptor<DescriptorFFmpeg>{};

} // namespace

} // namespace v_1_0_0
} // namespace arcsdec

