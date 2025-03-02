#ifndef __LIBARCSDEC_READERSNDFILE_HPP__
#error "Do not include readersndfile_details.hpp, include readersndfile.hpp instead"
#endif
#ifndef __LIBARCSDEC_READERSNDFILE_DETAILS_HPP__
#define __LIBARCSDEC_READERSNDFILE_DETAILS_HPP__

/**
 * \internal
 *
 * \file
 *
 * \brief Implementation details of readersndfile.hpp.
 */

#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"  // for AudioReaderImpl
#endif

#include <memory>   // for unique_ptr
#include <string>   // for string


namespace arcsdec
{
inline namespace v_1_0_0
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

	std::unique_ptr<AudioSize> do_acquire_size(const std::string& filename)
		final;

	void do_process_file(const std::string& filename) final;

	std::unique_ptr<FileReaderDescriptor> do_descriptor() const final;
};


/// @}

} // namespace sndfile
} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

#endif

