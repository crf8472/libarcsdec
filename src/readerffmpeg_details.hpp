#ifndef __LIBARCSDEC_READERFFMPEG_HPP__
#error "Do not include readerffmpeg_details.hpp, include readerffmpeg.hpp instead"
#endif
#ifndef __LIBARCSDEC_READERFFMPEG_DETAILS_HPP__
#define __LIBARCSDEC_READERFFMPEG_DETAILS_HPP__

/**
 * \internal
 *
 * \file
 *
 * \brief Implementation details of readerffmpeg.hpp.
 */

#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"      // for AudioReaderImpl
#endif

#ifndef __LIBARCSTK_SAMPLES_HPP__
#include <arcstk/samples.hpp>   // for SampleInputIterator
#endif
#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp> // for AudioSize
#endif

// ffmpeg
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/version.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
}

#include <cstddef>     // for size_t
#include <exception>   // for exception
#include <functional>  // for function
#include <memory>      // for unique_ptr
#include <queue>       // for queue
#include <string>      // for string
#include <type_traits> // for true_type, false_type
#include <utility>     // for pair


namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{

/**
 * \internal
 *
 * \brief Implementation details of readerffmpeg.
 */
namespace ffmpeg
{

using arcstk::SampleSequence;

using arcstk::SampleInputIterator;
using arcstk::SampleSequence;
using arcstk::AudioSize;
using arcstk::PlanarSamples;
using arcstk::InterleavedSamples;

/**
 * \internal
 *
 * \defgroup readerffmpegImpl Implementation
 *
 * \ingroup readerffmpeg
 *
 * @{
 */

// Replace macro av_err2str.
// This fixes the "using temporary adress" error with av_err2str in some
// versions of g++.
#ifdef av_err2str
#undef av_err2str
av_always_inline char* av_err2str(int errnum)
{
    static char str[AV_ERROR_MAX_STRING_SIZE];
    memset(str, 0, sizeof(str));
    return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}
#endif

extern "C"
{

/**
 * \internal
 * \brief A redirect callback for the ffmpeg log messages.
 *
 * Redirects ffmpeg messages leveled as errors, warnings and informations to the
 * libarcstk logging interface. Messages leveled as debug, trace or other are
 * discarded. All parameters except \c lvl and \c msg are ignored.
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
void arcs_av_log(void* /*v*/, int lvl, const char* msg, va_list /*l*/);

} // extern C

/**
 * \brief Encapsulates error code from the ffmpeg API.
 */
class FFmpegException final : public std::exception
{
public:

	/**
	 * \brief Construct the message from error code and function name.
	 *
	 * \param[in] error Error code
	 * \param[in] name  Name of function that returned the error code
	 */
	FFmpegException(const int error, const std::string& name);

	/**
	 * \brief The original error code.
	 *
	 * \return the original error code
	 */
	int error() const;

	char const* what() const noexcept final;

private:

	int error_;

	std::string msg_;
};


/**
 * \brief Free AVFormatContext* instances.
 *
 * Uses avformat_close_input to close and free the instance.
 */
struct Free_AVFormatContext final
{
	void operator()(::AVFormatContext* fctx) const;
};


using AVFormatContextPtr =
		std::unique_ptr<::AVFormatContext, Free_AVFormatContext>;


/**
 * \brief Free AVCodecContext* instances.
 *
 * Uses avcodec_free_context to close and free the instance.
 */
struct Free_AVCodecContext final
{
	void operator()(::AVCodecContext* cctx) const;
};


using AVCodecContextPtr =
		std::unique_ptr<::AVCodecContext, Free_AVCodecContext>;


/**
 * \brief Free AVPacket* instances.
 *
 * Uses av_packet_free to free the instance.
 */
struct Free_AVPacket final
{
	void operator()(::AVPacket* packet) const;
};


/**
 * \brief A unique_ptr for AVPackets using Free_AVPacket as a custom deleter.
 */
using AVPacketPtr = std::unique_ptr<::AVPacket, Free_AVPacket>;


/**
 * \brief Construction functor for AVPacketPtr instances.
 *
 * The packet is allocated with \c av_packet_alloc and \c av_init_packet
 * will be applied thereafter. The pointer to \c data will be \c nullptr and
 * \c size will be 0.
 *
 * If allocation fails, \c bad_alloc is thrown.
 */
struct Make_AVPacketPtr final
{
	AVPacketPtr operator()() const;
};


/**
 * \brief Functor for freeing AVFrame* instances.
 *
 * Uses av_free to free the instance.
 */
struct Free_AVFrame final
{
	void operator()(::AVFrame* frame) const;
};


/**
 * \brief A unique_ptr for AVFrame using Free_AVFrame as a custom deleter.
 */
using AVFramePtr = std::unique_ptr<::AVFrame, Free_AVFrame>;


/**
 * \brief Construction functor for AVFrame instances.
 *
 * The frame is allocated with \c av_frame_alloc.
 *
 * If allocation fails, \c bad_alloc is thrown.
 */
struct Make_AVFramePtr final
{
	AVFramePtr operator()() const;
};


/**
 * \brief Abstract getter for the i-th byte buffer of an object.
 *
 * Specialize this for adapting sample objects. Type \c T must provide \c
 * data[i] convertible to \c uint8_t*.
 *
 * Use-case is to get data[i] from ::AVFrame. A specialization for ::AVFrame is
 * therefore provided.
 *
 * \tparam T The sample object type to get the i-th byte buffer from
 */
template <typename T>
uint8_t* ByteBuffer(const T* object, const unsigned i);


// Specialization for ::AVFrame (planar + interleaved)
template <>
uint8_t* ByteBuffer(const ::AVFrame* f, const unsigned i)
{
	return f->data[i];
}

// TODO Provide template for int_buffer(const T* object, const unsigned i).


/**
 * \brief Abstract getter for total bytes per channel.
 *
 * Type \c T must provide \c nb_samples and \c channels both convertible to
 * std::size_t for ffmpeg < 5.1. Starting on ffmpeg >= 5.1, \c T must provide
 * \c ch_layout convertible to ::AVChannelLayout.
 *
 * Use-case is to estimate the number of bytes from ::AVFrame. A specialization
 * for ::AVFrame is therefore provided.
 *
 * \tparam S         The integer type to represent a sample
 * \tparam is_planar \c TRUE indicates planar buffer, \c FALSE indicates
 *                   interleaved buffer
 * \tparam T         The sample object type to get the bytes per plane from
 */
template <typename S, bool is_planar, typename T>
struct BytesPerPlane final
{
	/**
	 * \brief Get number of bytes per plane.
	 *
	 * \param[in] object The object to get bytes per plane from.
	 *
	 * \return Number of bytes per plane.
	 */
	static std::size_t get(const T* object);
};


// Specialization for ::AVFrame (planar)
template <typename S>
struct BytesPerPlane <S, true, ::AVFrame> final // for planar frames
{
	static std::size_t get(const ::AVFrame* f)
	{
		return static_cast<std::size_t>(f->nb_samples) * sizeof(S);
	}
};


// Specialization for ::AVFrame (interleaved)
template <typename S>
struct BytesPerPlane <S, false, ::AVFrame> final // for interleaved frames
{
	static std::size_t get(const ::AVFrame* f)
	{
#if LIBAVUTIL_VERSION_INT < AV_VERSION_INT(57, 24, 100) //  ffmpeg < 5.1
		return static_cast<std::size_t>(
				f->nb_samples * f->channels) * sizeof(S);
#else // ffmpeg >= 5.1
		return static_cast<std::size_t>(
				f->nb_samples * f->ch_layout.nb_channels) * sizeof(S);
#endif
	}
};


/**
 * \brief Abstract getter for channel order info.
 *
 * Type \c T must have \c channels (ffmpeg < 5.1) or \c ch_layout typed as
 * ::AVChannelLayout (ffmpeg >= 5.1).
 *
 * Use-case is to get the number of channels from ::AVCodecContext.
 *
 * \tparam T  The sample object type to get the first byte buffer from
 */
template <typename T>
int NumberOfChannels(const T* p)
{
#if LIBAVUTIL_VERSION_INT < AV_VERSION_INT(57, 24, 100) //  ffmpeg < 5.1
		return p->channels;
#else // ffmpeg >= 5.1
		return p->ch_layout.nb_channels;
#endif
};


/**
 * \brief Abstract getter for channel order info.
 */
struct ChannelOrder final
{
	/**
	 * Returns \c TRUE iff the channel order is front left + front right,
	 * otherwise \c FALSE.
	 *
	 * Type \c T must have \c channel_layout (ffmpeg < 5.1) or \c ch_layout
	 * typed as ::AVChannelLayout (ffmpeg >= 5.1).
	 *
	 * Use-case is to get the channel order from ::AVFrame or ::AVCodecContext.
	 *
	 * \tparam T  The sample object type to get the first byte buffer from
	 *
	 * \param[in] p Pointer to object of type \c T
	 */
	template <typename T>
	static bool is_leftright(const T* p)
	{
#if LIBAVUTIL_VERSION_INT < AV_VERSION_INT(57, 24, 100) //  ffmpeg < 5.1
		return p->channel_layout == AV_CH_LAYOUT_STEREO;
#else // ffmpeg >= 5.1
		// Does object specify native ordering and is it FL+FR?
		return (p->ch_layout.order == ::AV_CHANNEL_ORDER_NATIVE)
			&& (p->ch_layout.u.mask & AV_CH_LAYOUT_STEREO);
#endif
	}

	/**
	 * Returns \c TRUE iff the channel order is explicitly unspecified
	 * or the specified channel layout does not match FL+FR.
	 *
	 * Type \c T must have \c channel_layout (ffmpeg < 5.1) or \c ch_layout
	 * typed as ::AVChannelLayout (ffmpeg >= 5.1).
	 *
	 * Use-case is to get the channel order from ::AVFrame or ::AVCodecContext.
	 *
	 * \tparam T  The sample object type to get the first byte buffer from
	 *
	 * \param[in] p Pointer to object of type \c T
	 */
	template <typename T>
	static bool is_unspecified(const T* p)
	{
#if LIBAVUTIL_VERSION_INT < AV_VERSION_INT(57, 24, 100) //  ffmpeg < 5.1
		return p->channel_layout == 0;
#else // ffmpeg >= 5.1
		// Does object either not specify an ordering or has other than FL+FR?
		return p->ch_layout.order  == ::AV_CHANNEL_ORDER_UNSPEC
			|| (p->ch_layout.u.mask == 0);
#endif
	}
};


// Note:: libavcodec "normalizes" the channel ordering away, so we will ignore
// it and process everything als FL=0+FR=1.


/**
 * \brief A policy to define how to wrap the sample data in \c Container in a
 * SampleSequence.
 *
 * Basically, the WrappingPolicy implements the wrapping of planar and
 * interleaved byte buffers for a given pair of a sample type and frame type.
 *
 * A WrappingPolicy can create a sequence instance of a specified SequenceType
 * from its input or, alternatively, use an existing sequence instance to wrap
 * a sample sequence.
 *
 * \tparam S         The integer type to represent a sample
 * \tparam is_planar \c TRUE indicates planar buffer, \c FALSE indicates
 *                   interleaved buffer
 * \tparam Container The sample object container to wrap
 */
template <typename S, bool is_planar, typename Container,
		typename SequenceType = SampleSequence<S, is_planar>>
		//typename = details::IsSampleType<S>, // TODO SFINAE stuff
class WrappingPolicy final
{
	/* empty */
};


// TODO Functions of WrappingPolicy may make use of ChannelOrder::isleftright


// Specialization for wrapping an ::AVFrame into a byte buffer (planar)
template <typename S, typename SequenceType>
class WrappingPolicy<S, true, AVFramePtr, SequenceType> final
{
	using TotalBytesPerPlane = BytesPerPlane<S, true, ::AVFrame>;

public:

	static SequenceType create(const details::ffmpeg::AVFramePtr& f)
	{
		if (!f) { return SequenceType {}; }

		return SequenceType { ByteBuffer(f.get(), 0), ByteBuffer(f.get(), 1),
			TotalBytesPerPlane::get(f.get()) };
	}

	static void wrap(const details::ffmpeg::AVFramePtr& f,
			SequenceType& sequence)
	{
		sequence.wrap_byte_buffer(ByteBuffer(f.get(), 0),
				ByteBuffer(f.get(), 1),
				TotalBytesPerPlane::get(f.get()));
	}
};


// Specialization for wrapping an ::AVFrame into a byte buffer (interleaved)
template <typename S, typename SequenceType>
class WrappingPolicy<S, false, details::ffmpeg::AVFramePtr, SequenceType> final
{
	using TotalBytesPerPlane = BytesPerPlane<S, false, ::AVFrame>;

public:

	static SequenceType create(const details::ffmpeg::AVFramePtr& f)
	{
		if (!f) { return SequenceType {}; }

		return SequenceType { ByteBuffer(f.get(), 0),
			TotalBytesPerPlane::get(f.get()) };
	}

	static void wrap(const details::ffmpeg::AVFramePtr& f,
			SequenceType& sequence)
	{
		sequence.wrap_byte_buffer(ByteBuffer(f.get(), 0),
				TotalBytesPerPlane::get(f.get()));
	}
};


/**
 * \brief Get size-in-bytes of a type denoted by ::AVSampleFormat.
 */
template <enum ::AVSampleFormat>
struct SampleSize final { /* empty */ };

// legal specializations
template<> struct SampleSize<::AV_SAMPLE_FMT_S16> final
{ constexpr static std::size_t value = 2; };
template<> struct SampleSize<::AV_SAMPLE_FMT_S16P> final
{ constexpr static std::size_t value = 2; };
template<> struct SampleSize<::AV_SAMPLE_FMT_S32> final
{ constexpr static std::size_t value = 4; };
template<> struct SampleSize<::AV_SAMPLE_FMT_S32P> final
{ constexpr static std::size_t value = 4; };


/**
 * \brief Get signedness of a type denoted by ::AVSampleFormat.
 */
template <enum ::AVSampleFormat>
struct IsSigned final { /* empty */ };

// specializations
template<> struct IsSigned<::AV_SAMPLE_FMT_S16>  final : std::true_type {};
template<> struct IsSigned<::AV_SAMPLE_FMT_S16P> final : std::true_type {};
template<> struct IsSigned<::AV_SAMPLE_FMT_S32>  final : std::true_type {};
template<> struct IsSigned<::AV_SAMPLE_FMT_S32P> final : std::true_type {};


/**
 * \brief Get planarity status of a type denoted by ::AVSampleFormat.
 */
template <enum ::AVSampleFormat>
struct IsPlanar final { /* empty */ };

// specializations
template<> struct IsPlanar<::AV_SAMPLE_FMT_S16>  final : std::false_type {};
template<> struct IsPlanar<::AV_SAMPLE_FMT_S16P> final : std::true_type  {};
template<> struct IsPlanar<::AV_SAMPLE_FMT_S32>  final : std::false_type {};
template<> struct IsPlanar<::AV_SAMPLE_FMT_S32P> final : std::true_type  {};


/**
 * \brief Determine a base type for samples of specified size and signedness.
 *
 * \tparam S           Size-in-bytes of the underlying ::type
 * \tparam is_signed   \c TRUE indicates a signed type, \c FALSE indicates an
 *                     unsigned type
 */
template <int S, bool is_signed>
struct SampleType final { /* empty */ };

// legal specializations
template<> struct SampleType<2, true>  final { using type =  int16_t; };
template<> struct SampleType<2, false> final { using type = uint16_t; };
template<> struct SampleType<4, true>  final { using type =  int32_t; };
template<> struct SampleType<4, false> final { using type = uint32_t; };


/**
 * \brief Create an empty SampleSequence for a planar or interleaved
 * sample sequence of sample type \c S.
 *
 * \tparam S         Integer type that represents a 16 bit stereo sample
 * \tparam is_planar \c TRUE indicates to use a planar sequence, \c FALSE
 *                   indicates to use an interleaved sequence
 */
template <typename S, bool is_planar>
struct SequenceInstance final
{
	static auto create() -> SampleSequence<S, is_planar>
	{
		return SampleSequence<S, is_planar> {};
	}
};


/**
 * \brief Wrap an ::AVFrame in a compatible SampleSequence.
 *
 * \tparam F The sample format to handle
 *
 * \return SampleSequence instance wrapping the input frame
 */
template <::AVSampleFormat F,
	typename S =
		typename SampleType<SampleSize<F>::value, IsSigned<F>::value>::type>
auto sequence_for(const AVFramePtr& frame)
	-> SampleSequence<S, IsPlanar<F>::value>
{
	auto sequence = SequenceInstance<S, IsPlanar<F>::value>::create();

	using Policy  = WrappingPolicy<S, IsPlanar<F>::value, AVFramePtr>;
	Policy::wrap(frame, sequence);

	return sequence;
}
// Note: sequence_for is only required for FFmpegAudioReaderImpl::pass_frame()


/**
 * \brief A FIFO sequence of ::AVPacket instances.
 */
class FrameQueue final
{
	using Impl = std::queue<AVPacketPtr>;

	/**
	 * \brief Internal queue implementation.
	 */
	Impl frames_;

public:

	using size_type  = typename Impl::size_type;
	using value_type = typename Impl::value_type;

	/**
	 * \brief Constructor.
	 *
	 * \param[in] capacity Capacity in number of ::AVPacket instances to enqueue
	 */
	FrameQueue(const std::size_t capacity);

	/**
	 * \brief Constructor.
	 *
	 * Constructs a FrameQueue with a default capacity of 12 frames.
	 */
	FrameQueue()
		: FrameQueue (12)
	{
		/* empty */
	};

	FrameQueue(const FrameQueue&) = delete;

	FrameQueue& operator = (const FrameQueue&) = delete;

	~FrameQueue() noexcept = default;

	/**
	 * \brief Set the ::AVFormatContext to read from and the ::AVStream to read.
	 *
	 * \param[in] fctx         ::AVFormatContext to read from
	 * \param[in] stream_index ::AVStream to read
	 *
	 * \throws std::invalid_argument If either \c context or \c stream is NULL
	 */
	void set_source(::AVFormatContext* fctx, const int stream_index);

	/**
	 * \brief Return the frame source set for this instance.
	 *
	 * \return ::AVFormatContext and ::AVStream to read from
	 */
	std::pair<const ::AVFormatContext*, const int> source() const;

	/**
	 * \brief Set the decoder to use.
	 *
	 * \param[in] cctx The ::AVCodecContext to use for decoding
	 */
	void set_decoder(::AVCodecContext* cctx);

	/**
	 * \brief Decoder used for decoding packets.
	 *
	 * \return Internal ::AVCodecContext used for decoding packets
	 */
	const ::AVCodecContext* decoder() const;

	/**
	 * \brief Fill queue from assigned decoder.
	 *
	 * Fill queue as long as size() < capacity() and decoder provides packets.
	 *
	 * \return Size of the queue after filling
	 *
	 * \throws FFmpegException In case enqueuing fails
	 */
	std::size_t fill();

	/**
	 * \brief Enqueue a single frame from the specified source.
	 *
	 * EOF is signalled by returning \c FALSE, any error is indicated by
	 * throwing.
	 *
	 * \return \c TRUE if a frame was enqueued, \c FALSE on EOF.
	 *
	 * \throws FFmpegException With error code from \c av_read_frame
	 */
	bool enqueue_frame();

	/**
	 * \brief Provides next frame from queue.
	 *
	 * If the decoder has enough input to provide the next frame, it just
	 * provides it. Otherwise a packet is popped from the queue, decoded and the
	 * first frame is provided. Hence, size() may or may not have changed after
	 * a call of dequeue_frame (since audio packets may contain multiple
	 * frames).
	 *
	 * When the decoder has no more input to provide a frame, a nullptr is
	 * returned. All other reasons to not provide a frame are indicated by
	 * throwing.
	 *
	 * \return Next frame from queue.
	 *
	 * \throws FFmpegException With error code from \c avcodec_receive_frame
	 */
	AVFramePtr dequeue_frame();

	/**
	 * \brief Current number of packets in the queue.
	 *
	 * \return Current number of packets in the queue.
	 */
	std::size_t size() const;

	/**
	 * \brief Capacity as number of ::AVPacket instances to enqueue.
	 *
	 * \return Capacity as number of ::AVPacket instances to enqueue
	 */
	std::size_t capacity() const noexcept;

	/**
	 * \brief Set the capacity.
	 *
	 * \param[in] capacity Number of ::AVPacket instances to be enqueued.
	 */
	void set_capacity(const std::size_t capacity);

	/**
	 * \brief \c TRUE iff the queue is empty.
	 *
	 * \return  \c TRUE iff the queue is empty, otherwise \c FALSE
	 */
	bool empty() const;

private:

	/**
	 * \brief Pop next packet from queue and decode it.
	 *
	 * \param[in] packet The packet to decode
	 *
	 * \return \c TRUE on success, \c FALSE if decoder needs more input
	 */
	bool decode_packet(::AVPacket* packet);

	/**
	 * \brief Returns the index of the stream to be decoded.
	 *
	 * \return Index of the stream to be decoded
	 */
	int stream_index() const;

	/**
	 * \brief Internal decoding context.
	 */
	::AVCodecContext* decoder();

	/**
	 * \brief Internal ::AVFormatContext.
	 */
	::AVFormatContext* format_context();

	/**
	 * \brief Allocate a new ::AVPacket using Make_AVPacketPtr.
	 *
	 * \return Pointer to an allocated and initialized ::AVPacket.
	 */
	AVPacketPtr make_packet();

	/**
	 * \brief Allocate a new ::AVFrame using Make_AVFramePtr.
	 *
	 * \return Pointer to an allocated and initialized ::AVFrame.
	 */
	AVFramePtr make_frame();

	/**
	 * \brief The packet to receive frames from.
	 */
	AVPacketPtr current_packet_;

	/**
	 * \brief Index of the stream to read packets from.
	 */
	int stream_index_;

	/**
	 * \brief Internal decoder.
	 */
	::AVCodecContext* cctx_;

	/**
	 * \brief Internal file format context.
	 */
	::AVFormatContext* fctx_;

	/**
	 * \brief Capacity in number of ::AVPacket instances.
	 */
	std::size_t capacity_;
};


/**
 * \brief Functions for analyzing a media file with FFmpeg.
 */
class FFmpegFile final
{
public:

	/**
	 * \brief Open a media file.
	 *
	 * \param[in] filename The file to open
	 *
	 * \return The format context for the file.
	 *
	 * \throws FFmpegException If the file could not be opened
	 */
	static AVFormatContextPtr format_context(const std::string& filename);

	/**
	 * \brief Acquire stream index of the audio stream.
	 *
	 * \throws FFmpegException If the file could not be opened
	 */
	static int audio_stream(::AVFormatContext* fctx);

	/**
	 * \brief Create a decoder for the specified audio stream.
	 *
	 * \param[in] fctx       The FormatContext to use
	 * \param[in] stream_idx The stream to decode
	 *
	 * \return A decoder for the specified stream
	 *
	 * \throws invalid_argument If stream does not exist or is not an audio stream
	 * \throws runtime_error    If no decoder could be found for the stream
	 * \throws FFmpegException  If the decoder could not be opened
	 */
	static AVCodecContextPtr audio_decoder(::AVFormatContext* fctx,
			const int stream_idx);

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
	 * \param[in] cctx   The ::AVCodecContext to analyze
	 * \param[in] stream The ::AVStream to analyze
	 * \return Estimated total number of 32 bit PCM samples
	 */
	static int64_t total_samples(::AVCodecContext* cctx, ::AVStream* stream);

private:

	/**
	 * \brief Identify the best stream of the specified media type.
	 *
	 * ::AVCodec has to be tested for NULL by the caller. Any error in respect to
	 * the stream index will indiciated by throwing.
	 *
	 * \param[in] fctx       The format context of the streams to inspect
	 * \param[in] media_type The stream type to identify
	 *
	 * \return Stream index and codec
	 *
	 * \throws FFmpegException If no stream could be identified
	 */
	static std::pair<int, const ::AVCodec*> identify_stream(
			::AVFormatContext* fctx, const ::AVMediaType media_type);
};


/**
 * \brief Informs about the support for a specified sample format or codec.
 */
struct IsSupported final
{
	/**
	 * \brief Returns \c TRUE iff the format is supported, otherwise \c FALSE.
	 *
	 * \param[in] id The sample format to test
	 *
	 * \return \c TRUE iff the format is supported, otherwise \c FALSE.
	 */
	static bool format(const ::AVSampleFormat id);

	/**
	 * \brief Returns \c TRUE iff the codec is supported, otherwise \c FALSE.
	 *
	 * The list of supported codecs and sample formats contains FLAC, ALAC,
	 * APE as well as the PCM formats PCM_S16BE, PCM_S16LE, PCM_S16BE_PLANAR and
	 * PCM_S16LE_PLANAR.
	 *
	 * The list does not contain WavPack, since the FFmpeg-API seems not to
	 * provide a way to check whether the actual wavpack file is marked as
	 * losslessly compressed. (Also a file marked as losslessly compressed may
	 * actually be created from lossy input, but a file marked as lossy
	 * compressed must actually be refused since the ARCSs are guaranteed to be
	 * irrelevant and misleading.)
	 *
	 * \param[in] id The codec to test
	 *
	 * \return \c TRUE iff the codec is supported, otherwise \c FALSE.
	 */
	static bool codec(const ::AVCodecID id);
	// Add support for more PCM formats, for example there could be
	//::AV_CODEC_ID_PCM_U16LE, ::AV_CODEC_ID_PCM_U16BE, ::AV_CODEC_ID_PCM_S32LE,
	//::AV_CODEC_ID_PCM_S32BE, ::AV_CODEC_ID_PCM_U32LE, ::AV_CODEC_ID_PCM_U32BE,
	//::WMALOSSLESS and maybe more if reasonable.
};


/**
 * \brief Validator for ::AVCodecContext instances.
 */
class FFmpegValidator final : public DefaultValidator
{
public:

	/**
	 * \brief Validates stream for CDDA compliance.
	 *
	 * \param[in] cctx The ::AVCodecContext to analyze
	 */
	static bool cdda(::AVCodecContext* cctx);

private:

	codec_set_type do_codecs() const final;
};


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
	std::unique_ptr<FFmpegAudioStream> load(const std::string& filename) const;
};


