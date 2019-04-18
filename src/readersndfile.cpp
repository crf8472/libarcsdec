#ifndef __LIBARCSDEC_READERSNDFILE_HPP__
#include "readersndfile.hpp"
#endif

/**
 * \file readersndfile.cpp Implements Libsndfile-based generic audio reader
 */

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <sndfile.hh>

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>
#endif
#ifndef __LIBARCSTK_SAMPLES_HPP__
#include <arcstk/samples.hpp>
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"
#endif


namespace arcsdec
{

inline namespace v_1_0_0
{

using arcstk::PCMForwardIterator;
using arcstk::AudioSize;
using arcstk::CDDA;
using arcstk::SampleSequence;


namespace
{


/**
 * \cond IMPL_ONLY
 *
 * \internal \defgroup readersndfileImpl Implementation details for the libsndfile reader
 *
 * \ingroup readersndfile
 *
 * @{
 */


/**
 * Format independent audio file reader.
 *
 * Currently, this class is implemented by libsndfile and can open every
 * combination of file and audio format that libsndfile supports.
 *
 * \todo To support WAV formats as well as ALAC, FLAC, AIFF/AIFC, RAW
 */
class LibsndfileAudioReaderImpl : public BufferedAudioReaderImpl
{

public:

	/**
	 * Virtual default destructor
	 */
	virtual ~LibsndfileAudioReaderImpl() noexcept;


private:

	std::unique_ptr<AudioSize> do_acquire_size(const std::string &filename)
		override;

	void do_process_file(const std::string &filename) override;
};


/// @}
/// \endcond IMPL_ONLY


LibsndfileAudioReaderImpl::~LibsndfileAudioReaderImpl() noexcept = default;


std::unique_ptr<AudioSize> LibsndfileAudioReaderImpl::do_acquire_size(
	const std::string &filename)
{
	SndfileHandle audiofile(filename);

	auto audiosize { std::make_unique<AudioSize>() };
	audiosize->set_sample_count(audiofile.frames());
	// FIXME works only for WAV??

	return audiosize;
}


void LibsndfileAudioReaderImpl::do_process_file(const std::string &filename)
{
	SndfileHandle audiofile(filename, SFM_READ);

	// TODO Check whether this was successful

	// Perform validation

	if (audiofile.samplerate() - CDDA.SAMPLES_PER_SECOND != 0)
	{
		ARCS_LOG_DEBUG << "Samplerate: " << audiofile.samplerate();
		return;
	}

	if (audiofile.channels() - CDDA.NUMBER_OF_CHANNELS != 0)
	{
		ARCS_LOG_DEBUG << "Channels: " << audiofile.channels();
		return;
	}

	if (not (audiofile.format() | SF_FORMAT_PCM_16))
	{
		ARCS_LOG_DEBUG << "Format: " + audiofile.format();
		return;
	}

	ARCS_LOG_INFO << "Validation completed: file seems to be CDDA";


	// Update Calculation with sample count

	AudioSize audiosize;
	audiosize.set_sample_count(audiofile.frames());
	this->process_audiosize(audiosize);

	// Prepare Read buffer (16 bit samples)

	uint32_t buffer_len = this->samples_per_read() * CDDA.NUMBER_OF_CHANNELS;
	std::vector<int16_t> buffer(buffer_len);

	SampleSequence<int16_t, false> sequence;

	// Checking

	int ints_in_block = 0;

	// Logging

	uint64_t sample_count = 0;
	uint32_t blocks_processed = 0;

	// Read blocks

	while ((ints_in_block = audiofile.read (&buffer[0], buffer_len)))
	{
		++blocks_processed;

		ARCS_LOG_DEBUG << "READ BLOCK " << blocks_processed;

		// Expected amount was read?

		if (static_cast<unsigned int>(ints_in_block) != buffer_len)
		{
			// This is allowed only for the last block

			auto expected_total { audiosize.sample_count() - sample_count };

			if (expected_total != ints_in_block / CDDA.NUMBER_OF_CHANNELS)
			{
				std::stringstream ss;
				ss << "  Block contains "
					<< ints_in_block
					<< " integers, expected were "
					<< buffer_len
					<< ". Resize buffer";

				ARCS_LOG_ERROR << ss.str();

				throw FileReadException(ss.str());
			}

			// Resize to actual amount of integers

			ARCS_LOG_INFO << "  Last block contains "
					<< ints_in_block
					<< " integers instead of "
					<< buffer_len
					<< ". Resize buffer";

			buffer.resize(ints_in_block);
		}

		sequence.reset(&buffer[0], buffer.size());

		ARCS_LOG(DEBUG1) << "  Size: "
				<< (buffer.size() * sizeof(buffer[0])) << " bytes";
		ARCS_LOG(DEBUG1) << "        "
				<< (buffer.size() / CDDA.NUMBER_OF_CHANNELS)
				<< " Stereo PCM samples (32 bit)";

		this->process_samples(sequence.begin(), sequence.end());

		sample_count += sequence.size();
	}
}


} // namespace


// DescriptorSndfile


DescriptorSndfile::~DescriptorSndfile() noexcept = default;


std::string DescriptorSndfile::do_name() const
{
	return "unknown (handled by sndfile)";
}


bool DescriptorSndfile::do_accepts_bytes(const std::vector<char> & /* bytes */,
		const uint64_t & /* offset */) const
{
	return true;
}


bool DescriptorSndfile::do_accepts_suffix(const std::string & /* suffix */)
	const
{
	return true;
}


std::unique_ptr<FileReader> DescriptorSndfile::do_create_reader() const
{
	auto impl = std::make_unique<LibsndfileAudioReaderImpl>();

	return std::make_unique<AudioReader>(std::move(impl));
}


std::unique_ptr<FileReaderDescriptor> DescriptorSndfile::do_clone() const
{
	return std::make_unique<DescriptorSndfile>();
}

} // namespace v_1_0_0

} // namespace arcsdec

