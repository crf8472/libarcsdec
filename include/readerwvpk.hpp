/**
 * \file
 *
 * \brief Audio reader for Wavpack audio files
 */


#ifndef __LIBARCSDEC_READERWVPK_HPP__
#define __LIBARCSDEC_READERWVPK_HPP__

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
 * \defgroup readerwvpk Audio: Lossless Wavpack by libwavpack
 *
 * \brief An AudioReader for losslessly encoded Wavpack/Wv files.
 *
 * The Wavpack AudioReader will only read Wavpack files containing losslessly
 * compressed samples in integer format. Float samples are not supported.
 * Validation requires CDDA conform samples. Original file formats other than
 * WAV are not supported.
 *
 * @{
 */


/**
 * \brief Wavpack-5-based reader for losslessly encoded wavpack files.
 *
 * Represents a Wavpack container holding losslessly encoded samples conforming
 * to CDDA. That is 16 bit, 2 channels, 44100 samples/sec as integer
 * representation exclusively.
 */
class DescriptorWavpack : public FileReaderDescriptor
{
public:

	/**
	 * \brief Constructor.
	 */
	DescriptorWavpack()
		: FileReaderDescriptor { { "wv" } }
	{ /* empty */ }

	/**
	 * \brief Virtual default destructor.
	 */
	~DescriptorWavpack() noexcept override;

private:

	/**
	 * \brief Returns "Wavpack".
	 *
	 * \return "Wavpack"
	 */
	std::string do_name() const override;

	LibInfo do_libraries() const override;

	/**
	 * \brief Test if this format is recognized on the given input bytes.
	 *
	 * The test is made against a slice of at least 4 bytes with offset 0
	 * (from the beginning of the file). The following test is performed:
	 * Are bytes 0-3 of value 0x7776706B (== "wvpk" in ASCII)?
	 *
	 * \param[in] bytes  The byte sequence to check
	 * \param[in] offset The offset to byte 0 in the file
	 *
	 * \return TRUE if the bytes match the DescriptorWavpack, otherwise FALSE
	 */
	bool do_accepts_bytes(const std::vector<unsigned char> &bytes,
			const uint64_t &offset) const override;

	bool do_accepts(Codec codec) const override;

	std::set<Codec> do_codecs() const override;

	bool do_accepts(Format format) const override;

	std::set<Format> do_formats() const override;

	std::unique_ptr<FileReader> do_create_reader() const override;

	std::unique_ptr<FileReaderDescriptor> do_clone() const override;
};

/// @}

} // namespace v_1_0_0

} // namespace arcsdec

#endif

