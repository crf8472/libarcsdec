/**
 * \file
 *
 * \brief Audio reader for FLAC audio files
 */


#ifndef __LIBARCSDEC_READERFLAC_HPP__
#define __LIBARCSDEC_READERFLAC_HPP__

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
 * \internal
 * \defgroup readerflac Audio: fLaC by libFLAC++
 *
 * \brief An AudioReader for fLaC/fLaC files.
 *
 * The Flac AudioReader will only read files in fLaC file format. fLaC/Ogg is
 * currently not supported. Validation requires CDDA conform samples. Embedded
 * CUESheets are ignored.
 *
 * @{
 */


/**
 * \brief Libflac-based reader for fLaC containers holding fLaC data.
 *
 * Represents a Flac container holding samples conforming to CDDA. That
 * is 16 bit, 2 channels, 44100 samples/sec as integer representation.
 */
class DescriptorFlac : public FileReaderDescriptor
{
public:

	/**
	 * \brief Constructor.
	 */
	DescriptorFlac()
		: FileReaderDescriptor { { "flac" } }
	{ /* empty */ }

	/**
	 * \brief Virtual default destructor.
	 */
	~DescriptorFlac() noexcept override;

private:

	/**
	 * \brief Returns "Flac".
	 *
	 * \return "Flac"
	 */
	std::string do_name() const override;

	/**
	 * \brief Test if this format is recognized on the given input bytes.
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

	bool do_accepts(FileFormat format) const override;

	std::set<FileFormat> do_formats() const override;

	std::unique_ptr<FileReader> do_create_reader() const override;

	std::unique_ptr<FileReaderDescriptor> do_clone() const override;
};

/// @}

} // namespace v_1_0_0

} // namespace arcsdec

#endif

