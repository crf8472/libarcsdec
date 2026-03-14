#ifndef LIBARCSDEC_READERSNDFILE_HPP_
#define LIBARCSDEC_READERSNDFILE_HPP_

/**
 * \file
 *
 * \brief AudioReader for multiple lossless audio formats, based on libsndfile.
 */

#include <memory>   // for unique_ptr
#include <set>      // for set
#include <string>   // for string

#ifndef LIBARCSDEC_DESCRIPTOR_HPP_
#include "descriptor.hpp"  // for FileReaderDescriptor
#endif


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
 * \defgroup readersndfile Features based on libsndfile
 *
 * \ingroup audioreader
 *
 * @{
 */

/**
 * \internal
 *
 * \brief Generic libsndfile-based AudioReader for lossless formats.
 *
 * The LibsndfileAudioReaderImpl can also read FLAC encoded data in either
 * container format (since 1.0.18). As by configuration, the native FLAC reader
 * will always take precedence for reading .flac files, while FLAC/Ogg files can
 * not be read by the native FLAC reader.
 *
 * Other supported formats are WAV, AIFF/AIFFC, headerless RAW and ALAC/CAF.
 *
 * Currently (version 1.0.28) libsndfile does not read wavpack files.
 *
 * \todo Since checksumming makes only sense for losslessly encoded files, the
 * LibsndfileAudioReaderImpl uses a whitelist to verify whether the codec of
 * the input data is supported.
 */
class DescriptorSndfile final : public FileReaderDescriptor
{
public:

	/**
	 * \brief Default destructor.
	 */
	~DescriptorSndfile() noexcept final;

private:

	std::string do_id() const final;

	/**
	 * \brief Returns "unknown (handled by sndfile)"
	 *
	 * \return "unknown (handled by sndfile)"
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

