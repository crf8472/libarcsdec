/**
 * \file
 *
 * \brief Implements ffmpeg-based generic audio reader.
 */

#ifndef __LIBARCSDEC_READERFFMPEG_HPP__
#include "readerffmpeg.hpp"
#endif

#include <algorithm>  // for remove
//#include <chrono>     // for debugging
#include <climits>    // for CHAR_BIT
#include <cstdlib>    // for abs
//#include <ctime>      // for debugging
#include <functional> // for function
#include <stdexcept>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavcodec/version.h>
#include <libavformat/avformat.h>
#include <libavformat/version.h>
#include <libavutil/avutil.h>
#include <libavutil/version.h>
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
 * \relatesalso FFmpegAudioFile
 * \relatesalso FFmpegFileLoader
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


/**
 * \brief List of supported sample formats.
 */
class FFmpegSampleFormatList final
{

public:

	/**
	 * \brief Informs about whether a sample format is supported.
	 *
	 * \param[in] id Id of the sample format
	 *
	 * \return The SAMPLE_FORMAT to be used, or 0 if unsupported
	 */
	SAMPLE_FORMAT support(const ::AVSampleFormat &id);
};


/**
 * \brief List of supported codecs.
 *
 * The list of supported codecs and sample formats contains FLAC, ALAC,
 * APE as well as the PCM formats PCM_S16BE, PCM_S16LE and PCM_S16LE_PLANAR.
 *
 * \todo The list does not contain WavPack, since the FFmpeg-API seems not to
 * provide a way to not check whether the actual wavpack file is marked as
 * losslessly compressed. Also a file marked as losslessly compressed may
 * actually be created from lossy input, but a file marked as lossy compressed
 * must actually be refused since the ARCSs are guaranteed to be irrelevant and
 * misleading.
 *
 * \todo Add support for more PCM formats, for example there could be
 * AV_CODEC_ID_PCM_U16LE, AV_CODEC_ID_PCM_U16BE, AV_CODEC_ID_PCM_S32LE,
 * AV_CODEC_ID_PCM_S32BE, AV_CODEC_ID_PCM_U32LE, AV_CODEC_ID_PCM_U32BE
 * and maybe more if reasonable.
 */
class FFmpegCodecList final
{

public:

	/**
	 * \brief Informs about whether a codec is supported
	 *
	 * \param[in] id Id of the codec
	 *
	 * \return TRUE if the codec with the given id is supported, otherwise FALSE
	 */
	bool support(const ::AVCodecID &id);
};


// forward declaration
class FFmpegAudioFile;


/**
 * \brief Loads an audio file and returns a representation as FFmpegAudioFile.
 */
class FFmpegFileLoader final
{

public:

	/**
	 * \brief Load a file with ffmpeg.
	 *
	 * \param[in] filename The file to load
	 */
	std::unique_ptr<FFmpegAudioFile> load(const std::string &filename) const;


private:

	/**
	 * \brief Acquire an AVFormatContext for the specified file.
	 *
	 * \param[in] filename Name of the file to load
	 * \return The AVFormatContext for this file
	 * \throw FileReadException If no AVFormatContext could be acquired
	 */
	::AVFormatContext* acquire_format_context(const std::string &filename)
		const;

	/**
	 * \brief Identify the AVCodec and the relevant AudioStream
	 *
	 * \param[out] stream_idx Index of the audio stream of interest
	 * \param[in]  fctx The AVFormatContext to work with
	 * \return The codec for this stream
	 * \throw FileReadException If no AVCodec could be acquired
	 */
	::AVCodec* identify_stream_and_codec(int* stream_idx,
			::AVFormatContext* fctx) const;

	/**
	 * \brief Allocate and initialize the AVCodecContext
	 *
	 * \param[in] codec AVCodec to create context for
	 * \param[in] stream The stream to derive the AVCodecContext from
	 * \return The codec context
	 * \throw FileReadException If no AVCodecContext could be acquired
	 */
	::AVCodecContext* alloc_and_init_codec_context(
			::AVCodec* codec, ::AVStream* stream) const;

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
	int64_t estimate_total_samples(::AVCodecContext* cctx, ::AVStream* stream)
		const;

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
class FFmpegAudioFile final
{
	friend std::unique_ptr<FFmpegAudioFile> FFmpegFileLoader::load(
			const std::string &filename) const;

public:

	// make class non-copyable
	FFmpegAudioFile (const FFmpegAudioFile &file) = delete;

	// TODO Move constructor

	/**
	 * \brief Default destructor.
	 */
	virtual ~FFmpegAudioFile() noexcept;