/**
 * \brief Represents an audio stream.
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
			const std::string& filename) const;

public:

	// make class non-copyable
	FFmpegAudioStream (const FFmpegAudioStream& file) = delete;
	FFmpegAudioStream& operator = (const FFmpegAudioStream& file) = delete;

	FFmpegAudioStream (FFmpegAudioStream&& file) = default;
	FFmpegAudioStream& operator = (FFmpegAudioStream&& file) = default;

	/**
	 * \brief Return the sample format of this file.
	 *
	 * \return The sample format of this file
	 */
	::AVSampleFormat sample_format() const;

	/**
	 * \brief Number of planes.
	 *
	 * 1 for interleaved, 2 (== \c CDDA::NUMBER_OF_CHANNELS= for planar data.
	 *
	 * \return Number of planes, either 1 for interleaved or 2 for planar.
	 */
	int num_planes() const;

	/**
	 * \brief Return the channel layout of this file.
	 *
	 * \return \c TRUE for left0/right1, \c FALSE otherwise
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
	 * \brief Register the start_input() method.
	 *
	 * \param[in] func The start_input() method to use while reading
	 */
	void register_start_input(std::function<void()> func);

	/**
	 * \brief Register the push_frame() method.
	 */
	void register_push_frame(std::function<void(AVFramePtr frame)>);

	/**
	 * \brief Register the update_audiosize() method.
	 *
	 * \param[in] func The update_audiosize() method to use while reading
	 */
	void register_update_audiosize(
			std::function<void(const AudioSize& size)> func);

	/**
	 * \brief Register the end_input() method.
	 *
	 * \param[in] func The end_input() method to use while reading
	 */
	void register_end_input(std::function<void()> func);

