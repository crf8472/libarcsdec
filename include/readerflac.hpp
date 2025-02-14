/**
 * \file
 *
 * \brief Audio reader for FLAC audio files.
 */

#ifndef __LIBARCSDEC_READERFLAC_HPP__
#define __LIBARCSDEC_READERFLAC_HPP__

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
class DescriptorFlac final : public FileReaderDescriptor
{
public:

	/**
	 * \brief Virtual default destructor.
	 */
	~DescriptorFlac() noexcept final;

private:

	std::string do_id() const final;

	/**
	 * \brief Returns "Flac".
	 *
	 * \return "Flac"
	 */
	std::string do_name() const final;

	LibInfo do_libraries() const final;

	std::set<Format> define_formats() const final;

	std::set<Codec> define_codecs() const final;

	std::unique_ptr<FileReader> do_create_reader() const final;

	std::unique_ptr<FileReaderDescriptor> do_clone() const final;
};

/// @}

} // namespace v_1_0_0

} // namespace arcsdec

#endif

