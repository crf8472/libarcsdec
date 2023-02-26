/**
 * \file
 *
 * \brief Audio reader for RIFF/WAVE audio files with PCM.
 */


#ifndef __LIBARCSDEC_READERWAV_HPP__
#define __LIBARCSDEC_READERWAV_HPP__

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
 * \defgroup readerwav Audio: RIFF/WAVE with PCM encoding
 *
 * \brief AudioReader for RIFF/WAVE files with CDDA-compliant PCM data.
 *
 * Additional fields in the format subchunk are not supported. Validation
 * requires CDDA conform samples in PCM format. Non-standard subchunks are
 * ignored. RIFX containers are currently not supported.
 *
 * @{
 */


/**
 * \brief Reader for RIFF WAVE files containing PCM data.
 *
 * Represents a RIFF WAVE container holding PCM samples conforming to CDDA. That
 * is 16 bit, 2 channels, 44100 samples/sec as integer representation
 * exclusively.
 */
class DescriptorWavPCM : public FileReaderDescriptor
{
public:

	/**
	 * \brief Virtual default destructor
	 */
	~DescriptorWavPCM() noexcept override;

private:

	std::string do_id() const override;

	/**
	 * \brief Returns "RIFF/WAVE (PCM)".
	 *
	 * \return "RIFF/WAVE (PCM)"
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