	/**
	 * \brief Return total number of 32 bit PCM samples in file.
	 *
	 * Note that this number may differ from the total number of samples
	 * processed. Some codecs like ALAC insert "remainder frames" as a padding
	 * to fill the last frame to conform a standard frame size. At least for
	 * ALAC/CAF files FFmpeg let those remainder frames contribute to the number
	 * of total samples (cf. cafdec.c) but does not enumerate them when decoding
	 * packets. (I never figured out how ffmpeg keeps this information after
	 * having read the CAF file.) As a consequence, total_samples() will yield
	 * only an estimation until traverse_samples() was called. Thereafter it
	 * will know the factual number of samples contributing to the ARCSs.
	 *
	 * \return Total number of 32 bit PCM samples in file (including priming and
	 * remainder frames)
	 */
	int64_t total_samples() const;

	/**
	 * \brief Return the sample format of this file.
	 *
	 * \return The sample format of this file
	 */
	SAMPLE_FORMAT sample_format() const;

	/**
	 * \brief Return the channel layout of this file.
	 *
	 * \return TRUE for left0/right1, FALSE otherwise
	 */
	bool channels_swapped() const;

	/**
	 * \brief Traverse all 16 bit samples in the file, thereby accumulating 32
	 * bit samples in a buffer and automatically flushing it once it is full.
	 *
	 * Returns the number of samples processed. Note that this number may differ
	 * from the number initially returned by total_samples(). Once
	 * traverse_samples() is called, total_samples() will yield always the
	 * identical number thereafter.
	 *
	 * \return Number of 32 bit PCM samples enumerated.
	 * \throw FileReadException If an error occurrs while reading the file
	 */
	int64_t traverse_samples();

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
	FFmpegAudioFile& operator = (const FFmpegAudioFile &file) = delete;

	// TODO Move assignment


protected:

	/**
	 * \brief Set the number of total samples
	 *
	 * \param[in] total_samples The new number of total PCM 32bit stereo samples
	 */
	void set_total_samples(int64_t total_samples);


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
	bool decode_packet(::AVPacket packet, ::AVFrame* frame,
			int64_t* samples16, int64_t* frames, int64_t* bytes);

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
		const int64_t bytes_per_plane);

	/**
	 * \brief Internal format context pointer.
	 */
	::AVFormatContext* formatContext_;

	/**
	 * \brief Internal codec context pointer.
	 */
	::AVCodecContext* codecContext_;

	/**
	 * \brief Internal pointer to the audio stream.
	 */
	::AVStream* audioStream_;

	/**
	 * \brief Total number of 32 bit PCM samples in the file.
	 *
	 * The actual value may
	 * be an estimation and may deviate from the factual total number of
	 * samples. This will occurr for files for which padding frames contribute
	 * to the duration or in cases where the duration or time base is broken.
	 */
	int64_t total_samples_;

	/**
	 * \brief Number of planes
	 * (1 for interleaved data, CDDA.NUMBER_OF_CHANNELS for planar data)
	 */
	int num_planes_;

	/**
	 * \brief Sample format of this file.
	 */
	SAMPLE_FORMAT format_;