private:

	/**
	 * \brief Get the index of the decoded audio stream.
	 *
	 * \return Index of the decoded audio stream.
	 */
	int stream_index() const;

	/**
	 * \brief Internal format context pointer.
	 */
	AVFormatContextPtr formatContext_;

	/**
	 * \brief Internal codec context pointer.
	 */
	AVCodecContextPtr codecContext_;

	/**
	 * \brief Index of the ::AVStream to be decoded.
	 */
	int stream_index_;

	/**
	 * \brief Number of planes (1 for interleaved data, 2 for planar data).
	 */
	int num_planes_;

	/**
	 * \brief \c TRUE indicates left0/right1, \c FALSE otherwise.
	 */
	bool channels_swapped_;

	/**
	 * \brief Total number of 32 bit PCM samples in the file estimated by
	 * duration.
	 */
	int64_t total_samples_declared_;

	/**
	 * \brief Callback for starting input.
	 */
	std::function<void()> start_input_;

	/**
	 * \brief Callback for pushing an AVFramePtr
	 */
	std::function<void(AVFramePtr frame)> push_frame_;

	/**
	 * \brief Callback for notifying outside world about the correct AudioSize.
	 */
	std::function<void(const AudioSize& size)> update_audiosize_;

	/**
	 * \brief Callback for ending input.
	 */
	std::function<void()> end_input_;

	/**
	 * \brief Constructor.
	 */
	FFmpegAudioStream();
};


