#ifndef __LIBARCSDEC_READERFFMPEG_HPP__
#error "Do not include readerffmpeg_details.hpp, include readerffmpeg.hpp instead"
#endif

/**
 * \file
 *
 * \brief Internal APIs for FFmpeg-based generic audio reader
 */

#ifndef __LIBARCSDEC_READERFFMPEG_DETAILS_HPP__
#define __LIBARCSDEC_READERFFMPEG_DETAILS_HPP__

/**
 * \internal
 * \defgroup readerffmpegInternal Implementation of the FFmpeg-based reader
 *
 * \ingroup readerffmpeg
 * @{
 */

#include <exception> // for exception
#include <limits>    // for numeric_limits
#include <memory>    // for unique_ptr
#include <queue>     // for queue
#include <sstream>   // for ostringstream
#include <string>
#include <utility>   // for pair
#include <vector>


extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/version.h>
#include <libavutil/avutil.h>
}


namespace arcsdec
{
inline namespace v_1_0_0
{

// This fixes the "using temporary adress" error with av_err2str in some
// versions of g++
#ifdef av_err2str
#undef av_err2str
av_always_inline char* av_err2str(int errnum)
{
    static char str[AV_ERROR_MAX_STRING_SIZE];
    memset(str, 0, sizeof(str));
    return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}
#endif


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
	explicit FFmpegException(const int error, const std::string &name);

	/**
	 * \brief The original error code.
	 *
	 * \return the original error code
	 */
	int error() const;

	char const * what() const noexcept override;

private:

	int error_;

	std::string msg_;
};


/**
 * \brief Functor for freeing AVFormatContext* instances.
 */
struct Free_AVFormatContext final
{
	void operator()(::AVFormatContext* fctx) const;
};


using AVFormatContextPtr =
		std::unique_ptr<::AVFormatContext, Free_AVFormatContext>;


/**
 * \brief Functor for freeing AVCodecContext* instances.
 */
struct Free_AVCodecContext final
{
	void operator()(::AVCodecContext* cctx) const;
};


using AVCodecContextPtr =
		std::unique_ptr<::AVCodecContext, Free_AVCodecContext>;


/**
 * \brief Functor for freeing AVPacket* instances.
 */
struct Free_AVPacket final
{
	void operator()(::AVPacket* packet) const;
};


/**
 * \brief A unique_ptr for AVPackets with a specialized deleter
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
 */
struct Free_AVFrame final
{
	void operator()(::AVFrame* frame) const;
};


/**
 * \brief A unique_ptr for AVFrame instances with a specialized deleter
 */
using AVFramePtr = std::unique_ptr<::AVFrame, Free_AVFrame>;


/**
 * \brief Construction functor for AVFrame instances.
 *
 * Frame is allocated with \c av_frame_alloc.
 *
 * If allocation fails, \c bad_alloc is thrown.
 */
struct Make_AVFramePtr final
{
	AVFramePtr operator()() const;
};


/**
 * \brief Represents a short sequence of frames.
 */
class PacketQueue final
{
	using Impl = std::queue<AVPacketPtr>;

	Impl packets_;

public:

	using size_type  = typename Impl::size_type;
	using value_type = typename Impl::value_type;

	/**
	 * \brief Constructor.
	 */
	PacketQueue();

	PacketQueue(const PacketQueue&) = delete;

	PacketQueue& operator = (const PacketQueue&) = delete;

	~PacketQueue() noexcept = default;

	/**
	 * \brief Set the AVFormatContext to read from and the AVStream to read.
	 *
	 * \param[in] fctx         AVFormatContext to read from
	 * \param[in] stream_index AVStream to read
	 *
	 * \throws std::invalid_argument If either \c context or \c stream is NULL
	 */
	void set_source(::AVFormatContext* context, const int stream_index);

	/**
	 * \brief Return the frame source set for this instance.
	 *
	 * \return AVFormatContext and AVStream to read from
	 */
	std::pair<const ::AVFormatContext*, const int> source() const;