	/**
	 * \brief True indicates left0/right1, false otherwise.
	 */
	bool channels_swapped_;

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
	FFmpegAudioFile();
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


// FFmpegSampleFormatList


SAMPLE_FORMAT FFmpegSampleFormatList::support(const ::AVSampleFormat &fmt)
{
	// TODO Ugly if-else's, replace with something less ugly

	if (::AV_SAMPLE_FMT_S16P == fmt)
	{
		return SAMPLE_FORMAT::S16P;
	} else
	if (::AV_SAMPLE_FMT_S16 == fmt)
	{
		return SAMPLE_FORMAT::S16;
	} else
	if (::AV_SAMPLE_FMT_S32P == fmt)
	{
		return SAMPLE_FORMAT::S32P;
	} else
	if (::AV_SAMPLE_FMT_S32  == fmt)
	{
		return SAMPLE_FORMAT::S32;
	}

	return SAMPLE_FORMAT::UNKNOWN;
}


// FFmpegCodecList


bool FFmpegCodecList::support(const ::AVCodecID &id)
{
	switch (id)
	{
		case ::AV_CODEC_ID_ALAC:

		case ::AV_CODEC_ID_APE:

		case ::AV_CODEC_ID_FLAC:

		case ::AV_CODEC_ID_PCM_S16BE: // AIFF

		case ::AV_CODEC_ID_PCM_S16LE: // RIFF/WAV

		/* TODO untested */ case ::AV_CODEC_ID_PCM_S16BE_PLANAR:

		/* TODO untested */ case ::AV_CODEC_ID_PCM_S16LE_PLANAR:

		//case ::AV_CODEC_ID_WAVPACK: // Removed: could not check for lossy

		// TODO WMALOSSLESS should be respected

			return true;

		default:
			break;
	}

	return false;
}


// FFmpegFileLoader


std::unique_ptr<FFmpegAudioFile> FFmpegFileLoader::load(
		const std::string &filename) const
{
	ARCS_LOG_DEBUG << "Start to analyze audio file with ffmpeg";

#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(58, 9, 100) //  < ffmpeg 4.0

	// Initialize all formats, decoders, muxers etc...
	// This is useful for getting more information about the file, but we will
	// support only a few codecs.
	::av_register_all();
#endif

	// This is what we need for the FFmpegAudioFile

	::AVFormatContext* format_ctx   = nullptr;
	::AVStream*        audio_stream = nullptr;
	::AVCodecContext*  codec_ctx    = nullptr;

	// Acquire AVFormatContext

	try
	{
		format_ctx = this->acquire_format_context(filename);

	} catch (const std::exception& e)
	{
		if (format_ctx)
		{
			::avformat_close_input(&format_ctx);
			::av_free(format_ctx);
		}

		//ARCS_LOG_ERROR << "Failed to acquire format context for file "
		//	<< filename.c_str();

		throw;
	}

	// Acquire AVCodec and AVStream
	// Allocate and initialize AVCodecContext

	try
	{
		int stream_idx;
		::AVCodec* codec = nullptr;

		codec        = this->identify_stream_and_codec(&stream_idx, format_ctx);
		audio_stream = format_ctx->streams[stream_idx];
		codec_ctx    = this->alloc_and_init_codec_context(codec, audio_stream);

	} catch (const std::exception& e)
	{
		if (codec_ctx)
		{
			::avcodec_close(codec_ctx);
			::avcodec_free_context(&codec_ctx);
		}

		if (format_ctx)
		{
			::avformat_close_input(&format_ctx);
			::av_free(format_ctx);
		}

		//ARCS_LOG_ERROR << "Failed to acquire codec and stream information "
		//	<< "for file " << filename;

		throw;
	}

	// Is the identified sample format supported?

	SAMPLE_FORMAT format;
	{
		FFmpegSampleFormatList formats;

		format = formats.support(codec_ctx->sample_fmt);

		if (SAMPLE_FORMAT::UNKNOWN == format)
		{
			std::stringstream message;
			message << "Sample format not supported: "
				<< ::av_get_sample_fmt_name(codec_ctx->sample_fmt);

			ARCS_LOG_ERROR << message.str();
			throw FileReadException(message.str());
		}
	}

	// Is the identified codec supported?

	{
		FFmpegCodecList allow;

		if (not allow.support(codec_ctx->codec->id))
		{
			std::stringstream message;
			message << "Codec not supported: " << codec_ctx->codec->long_name;

			ARCS_LOG_ERROR << message.str();
			throw FileReadException(message.str());
		}

		// Instead of using a whitelist we could allow each codec with
		// PROP_LOSSLESS that does not have PROP_LOSSY.

		// To further allow codecs with PROP_LOSSLESS that also have PROP_LOSSY
		// would require us to safely recognize whether the particular file to
		// analyze was losslessly compressed. At least for wavpack, this does
		// not seem to be currently possible by ffmpeg only. (libwavpack can
		// of course do this easily.)
	}

	// CDDA ?

	if (not this->validate_cdda(codec_ctx))
	{
		std::stringstream message;
		message << "Audio is not CDDA compliant";

		ARCS_LOG_ERROR << message.str();
		throw InvalidAudioException(message.str());
	}

	// Configure file object with ffmpeg properties

	std::unique_ptr<FFmpegAudioFile> file = nullptr;
	{
		// Cannot use std::make_unique due to private constructor

		FFmpegAudioFile* f = new FFmpegAudioFile();
		file.reset(f);
	}

	file->formatContext_ = format_ctx;
	file->audioStream_   = audio_stream;
	file->codecContext_  = codec_ctx;
	file->num_planes_    = ::av_sample_fmt_is_planar(codec_ctx->sample_fmt)
		? CDDA.NUMBER_OF_CHANNELS
		: 1 ;
	file->format_  = format;

	file->channels_swapped_ = (codec_ctx->channel_layout != 3);
	// '3' == stereo left/right (== FL+FR).
	// Since we already have tested for having 2 channels, anything except
	// the standard layout must mean channels are swapped.

	file->total_samples_ = this->estimate_total_samples(
		codec_ctx, audio_stream);
	// This is probably not sensible. In most cases, the estimation is correct
	// if the file is intact and the codec does not use padding (priming or
	// remainder frames). The guess is reliable for PCM*, FLAC, WAVPACK, APE.
	// It fails for ALAC. However, we count the samples and correct the
	// estimation before flushing the last relevant block.
	// NOTE This requires:
	// - no block is smaller than the smallest frame (say: 8 MB)
	// - frame length is constant (we check that)
	// But these are considerable costs. The requirement is _only_ to notify
	// the BlockCreator immediately _before_ the last block is passed.


	// Log some information about the file

	// Commented out:
	// Output ffmpeg-sytle streaminfo (for debug only)
	//::av_dump_format(formatContext_, 0, filename.c_str(), 0);

	if (Logging::instance().has_level(LOGLEVEL::DEBUG))
	{
		this->log_codec_info(codec_ctx);
	}
	if (Logging::instance().has_level(LOGLEVEL::DEBUG1))
	{
		this->log_format_info(format_ctx);
		this->log_stream_info(audio_stream);
	}

	return file;
}


AVFormatContext* FFmpegFileLoader::acquire_format_context(
		const std::string &filename) const
{
	// Open input stream of the file and read the header

	::AVFormatContext* ctx  = nullptr;
	::AVInputFormat* detect = nullptr; // TODO Just a stub
	::AVDictionary* options = nullptr; // TODO Just a stub

	if (::avformat_open_input(&ctx , filename.c_str(), detect, &options) != 0)
	{
		std::stringstream message;
		message << "Error opening audio file: " << filename;

		ARCS_LOG_ERROR << message.str();
		throw FileReadException(message.str());
	}

	// Read some packets to acquire information about the streams
	// (This is useful for formats without a header)

	if (::avformat_find_stream_info(ctx, nullptr) < 0)
	{
		::avformat_close_input(&ctx);

		std::stringstream message;
		message << "Could not acquire stream info in file: " << filename;

		ARCS_LOG_ERROR << message.str();
		throw FileReadException(message.str());
	}

	return ctx;
}


AVCodec* FFmpegFileLoader::identify_stream_and_codec(int* stream_index,
		::AVFormatContext* fctx) const
{
	// Determine the audio stream and prepare the matching codec

	::AVCodec* codec = nullptr;

	*stream_index = ::av_find_best_stream(
		fctx, ::AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);

	if (*stream_index < 0)
	{
		std::string message("No audio stream found in file");

		ARCS_LOG_ERROR << message;
		throw FileReadException(message);
	}

	if (!codec)
	{
		std::stringstream message;
		message << "No codec found for audio stream " << *stream_index;

		ARCS_LOG_ERROR << message.str();
		throw FileReadException(message.str());
	}

	// Commented out:
	// Alternate method to create the codec when already having the AVStream
	// AVCodec* codec = ::avcodec_find_decoder(audio_stream->codecpar->codec_id);
	// but it seems ok to just use av_find_best_stream:
	// https://stackoverflow.com/a/39917045

	return codec;
}


AVCodecContext* FFmpegFileLoader::alloc_and_init_codec_context(::AVCodec* codec,
		::AVStream* stream) const
{
	// Allocate CodecContext

	::AVCodecContext* cctx = ::avcodec_alloc_context3(nullptr);

	if (!cctx)
	{
		std::string message("Failed to allocate codec context");

		ARCS_LOG_ERROR << message;
		throw FileReadException(message);
	}

	// Initialize CodecContext: copy codec parameters from stream to context

	if (::avcodec_parameters_to_context(cctx, stream->codecpar) < 0)
	{
		::avcodec_free_context(&cctx);

		std::string message(
			"Failed to copy stream parameters to codec context");

		ARCS_LOG_ERROR << message;
		throw FileReadException(message);
	}

	// Open CodecContext

	if (::avcodec_open2(cctx, codec, nullptr) != 0)
	{
		::avcodec_close(cctx);
		::avcodec_free_context(&cctx);

		std::string message("Failed to open codec context");

		ARCS_LOG_ERROR << message;
		throw FileReadException(message);
	}

	return cctx;
}


bool FFmpegFileLoader::validate_cdda(::AVCodecContext *ctx) const
{
	// Validate for CDDA

	CDDAValidator validator;

	if (::av_get_bytes_per_sample(ctx->sample_fmt) < 0)
	{
		ARCS_LOG_ERROR << "Could not validate CDDA: negative bits per sample";
		return false;
	}

	if (not validator.bits_per_sample(
				::av_get_bytes_per_sample(ctx->sample_fmt) * CHAR_BIT))
	{
		ARCS_LOG_ERROR << "Not CDDA: not 16 bits per sample";
		return false;
	}


	if (ctx->channels < 0)
	{
		ARCS_LOG_ERROR
			<< "Could not validate CDDA: negative number of channels";
		return false;
	}

	if (not validator.num_channels(ctx->channels))
	{
		ARCS_LOG_ERROR << "Not CDDA: not stereo";
		return false;
	}


	if (ctx->sample_rate < 0)
	{
		ARCS_LOG_ERROR << "Could not validate CDDA: negative sample rate";
		return false;
	}

	if (not validator.samples_per_second(ctx->sample_rate))
	{
		ARCS_LOG_ERROR << "Not CDDA: sample rate is not 44100 Hz";
		return false;
	}

	return true;
}


int64_t FFmpegFileLoader::estimate_total_samples(::AVCodecContext* cctx,
		::AVStream* stream) const
{
	int64_t total_samples = 0;

	{
		// Deduce number of samples from duration, which should be accurate
		// if stream metadata is intact

		double time_base =
			static_cast<double>(stream->time_base.num) /
			static_cast<double>(stream->time_base.den);

		double duration_secs =
			static_cast<double>(stream->duration) * time_base;

		ARCS_LOG_DEBUG << "Estimate duration:       " << duration_secs
			<< " secs";

		total_samples = duration_secs * cctx->sample_rate;
	}

	ARCS_LOG_INFO << "Estimate total samples:  " << total_samples;

	return total_samples;
}


void FFmpegFileLoader::log_format_info(::AVFormatContext *ctx) const
{
	// Print Format Context metadata

	ARCS_LOG(DEBUG1) << "FORMAT INFORMATION:";

	::AVDictionaryEntry *tag = nullptr;

	while ((tag = ::av_dict_get(ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
	{
		ARCS_LOG(DEBUG1) << "  metadata Name: " << tag->key
				<< "  Value: "    << tag->value;
	}

	ARCS_LOG(DEBUG1) << "  packet_size:  "
		<< std::to_string(ctx->packet_size);
}


void FFmpegFileLoader::log_codec_info(::AVCodecContext *ctx) const
{
	// Print file audio information

	ARCS_LOG_INFO << "CODEC INFORMATION:";

	ARCS_LOG_INFO << "  Codec name:     " << ctx->codec->long_name;

	ARCS_LOG_INFO << "  Short name:     " << ctx->codec->name;

	ARCS_LOG_INFO << "  Sample format:  "
			<< ::av_get_sample_fmt_name(ctx->sample_fmt);

	// Analyze planarity / interleavedness

	{
		bool is_planar = ::av_sample_fmt_is_planar(ctx->sample_fmt);

		ARCS_LOG_INFO << "  Is planar:      " << (is_planar ? "yes" : "no");

		if (is_planar and ctx->channels > AV_NUM_DATA_POINTERS)
		{
			// We have already ensured 2 channels by validating against CDDA.
			// Can 2 channels ever be too much??

			ARCS_LOG_INFO <<
			"Too many channels for frame->data, respect frame->extended_data";
		}
	}

	ARCS_LOG_INFO << "  Bytes/Sample:   "
			<< ::av_get_bytes_per_sample(ctx->sample_fmt)
			<< " (= "
			<< (::av_get_bytes_per_sample(ctx->sample_fmt) * CHAR_BIT)
			<< " bit)";

	ARCS_LOG_INFO << "  Channels:       " << ctx->channels;

	ARCS_LOG_INFO << "  Channel layout: " << ctx->channel_layout;

	ARCS_LOG_INFO << "  Samplerate:     " << ctx->sample_rate
			<< " Hz (samples/sec)";

	// ----

	ARCS_LOG_INFO << "  --Properties--";

	{
		// Losslessness ?

		bool codec_prop_lossless =
			ctx->codec_descriptor->props & AV_CODEC_PROP_LOSSLESS;

		bool codec_prop_lossy =
			ctx->codec_descriptor->props & AV_CODEC_PROP_LOSSY;

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
					ctx->codec->capabilities;

		ARCS_LOG_DEBUG << "  Capability bits:         " <<
					(sizeof(ctx->codec->capabilities) * 8);

		// Constant frame length ?

		bool codec_cap_variable_frame_size =
			ctx->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE;

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

		bool codec_cap_small_last_frame =
			ctx->codec->capabilities & AV_CODEC_CAP_SMALL_LAST_FRAME;

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

		bool codec_cap_lossless = static_cast<unsigned long>
			(ctx->codec->capabilities) & AV_CODEC_CAP_LOSSLESS;

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
			bool codec_cap_delay =
				ctx->codec->capabilities & AV_CODEC_CAP_DELAY;

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

		ARCS_LOG(DEBUG1) << "  skip_bottom:      " << ctx->skip_bottom;
		ARCS_LOG(DEBUG1) << "  frame_number:     " << ctx->frame_number;
		ARCS_LOG(DEBUG1) << "  frame_size:       " << ctx->frame_size;
		ARCS_LOG(DEBUG1) << "  initial_padding:  " << ctx->initial_padding;
		ARCS_LOG(DEBUG1) << "  trailing_padding: " << ctx->trailing_padding;

		// Commented out these logs because they are mostly unnecessary
		// for practical means, but I wanted to keep them at hand if needed.

		//ARCS_LOG(DEBUG1) << "  skip_frame:       " << ctx->skip_frame;

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


void FFmpegFileLoader::log_stream_info(::AVStream *stream) const
{
	// Print stream metadata

	{
		ARCS_LOG(DEBUG1) << "Stream information:";

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
		ARCS_LOG(DEBUG1) << "  nb_side_data:     " << stream->nb_side_data;
		ARCS_LOG(DEBUG1) << "  nb_frames:        " << stream->nb_frames;

		uint8_t *data = ::av_stream_get_side_data(
				stream,
				AV_PKT_DATA_SKIP_SAMPLES,
				nullptr);
		if (data)
		{
			ARCS_LOG_WARNING << "Client has to SKIP some frames! Inspect!";
		}
	}
}


// FFmpegAudioFile


FFmpegAudioFile::FFmpegAudioFile()
	: formatContext_(nullptr)
	, codecContext_(nullptr)
	, audioStream_(nullptr)
	, total_samples_(0)
	, num_planes_(0)
	, format_(SAMPLE_FORMAT::UNKNOWN)
	, channels_swapped_(false)
	, update_audiosize_()
	, append_samples_()
{
	// empty
}


FFmpegAudioFile::~FFmpegAudioFile() noexcept
{
	if (codecContext_)
	{
		::avcodec_close(codecContext_);
		::avcodec_free_context(&codecContext_);
	}

	if (formatContext_)
	{
		::avformat_close_input(&formatContext_);
		::av_free(formatContext_);
	}
}


int64_t FFmpegAudioFile::total_samples() const
{
	return total_samples_;
}


SAMPLE_FORMAT FFmpegAudioFile::sample_format() const
{
	return format_;
}


bool FFmpegAudioFile::channels_swapped() const
{
	return channels_swapped_;
}


void FFmpegAudioFile::register_append_samples(
		std::function<void(SampleInputIterator begin,
			SampleInputIterator end)> func)
{
	append_samples_ = func;
}


void FFmpegAudioFile::register_update_audiosize(
		std::function<void(const AudioSize &size)> func)
{
	update_audiosize_ = func;
}


void FFmpegAudioFile::set_total_samples(int64_t total_samples)
{
	total_samples_ = total_samples;
}


bool FFmpegAudioFile::decode_packet(::AVPacket packet, ::AVFrame *frame,
		int64_t *samples16, int64_t *frames, int64_t *bytes)
{
	// This is incompatible to the old API from ffmpeg 0.9
	// introduced by libavcodec version 53.25.0 in 2011-12-11.

	// Decode current packet

	if (::avcodec_send_packet(codecContext_, &packet) < 0)
	{
		::av_log(nullptr, AV_LOG_WARNING, "Error sending packet to decoder\n");
		return false;
	}

	int64_t sample16_count = 0;
	int64_t frame_count    = 0;

	// Track the frame size to recognize last frame
	// (assumes fixed frame length)

	static int      frame_size       = 0;
	static uint32_t frame_sz_changed = 0xFFFFFFFF; // 1st frame wraps this to 0

	// Decode all frames in packet

	int result = 1;

	while (result >= 0)
	{
		result = ::avcodec_receive_frame(codecContext_, frame);

		if (result < 0) // some error occurred
		{
			if (AVERROR(EAGAIN) == result)
			{
				// Decoder requires more input packets before it can provide
				// any frames, so just finish the processing of this packet
				// and provide next packet to decoder.

				break;
			}

			if (AVERROR_EOF == result)
			{
				// Unexpected end of file

				::av_log(nullptr, AV_LOG_ERROR, "Unexpected end of file\n");
				break;
			}

			::av_log(nullptr, AV_LOG_ERROR, "Error receiving frame\n");
			break;
		}

		// From here on, frame is a handle for a completely decoded frame

		++frame_count;

		sample16_count += frame->nb_samples * CDDA.NUMBER_OF_CHANNELS;

		// For audio in general only linesize[0] will be defined since the
		// planes have to be of identical size.

		// For planar audio, the planes are just channels.

		*bytes += frame->linesize[0] * num_planes_;

		// Track frame size to recognize last frame.
		// Correct estimated number of total samples with counted samples.
		// If TOC was complete, this should not be necessary, though.

		if (frame_size != frame->nb_samples)
		{
			++frame_sz_changed;

			if (frame_sz_changed) // frame size changed: check for last frame
			{
				ARCS_LOG_DEBUG << "Frame length changed from "
						<< frame_size
						<< " to "
						<< frame->nb_samples
						<< ". Guess whether this is the last frame.";

				// This is the real total samples respected so far

				int64_t samples32_counted = (*samples16 + sample16_count) / 2;

				// diff estimated total samples32 vs. counted samples32
				int64_t total_diff = total_samples_ - samples32_counted;

				// diff size of current frame against previous frame sizes
				int32_t frame_diff = frame_size - frame->nb_samples;
				// Since frame_diff counts samples16, total_diff and frame_diff
				// are not comparable for equality in case of non-planar data

				// Assuming that the estimation will not deviate by more than
				// one frame, check if we are "near" to the end of the stream
				if (std::abs(total_diff) <= frame_size)
				{
					ARCS_LOG(DEBUG1) << "READ LAST FRAME";
					ARCS_LOG(DEBUG1) << "  index: "
						<< (*frames + frame_count);
					ARCS_LOG(DEBUG1) << "  size:  "
						<< frame->nb_samples << " samples of 16 bit";
					ARCS_LOG(DEBUG1) << "  previous frame size: "
						<< frame_size;

					ARCS_LOG(DEBUG1)
						<< "  total diff counted samples (32):   "
						<< total_diff;
					ARCS_LOG(DEBUG1) << "  frame diff to previous frame (16): "
						<< frame_diff;

					// Correct total samples and update outside world

					if (samples32_counted != this->total_samples())
					{
						auto old_total = this->total_samples();

						this->set_total_samples(samples32_counted);
						// Does also work (subtract positive, add negative)
						//total_samples_ -= total_diff;

						AudioSize newsize;
						newsize.set_total_samples(this->total_samples());

						ARCS_LOG_INFO << "Update total number of samples to: "
								<< newsize.total_samples()
								<< " (was "
								<< old_total << " before)";

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
		this->pass_samples(frame->data[0], frame->data[1], frame->linesize[0]);
	}

	*frames    += frame_count;
	*samples16 += sample16_count;

	return sample16_count > 0;
}


int FFmpegAudioFile::pass_samples(const uint8_t* ch0, const uint8_t* ch1,
		const int64_t bytes_per_plane)
{
	// Note:: ffmpeg "normalizes" the channel ordering away, so we will ignore
	// it and process anything als lef0/right1

	// TODO Find less ugly implementation

	if (SAMPLE_FORMAT::S16P == this->sample_format())
	{
		SampleSequence<int16_t, true> sequence;
		sequence.wrap_bytes(ch0, ch1, static_cast<uint64_t>(bytes_per_plane));

		append_samples_(sequence.begin(), sequence.end());

		return 0;
	}

	if (SAMPLE_FORMAT::S16  == this->sample_format())
	{
		SampleSequence<int16_t, false> sequence;
		sequence.wrap_bytes(ch0, static_cast<uint64_t>(bytes_per_plane));

		append_samples_(sequence.begin(), sequence.end());

		return 0;
	}

	if (SAMPLE_FORMAT::S32P == this->sample_format()) // e.g. flac reader
	{
		SampleSequence<int32_t, true> sequence;
		sequence.wrap_bytes(ch0, ch1, static_cast<uint64_t>(bytes_per_plane));

		append_samples_(sequence.begin(), sequence.end());

		return 0;
	}

	if (SAMPLE_FORMAT::S32  == this->sample_format()) // e.g. wavpack reader
	{
		SampleSequence<int32_t, false> sequence;
		sequence.wrap_bytes(ch0, static_cast<uint64_t>(bytes_per_plane));

		append_samples_(sequence.begin(), sequence.end());

		return 0;
	}

	return 1;
}


int64_t FFmpegAudioFile::traverse_samples()
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

	while (::av_read_frame(formatContext_, &packet) == 0)
	{
		// Respect only packets from the specified stream

		if (packet.stream_index != audioStream_->index)
		{
			::av_packet_unref(&packet);

			continue;
		}

		++packet_count;

		// Commented out: Print packet side data telling to skip (for debug)
//		{
//			uint8_t *data = ::av_packet_get_side_data(
//					&packet,
//					AV_PKT_DATA_SKIP_SAMPLES,
//					nullptr);
//			if (data)
//			{
//				ARCS_LOG_DEBUG << "Something to SKIP");
//			}
//		}

		// Note: Packet needs to be copied, so do pass-by-value
		got_samples = this->decode_packet(packet, frame,
				&sample16_count, &frame_count, &byte_count);

		// Commented out: Print frame metadata (for debugging every frame only)
//		{
//			AVDictionaryEntry *tag = nullptr;
//			ARCS_LOG_DEBUG << "Frame metadata:");
//			while ((tag = ::av_dict_get(frame->metadata,
//							"", tag, AV_DICT_IGNORE_SUFFIX)))
//			{
//				ARCS_LOG_DEBUG << "  Name: " + std::string(tag->key)
//						+ "  Value: "    + std::string(tag->value));
//			}
//		}

		if (not got_samples)
		{
			::av_packet_unref(&packet);
			::av_free(frame);

			return sample16_count;
		}

		::av_packet_unref(&packet);
	} // while ::av_read_frame


	// Some codecs (Monkey's Audio for example) will cause frames to be buffered
	// up in the decoding process. If there are buffered up frames that have
	// not yet been processed, the buffer needs to be flushed. Otherwise, those
	// samples will not contribute to the checksum.

	if (AV_CODEC_CAP_DELAY & codecContext_->codec->capabilities)
	{
		ARCS_LOG_DEBUG << "Flush buffered up frames (if any)";

		int64_t buf_sample16_count = 0;
		int64_t buf_frame_count    = 0;
		int64_t buf_byte_count     = 0;
		int64_t buf_packet_count   = 0;

		bool has_packet = true;
		int result = 0;

		// Decode all the remaining frames in the buffer

		::av_init_packet(&packet);
		while (has_packet)
		{
			// Respect only packets from the specified stream

			// TODO Necessary? Only relevant packets should have been buffered
			if (packet.stream_index != audioStream_->index)
			{
				::av_packet_unref(&packet);

				continue;
			}

			// Decode current packet

			result = ::avcodec_send_packet(codecContext_, &packet);
			has_packet = result >= 0;

			if (not has_packet)
			{
				::av_packet_unref(&packet);

				ARCS_LOG_DEBUG << "No more buffered frames. Stop reading.";

				break;
			}

			++buf_packet_count;

			got_samples = this->decode_packet(packet, frame,
					&buf_sample16_count, &buf_frame_count, &buf_byte_count);

			::av_packet_unref(&packet);
		}

		ARCS_LOG_DEBUG << "Buffered Samples(16): " << buf_sample16_count;
		ARCS_LOG_DEBUG << "Buffered bytes:       " << buf_byte_count;
		ARCS_LOG_DEBUG << "Buffered frames:      " << buf_frame_count;
		ARCS_LOG_DEBUG << "Buffered packets:     " << buf_packet_count;
	}

	::av_free(frame);

	// Log some information

	ARCS_LOG_DEBUG << "Reading finished";
	ARCS_LOG_DEBUG << "  Samples(32): "
		<< (sample16_count / CDDA.NUMBER_OF_CHANNELS);
	ARCS_LOG_DEBUG << "  Samples(16): " << sample16_count;
	ARCS_LOG_DEBUG << "  Bytes:       " << byte_count;
	ARCS_LOG_DEBUG << "  Frames:      " << frame_count;
	ARCS_LOG_DEBUG << "  Packets:     " << packet_count;

	return sample16_count / CDDA.NUMBER_OF_CHANNELS;
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

	FFmpegFileLoader loader;
	auto audiofile = loader.load(filename);
	audiosize->set_total_samples(audiofile->total_samples()); // estimated!

	return audiosize;
}


void FFmpegAudioReaderImpl::do_process_file(const std::string &filename)
{
	// Redirect ffmpeg logging to arcs logging

	::av_log_set_callback(arcs_av_log);


	// Plug file, buffer and processor together

	FFmpegFileLoader loader;
	auto audiofile = loader.load(filename);

	if (audiofile->channels_swapped())
	{
		ARCS_LOG_INFO << "FFmpeg says, channels are swapped.";
	}

	// Provide estimation

	AudioSize size;
	size.set_total_samples(audiofile->total_samples());
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

	auto sample_count_expect { size.total_samples() };
	auto sample_count_fact   { audiofile->traverse_samples() };


	// Do some logging

	ARCS_LOG_DEBUG << "Respected samples: " << sample_count_fact;

	if (sample_count_fact != sample_count_expect)
	{
		ARCS_LOG_INFO << "Expected " << sample_count_expect
					<< " samples, but encountered " << sample_count_fact
					<< " ("
					<< std::abs(sample_count_expect - sample_count_fact)
					<< ((sample_count_expect < sample_count_fact)
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


bool DescriptorFFmpeg::do_accepts_bytes(const std::vector<char> & /* bytes */,
		const uint64_t & /* offset */) const
{
	return true;
}


bool DescriptorFFmpeg::do_accepts_name(const std::string &/* filename */)
	const
{
	return true;
}


bool DescriptorFFmpeg::do_accepts(FileFormat format) const
{
	return is_audio_format(format);
}


std::set<FileFormat> DescriptorFFmpeg::do_formats() const
{
	return { FileFormat::ANY_AUDIO };
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

} // namespace v_1_0_0

} // namespace arcsdec

