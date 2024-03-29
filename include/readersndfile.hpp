/**
 * \file
 *
 * \brief Libsndfile-based generic audio reader.
 */


#ifndef __LIBARCSDEC_READERSNDFILE_HPP__
#define __LIBARCSDEC_READERSNDFILE_HPP__

#include <cstdint>  // for uint64_t
#include <memory>   // for unique_ptr
#include <set>      // for set
#include <string>   // for string
#include <vector>   // for vector

#ifndef __LIBARCSDEC_DESCRIPTOR_HPP__
#include "descriptor.hpp"  // for FileReaderDescriptor
#endif


namespace arcsdec
{
inline namespace v_1_0_0
{

/**
 * \internal
 * \defgroup readersndfile Audio: Generic by libsndfile
 *
 * \brief A generic AudioReader for losslessly encoded audio files.
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
 *
 * @{
 */


/**
 * \brief Libsndfile-based reader for some lossless audio input formats.
 */
class DescriptorSndfile : public FileReaderDescriptor
{

public:

	/**
	 * \brief Virtual default destructor.
	 */
	~DescriptorSndfile() noexcept override;


private:

	std::string do_id() const override;

	/**
	 * \brief Returns "unknown (handled by sndfile)"
	 *
	 * \return "unknown (handled by sndfile)"
	 */
	std::string do_name() const override;

	std::set<Format> define_formats() const override;

	std::set<Codec> define_codecs() const override;

	LibInfo do_libraries() const override;

	std::unique_ptr<FileReader> do_create_reader() const override;

	std::unique_ptr<FileReaderDescriptor> do_clone() const override;
};

/// @}

} // namespace v_1_0_0
} // namespace arcsdec

#endif

