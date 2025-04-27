#ifndef __LIBARCSDEC_READERWAV_HPP__
#define __LIBARCSDEC_READERWAV_HPP__

/**
 * \file
 *
 * \brief Audio reader for RIFF/WAVE audio files with PCM samples.
 */

#ifndef __LIBARCSDEC_DESCRIPTOR_HPP__
#include "descriptor.hpp"  // for FileReaderDescriptor
#endif

#include <memory>   // for unique_ptr
#include <set>      // for set
#include <string>   // for string


namespace arcsdec
{
inline namespace v_1_0_0
{

/**
 * \brief AudioReader for RIFF/WAVE files with CDDA-compliant PCM data.
 *
 * Represents a RIFF WAVE container holding PCM samples conforming to CDDA. That
 * is 16 bit, 2 channels, 44100 samples/sec as integer representation
 * exclusively.
 *
 * Additional fields in the format subchunk are not supported. Validation
 * requires CDDA conform samples in PCM format. Non-standard subchunks are
 * ignored. RIFX containers are currently not supported.
 */
class DescriptorWavPCM final : public FileReaderDescriptor
{
public:

	/**
	 * \brief Default destructor.
	 */
	~DescriptorWavPCM() noexcept final;


private:

	std::string do_id() const final;

	/**
	 * \brief Returns "RIFF/WAVE (PCM)".
	 *
	 * \return "RIFF/WAVE (PCM)"
	 */
	std::string do_name() const final;

	std::set<Format> define_formats() const final;

	std::set<Codec> define_codecs() const final;

	LibInfo do_libraries() const final;

	std::unique_ptr<FileReader> do_create_reader() const final;

	std::unique_ptr<FileReaderDescriptor> do_clone() const final;
};

} // namespace v_1_0_0
} // namespace arcsdec

#endif

