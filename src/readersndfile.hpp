/**
 * \file readersndfile.hpp Libsndfile-based generic audio reader
 *
 */


#ifndef __LIBARCSDEC_READERSNDFILE_HPP__
#define __LIBARCSDEC_READERSNDFILE_HPP__

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"
#endif


namespace arcs
{

/**
 * \internal \defgroup readersndfile Audio: Generic Audio Reading with Libsndfile
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
 * Represents any combination of container and codec that can be read by
 * libsndfile.
 */
class DescriptorSndfile : public FileReaderDescriptor
{

public:


	/**
	 * Virtual default destructor
	 */
	~DescriptorSndfile() noexcept override;


private:

	/**
	 * Returns "unknown (handled by sndfile)"
	 *
	 * \return "unknown (handled by sndfile)"
	 */
	std::string do_name() const override;

	/**
	 * Returns TRUE for every input implying this format matches all files.
	 *
	 * \param[in] bytes  The byte sequence to check (ignored)
	 * \param[in] offset The offset to byte 0 in the file (ignored)
	 *
	 * \return TRUE
	 */
	bool do_accepts_bytes(const std::vector<char> &bytes,
			const uint64_t &offset) const override;

	/**
	 * Returns TRUE for every input implying this format matches all files.
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

} // namespace arcs

#endif

