/**
 * \file
 *
 * \brief Implements ffmpeg-based generic audio reader.
 */

#ifndef __LIBARCSDEC_READERFFMPEG_HPP__
#include "readerffmpeg.hpp"
#endif
#ifndef __LIBARCSDEC_READERFFMPEG_DETAILS_HPP__
#include "readerffmpeg_details.hpp"
#endif

#include <algorithm>  // for remove
#include <climits>    // for CHAR_BIT
#include <cstdlib>    // for abs
#include <exception>  // for exception
#include <functional> // for function
#include <new>        // for bad_alloc
#include <numeric>    // for accumulate
#include <queue>      // for queue
#include <stdexcept>  // for invalid_argument
#include <string>

#include <iostream>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/version.h>
#include <libavutil/avutil.h>
}

#ifndef __LIBARCSTK_SAMPLES_HPP__
#include <arcstk/samples.hpp>
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"
#endif


namespace arcsdec
{


inline namespace v_1_0_0
{

using arcstk::SampleInputIterator;
using arcstk::AudioSize;
using arcstk::Logging;
using arcstk::InvalidAudioException;
using arcstk::CDDA;
using arcstk::SampleSequence;

using arcstk::LOGLEVEL;


// FFmpegException


FFmpegException::FFmpegException(const int error, const std::string &name)
	: error_ { error }
	, msg_   {}
{
	std::ostringstream msg;
	msg << "Function " << name  << " returned error "  << error_
		<< ": '" << av_err2str(error_) << "'";
	msg_ = msg.str();
}


int FFmpegException::error() const
{
	return error_;
}


char const * FFmpegException::what() const noexcept
{
	return msg_.c_str();
}


// Free_AVFormatContext


void Free_AVFormatContext::operator()(::AVFormatContext* fctx) const
{
	if (fctx)
	{
		::avformat_close_input(&fctx);
		// Does call ::avformat_free_context internally
	}
}


// Free_AVCodecContext


void Free_AVCodecContext::operator()(::AVCodecContext* cctx) const
{
	if (cctx)
	{
		::avcodec_free_context(&cctx);
		// Does call ::avcodec_close internally
	}
}


// Free_AVPacket


void Free_AVPacket::operator()(::AVPacket* packet) const
{
	::av_packet_free(&packet);
}


// Make_AVPacketPtr


AVPacketPtr Make_AVPacketPtr::operator()() const
{
	auto packet = ::av_packet_alloc();

	if (!packet)
	{
		throw std::bad_alloc();
	}

	::av_init_packet(packet);
	packet->data = nullptr;
	packet->size = 0;

	return AVPacketPtr(packet);
}


// Free_AVFrame


void Free_AVFrame::operator()(::AVFrame* frame) const
{
	::av_free(frame);
}


// Make_AVFramePtr


AVFramePtr  Make_AVFramePtr::operator()() const
{
	::AVFrame* f = ::av_frame_alloc();

	if (!f)
	{
		throw std::bad_alloc();
	}

	return AVFramePtr(f);
}


// PacketQueue


PacketQueue::PacketQueue()
	: packets_        {}
	, current_packet_ {}
	, stream_index_   { 0 }
	, cctx_   { nullptr }
	, fctx_   { nullptr }
{
	// empty
}


void PacketQueue::set_source(::AVFormatContext* fctx, const int stream_index)
{
	fctx_         = fctx;
	stream_index_ = stream_index;
}


std::pair<const ::AVFormatContext*, const int> PacketQueue::source() const
{
	return std::make_pair(fctx_, stream_index_);
}


void PacketQueue::set_decoder(::AVCodecContext* cctx)
{
	cctx_ = cctx;
}


const ::AVCodecContext* PacketQueue::decoder() const
{
	return cctx_;
}


bool PacketQueue::enqueue_frame()
{
	auto error  = int { -1 };
	auto packet = make_packet();

	while (error < 0)
	{
		error = ::av_read_frame(format_context(), packet.get());

		// For audio, the packet contains:
		// - an integer number of frames if each frame has a known fixed size
		//   (e.g. PCM or ADPCM data).
		// - If the audio frames have a variable size (e.g. MPEG audio), then
		//   the packet contains one frame.

		// On error, packet is blank, like if it came from av_packet_alloc()

		if (AVERROR_EOF == error)
		{
			std::cout << "No more frames" << std::endl;
			return false;
		}

		if (error < 0)
		{
			throw FFmpegException(error, "av_read_frame");
		}

		// Respect only packets from the specified stream

		if (packet->stream_index != stream_index())
		{
			std::cout << "Discard packet from other stream" << std::endl;
			::av_packet_unref(packet.get());

			error = -1; // Restart loop
			continue;
		}
	}

	packets_.push(std::move(packet));
	std::cout << "Pushed frame, size: " << packets_.size() << std::endl;

	return true;
}


AVFramePtr PacketQueue::dequeue_frame()
{
	auto error          = int  { 0 };
	auto frame          = make_frame();
	auto decode_success = bool { false };

	while (error >= 0)
	{
		std::cout << "Try to receive frame" << std::endl;
		error = ::avcodec_receive_frame(decoder(), frame.get());

		if (AVERROR(EAGAIN) == error)
		{
			// Decoder requires more input packets before it can provide
			// any frames, so just finish the processing of this packet
			// and provide next packet to decoder.

			std::cout << "Decoder needs more input" << std::endl;
			if (packets_.empty())
			{
				std::cout << "Queue is empty" << std::endl;
				return nullptr; // enqueue_frame() needs to be called
			}

			current_packet_ = std::move(packets_.front());
			packets_.pop();

			std::cout << "Popped frame, size: " << packets_.size() << std::endl;

			try
			{
				decode_success = decode_packet();

			} catch (const FFmpegException &e)
			{
				if (AVERROR(EAGAIN) == e.error())
				{
					std::cout << "More input required" << std::endl;
					return nullptr; // enqueue_frame() needs to be called
				}

				throw;
			}

			if (decode_success)
			{
				std::cout << "Decode success" << std::endl;
				error = 0;
				continue;
			} else
			{
				std::cout << "Decoding failed" << std::endl;
				return nullptr; // enqueue_frame() needs to be called
			}
		}

		if (error < 0) // some error occurred
		{
			throw FFmpegException(error, "avcodec_receive_frame");

			// Possible errors (other then EAGAIN) are:
			// AVERROR(EINVAL) : Codec not opened.
			// AVERROR_EOF     : The decoder has been flushed.
		}

		std::cout << "Frame received" << std::endl;
		break;
	}

	return frame;
}


bool PacketQueue::decode_packet()
{
	std::cout << "Decode frame" << std::endl;
	const auto error = ::avcodec_send_packet(decoder(), current_packet_.get());

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


std::size_t PacketQueue::size() const
{
	return packets_.size();
}


::AVCodecContext* PacketQueue::decoder()
{
	return cctx_;
}


::AVFormatContext* PacketQueue::format_context()
{
	return fctx_;
}


int PacketQueue::stream_index() const
{
	return stream_index_;
}


AVPacketPtr PacketQueue::make_packet()
{
	static const Make_AVPacketPtr new_packet;

	return new_packet();
}


AVFramePtr PacketQueue::make_frame()
{
	static const Make_AVFramePtr new_frame;

	return new_frame();
}


// open_file()


AVFormatContextPtr open_file(const std::string &filename)
{
	// Open input stream of the file and read the header

	::AVFormatContext* fctx = nullptr;
	::AVInputFormat* detect = nullptr; // TODO Currently unused
	::AVDictionary* options = nullptr; // TODO Currently unused

	const auto error_open = ::avformat_open_input(&fctx , filename.c_str(),
			detect, &options);

	if (error_open != 0)
	{
		throw FFmpegException(error_open, "avformat_open_input");
	}

	// Read some packets to acquire information about the streams
	// (This is useful for formats without a header)

	const auto error_find = ::avformat_find_stream_info(fctx, nullptr);

	if (error_find < 0)
	{
		::avformat_close_input(&fctx);

		throw FFmpegException(error_find, "avformat_find_stream_info");
	}

	return AVFormatContextPtr(fctx);
}


// identify_stream()


std::pair<int, AVCodec*> identify_stream(::AVFormatContext* fctx,
		const ::AVMediaType media_type)
{
	::AVCodec* codec = nullptr;

	const int stream_index = ::av_find_best_stream(fctx, media_type,
			-1/*no wanted stream*/, -1/*no related stream*/,
			&codec/*request codec*/, 0/*no flags*/);

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
		std::ostringstream message;
		message << "No codec found for audio stream " << stream_index;

		ARCS_LOG_ERROR << message.str();

		// This is not a problem since we will try to identify the codec
		// with create_audio_decoder() and throw a proper exception there.
		// Caller must check codec.
	}

	return std::make_pair(stream_index, codec);
}


// create_audio_decoder()


AVCodecContextPtr create_audio_decoder(::AVFormatContext *fctx,
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

	::AVStream* stream = fctx->streams[stream_idx];

	if (!stream)
	{
		std::ostringstream msg;
		msg << "Could not find stream for requested index ";
		msg << stream_idx;

		throw std::invalid_argument(msg.str());
	}

	auto stream_params = stream->codecpar;

	if (!stream_params)
	{
		throw std::runtime_error("No stream parameters found");
	}

	if (stream_params->codec_type != AVMEDIA_TYPE_AUDIO)
	{
		std::ostringstream msg;
		msg << "Stream with requested index ";
		msg << stream_idx;
		msg << " is not an audio stream";

		throw std::invalid_argument(msg.str());
	}

	::AVCodec *codec = nullptr;

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

	::AVCodecContext* cctx = ::avcodec_alloc_context3(codec);

	if (!cctx)
	{
		ARCS_LOG_ERROR << "Could not allocate AVCodecContext for decoding";
		throw std::bad_alloc();
	}

	auto context = AVCodecContextPtr(cctx);

	const auto error_pars =
		::avcodec_parameters_to_context(cctx, stream_params);

	if (error_pars < 0) // success: >= 0
	{
		throw FFmpegException(error_pars, "avcodec_parameters_to_context");
	}

	const auto error_open = ::avcodec_open2(cctx, codec, nullptr);

	if (error_open < 0) // success: == 0
	{
		throw FFmpegException(error_open, "avcodec_open2");
	}

	return context;
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


// ******* OLD ************


namespace
{

/**
 * \internal \defgroup readerffmpegImpl Implementation
 *
 * \ingroup readerffmpeg
 *
 * @{
 */

extern "C"
{

/**
 * \brief A logger callback for the ffmpeg logging API.
 *
 * It logs ffmpeg messages leveled as errors, warnings and informations by the
 * libarcstk logging interface. Messages leveled as debug, trace or other are
 * discarded. All parameters except lvl and msg are ignored.
 *
 * Since this function will be passed by a function pointer to a C function, it
 * has to be a static or global function with C linkage to provide a portable
 * way of setting a C++ function as a callback for a C function.
 *
 * \relatesalso FFmpegAudioStream
 * \relatesalso FFmpegAudioStreamLoader
 *
 * \param[in] lvl The loglevel as defined by the ffmpeg API (e.g. AV_LOG_INFO)
 * \param[in] msg The message to log
 */
void arcs_av_log(void* /*v*/, int lvl, const char* msg, va_list /*l*/)
{
	// Remove newline(s) from message text

	std::string message(msg);
	message.erase(std::remove(message.begin(), message.end(), '\n'),
			message.end());


	// Log according to the loglevel

	if (AV_LOG_ERROR == lvl)
	{
		ARCS_LOG_ERROR << "[FFMPEG] " << message;
	} else
	if (AV_LOG_WARNING == lvl)
	{
		ARCS_LOG_WARNING << "[FFMPEG] " << message;
	} else

	// clip info and above since it renders output unreadable

	if (AV_LOG_INFO == lvl)
	{
		// ARCS_LOG_INFO << "[FFMPEG] " << message;
	} else
	if (AV_LOG_DEBUG == lvl)
	{
		// ARCS_LOG_DEBUG << "[FFMPEG] " << message;
	} else
	if (AV_LOG_TRACE == lvl)
	{
		// ARCS_LOG(DEBUG1) << "[FFMPEG] " << message;
	} else
	{
		// If level is totally unknown, at least show it when debugging

		ARCS_LOG_DEBUG << "[FFMPEG] " << message;
	}
}

} // extern C


// forward declaration
class FFmpegAudioStream;


/**
 * \brief Loads an audio file and returns a representation as FFmpegAudioStream.
 */
class FFmpegAudioStreamLoader final
{
public:

	/**
	 * \brief Load a stream from a file with ffmpeg.
	 *
	 * \param[in] filename Filename
	 */
	std::unique_ptr<FFmpegAudioStream> load(const std::string &filename) const;

private:

	/**
	 * \brief Validates stream for CDDA compliance.
	 *
	 * \param[in] cctx The AVCodecContext to analyze
	 */
	bool validate_cdda(::AVCodecContext* cctx) const;

	/**
	 * \brief Estimate the total number of samples from the the information
	 * provided by stream and codec context.
	 *
	 * Given a constant frame size, the estimation helps to recognize the last
	 * frame. Without the estimation we could only check for a frame with a
	 * different size and consider it to be the last. With an estimation, we can
	 * check whether the sample count differs from the estimation by less than
	 * the size of one frame. This seems to ensure a "better" decision than
	 * just the comparison to the previous frame.
	 *
	 * \todo Is an estimation really required? To work correctly,
	 * Calculation has to know about the last relevant block when it encounters
	 * it. It is completely sufficient to know the correct total number of
	 * samples BEFORE flushing the last block. For this, no estimation is
	 * necessary.
	 *
	 * \param[in] cctx   The AVCodecContext to analyze
	 * \param[in] stream The AVStream to analyze
	 * \return Estimated total number of 32 bit PCM samples
	 */
	int64_t get_total_samples_declared(::AVCodecContext* cctx,
			::AVStream* stream) const;

	/**
	 * \brief Log some information about the format.
	 *
	 * \param[in] fctx The AVFormatContext to analyze
	 */
	void log_format_info(::AVFormatContext* fctx) const;

	/**
	 * \brief Log some information about the codec.
	 *
	 * \param[in] cctx The AVCodecContext to analyze
	 */
	void log_codec_info(::AVCodecContext* cctx) const;

	/**
	 * \brief Log some information about the stream.
	 *
	 * \param[in] stream The AVStream to analyze
	 */
	void log_stream_info(::AVStream* stream) const;
};


/**
 * \brief Represents an audio file opened by ffmpeg.
 *
 * Any container format is supported while it is only sensible to allow lossless
 * codecs. Therefore, the support is limited to the following codes:
 * - PCM S16LE,S16BE (WAV, AIFF)
 * - fLaC
 * - ALAC
 * - Monkey's Audio
 *
 * \todo If ffmpeg API would allow to check for lossless encoding on Wavpack
 * files, Wavpack should be added to the list
 */
class FFmpegAudioStream final
{
	friend std::unique_ptr<FFmpegAudioStream> FFmpegAudioStreamLoader::load(
			const std::string &filename) const;

public:

	// make class non-copyable
	FFmpegAudioStream (const FFmpegAudioStream &file) = delete;

	FFmpegAudioStream (FFmpegAudioStream &&file) = default;

	/**
	 * \brief Return the sample format of this file.
	 *
	 * \return The sample format of this file
	 */
	AVSampleFormat sample_format() const;

	/**
	 * \brief Number of planes.
	 *
	 * 1 for interleaved, 2 (== \c CDDA.NUMBER_OF_CHANNELS= for planar data.
	 *
	 * \return Number of planes, either 1 for interleaved or 2 for planar.
	 */
	int num_planes() const;

	/**
	 * \brief Return the channel layout of this file.
	 *
	 * \return TRUE for left0/right1, FALSE otherwise
	 */
	bool channels_swapped() const;

	/**
	 * \brief Total number of 32 bit PCM samples as declared by the stream.
	 *
	 * Note that this number may differ from the total number of samples
	 * processed. Some codecs like ALAC insert "priming frames" or
	 * "remainder frames" as a padding to fill the last frame to conform a
	 * standard frame size. At least for ALAC/CAF files FFmpeg let those
	 * padding frames contribute to the number of total samples (cf.
	 * \c cafdec.c) but does not enumerate them when decoding packets. (I never
	 * figured out how ffmpeg keeps this information after having read the CAF
	 * file.)
	 *
	 * As a consequence, total_samples_declared() will yield only an estimation
	 * of samples, i.e. the total number of samples as declared in the stream.
	 *
	 * The actual number of samples contributing to the ARCSs is returned by
	 * traverse_samples().
	 *
	 * \return Total number of 32 bit PCM samples in file (including priming and
	 * remainder frames)
	 */
	int64_t total_samples_declared() const;

	/**
	 * \brief Traverse all 16 bit samples in the file, thereby accumulating 32
	 * bit samples in a buffer and automatically flushing it once it is full.
	 *
	 * Returns the number of 32 bit samples processed. Note that this number may
	 * differ from the number returned by total_samples_declared().
	 *
	 * \return Number of 32 bit PCM samples enumerated.
	 * \throw FileReadException If an error occurrs while reading the file
	 */
	int64_t traverse_samples();

	/**
	 * \brief Traverse all 16 bit samples in the file, thereby accumulating 32
	 * bit samples in a buffer and automatically flushing it once it is full.
	 *
	 * Returns the number of samples processed. Note that this number may differ
	 * from the number returned by total_samples_declared().
	 *
	 * \return Number of 32 bit PCM samples enumerated.
	 * \throw FileReadException If an error occurrs while reading the file
	 */
	int64_t traverse_samples_old();

	/**
	 * \brief Register the append_samples() method.
	 *
	 * \param[in] func The append_samples() method to use while reading
	 */
	void register_append_samples(
		std::function<void(SampleInputIterator begin, SampleInputIterator end)>
			func);

	/**
	 * \brief Register the update_audiosize() method.
	 *
	 * \param[in] func The update_audiosize() method to use while reading
	 */
	void register_update_audiosize(
			std::function<void(const AudioSize &size)> func);

	// make class non-copyable
	FFmpegAudioStream& operator = (const FFmpegAudioStream &file) = delete;

	FFmpegAudioStream& operator = (FFmpegAudioStream &&file) = default;

private:

	/**
	 * \brief Decode a single packet completely.
	 *
	 * \param[in]  packet    The packet to decode (call-by-value ensures copy)
	 * \param[in]  frame     The frame pointer to use for decoding
	 * \param[out] samples16 Accumulative counter for 16 bit samples
	 * \param[out] frames    Accumulative counter of frames read
	 * \param[out] bytes     Accumulative counter of bytes read
	 *
	 * \return TRUE iff samples were decoded from the packet, otherwise FALSE
	 */
	bool decode_packet_old(::AVPacket packet, ::AVFrame* frame,
			int64_t* samples16, int64_t* frames, int64_t* bytes);

	/**
	 * \brief Get the index of the decoded audio stream.
	 *
	 * \return Index of the decoded audio stream.
	 */
	int stream_index() const;

	/**
	 * \brief Pass a sequence of samples to consumer.
	 *
	 * \param[in] ch0 Samples for channel 0 (all samples in non-planar layout)
	 * \param[in] ch1 Samples for channel 1 (nullptr in non-planar layout)
	 * \param[in] bytes_per_plane Number of bytes per plane
	 *
	 * \return 0 on success, otherwise non-zero error code
	 */
	int pass_samples(const uint8_t* ch0, const uint8_t* ch1,
		const int64_t bytes_per_plane) const;

	/**
	 * \brief Internal format context pointer.
	 */
	AVFormatContextPtr formatContext_;

	/**
	 * \brief Internal codec context pointer.
	 */
	AVCodecContextPtr codecContext_;

	/**
	 * \brief Index of the AVStream to be decoded.
	 */
	int stream_index_;

	/**
	 * \brief Number of planes (1 for interleaved data, 2 for planar data).
	 */
	int num_planes_;

	/**
	 * \brief TRUE indicates left0/right1, FALSE otherwise.
	 */
	bool channels_swapped_;

	/**
	 * \brief Total number of 32 bit PCM samples in the file estimated by
	 * duration.
	 */
	int64_t total_samples_declared_;

	/**
	 * \brief Callback for notifying outside world about the correct AudioSize.
	 */
	std::function<void(const AudioSize &size)> update_audiosize_;

	/**
	 * \brief Callback for notifying outside world about a new sequence of
	 * samples.
	 */
	std::function<void(SampleInputIterator begin, SampleInputIterator end)>
		append_samples_;

	/**
	 * \brief Constructor.
	 */
	FFmpegAudioStream();
};


/**
 * \brief Format and codec independent audio file reader.
 *
 * This is a AudioReader implementation by libavformat and libavcodec. It can
 * open files in virtually every combination of container and audio format that
 * ffmpeg supports.
 *
 * It is internally limited to a set of lossless codecs.
 *
 * For CDDA compliant formats, it provides 16 bit samples as int16_t and
 * therefore needs a buffer interface for this.
 */
class FFmpegAudioReaderImpl : public AudioReaderImpl
{
public:

	/**
	 * \brief Default constructor.
	 */
	FFmpegAudioReaderImpl();

	/**
	 * \brief Virtual default destructor.
	 */
	~FFmpegAudioReaderImpl() noexcept override;

private:

	std::unique_ptr<AudioSize> do_acquire_size(const std::string &filename)
		override;

	void do_process_file(const std::string &filename) override;

	std::unique_ptr<FileReaderDescriptor> do_descriptor() const override;
};


/// @}


// FFmpegAudioStreamLoader


std::unique_ptr<FFmpegAudioStream> FFmpegAudioStreamLoader::load(
		const std::string &filename) const
{
	ARCS_LOG_DEBUG << "Start to analyze audio file with ffmpeg";

#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(58, 9, 100) //  < ffmpeg 4.0

	// Initialize all formats, decoders, muxers etc...
	// This is useful for getting more information about the file, but we will
	// support only a few codecs.
	::av_register_all();
#endif

	auto fctx = open_file(filename);

	auto stream_and_codec = identify_stream(fctx.get(),
			::AVMEDIA_TYPE_AUDIO);

	const auto stream_idx = int { stream_and_codec.first };

	auto cctx = create_audio_decoder(fctx.get(), stream_idx);


	if (not IsSupported::format(cctx->sample_fmt))
	{
		std::ostringstream message;
		message << "Sample format not supported: "
			<< ::av_get_sample_fmt_name(cctx->sample_fmt);

		throw InvalidAudioException(message.str());
	}


	if (not IsSupported::codec(cctx->codec->id))
	{
		std::ostringstream message;
		message << "Codec not supported: " << cctx->codec->long_name;

		throw InvalidAudioException(message.str());
	}


	if (not this->validate_cdda(cctx.get()))
	{
		std::stringstream message;
		message << "Audio is not CDDA compliant";

		ARCS_LOG_ERROR << message.str();
		throw InvalidAudioException(message.str());
	}


	// Configure file object with ffmpeg properties

	std::unique_ptr<FFmpegAudioStream> stream = nullptr;
	{
		// Cannot use std::make_unique due to private constructor

		FFmpegAudioStream* f = new FFmpegAudioStream();
		stream.reset(f);
	}

	const auto estimated_samples = this->get_total_samples_declared(
			cctx.get(), fctx->streams[stream_idx]);
	ARCS_LOG_DEBUG << "Stream duration suggests a total amount of "
		<< estimated_samples << " samples";
	// We have to update the expected AudioSize before the last block of samples
	// is passed.
	//
	// We estimate the number of samples by the duration of the stream. We use
	// this estimation to decide at which point we start to buffer frames till
	// EOF is seen. We assume that the declaration does not yield an estimation
	// smaller than 6 frames less of the physical total samples. So, if the the
	// declared total sample amount is 6 frames away, we just start to read
	// until EOF.
	//
	// In most cases, the estimation is correct if the file is intact and the
	// codec does not use padding (priming or remainder frames). The guess is
	// reliable for PCM*, FLAC, WAVPACK, APE. It fails for ALAC. However, we
	// count the samples and correct the estimation before flushing the last
	// relevant block.

	stream->num_planes_ = ::av_sample_fmt_is_planar(cctx->sample_fmt) ? 2 : 1;

	stream->channels_swapped_ = (cctx->channel_layout != 3);
	// '3' == stereo left/right (== FL+FR).
	// Since we already have tested for having 2 channels, anything except
	// the standard layout must mean channels are swapped.

	stream->total_samples_declared_ = estimated_samples;
	stream->stream_index_ = stream_idx;


	// Log some information about the file

	// Commented out:
	// Output ffmpeg-sytle streaminfo (for debug only)
	//::av_dump_format(formatContext_, 0, filename.c_str(), 0);

	if (Logging::instance().has_level(LOGLEVEL::DEBUG))
	{
		this->log_codec_info(cctx.get());
	}
	if (Logging::instance().has_level(LOGLEVEL::DEBUG1))
	{
		this->log_format_info(fctx.get());
		//this->log_stream_info(fctx->streams[stream_idx]);
	}

	stream->codecContext_  = std::move(cctx);
	stream->formatContext_ = std::move(fctx);

	ARCS_LOG_DEBUG << "Input stream ready";

	return stream;
}


bool FFmpegAudioStreamLoader::validate_cdda(::AVCodecContext *cctx) const
{
	bool is_validated = true;

	CDDAValidator validator;

	if (not validator.bits_per_sample(
				::av_get_bytes_per_sample(cctx->sample_fmt) * CHAR_BIT))
	{
		ARCS_LOG_ERROR << "Not CDDA: not 16 bits per sample";
		is_validated = false;
	}

	if (not validator.num_channels(cctx->channels))
	{
		ARCS_LOG_ERROR << "Not CDDA: not stereo";
		is_validated = false;
	}

	if (not validator.samples_per_second(cctx->sample_rate))
	{
		ARCS_LOG_ERROR << "Not CDDA: sample rate is not 44100 Hz";
		is_validated = false;
	}

	return is_validated;
}


int64_t FFmpegAudioStreamLoader::get_total_samples_declared(
		::AVCodecContext* cctx, ::AVStream* stream) const
{
	// Deduce number of samples from duration, which should be accurate
	// if stream metadata is intact

	const double time_base =
		static_cast<double>(stream->time_base.num) /
		static_cast<double>(stream->time_base.den);

	const double duration_secs =
		static_cast<double>(stream->duration) * time_base;

	ARCS_LOG_DEBUG << "Estimate duration:       " << duration_secs << " secs";

	return duration_secs * cctx->sample_rate;
}


void FFmpegAudioStreamLoader::log_format_info(::AVFormatContext *fctx) const
{
	// Print Format Context metadata

	ARCS_LOG(DEBUG1) << "FORMAT INFORMATION:";

	::AVDictionaryEntry *tag = nullptr;

	while ((tag =
				::av_dict_get(fctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
	{
		ARCS_LOG(DEBUG1) << "  metadata Name: " << tag->key
				<< "  Value: "    << tag->value;
	}

	ARCS_LOG(DEBUG1) << "  packet_size:  "
		<< std::to_string(fctx->packet_size);
}


void FFmpegAudioStreamLoader::log_codec_info(::AVCodecContext *cctx) const
{
	// Print file audio information

	ARCS_LOG_INFO << "CODEC INFORMATION:";

	ARCS_LOG_INFO << "  Codec name:     " << cctx->codec->long_name;

	ARCS_LOG_INFO << "  Short name:     " << cctx->codec->name;

	ARCS_LOG_INFO << "  Sample format:  "
			<< ::av_get_sample_fmt_name(cctx->sample_fmt);

	// Analyze planarity / interleavedness

	{
		const bool is_planar = ::av_sample_fmt_is_planar(cctx->sample_fmt);

		ARCS_LOG_INFO << "  Is planar:      " << (is_planar ? "yes" : "no");

		if (is_planar and cctx->channels > AV_NUM_DATA_POINTERS)
		{
			// We have already ensured 2 channels by validating against CDDA.
			// Can 2 channels ever be too much??

			ARCS_LOG_INFO <<
			"Too many channels for frame->data, respect frame->extended_data";
		}
	}

	ARCS_LOG_INFO << "  Bytes/Sample:   "
			<< ::av_get_bytes_per_sample(cctx->sample_fmt)
			<< " (= "
			<< (::av_get_bytes_per_sample(cctx->sample_fmt) * CHAR_BIT)
			<< " bit)";

	ARCS_LOG_INFO << "  Channels:       " << cctx->channels;

	ARCS_LOG_INFO << "  Channel layout: " << cctx->channel_layout;

	ARCS_LOG_INFO << "  Samplerate:     " << cctx->sample_rate
			<< " Hz (samples/sec)";

	// ----

	ARCS_LOG_INFO << "  --Properties--";

	{
		// Losslessness ?

		const bool codec_prop_lossless =
			cctx->codec_descriptor->props & AV_CODEC_PROP_LOSSLESS;

		const bool codec_prop_lossy =
			cctx->codec_descriptor->props & AV_CODEC_PROP_LOSSY;

		ARCS_LOG_DEBUG << "  PROP_LOSSLESS:  "
			<< (codec_prop_lossless ? "yes" : "no");

		ARCS_LOG_DEBUG << "  PROP_LOSSY:     "
			<< (codec_prop_lossy ? "yes" : "no");

		if (codec_prop_lossy)
		{
			if (not codec_prop_lossless)
			{
				ARCS_LOG_WARNING <<
					"Codec is flagged as lossy only, bail out";
			} else
			{
				ARCS_LOG_WARNING <<
					"Codec is flagged to support lossy encoding";
				ARCS_LOG_WARNING <<
					"If you know that your file is lossless, proceed";
			}
		}
	}

	// ----

	ARCS_LOG_INFO << "  --Capabilities--";

	// Validate for format & codec:
	// losslessness and constant frame length

	{

		ARCS_LOG_DEBUG << "  Capabilities:            " <<
					cctx->codec->capabilities;

		ARCS_LOG_DEBUG << "  Capability bits:         " <<
					(sizeof(cctx->codec->capabilities) * 8);

		// Constant frame length ?

		const bool codec_cap_variable_frame_size =
			cctx->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE;

		ARCS_LOG_DEBUG << "  CAP_VARIABLE_FRAME_SIZE: "
			<< (codec_cap_variable_frame_size ? "yes" : "no");

		if (codec_cap_variable_frame_size)
		{
			ARCS_LOG_WARNING << "Codec supports variable frame size";

			// This is okay iff CUE/TOC information is already complete.
			// If we count samples, we rely on recognizing the last frame
			// by a different size. This will fail at variable frame sizes.
		}

		// Last frame smaller ?

		const bool codec_cap_small_last_frame =
			cctx->codec->capabilities & AV_CODEC_CAP_SMALL_LAST_FRAME;

		ARCS_LOG_DEBUG << "  CAP_SMALL_LAST_FRAME:    "
			<< (codec_cap_variable_frame_size ? "yes" : "no");

		if (codec_cap_small_last_frame)
		{
			ARCS_LOG_WARNING << "Codec supports smaller last frame";
		}

		// Codec is lossless ?

		// FFmpeg seems either to lie about this for flac, wavpack,
		// alac and ape or I just did not get what this capability means.
		// FFmpeg does not qualify these codecs to have this capability,
		// so it is probably useless to test for. Seems to be expressed by
		// properties instead, see above.

		const bool codec_cap_lossless = static_cast<unsigned long>
			(cctx->codec->capabilities) & AV_CODEC_CAP_LOSSLESS;

		ARCS_LOG_DEBUG << "  CAP_LOSSLESS:            "
			<< (codec_cap_lossless ? "yes" : "no");

		if (not codec_cap_lossless)
		{
			ARCS_LOG_INFO <<
				"   => Codec is not declared with lossless capability."
				<< " This is probably ok, though.";
		}

		// Analyze delay capability

		{
			const bool codec_cap_delay =
				cctx->codec->capabilities & AV_CODEC_CAP_DELAY;

			ARCS_LOG_INFO << "   CAP_DELAY:               "
				<< (codec_cap_delay ? "yes" : "no");

			//ARCS_LOG_INFO << "  => Codec buffers frames";
		}

		// Could also inspect:
		// AV_CODEC_CAP_SUBFRAMES => Codec can yield more than 1 frame per pckt
	}

	// Print Codec Context data

	{
		ARCS_LOG(DEBUG1) << "Codec Context information:";

		ARCS_LOG(DEBUG1) << "  skip_bottom:      " << cctx->skip_bottom;
		ARCS_LOG(DEBUG1) << "  frame_number:     " << cctx->frame_number;
		ARCS_LOG(DEBUG1) << "  frame_size:       " << cctx->frame_size;
		ARCS_LOG(DEBUG1) << "  initial_padding:  " << cctx->initial_padding;
		ARCS_LOG(DEBUG1) << "  trailing_padding: " << cctx->trailing_padding;

		// Commented out these logs because they are mostly unnecessary
		// for practical means, but I wanted to keep them at hand if needed.

		//ARCS_LOG(DEBUG1) << "  skip_frame:       " << cctx->skip_frame;

		//// applies for flac, alac, ape
		//ARCS_LOG(DEBUG1) << "  CAP_DR1:            "
		//	<< std::string(
		//		(codecContext_->codec->capabilities & AV_CODEC_CAP_DR1)
		//		? "yes"
		//		: "no"));

		//// applies for ape
		//ARCS_LOG(DEBUG1) << "  CAP_SUBFRAMES:      "
		//	<< std::string(
		//		(codecContext_->codec->capabilities & AV_CODEC_CAP_SUBFRAMES)
		//		? "yes"
		//		: "no"));

		//// applies for flac, alac
		//ARCS_LOG(DEBUG1) << "  CAP_FRAME_THREADS:  "
		//	<< std::string(
		//		(codecContext_->codec->capabilities &
		//			AV_CODEC_CAP_FRAME_THREADS)
		//		? "yes"
		//		: "no"));

		//ARCS_LOG_DEBUG << "  property name:      " << std::string(
		//			codecContext_->codec_descriptor->name));

		//ARCS_LOG_DEBUG << "  property long_name: " << std::string(
		//			codecContext_->codec_descriptor->long_name));
	}
}


void FFmpegAudioStreamLoader::log_stream_info(::AVStream *stream) const
{
	// Print stream metadata

	ARCS_LOG(DEBUG1) << "Stream information:";

	// stream metadata
	::AVDictionaryEntry *tag = nullptr;
	while ((tag = ::av_dict_get(stream->metadata,
					"", tag, AV_DICT_IGNORE_SUFFIX)))
	{
		ARCS_LOG(DEBUG1) << "  metadata Name: " << tag->key
				<< "  Value: "    << tag->value;
	}
	ARCS_LOG(DEBUG1) << "  initial_padding:  " <<
		stream->codecpar->initial_padding;
	ARCS_LOG(DEBUG1) << "  trailing_padding: " <<
		stream->codecpar->trailing_padding;
	ARCS_LOG(DEBUG1) << "  frame_size:       " << stream->codecpar->frame_size;
	ARCS_LOG(DEBUG1) << "  nb_side_data:     " << stream->nb_side_data;
	ARCS_LOG(DEBUG1) << "  nb_frames:        " << stream->nb_frames;

	// stream side data
	uint8_t *data = ::av_stream_get_side_data(
			stream,
			AV_PKT_DATA_SKIP_SAMPLES,
			nullptr);
	if (data)
	{
		ARCS_LOG_WARNING << "Client has to SKIP some frames! Inspect!";
	}
}


// FFmpegAudioStream


FFmpegAudioStream::FFmpegAudioStream()
	: formatContext_ { nullptr }
	, codecContext_  { nullptr }
	, stream_index_     { 0 }
	, num_planes_       { 0 }
	, channels_swapped_ { false }
	, total_samples_declared_ { 0 }
	, update_audiosize_ { }
	, append_samples_   { }
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


void FFmpegAudioStream::register_append_samples(
		std::function<void(SampleInputIterator begin,
			SampleInputIterator end)> func)
{
	append_samples_ = func;
}


void FFmpegAudioStream::register_update_audiosize(
		std::function<void(const AudioSize &size)> func)
{
	update_audiosize_ = func;
}


int FFmpegAudioStream::num_planes() const
{
	return num_planes_;
}


int64_t FFmpegAudioStream::traverse_samples_old()
{
	// This is incompatible to the old API from ffmpeg 0.9
	// introduced by libavcodec version 53.25.0 in 2011-12-11.

	// Counters

	int64_t sample16_count   = 0;
	int64_t frame_count      = 0;
	int64_t byte_count       = 0;
	int64_t packet_count     = 0;

	// Pointer to current frame

	::AVFrame* frame = ::av_frame_alloc();

	if (!frame)
	{
		throw FileReadException("Error allocating frame pointer");
	}

	// Read packets

	::AVPacket packet;
	::av_init_packet(&packet);
	bool got_samples = true;

	ARCS_LOG_DEBUG << "START READING SAMPLES";

	while (::av_read_frame(formatContext_.get(), &packet) == 0)
	{
		// Respect only packets from the specified stream

		if (packet.stream_index != this->stream_index())
		{
			::av_packet_unref(&packet);

			continue;
		}

		++packet_count;

		// Note: ffmpeg-API requires packet to be copied, we ensure that doing
		// a pass-by-value
		got_samples = this->decode_packet_old(packet, frame,
				&sample16_count, &frame_count, &byte_count);

		if (not got_samples)
		{
			::av_packet_unref(&packet);
			::av_free(frame);

			ARCS_LOG_DEBUG << "Expected samples but did not get any";

			return sample16_count / 2;
		}

		::av_packet_unref(&packet);
	} // while ::av_read_frame

	::av_free(frame);
	const auto total_samples = sample16_count / 2;

	// Some codecs (Monkey's Audio for example) will cause frames to be buffered
	// up in the decoding process. If there are buffered up frames that have
	// not yet been processed, the buffer needs to be flushed. Otherwise, those
	// samples will not contribute to the checksum.

	if (AV_CODEC_CAP_DELAY & codecContext_->codec->capabilities)
	{
		ARCS_LOG_DEBUG << "Codec is declared to have delay capability, so "
			"check for delayed frames";

		// https://ffmpeg.org/doxygen/4.1/group__lavc__encdec.html

		// Enter "draining mode"
		if (::avcodec_send_packet(codecContext_.get(), nullptr) < 0)
		{
			ARCS_LOG_DEBUG << "Codec does not seem to have any delayed frames";

			return total_samples;
		}

		::AVFrame* f = ::av_frame_alloc();

		if (!f)
		{
			throw std::runtime_error("Error allocating frame pointer");
		}

		int64_t total_frames_delayed = 0;
		int samples_in_frame = 0;

		int result = 1;
		do
		{
			result = ::avcodec_receive_frame(codecContext_.get(), f);

			if (result == AVERROR_EOF)
			{
				ARCS_LOG_DEBUG << "EOF reached while draining delayed frames";
				break;
			}

			// Pass frame to calculation

			samples_in_frame = f->nb_samples * CDDA.NUMBER_OF_CHANNELS;

			// FIXME This is buggy, the AudioSize MUST have been updated BEFORE
			// sending these samples. Otherwise, libarcstk will get "last part"
			// twice and recognize that it cannot save the last track because
			// it already was saved (prematurely).

			this->pass_samples(f->data[0], f->data[1],
				(num_planes() == 2 ? 1 : 2) * samples_in_frame);

			++total_frames_delayed;

		} while (result >= 0);

		::av_free(f);

		if (total_frames_delayed)
		{
			ARCS_LOG_DEBUG << "Passed "
				<< total_frames_delayed << " more (delayed) frames";
		} else
		{
			ARCS_LOG_DEBUG << "No delayed frames recognized";
		}
	}

	// Log some information

	ARCS_LOG_DEBUG << "Reading finished";
	ARCS_LOG_DEBUG << "  Samples(32): " << total_samples;
	ARCS_LOG_DEBUG << "  Samples(16): " << sample16_count;
	ARCS_LOG_DEBUG << "  Bytes:       " << byte_count;
	ARCS_LOG_DEBUG << "  Frames:      " << frame_count;
	ARCS_LOG_DEBUG << "  Packets:     " << packet_count;

	return total_samples;
}


bool FFmpegAudioStream::decode_packet_old(::AVPacket packet, ::AVFrame *frame,
		int64_t *samples16, int64_t *frames, int64_t *bytes)
{
	// This is incompatible to the old API from ffmpeg 0.9
	// introduced by libavcodec version 53.25.0 in 2011-12-11.

	// Decode current packet

	if (::avcodec_send_packet(codecContext_.get(), &packet) < 0)
	{
		::av_log(nullptr, AV_LOG_WARNING, "Error sending packet to decoder\n");
		return false;
	}

	int64_t samples16_in_packet = 0;
	int64_t frames_in_packet    = 0;

	// Track the frame size to recognize last frame
	// (assumes fixed frame length)

	static int      frame_size       = 0;
	static uint32_t frame_sz_changed = 0xFFFFFFFF; // 1st frame wraps this to 0

	// Decode all frames in packet

	int result = 1;
	int samples_in_frame = 0;

	while (result >= 0)
	{
		// PROBLEM HERE is that if file declares less samples than are actually
		// present, the size cannot be updated before the the presumably last
		// block is passed since we see the frames one by one without a
		// lookahead. We just won't know whether there are frames to follow and
		// libarcstk will finish the calculation too early!

		result = ::avcodec_receive_frame(codecContext_.get(), frame);

		if (result < 0) // some error occurred
		{
			if (AVERROR(EAGAIN) == result)
			{
				// Decoder requires more input packets before it can provide
				// any frames, so just finish the processing of this packet
				// and provide next packet to decoder.

				break;
			}

			if (AVERROR_EOF == result) // Unexpected end of file
			{
				::av_log(nullptr, AV_LOG_ERROR, "Unexpected end of file\n");
				break;
			}

			::av_log(nullptr, AV_LOG_ERROR, "Error receiving frame\n");
			break;
		}

		++frames_in_packet;

		samples_in_frame = frame->nb_samples * CDDA.NUMBER_OF_CHANNELS;
		// Not num_planes() since nb_samples is the number of samples
		// per channel and we always have 2 channels (stereo),
		// regardless whether each channel has a separate plane.

		// For audio in general, linesize will only be defined for the first
		// plane (linesize[0]) since the planes have to be of identical size.

		// For planar audio, the planes are just channels.

		samples16_in_packet += samples_in_frame;
		*bytes              += samples_in_frame * 2/*bytes per 16-bit-sample*/;
		// Note: linesize[0] counts bytes but may be misleading since it counts
		// also the padding bytes!

		// Track frame size to recognize last frame.
		// Correct estimated number of total samples with counted samples.
		// If TOC was complete, this should not be necessary, though.

		if (frame_size != frame->nb_samples)
		{
			++frame_sz_changed;

			if (frame_sz_changed) // frame size changed: check for last frame
			{
				ARCS_LOG_DEBUG << "Frame size changed from "
						<< frame_size
						<< " to "
						<< frame->nb_samples
						<< " (samples).";

				// This is the real total samples respected so far

				int64_t samples32_counted =
					(*samples16 + samples16_in_packet) / 2;

				// diff estimated total samples32 vs. counted samples32
				int64_t total_diff =
					total_samples_declared() - samples32_counted;

				ARCS_LOG_DEBUG << "Counted "
						<< samples32_counted << " after expecting "
						<< total_samples_declared() << " (error of "
						<< total_diff << ").";

				// Assuming that the estimation will not deviate by more than
				// one frame, check if we are "near" to the end of the stream
				if (std::abs(total_diff) <= frame_size)
				{
					// diff size of current frame against previous frame sizes
					int32_t frame_diff = frame_size - frame->nb_samples;
					// Variable frame_diff counts samples16. Therefore,
					// total_diff and frame_diff are not comparable for equality
					// in case of non-planar data.

					ARCS_LOG(DEBUG1) << "EXPECT LAST FRAME";
					ARCS_LOG(DEBUG1) << "  index: "
						<< (*frames + frames_in_packet);
					ARCS_LOG(DEBUG1) << "  size:  "
						<< frame->nb_samples << " samples of 16 bit";
					ARCS_LOG(DEBUG1) << "  previous frame size: "
						<< frame_size;
					ARCS_LOG(DEBUG1) << "  diff to previous frame (16): "
						<< frame_diff;

					ARCS_LOG(DEBUG1)
						<< "  total diff counted samples (32):   "
						<< total_diff;

					// Update outside world with corrected total samples

					if (samples32_counted != this->total_samples_declared())
					{
						AudioSize newsize;
						newsize.set_total_samples(samples32_counted);

						// Note: Equivalent with
						// this->total_samples_declared() - total_diff;

						ARCS_LOG_INFO << "Update total number of samples to: "
								<< newsize.total_samples();

						update_audiosize_(newsize);
					}
				}
			}

			// Note: if the last frame is of the same size as all frames before,
			// this heuristic will fail, resulting in not correcting the
			// estimated total samples. If the last frame is completely filled
			// with valid frames, the estimation should be correct, though.
			// Nonetheless, the duration info may be broken.

			frame_size = frame->nb_samples;
		}

		// Push sample sequence to processing/buffering
		this->pass_samples(frame->data[0], frame->data[1],
				(num_planes() == 2 ? 1 : 2) * samples_in_frame);
		// Bytes per plane, not bytes per channel! If we have a single
		// plane, this plane contains twice as much bytes as each of
		// the two planes in a planar codec.
	}

	*frames    += frames_in_packet;
	*samples16 += samples16_in_packet;

	return samples16_in_packet > 0;
}


int64_t FFmpegAudioStream::traverse_samples()
{
	PacketQueue queue;
	const auto avg_queue_size = decltype( queue )::size_type { 12 };

	auto samples_in_frame = int32_t { 0 };
	auto total_samples    = int32_t { 0 };

	// Fill queue to its average size

	queue.set_source(formatContext_.get(), stream_index());
	queue.set_decoder(codecContext_.get());

	while (queue.size() < avg_queue_size)
	{
		ARCS_LOG_DEBUG << "Enqueue frame";

		if (not queue.enqueue_frame())
		{
			// TODO EOF reached
			break;
		}
	}

	ARCS_LOG_DEBUG << "Loaded " << queue.size() << "packets to queue";

	// Manage queue while new frames are available

	auto frame = AVFramePtr { nullptr };

	while ((frame = queue.dequeue_frame()))
	{
		samples_in_frame = frame->nb_samples * CDDA.NUMBER_OF_CHANNELS;

		this->pass_samples(frame->data[0], frame->data[1],
				(num_planes() == 2 ? 1 : 2) * samples_in_frame);

		total_samples += samples_in_frame;

		if (not queue.enqueue_frame())
		{
			// TODO EOF reached
			break;
		}
	}

	// Decode last frames to get size for update

	std::vector<AVFramePtr> frames;
	frames.reserve(queue.size());

	while ((frame = queue.dequeue_frame()))
	{
		samples_in_frame = frame->nb_samples * CDDA.NUMBER_OF_CHANNELS;
		total_samples += samples_in_frame;

		frames.push_back(std::move(frame));
	}
	total_samples /= 2;

	// Update audiosize

	AudioSize newsize;
	newsize.set_total_samples(total_samples);

	ARCS_LOG_INFO << "Update total number of samples to: "
		<< newsize.total_samples();

	update_audiosize_(newsize);

	// Pass last samples

	for (auto& f : frames)
	{
		samples_in_frame = f->nb_samples * CDDA.NUMBER_OF_CHANNELS;

		this->pass_samples(f->data[0], f->data[1],
				(num_planes() == 2 ? 1 : 2) * samples_in_frame);
	}

	return total_samples;
}


int FFmpegAudioStream::pass_samples(const uint8_t* ch0, const uint8_t* ch1,
		const int64_t bytes_per_plane) const
{
	// Note:: ffmpeg "normalizes" the channel ordering away, so we will ignore
	// it and process anything als left0/right1

	// TODO Find less ugly implementation

	if (::AV_SAMPLE_FMT_S16P == this->sample_format())
	{
		SampleSequence<int16_t, true> sequence;
		sequence.wrap_bytes(ch0, ch1, static_cast<uint64_t>(bytes_per_plane));

		append_samples_(sequence.begin(), sequence.end());

		return 0;
	}

	if (::AV_SAMPLE_FMT_S16  == this->sample_format())
	{
		SampleSequence<int16_t, false> sequence;
		sequence.wrap_bytes(ch0, static_cast<uint64_t>(bytes_per_plane));

		append_samples_(sequence.begin(), sequence.end());

		return 0;
	}

	if (::AV_SAMPLE_FMT_S32P == this->sample_format()) // e.g. flac reader
	{
		SampleSequence<int32_t, true> sequence;
		sequence.wrap_bytes(ch0, ch1, static_cast<uint64_t>(bytes_per_plane));

		append_samples_(sequence.begin(), sequence.end());

		return 0;
	}

	if (::AV_SAMPLE_FMT_S32  == this->sample_format()) // e.g. wavpack reader
	{
		SampleSequence<int32_t, false> sequence;
		sequence.wrap_bytes(ch0, static_cast<uint64_t>(bytes_per_plane));

		append_samples_(sequence.begin(), sequence.end());

		return 0;
	}

	return 1;
}


// FFmpegAudioReaderImpl


FFmpegAudioReaderImpl::FFmpegAudioReaderImpl()
	: AudioReaderImpl()
{
	// empty
}


FFmpegAudioReaderImpl::~FFmpegAudioReaderImpl() noexcept = default;


std::unique_ptr<AudioSize> FFmpegAudioReaderImpl::do_acquire_size(
	const std::string &filename)
{
	std::unique_ptr<AudioSize> audiosize =
		std::make_unique<AudioSize>();

	FFmpegAudioStreamLoader loader;
	auto audiofile = loader.load(filename);
	audiosize->set_total_samples(audiofile->total_samples_declared());

	return audiosize;
}


void FFmpegAudioReaderImpl::do_process_file(const std::string &filename)
{
	// Redirect ffmpeg logging to arcs logging

	::av_log_set_callback(arcs_av_log);


	// Plug file, buffer and processor together

	FFmpegAudioStreamLoader loader;
	auto audiofile = loader.load(filename);

	if (audiofile->channels_swapped())
	{
		ARCS_LOG_INFO << "FFmpeg says, channels are swapped.";
	}


	// Provide estimation

	AudioSize size;
	size.set_total_samples(audiofile->total_samples_declared());
	this->process_audiosize(size);


	// Register this AudioReaderImpl instance as the target for samples and
	// metadata updates

	audiofile->register_append_samples(
		std::bind(&FFmpegAudioReaderImpl::process_samples,
			this,
			std::placeholders::_1, std::placeholders::_2));

	audiofile->register_update_audiosize(
		std::bind(&FFmpegAudioReaderImpl::process_audiosize,
			this,
			std::placeholders::_1));


	// Process file

	auto total_samples_expected { size.total_samples() };
	ARCS_LOG_DEBUG << "Start traversing samples";
	auto total_samples          { audiofile->traverse_samples() };


	// Do some logging

	ARCS_LOG_DEBUG << "Respected samples: " << total_samples;

	if (total_samples != total_samples_expected)
	{
		ARCS_LOG_INFO << "Expected " << total_samples_expected
					<< " samples, but encountered " << total_samples
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


} // namespace


// DescriptorFFmpeg


DescriptorFFmpeg::~DescriptorFFmpeg() noexcept = default;


std::string DescriptorFFmpeg::do_name() const
{
	return "FFmpeg";
}


LibInfo  DescriptorFFmpeg::do_libraries() const
{
	using details::find_lib;
	using details::libarcsdec_libs;

	return { { "libavformat", find_lib(libarcsdec_libs(), "libavformat") },
			 { "libavcodec" , find_lib(libarcsdec_libs(), "libavcodec")  },
			 { "libavutil"  , find_lib(libarcsdec_libs(), "libavutil")   }
	};
}


bool DescriptorFFmpeg::do_accepts_bytes(
		const std::vector<unsigned char> & /* bytes */,
		const uint64_t & /* offset */) const
{
	return true;
}


bool DescriptorFFmpeg::do_accepts_name(const std::string &/* filename */)
	const
{
	return true;
}


bool DescriptorFFmpeg::do_accepts(Codec codec) const
{
	return codecs().find(codec) != codecs().end();
}


std::set<Codec> DescriptorFFmpeg::do_codecs() const
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
		Codec::ALAC,
		Codec::WMALOSSLESS
	};
}


bool DescriptorFFmpeg::do_accepts(Format format) const
{
	return is_audio_format(format) and format != Format::WV;
}


std::set<Format> DescriptorFFmpeg::do_formats() const
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
		Format::AIFF,
		Format::WMA
	};
}


std::unique_ptr<FileReader> DescriptorFFmpeg::do_create_reader() const
{
	auto impl = std::make_unique<FFmpegAudioReaderImpl>();

	return std::make_unique<AudioReader>(std::move(impl));
}


std::unique_ptr<FileReaderDescriptor> DescriptorFFmpeg::do_clone() const
{
	return std::make_unique<DescriptorFFmpeg>();
}


// Add this descriptor to the audio descriptor registry

namespace {

const auto d = RegisterAudioDescriptor<DescriptorFFmpeg>();

} // namespace

} // namespace v_1_0_0

} // namespace arcsdec

