/**
 * \file readerwvpk.cpp Implements audio reader for Wavpack audio files
 *
 */


#ifndef __LIBARCSDEC_READERWVPK_HPP__
#include "readerwvpk.hpp"
#endif

extern "C" {
#include <wavpack/wavpack.h>
}

#include <cstdlib>
#include <cstdint>
#include <locale> // for locale
#include <memory>
#include <stdexcept>
#include <string>

#ifndef __LIBARCS_CALCULATE_HPP__
#include <arcs/calculate.hpp> // from .h
#endif
#ifndef __LIBARCS_SAMPLES_HPP__
#include <arcs/samples.hpp>
#endif
#ifndef __LIBARCS_LOGGING_HPP__
#include <arcs/logging.hpp>
#endif

#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp" // from .h
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
 * \internal \defgroup readerwvpkImpl Implementation details for reading Wavpack files
 *
 * \ingroup readerwvpk
 *
 * The Wavpack AudioReader will only read Wavpack files containing losslessly
 * compressed integer samples. Validation requires CDDA conform samples in PCM
 * format. Float samples are not supported. Original file formats other than
 * WAV are not supported
 *
 * @{
 */


/**
 * Represents an interface for different reference CDDA representations for the
 * WAV format. For a concrete format like RIFFWAV/PCM, this interface can just
 * be implemented.
 */
class WAVPACK_CDDA_t
{

public:


	/**
	 * Default destructor
	 */
	virtual ~WAVPACK_CDDA_t() noexcept;

	/**
	 * Declare whether WAV format is required
	 *
	 * \return TRUE iff only WAV format is exclusively required, otherwise FALSE
	 */
	virtual bool wav_format_only() const = 0;

	/**
	 * Expect lossless compression. This method is non-virtual because it
	 * makes no sense to ever change this requirement.
	 *
	 * \return TRUE, since ARCS cannot be computed on lossly compressed files
	 */
	bool lossless() const;

	/**
	 * Declare whether it is required to have samples represented as integers.
	 * In future versions, it may be supported to process float samples.
	 *
	 * \return TRUE iff float samples can be processed, otherwise FALSE
	 */
	virtual bool floats_ok() const = 0;

	/**
	 * Declare the least version of wavpack that is supported. Files with a
	 * lower version cannot be processed.
	 *
	 * \return The least version of wavpack supported
	 */
	virtual uint8_t at_least_version() const;

	/**
	 * Declare the highest version of wavpack that is supported. Files with a
	 * higher version cannot be processed.
	 *
	 * \return The highest version of wavpack supported
	 */
	virtual uint8_t at_most_version() const;

	/**
	 * Declare the required number of bytes per sample and channel. (For CDDA,
	 * this is 2 for 16 bit.)
	 *
	 * \return Require 2 bytes (for 16 bit).
	 */
	virtual uint16_t bytes_per_sample() const;
};


WAVPACK_CDDA_t::~WAVPACK_CDDA_t() noexcept = default;


bool WAVPACK_CDDA_t::lossless() const
{
	return true; // Always require losless compression
}


uint8_t WAVPACK_CDDA_t::at_least_version() const
{
	return 1;
}


uint8_t WAVPACK_CDDA_t::at_most_version() const
{
	return 5;
}


uint16_t WAVPACK_CDDA_t::bytes_per_sample() const
{
	return 2; // Always require 16bit per sample and channel
}


// WAVPACK_CDDA_WAV_PCM_t


/**
 * Implements reference values for CDDA compliant WAVPACK of integer PCM.
 */
class WAVPACK_CDDA_WAV_PCM_t : public WAVPACK_CDDA_t
{

public:


	/**
	 * Default destructor
	 */
	virtual ~WAVPACK_CDDA_WAV_PCM_t() noexcept;

	/**
	 * Specifies WAV format as exclusively required.
	 *
	 * \return TRUE
	 */
	bool wav_format_only() const;

	/**
	 * Specifies float samples as not supported. (This in fact, restricts the
	 * support to integer samples.)
	 *
	 * \return FALSE
	 */
	virtual bool floats_ok() const;
};


WAVPACK_CDDA_WAV_PCM_t::~WAVPACK_CDDA_WAV_PCM_t() noexcept = default;


bool WAVPACK_CDDA_WAV_PCM_t::wav_format_only() const
{
	return true;
}


bool WAVPACK_CDDA_WAV_PCM_t::floats_ok() const
{
	return false;
}


// WavpackOpenFile


/**
 * Represents an opened wavpack audio file.
 */
class WavpackOpenFile
{

public:

	/**
	 * Open wavpack file with the given name. This class does RAII and does
	 * not require you to manually close the instance.
	 *
	 * \param[in] filename The file to open
	 *
	 * \throw FileReadException If file could not be opened
	 */
	explicit WavpackOpenFile(const std::string &filename);

	/**
	 * Destructor
	 */
	virtual ~WavpackOpenFile() noexcept;

	/**
	 * Returns TRUE if file is lossless.
	 *
	 * \return TRUE iff file contains losslessly compressed data, otherwise
	 * FALSE
	 */
	bool is_lossless() const;

	/**
	 * Returns TRUE if original file was WAV.
	 *
	 * \return TRUE iff the original file had WAV format, otherwise FALSE.
	 */
	bool has_wav_format() const;

	/**
	 * Returns TRUE if original file had float samples.
	 *
	 * \return TRUE iff the original file contained float samples
	 */
	bool has_float_samples() const;

	/**
	 * Returns the Wavpack version that built this file.
	 *
	 * \return Number of the Wavpack version used for encoding this file
	 */
	uint8_t version() const;

	/**
	 * Returns the bits per sample.
	 *
	 * \return Number of bits per sample
	 */
	uint16_t bits_per_sample() const;

	/**
	 * Returns the number of channels.
	 *
	 * \return Number of channels
	 */
	uint16_t num_channels() const;

	/**
	 * Returns the sampling rate as number of samples per second
	 *
	 * \return Number of samples per second
	 */
	uint32_t samples_per_second() const;

	/**
	 * Returns the bytes per sample (and channel, hence it is 2 for CDDA)
	 *
	 * \return Number of bytes per sample
	 */
	uint16_t bytes_per_sample() const;

	/**
	 * Returns the total number of 32bit PCM samples this file contains
	 *
	 * \return Total number of 32bit PCM samples in this file
	 */
	uint32_t total_pcm_samples() const;

	/**
	 * Returns TRUE iff the channel ordering is left/right, otherwise FALSE.
	 *
	 * \return TRUE iff the channel ordering is left/right, otherwise FALSE.
	 */
	bool channel_order() const;

	/**
	 * Read the specified number of 32 bit PCM samples. Note that each
	 * 16 bit sample will be stored as a single int32, so buffer will require
	 * a size of pcm_samples_to_read * number_of_channels to store the requested
	 * number of samples.
	 *
	 * \param[in] pcm_samples_to_read Number of 32 bit samples to read
	 * \param[in,out] buffer The buffer to read the samples to
	 *
	 * \return Number of 32 bit PCM samples actually read
	 */
	uint32_t read_pcm_samples(const uint32_t &pcm_samples_to_read,
		std::vector<int32_t> *buffer) const;


private:

	/**
	 * Internal WavpackContext
	 */
	WavpackContext *context_;

	/**
	 * Private copy constructor.
	 *
	 * Implemented empty. This class is non-copyable.
	 */
	WavpackOpenFile(const WavpackOpenFile &file) = delete;

	/**
	 * Private copy assignment operator.
	 *
	 * Implemented empty. This class is non-copyable.
	 */
	WavpackOpenFile& operator = (WavpackOpenFile &file) = delete;
};


WavpackOpenFile::WavpackOpenFile(const std::string &filename)
	: context_()
{
	int   flags = OPEN_WVC | OPEN_NO_CHECKSUM ;
	char *error = nullptr;

	context_ = WavpackOpenFileInput(filename.c_str(), error, flags, 0);

	if (!context_)
	{
		if (error)
		{
			ARCS_LOG_ERROR << "Could not open file. Error was: "
				+ std::string(error);

			free(error);
			error = nullptr;
		}

		throw FileReadException("Wavpack file could not be opened");
	}
}


WavpackOpenFile::~WavpackOpenFile() noexcept
{
	if (context_)
	{
		context_ = WavpackCloseFile(context_);

		if (!context_)
		{
			ARCS_LOG_DEBUG << "Audio file closed.";
		} else
		{
			ARCS_LOG_ERROR <<
				"Problem closing audio file. Possible leak!";
		}
	}
}


bool WavpackOpenFile::is_lossless() const
{
	return MODE_LOSSLESS & WavpackGetMode(context_);
}


bool WavpackOpenFile::has_wav_format() const
{
	return WP_FORMAT_WAV == WavpackGetFileFormat(context_);
}


bool WavpackOpenFile::has_float_samples() const
{
	return MODE_FLOAT & WavpackGetMode(context_);
}


uint8_t WavpackOpenFile::version() const
{
	return WavpackGetVersion(context_);
}


uint16_t WavpackOpenFile::bits_per_sample() const
{
	return WavpackGetBitsPerSample(context_);
}


uint16_t WavpackOpenFile::num_channels() const
{
	return WavpackGetNumChannels(context_);
}


uint32_t WavpackOpenFile::samples_per_second() const
{
	return WavpackGetSampleRate(context_);
}


uint16_t WavpackOpenFile::bytes_per_sample() const
{
	return WavpackGetBytesPerSample(context_);
}


uint32_t WavpackOpenFile::total_pcm_samples() const
{
	// We do not need to handle files with more than 0xFFFFFFFF samples
	// since we only check compact disc images

	int64_t total_pcm_samples = WavpackGetNumSamples64(context_);

	if (total_pcm_samples == -1)
	{
		ARCS_LOG_ERROR << "Could not determine total number of samples";
		return 0;
	}

	if (total_pcm_samples > 0xFFFFFFFF) // maximum value for uint32_t
	{
		ARCS_LOG_ERROR << "Number of samples is too big: "
				+ std::to_string(total_pcm_samples);
		return 0;
	}

	return total_pcm_samples;
}


bool WavpackOpenFile::channel_order() const
{
	// TODO Correctly determine whether the channel ordering is not left/right

	// Channel mask

	int channel_mask = WavpackGetChannelMask(context_);
	ARCS_LOG_DEBUG << "Channel mask: " << channel_mask;
	// '3' should indicate stereo (countercheck necessary??)

	// Channel layout

// Commented out: unclear to me whether layout is of use, commented out for now
//
//  // Extract layout, counter-check channel number and test for reordering
//
//	int channels = 0;
//
//	{
//		unsigned char* reorder_str;
//
//		uint32_t channel_layout =
//			WavpackGetChannelLayout(context_, reorder_str);
//
//		ARCS_LOG_DEBUG << "Channel layout: " << channel_layout;
//
//		if (reorder_str)
//		{
//			ARCS_LOG_DEBUG << "Channel reorder: " << reorder_str;
//		} else
//		{
//			ARCS_LOG_DEBUG << "No reordering defined";
//		}
//
//		// Count channels in layout
//
//		int bits = 16;
//		while (channel_layout && bits > 0)
//		{
//			channels += channel_layout & 1;
//			channel_layout >>= 1;
//			--bits;
//		}
//		ARCS_LOG_INFO << "Number of channels declared in layout: " << channels;
//	}

	// Channel identities

	// Remember that this->num_channels() == CDDA.NUMBER_OF_CHANNELS is part of
	// validation

	if (channel_mask == 3)
	{
		unsigned char identities[3] = { '0', '0', '\0' };

		WavpackGetChannelIdentities(context_, identities);

		if (identities)
		{
			auto first  { identities[0] };
			auto second { identities[1] };

			ARCS_LOG_DEBUG << "Channel identities: channel 0 = '"
				<< std::to_string(first)
				<< "', channel 1 = '" << std::to_string(second) << "'";

			if (first == 1 and second == 2)
			{
				ARCS_LOG_INFO << "Channel assignment: left/right";
				return true;
			}
		}
	}

	return false;
}


uint32_t WavpackOpenFile::read_pcm_samples(const uint32_t &pcm_samples_to_read,
		std::vector<int32_t> *buffer) const
{
	if (buffer->size() < pcm_samples_to_read * CDDA.NUMBER_OF_CHANNELS)
	{
		ARCS_LOG_ERROR << "Buffer size " << buffer->size()
				<< " is too small for the requested "
				<< std::to_string(pcm_samples_to_read)
				<< " samples. At least size of "
				<< (pcm_samples_to_read * CDDA.NUMBER_OF_CHANNELS)
				<< " integers is required.";

		return 0;
	}

	uint32_t samples_read = WavpackUnpackSamples(context_, &((*buffer)[0]),
			pcm_samples_to_read);

	ARCS_LOG_DEBUG << "    Read " << samples_read << " PCM samples (32 bit)";

	return samples_read;
}


// WavpackValidatingHandler


/**
 * A handler to validate wavpack files for whether they can be processed by
 * this WavpackAudioReaderImpl.
 */
class WavpackValidatingHandler : public ReaderValidatingHandler
{

public:

	/**
	 * Constructor with reference values
	 */
	explicit WavpackValidatingHandler(
			std::unique_ptr<WAVPACK_CDDA_t> valid_values);

	/**
	 * Virtual default destructor
	 */
	virtual ~WavpackValidatingHandler() noexcept;

	/**
	 * Validate the format of the wavpack file
	 *
	 * \param[in] file The opened file
	 *
	 * \return TRUE if validation succeeded, otherwise FALSE
	 */
	bool validate_format(const WavpackOpenFile &file);

	/**
	 * Validate properties expressed by the mode of the wavpack file
	 *
	 * \param[in] file The opened file
	 *
	 * \return TRUE if validation succeeded, otherwise FALSE
	 */
	bool validate_mode(const WavpackOpenFile &file);

	/**
	 * Validate audio content of the wavpack file for CDDA conformity
	 *
	 * \param[in] file The opened file
	 *
	 * \return TRUE if validation succeeded, otherwise FALSE
	 */
	bool validate_cdda(const WavpackOpenFile &file);

	/**
	 * Validate the wavpack version of the wavpack file
	 *
	 * \param[in] file The opened file
	 *
	 * \return TRUE if validation succeeded, otherwise FALSE
	 */
	bool validate_version(const WavpackOpenFile &file);


private:

	/**
	 * Configuration: Reference values for validation
	 */
	std::unique_ptr<WAVPACK_CDDA_t> valid_;
};


WavpackValidatingHandler::WavpackValidatingHandler(
		std::unique_ptr<WAVPACK_CDDA_t> valid_values)
	: ReaderValidatingHandler()
	, valid_(std::move(valid_values))
{
	// empty
}


WavpackValidatingHandler::~WavpackValidatingHandler() noexcept = default;


bool WavpackValidatingHandler::validate_format(const WavpackOpenFile &file)
{
	if (not this->assert_true("Test: WAV file format",
		file.has_wav_format() == valid_->wav_format_only(),
		"File format is not correct"))
	{
		ARCS_LOG_ERROR << this->last_error();
		return false;
	}

	return true;
}


bool WavpackValidatingHandler::validate_mode(const WavpackOpenFile &file)
{
	if (not this->assert_true("Test: Lossless compression",
		file.is_lossless() == valid_->lossless(),
		"Mode is not lossless"))
	{
		ARCS_LOG_ERROR << this->last_error();
		return false;
	}

	if (not this->assert_true("Test: No float samples",
		file.has_float_samples() == valid_->floats_ok(),
		"Float format not supported in this version"))
	{
		ARCS_LOG_ERROR << this->last_error();
		return false;
	}

	return true;
}


bool WavpackValidatingHandler::validate_cdda(const WavpackOpenFile &file)
{
	CDDAValidator validate;

	if (not this->assert_true("Test: Bits per sample",
		validate.bits_per_sample(file.bits_per_sample()),
		"Number of bits per sample does not conform to CDDA"))
	{
		ARCS_LOG_ERROR << this->last_error();
		return false;
	}

	if (not this->assert_true("Test: Channels",
		validate.num_channels(file.num_channels()),
		"Number of channels does not conform to CDDA"))
	{
		ARCS_LOG_ERROR << this->last_error();
		return false;
	}

	if (not this->assert_true("Test: Samples per second",
		validate.samples_per_second(file.samples_per_second()),
		"Number of samples per second does not conform to CDDA"))
	{
		ARCS_LOG_ERROR << this->last_error();
		return false;
	}

	if (not this->assert_equals("Test: Bytes per sample",
		file.bytes_per_sample(), valid_->bytes_per_sample(),
		"Number of samples per second does not conform to CDDA"))
	{
		ARCS_LOG_ERROR << this->last_error();
		return false;
	}

	return true;
}


bool WavpackValidatingHandler::validate_version(const WavpackOpenFile &file)
{
	if (not this->assert_at_least("Test: Wavpack least version supported",
		file.version(), valid_->at_least_version(),
		"Wavpack version is not supported"))
	{
		ARCS_LOG_ERROR << this->last_error();
		return false;
	}

	if (not this->assert_at_most("Test: Wavpack highest version supported",
		file.version(), valid_->at_most_version(),
		"Wavpack version is not supported"))
	{
		ARCS_LOG_ERROR << this->last_error();
		return false;
	}

	return true;
}


// WavpackAudioReaderImpl


/**
 * Implementation of a AudioReader for the Wavpack format.
 */
class WavpackAudioReaderImpl : public BufferedAudioReaderImpl
{

public:

	/**
	 * Default constructor
	 */
	WavpackAudioReaderImpl();

	/**
	 * Default destructor
	 */
	virtual ~WavpackAudioReaderImpl() noexcept;

	/**
	 * Register a validating handler.
	 *
	 * \param[in] v The validating handler to register
	 */
	void register_validate_handler(std::unique_ptr<WavpackValidatingHandler> v);

	/**
	 * Access to the validating handler.
	 *
	 * \return The validating handler of this instance
	 */
	const WavpackValidatingHandler& validate_handler();


private:

	std::unique_ptr<AudioSize> do_acquire_size(const std::string &filename)
		override;

	void do_process_file(const std::string &filename) override;

	/**
	 * Perform the actual validation process
	 *
	 * \param[in] file The file to validate
	 *
	 * \return TRUE iff validation succeeded, otherwise FALSE
	 */
	bool perform_validations(const WavpackOpenFile &file);

	/**
	 * Validating handler of this instance
	 */
	std::unique_ptr<WavpackValidatingHandler> validate_handler_;
};


WavpackAudioReaderImpl::WavpackAudioReaderImpl()
	: validate_handler_()
{
	// empty
}


WavpackAudioReaderImpl::~WavpackAudioReaderImpl() noexcept = default;


std::unique_ptr<AudioSize> WavpackAudioReaderImpl::do_acquire_size(
	const std::string &filename)
{
	std::unique_ptr<AudioSize> audiosize = std::make_unique<AudioSize>();

	WavpackOpenFile file(filename);

	audiosize->set_sample_count(file.total_pcm_samples());

	return audiosize;
}


void WavpackAudioReaderImpl::do_process_file(const std::string &filename)
{
	ARCS_LOG_DEBUG << "Start validating Wavpack file: " << filename;
	WavpackOpenFile file(filename);

	if (not validate_handler_)
	{
		ARCS_LOG_ERROR << "No validator configured, cannot validate file.";
	}

	if (not perform_validations(file))
	{
		ARCS_LOG_ERROR << "Validation failed";
		return;
	}

	ARCS_LOG_DEBUG << "Completed validation of Wavpack file";

	uint32_t total_samples = file.total_pcm_samples();
	ARCS_LOG_INFO << "Total samples: " << total_samples;

	// Notify about correct size
	{
		AudioSize size;
		size.set_sample_count(total_samples);
		this->process_audiosize(size);
	}

	// Samples reading loop
	{
		std::vector<int32_t> samples;
		SampleSequence<int32_t, false> sequence(file.channel_order());

		samples.resize(this->samples_per_read());

		// Request half the number of samples in a block, thus a sequence will
		// have exactly the size of a block.
		uint32_t wv_sample_count =
			this->samples_per_read() / CDDA.NUMBER_OF_CHANNELS;
		uint32_t samples_read    = 0;

		for (int32_t i = total_samples; i > 0; i -= wv_sample_count)
		{
			ARCS_LOG_DEBUG << "READ SEQUENCE, remaining samples " << i;

			samples_read = file.read_pcm_samples(wv_sample_count, &samples);

			if (samples_read != wv_sample_count)
			{
				// Chunk is smaller than declared, this only ok for last chunk
				// (last run will see i == total_samples % wv_sample_count)

				if (samples_read != static_cast<uint32_t>(i))
				{
					std::stringstream ss;
					ss << "    Read unexpected number of samples: "
						<< samples_read
						<< ", but expected "
						<< wv_sample_count
						<< ". Resize the sequence accordingly.";

					ARCS_LOG_ERROR << ss.str();

					throw FileReadException(ss.str());
				}

				samples.resize(samples_read * CDDA.NUMBER_OF_CHANNELS);
			}

			ARCS_LOG_DEBUG << "    Size: " << samples.size()
					<< " integers, add to current block";

			sequence.reset(samples.data(), samples.size());
			// NOTE That we use the number of 16 bit samples per channel, not
			// the total number of 16 bit samples in the chunk

			this->process_samples(sequence.begin(), sequence.end());
		}
	}
}


void WavpackAudioReaderImpl::register_validate_handler(
		std::unique_ptr<WavpackValidatingHandler> validator)
{
	validate_handler_ = std::move(validator);
}


const WavpackValidatingHandler& WavpackAudioReaderImpl::validate_handler()
{
	return *validate_handler_;
}


bool WavpackAudioReaderImpl::perform_validations(const WavpackOpenFile &file)
{
	// format

	if (not validate_handler_->validate_format(file))
	{
		return false;
	}

	// mode

	if (not validate_handler_->validate_mode(file))
	{
		return false;
	}

	// cdda

	if (not validate_handler_->validate_cdda(file))
	{
		return false;
	}

	// version

	if (not validate_handler_->validate_version(file))
	{
		return false;
	}

	return true;
}

/// @}

} // namespace