	/**
	 * \brief Set the decoder to use.
	 *
	 * \param[in] cctx The AVCodecContext to use for decoding
	 */
	void set_decoder(::AVCodecContext* cctx);

	/**
	 * \brief Decoder used for decoding packets.
	 *
	 * \return Internal AVCodecContext used for decoding packets
	 */
	const ::AVCodecContext* decoder() const;

	/**
	 * \brief Enqueue a single frame from the specified source.
	 *
	 * EOF is signalled by returning FALSE, any error is indicated by throwing.
	 *
	 * \return True if a frame was enqueue, False on EOF.
	 *
	 * \throws FFmpegException With error code from \c av_read_frame
	 */
	bool enqueue_frame();

	/**
	 * \brief Provides next frame from queue.
	 *
	 * Expects a pointer to an allocated frame. The target object is filled with
	 * the data of the provided frame.
	 *
	 * \param[in,out] frame The frame to provide
	 *
	 * \return Total number of 16-bit samples in the provided frame.
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

private:

	/**
	 * \brief Pop next packet from queue and decode it.
	 *
	 * \return TRUE on succes, FALSE if decoder needs more input
	 */
	bool decode_packet();

	/**
	 * \brief Returns the index of the stream to be decoded.
	 */
	int stream_index() const;

	/**
	 * \brief Internal decoding context.
	 */
	::AVCodecContext* decoder();

	/**
	 * \brief Internal AVFormatContext.
	 */
	::AVFormatContext* format_context();

	/**
	 * \brief Allocate a new AVPacket using Make_AVPacketPtr.
	 *
	 * \return Pointer to an allocated and initialized AVPacket.
	 */
	AVPacketPtr make_packet();

	/**
	 * \brief Allocate a new AVFrame using Make_AVFramePtr.
	 *
	 * \return Pointer to an allocated and initialized AVFrame.
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
	AVCodecContext *cctx_;

	/**
	 * \brief Internal file format context.
	 */
	::AVFormatContext *fctx_;
};


/**
 * \brief Open a media file.
 *
 * \param[in] filename The file to open
 *
 * \return The format context for the file.
 *
 * \throws FFmpegException If the file could not be opened
 */
AVFormatContextPtr open_file(const std::string &filename);


/**
 * \brief Identify the best stream of the specified media type.
 *
 * \param[in] fctx       The format context of the streams to inspect
 * \param[in] media_type The stream type to identify
 *
 * AVCodec has to be tested for NULL by the caller.
 *
 * \return Stream index and codec
 *
 * \throws FFmpegException If no stream could be identified
 */
std::pair<int, AVCodec*> identify_stream(::AVFormatContext* fctx,
		const ::AVMediaType media_type);


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
AVCodecContextPtr create_audio_decoder(::AVFormatContext *fctx,
		const int stream_idx);


/**
 * \brief Informs about the support for a specified sample format or codec.
 */
struct IsSupported final
{
	/**
	 * \brief Returns TRUE iff the format is supported, otherwise FALSE.
	 *
	 * \return TRUE iff the format is supported, otherwise FALSE.
	 */
	static bool format(const ::AVSampleFormat id);

	/**
	 * \brief Returns TRUE iff the codec is supported, otherwise FALSE.
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
	 * \todo Add support for more PCM formats, for example there could be
	 * AV_CODEC_ID_PCM_U16LE, AV_CODEC_ID_PCM_U16BE, AV_CODEC_ID_PCM_S32LE,
	 * AV_CODEC_ID_PCM_S32BE, AV_CODEC_ID_PCM_U32LE, AV_CODEC_ID_PCM_U32BE,
	 * WMALOSSLESS and maybe more if reasonable.
	 *
	 * \return TRUE iff the codec is supported, otherwise FALSE.
	 */
	static bool codec(const ::AVCodecID id);
};


} // namespace v_1_0_0

} // namespace arcsdec

/** @} */

#endif

