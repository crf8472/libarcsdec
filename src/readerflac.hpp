/**
 * \file readerflac.hpp Audio reader for FLAC audio files
 *
 */


#ifndef __LIBARCSDEC_READERFLAC_HPP__
#define __LIBARCSDEC_READERFLAC_HPP__

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#ifndef __LIBARCSDEC_FILEFORMATS_HPP__
#include "descriptors.hpp"
#endif


namespace arcs
{

inline namespace v_1_0_0
{

/**
 * \internal \defgroup readerflac Audio: FLAC
 *
 * \brief An AudioReader for FLAC/FLAC files.
 *
 * The Flac AudioReader will only read files in FLAC file format. FLAC/Ogg is
 * currently not supported. Validation requires CDDA conform samples. Embedded
 * CUEsheets are ignored.
 *
 * @{
 */


/**
 * Represents the Flac file format.
 *
 * Represents a Flac container holding samples conforming to CDDA. That
 * is 16 bit, 2 channels, 44100 samples/sec as integer representation.
 */
class DescriptorFlac : public FileReaderDescriptor
{

public:

	/**
	 * Virtual default destructor
	 */
	~DescriptorFlac() noexcept override;


private:

	/**
	 * Returns "Flac".
	 *
	 * \return "Flac"
	 */
	std::string do_name() const override;

	/**
	 * Test if this format is recognized on the given input bytes.
	 *
	 * The test is made against a slice of at least 4 bytes with offset from
	 * the beginning of the file. The following three tests are performed:
	 * Are bytes 0-3 of value 0x664C6143 (which is "fLaC" in ASCII)?
	 *
	 * \param[in] bytes  The byte sequence to check
	 * \param[in] offset The offset to byte 0 in the file
	 *
	 * \return TRUE if the bytes match the DescriptorFlac, otherwise FALSE
	 */
	bool do_accepts_bytes(const std::vector<char> &bytes,
			const uint64_t &offset) const override;

	/**
	 * Test whether suffix is case insensitively equal to "flac"
	 *
	 * \param[in] suffix The suffix to test
	 *
	 * \return TRUE iff suffix is case insensitively equal to "flac", otherwise
	 * FALSE
	 */
	bool do_accepts_suffix(const std::string &suffix) const override;

	std::unique_ptr<FileReader> do_create_reader() const override;

	std::unique_ptr<FileReaderDescriptor> do_clone() const override;
};

/// @}

} // namespace v_1_0_0

} // namespace arcs

#endif

