/**
 * \file readerwav.hpp Audio reader for RIFF/WAV audio files with PCM
 *
 */


#ifndef __LIBARCSDEC_READERWAV_HPP__
#define __LIBARCSDEC_READERWAV_HPP__

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#ifndef __LIBARCSDEC_DESCRIPTORS_HPP__
#include "descriptors.hpp"
#endif


namespace arcsdec
{

inline namespace v_1_0_0
{

/**
 * \internal \defgroup readerwav Audio: RIFF/WAV with PCM encoding
 *
 * \brief AudioReader for RIFF/WAV files with CDDA-compliant PCM data.
 *
 * Additional fields in the format subchunk are not supported. Validation
 * requires CDDA conform samples in PCM format. Non-standard subchunks are
 * ignored. RIFX containers are currently not supported.
 *
 * @{
 */


/**
 * \brief Reader for RIFF WAV files containing PCM data.
 *
 * Represents a RIFF WAV container holding PCM samples conforming to CDDA. That
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

	/**
	 * \brief Returns "RIFF/WAV (PCM)".
	 *
	 * \return "RIFF/WAV (PCM)"
	 */
	std::string do_name() const override;

	/**
	 * \brief Test if this format is recognized on the given input bytes.
	 *
	 * The test is made against a slice of at least 24 bytes with offset 0 (from
	 * the beginning of the file).
	 *
	 * The following three tests are performed:
	 * 1.) Are bytes 0-3 of value 0x52494646 (which is "RIFF" in ASCII)?
	 * 2.) Are bytes 8-11 of value 0x47514655 (which is "WAVE" in ASCII)?
	 * 3.) Are bytes 20-21 of value 0x0100
	 * (which indicates PCM in ASCII as value "1" in 16 bit low endian)?
	 *
	 * \param[in] bytes  The byte sequence to check
	 * \param[in] offset The offset to byte 0 in the file
	 *
	 * \return TRUE if the bytes match the DescriptorWavPCM, otherwise FALSE
	 */
	bool do_accepts_bytes(const std::vector<char> &bytes,
			const uint64_t &offset) const override;

	bool do_accepts_suffix(const std::string &suffix) const override;

	std::unique_ptr<FileReader> do_create_reader() const override;

	std::unique_ptr<FileReaderDescriptor> do_clone() const override;
};

/// @}

} // namespace v_1_0_0

} // namespace arcsdec

#endif