/**
 * \brief Audio file reader implemented by FFmpeg API.
 *
 * This is a AudioReader implementation by libavformat and libavcodec. It can
 * open files in virtually any combination of container and audio format.
 *
 * It is internally limited to a set of lossless codecs.
 */
class FFmpegAudioReaderImpl final : public AudioReaderImpl
{
public:

	/**
	 * \brief Default constructor.
	 */
	FFmpegAudioReaderImpl();

	/**
	 * \brief Virtual default destructor.
	 */
	~FFmpegAudioReaderImpl() noexcept final;

private:

	// AudioReaderImpl

	std::unique_ptr<AudioSize> do_acquire_size(const std::string& filename)
		final;

	void do_process_file(const std::string& filename) final;

	std::unique_ptr<FileReaderDescriptor> do_descriptor() const final;

	/**
	 * \brief Callback for decoded single frame.
	 *
	 * \param[in] frame Next decoded frame to update Calculation with
	 */
	void frame_callback(AVFramePtr frame);

	/**
	 * \brief Pass single frame to next processor.
	 *
	 * \param[in] frame Next decoded frame to update Calculation with
	 */
	void pass_frame(AVFramePtr frame);

	/**
	 * \brief Pass samples of a single frame to next processor.
	 *
	 * Part of implementation of pass_frame.
	 *
	 * \param[in] frame Next decoded frame to update Calculation with
	 */
	template<enum ::AVSampleFormat>
	void pass_samples(AVFramePtr frame);
};


/**
 * \brief Pretty-print an AVDictionary.
 *
 * \param[in] out  The stream to print
 * \param[in] dict The dictionary to print
 */
void print_dictionary(std::ostream& out, const ::AVDictionary* dict);


/**
 * \brief Log some information about the codec.
 *
 * \param[in] cctx The ::AVCodecContext to analyze
 */
void print_codec_info(std::ostream& out, const ::AVCodecContext* cctx);


/**
 * \brief Log some information about the format.
 *
 * \param[in] out  The ostream to log to
 * \param[in] fctx The ::AVFormatContext to analyze
 */
void print_format_info(std::ostream& out, const ::AVFormatContext* fctx);


/**
 * \brief Log some information about the stream.
 *
 * \param[in] stream The ::AVStream to analyze
 */
void print_stream_info(std::ostream& out, const ::AVStream* stream);

/// @}

} // namespace ffmpeg
} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

#endif

