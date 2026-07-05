#ifndef LIBARCSDEC_READERSNDFILE_HPP_
#error "Do not include readersndfile_details.hpp, include readersndfile.hpp instead"
#endif
#ifndef LIBARCSDEC_READERSNDFILE_DETAILS_HPP_
#define LIBARCSDEC_READERSNDFILE_DETAILS_HPP_

/**
 * \internal
 *
 * \file
 *
 * \brief Implementation details of readersndfile.hpp.
 */

#include <memory>   // for unique_ptr
#include <string>   // for string

#ifndef LIBARCSDEC_AUDIOREADER_HPP_
#include "audioreader.hpp"  // for AudioReaderImpl
#endif


namespace arcsdec
{
                                                  /** \cond NAMESPACE_v_1_0_0 */
inline namespace v_1_0_0
{
                                                                 /** \endcond */
namespace read
{
namespace details
{

/**
 * \internal
 *
 * \brief Implementation details of readersndfile.
 */
namespace sndfile
{

/**
 * \internal
 *
 * \defgroup readersndfileImpl Implementation
 *
 * \ingroup readersndfile
 *
 * @{
 */

/**
 * \brief Format independent audio file reader.
 *
 * Currently, this class is implemented by libsndfile and can open every
 * combination of file and audio format that libsndfile supports.
 *
 * \todo To support WAV formats as well as ALAC, FLAC, AIFF/AIFC, RAW
 */
class LibsndfileAudioReaderImpl final : public AudioReaderImpl
{

public:

	/**
	 * \brief Default destructor.
	 */
	~LibsndfileAudioReaderImpl() noexcept final;

private:

	AudioSize do_acquire_size(const std::string& filename) final;

	void do_process_file(const std::string& filename) final;

	std::unique_ptr<FileReaderDescriptor> do_descriptor() const final;
};


/// @}

} // namespace sndfile
} // namespace details
} // namespace read
                                                  /** \cond NAMESPACE_v_1_0_0 */
} // namespace v_1_0_0
                                                                 /** \endcond */
} // namespace arcsdec

#endif

