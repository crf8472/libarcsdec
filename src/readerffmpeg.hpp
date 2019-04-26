/**
 * \file readerffmpeg.hpp FFmpeg-based generic audio reader
 *
 */


#ifndef __LIBARCSDEC_READERFFMPEG_HPP__
#define __LIBARCSDEC_READERFFMPEG_HPP__

#include <chrono> // for debugging
#include <functional>
#include <memory>
#include <string>
#include <vector>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

#ifndef __LIBARCSDEC_DESCRIPTORS_HPP__
#include "descriptors.hpp"
#endif


namespace arcsdec
{

inline namespace v_1_0_0
{

/**
 * \internal \defgroup readerffmpeg Audio: Generic by FFmpeg
 *
 * \brief A generic AudioReader for losslessly encoded audio files.
 *
 * Since checksumming makes only sense for losslessly encoded files, the
 * FFmpegAudioReaderImpl uses a whitelist to verify whether the codec of the
 * input data is supported. It is nonetheless possible to check whether the
 * actual codec supports only lossless compression. (Excluding codecs that allow
 * lossy compression would exclude wavpack, where on the other hand ffmpeg does
 * not seem to have access to the lossless flag in the actual file. It may
 * therefore be sensible to exclude wavpack from being read by
 * FFmpegAudioReaderImpl for now.)
 *
 * The FFmpegAudioReaderImpl can also read FLAC encoded data in either container
 * format. As by configuration, the native FLAC reader will always take
 * precedence for reading .flac files, while FLAC/OGG files can not be read with
 * the original FLAC reader.
 *
 * This implementation uses the decoding API introduced by ffmpeg release 3.1
 * with libavcodec version 57.48.101 in 2016-06-27 (the libavcodec version that
 * introduced the new API was 57.37.100 on 2016-04-21). It can not be compiled
 * with ffmpeg versions prior to 3.1 (at least with libavcodec prior to
 * 57.37.100).
 *
 * @{
 */


/**
 * \brief FFmpeg-based AudioReader for virtually any lossless audio format.
 */
class DescriptorFFmpeg : public FileReaderDescriptor
{

public:

	/**
	 * \brief Virtual default destructor.
	 */
	~DescriptorFFmpeg() noexcept override;


private:

	/**
	 * \brief Returns "unknown (handled by ffmpeg)".
	 *
	 * \return "unknown (handled by ffmpeg)"
	 */
	std::string do_name() const override;

	/**
	 * \brief Returns TRUE for every input implying this format matches all
	 * files.
	 *
	 * \param[in] bytes  The byte sequence to check (ignored)
	 * \param[in] offset The offset to byte 0 in the file (ignored)
	 *
	 * \return TRUE
	 */
	bool do_accepts_bytes(const std::vector<char> &bytes,
			const uint64_t &offset) const override;

	/**
	 * \brief Returns TRUE for every input implying this format matches all
	 * files.
	 *
	 * \param[in] suffix The file suffix to check (ignored)
	 *
	 * \return TRUE
	 */
	bool do_accepts_suffix(const std::string &suffix) const override;

	std::unique_ptr<FileReader> do_create_reader() const override;

	std::unique_ptr<FileReaderDescriptor> do_clone() const override;
};

/// @}

} // namespace v_1_0_0

} // namespace arcsdec

#endif

