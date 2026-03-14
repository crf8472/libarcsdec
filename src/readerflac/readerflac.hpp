#ifndef LIBARCSDEC_READERFLAC_HPP__
#define LIBARCSDEC_READERFLAC_HPP__

/**
 * \file
 *
 * \brief Audio reader for FLAC audio files, implemented with libflac.
 */

#include <memory>   // for unique_ptr
#include <set>      // for set
#include <string>   // for string

#ifndef LIBARCSDEC_DESCRIPTOR_HPP__
#include "descriptor.hpp"  // for FileReaderDescriptor
#endif


namespace arcsdec
{
                                                  /** \cond NAMESPACE_v_1_0_0 */
inline namespace v_1_0_0
{
                                                                 /** \endcond */
namespace read
{

/**
 * \internal
 *
 * \defgroup readerflac Features based on fLaC
 *
 * \ingroup audioreader
 *
 * @{
 */

/**
 * \internal
 *
 * \brief An AudioReader for fLaC-encoded files in fLaC containers.
 *
 * Represents a fLaC container holding samples conforming to CDDA. That
 * is 16 bit, 2 channels, 44100 samples/sec as integer representation.
 *
 * The fLaC AudioReader will only read files in fLaC file format. fLaC/Ogg is
 * currently not supported. Validation requires samples conforming to CDDA.
 * Embedded Cuesheets are ignored.
 */
class DescriptorFlac final : public FileReaderDescriptor
{
public:

	/**
	 * \brief Default destructor.
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

} // namespace read
                                                  /** \cond NAMESPACE_v_1_0_0 */
} // namespace v_1_0_0
                                                                 /** \endcond */
} // namespace arcsdec

#endif