// FileFormatWavpack


FileFormatWavpack::~FileFormatWavpack() noexcept = default;


std::string FileFormatWavpack::do_name() const
{
	return "Wavpack";
}


bool FileFormatWavpack::do_can_have_bytes(const std::vector<char> &bytes,
		const uint64_t &offset) const
{
	return  bytes.size() >= 4
		and offset       == 0
		and bytes [0]    == 0x77  // w
		and bytes [1]    == 0x76  // v
		and bytes [2]    == 0x70  // p
		and bytes [3]    == 0x6B  // k
		;
}


bool FileFormatWavpack::do_can_have_suffix(const std::string &suffix) const
{
	std::locale locale;
	return std::tolower(suffix, locale) == "wv";
}


std::unique_ptr<FileReader> FileFormatWavpack::do_create_reader() const
{
	auto impl = std::make_unique<WavpackAudioReaderImpl>();

	std::unique_ptr<WAVPACK_CDDA_t> valid =
		std::make_unique<WAVPACK_CDDA_WAV_PCM_t>();
	auto validator =
		std::make_unique<WavpackValidatingHandler>(std::move(valid));

	impl->register_validate_handler(std::move(validator));

	return std::make_unique<AudioReader>(std::move(impl));
}


std::unique_ptr<FileFormat> FileFormatWavpack::do_clone() const
{
	return std::make_unique<FileFormatWavpack>();
}

} // namespace arcs

