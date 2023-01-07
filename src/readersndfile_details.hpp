#ifndef __LIBARCSDEC_READERSNDFILE_HPP__
#error "Do not include readersndfile_details.hpp, include readersndfile.hpp instead"
#endif

/**
 * \file
 *
 * \brief Internal APIs for Libsndfile-based generic audio reader.
 */

#ifndef __LIBARCSDEC_READERSNDFILE_DETAILS_HPP__
#define __LIBARCSDEC_READERSNDFILE_DETAILS_HPP__


#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"      // for AudioReaderImpl
#endif

namespace arcsdec
{
inline namespace v_1_0_0
{

namespace details
{
namespace sndfile
{

/**
 * \internal \defgroup readersndfileImpl Implementation
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
class LibsndfileAudioReaderImpl : public AudioReaderImpl
{

public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~LibsndfileAudioReaderImpl() noexcept;

private:

	std::unique_ptr<AudioSize> do_acquire_size(const std::string &filename)
		final;

	void do_process_file(const std::string &filename) final;

	std::unique_ptr<FileReaderDescriptor> do_descriptor() const final;
};


/// @}

} // namespace sndfile
} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

#endif

