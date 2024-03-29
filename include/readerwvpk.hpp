/**
 * \file
 *
 * \brief Audio reader for Wavpack audio files.
 */


#ifndef __LIBARCSDEC_READERWVPK_HPP__
#define __LIBARCSDEC_READERWVPK_HPP__

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
	 * \brief Virtual default destructor.
	 */
	~DescriptorWavpack() noexcept override;

private:

	std::string do_id() const override;

	/**
	 * \brief Returns "Wavpack".
	 *
	 * \return "Wavpack"
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

