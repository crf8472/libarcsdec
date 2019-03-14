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
#include "fileformats.hpp"
#endif


namespace arcs
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
class FileFormatFlac : public FileFormat
{

public:

	/**
	 * Virtual default destructor
	 */
	~FileFormatFlac() noexcept override;


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
	 * \return TRUE if the bytes match the FileFormatFlac, otherwise FALSE
	 */
	bool do_can_have_bytes(const std::vector<char> &bytes,
			const uint64_t &offset) const override;

	/**
	 * Test whether suffix is case insensitively equal to "flac"
	 *
	 * \param[in] suffix The suffix to test
	 *
	 * \return TRUE iff suffix is case insensitively equal to "flac", otherwise
	 * FALSE
	 */
	bool do_can_have_suffix(const std::string &suffix) const override;

	std::unique_ptr<FileReader> do_create_reader() const override;

	std::unique_ptr<FileFormat> do_clone() const override;
};

/// @}

} // namespace arcs

#endif

