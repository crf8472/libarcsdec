#ifndef LIBARCSDEC_READERWVPK_HPP__
#define LIBARCSDEC_READERWVPK_HPP__

/**
 * \file
 *
 * \brief Audio reader for Wavpack audio files, implmented with libwavpack.
 *
 * The Wavpack AudioReader will only read Wavpack files containing losslessly
 * compressed integer samples. Validation requires CDDA conform samples in PCM
 * format. Float samples are not supported. Original file formats other than
 * WAV are not supported.
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
 * \defgroup readerwvpk Features based on Wavpack
 *
 * \ingroup audioreader
 *
 * @{
 */

/**
 * \internal
 *
 * \brief An AudioReader for losslessly encoded wavpack files.
 *
 * Represents a Wavpack container holding losslessly encoded samples conforming
 * to CDDA. That is 16 bit, 2 channels, 44100 samples/sec as integer
 * representation exclusively.
 *
 * The Wavpack AudioReader will only read Wavpack files containing losslessly
 * compressed samples in integer format. Float samples are not supported.
 * Validation requires CDDA conform samples. Original file formats other than
 * WAV are not supported.
 */
class DescriptorWavpack final : public FileReaderDescriptor
{
public:

	/**
	 * \brief Default destructor.
	 */
	~DescriptorWavpack() noexcept final;

private:

	std::string do_id() const final;

	/**
	 * \brief Returns "Wavpack".
	 *
	 * \return "Wavpack"
	 */
	std::string do_name() const final;

	std::set<Format> define_formats() const final;

	std::set<Codec> define_codecs() const final;

	LibInfo do_libraries() const final;

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

