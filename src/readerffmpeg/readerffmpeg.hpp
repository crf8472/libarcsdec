#ifndef LIBARCSDEC_READERFFMPEG_HPP__
#define LIBARCSDEC_READERFFMPEG_HPP__

/**
 * \file
 *
 * \brief AudioReader for multiple lossless audio formats, based on ffmpeg.
 */

#ifndef LIBARCSDEC_DESCRIPTOR_HPP__
#include "descriptor.hpp"  // for FileReaderDescriptor
#endif

#include <memory>   // for unique_ptr
#include <set>      // for set
#include <string>   // for string


namespace arcsdec
{
                                                  /** \cond NAMESPACE_v_1_0_0 */
inline namespace v_1_0_0
{
                                                                 /** \endcond */
namespace read
{

/**
 * \internal
 *
 * \defgroup readerffmpeg Features based on ffmpeg
 *
 * \ingroup audioreader
 *
 * @{
 */

/**
 * \internal
 *
 * \brief Generic ffmpeg-based AudioReader for lossless formats.
 *
 * \details
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
 */
class DescriptorFFmpeg final : public FileReaderDescriptor
{
public:

	/**
	 * \brief Default destructor.
	 */
	~DescriptorFFmpeg() noexcept final;


private:

	std::string do_id() const final;

	/**
	 * \brief Returns "FFmpeg".
	 *
	 * \return "FFmpeg"
	 */
	std::string do_name() const final;

	std::set<Format> define_formats() const final;

	std::set<Codec> define_codecs() const final;

	LibInfo do_libraries() const final;

	std::unique_ptr<FileReader> do_create_reader() const final;

	std::unique_ptr<FileReaderDescriptor> do_clone() const final;
};

/// @}

} // namespace read
                                                  /** \cond NAMESPACE_v_1_0_0 */
} // namespace v_1_0_0
                                                                 /** \endcond */
} // namespace arcsdec

#endif

