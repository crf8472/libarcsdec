/**
 * \file readersndfile.cpp Implements Libsndfile-based generic audio reader
 *
 */


#ifndef __LIBARCSDEC_READERSNDFILE_HPP__
#include "readersndfile.hpp"
#endif

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <sndfile.hh>

#ifndef __LIBARCS_LOGGING_HPP__
#include <arcs/logging.hpp>
#endif

#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"
#endif
#ifndef __LIBARCSDEC_AUDIOBUFFER_HPP__
#include "audiobuffer.hpp"
#endif


namespace arcs
{


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
class LibsndfileAudioReaderImpl : public AudioReaderImpl
{

public:

	/**
	 * Default constructor
	 */
	LibsndfileAudioReaderImpl();

	/**
	 * Virtual default destructor
	 */
	virtual ~LibsndfileAudioReaderImpl() noexcept;

	/**
	 * Return the internal calculator instance
	 *
	 * \return Calculation result
	 */
	Checksums get_result();


private:

	std::unique_ptr<AudioSize> do_acquire_size(const std::string &filename)
		override;

	Checksums do_process_file(const std::string &filename) override;

	void do_set_samples_per_block(const uint32_t &samples_per_block) override;

	uint32_t do_get_samples_per_block() const override;

	void do_set_calc(std::unique_ptr<Calculation> calc) override;

	const Calculation& do_get_calc() const override;

	/**
	 * Non-const access to internal calculator instance
	 *
	 * \return Non-const reference to the internal ARCSCalc
	 */
	Calculation& use_calc();

	/**
	 * Number of samples to be read in one block
	 */
	uint32_t samples_per_block_;

	/**
	 * Internal calculator instance
	 */
	std::unique_ptr<Calculation> calc_;
};


/// @}
/// \endcond IMPL_ONLY


LibsndfileAudioReaderImpl::LibsndfileAudioReaderImpl()
	: AudioReaderImpl()
	, samples_per_block_(BLOCKSIZE::DEFAULT)
	, calc_()
{
	// empty
}


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


Checksums LibsndfileAudioReaderImpl::do_process_file(const std::string &filename)
{
	SndfileHandle audiofile(filename, SFM_READ);

	// TODO Check whether this was successful

	// Perform validation

	if (audiofile.samplerate() - CDDA.SAMPLES_PER_SECOND != 0)
	{
		ARCS_LOG_DEBUG << "Samplerate: " << audiofile.samplerate();
		return Checksums(0);
	}

	if (audiofile.channels() - CDDA.NUMBER_OF_CHANNELS != 0)
	{
		ARCS_LOG_DEBUG << "Channels: " << audiofile.channels();
		return Checksums(0);
	}

	if (not (audiofile.format() | SF_FORMAT_PCM_16))
	{
		ARCS_LOG_DEBUG << "Format: " + audiofile.format();
		return Checksums(0);
	}

	ARCS_LOG_INFO << "Validation completed: file seems to be CDDA";


	// Update Calculation with sample count

	AudioSize audiosize;
	audiosize.set_sample_count(audiofile.frames());
	use_calc().update_audiosize(audiosize);

	// Prepare Read buffer (16 bit samples)

	uint32_t buffer_len = this->samples_per_block() * CDDA.NUMBER_OF_CHANNELS;
	std::vector<int16_t> buffer(buffer_len);

	// Prepare sequence (entire buffer content as 32 bit samples)

	std::vector<uint32_t> sequence(this->samples_per_block());
	auto sample = sequence.begin();

	constexpr uint_fast16_t TO_UINT_16 = 0xFFFF;
	uint32_t left  = 0;
	uint32_t right = 1;

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

			auto known_sample_count {
				use_calc().context().audio_size().sample_count() };

			if (known_sample_count - sample_count
					!= ints_in_block / CDDA.NUMBER_OF_CHANNELS)
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
			//sequence.resize(buffer.size() / CDDA.NUMBER_OF_CHANNELS);
		}

		// Prepare sequence

		left  = 0;
		right = 1;
		for (sample = sequence.begin(); sample != sequence.end();
				++sample, left += 2, right += 2)
		{
			*sample  = static_cast<uint32_t>(buffer[right]) << 16u;
			*sample += static_cast<uint32_t>(buffer[left] )  & TO_UINT_16;
		}
		sample_count += sequence.size();


		ARCS_LOG(LOG_DEBUG1) << "  Size: "
				<< (buffer.size() * sizeof(buffer[0])) << " bytes";
		ARCS_LOG(LOG_DEBUG1) << "        "
				<< (buffer.size() / CDDA.NUMBER_OF_CHANNELS)
				<< " Stereo PCM samples (32 bit)";

		use_calc().update(sequence.begin(), sequence.end());
	}

	return this->get_result();
}


Checksums LibsndfileAudioReaderImpl::get_result()
{
	return calc_->result();
}


void LibsndfileAudioReaderImpl::do_set_samples_per_block(
		const uint32_t &samples_per_block)
{
	samples_per_block_ = samples_per_block;
}


uint32_t LibsndfileAudioReaderImpl::do_get_samples_per_block() const
{
	return samples_per_block_;
}


void LibsndfileAudioReaderImpl::do_set_calc(std::unique_ptr<Calculation> calc)
{
	calc_ = std::move(calc);
}


const Calculation& LibsndfileAudioReaderImpl::do_get_calc() const
{
	return *calc_;
}


Calculation& LibsndfileAudioReaderImpl::use_calc()
{
	return *calc_;
}


} // namespace


// FileFormatSndfile


FileFormatSndfile::~FileFormatSndfile() noexcept = default;


std::string FileFormatSndfile::do_name() const
{
	return "unknown (handled by sndfile)";
}


bool FileFormatSndfile::do_can_have_bytes(const std::vector<char> & /* bytes */,
		const uint64_t & /* offset */) const
{
	return true;
}


bool FileFormatSndfile::do_can_have_suffix(const std::string & /* suffix */)
	const
{
	return true;
}


std::unique_ptr<FileReader> FileFormatSndfile::do_create_reader() const
{
	auto impl = std::make_unique<LibsndfileAudioReaderImpl>();

	return std::make_unique<AudioReader>(std::move(impl));
}


std::unique_ptr<FileFormat> FileFormatSndfile::do_clone() const
{
	return std::make_unique<FileFormatSndfile>();
}

} // namespace arcs

