/**
 * \file
 *
 * \brief Implements audio reader for Wavpack audio files.
 */

#ifndef __LIBARCSDEC_READERWVPK_HPP__
#include "readerwvpk.hpp"
#endif
#ifndef __LIBARCSDEC_READERWVPK_DETAILS_HPP__
#include "readerwvpk_details.hpp"
#endif

extern "C" {
#include <wavpack/wavpack.h>
}

#include <cstdint>
#include <memory>
#include <stdexcept> // for invalid_argument
#include <sstream>   // for ostringstream
#include <string>
#include <vector>

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>     // for CDDA
#endif
#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>
#endif
#ifndef __LIBARCSTK_SAMPLES_HPP__
#include <arcstk/samples.hpp>        // for SampleSequence
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
namespace details
{
namespace wavpack
{

using arcstk::SampleInputIterator;
using arcstk::AudioSize;
using arcstk::CDDA;
using arcstk::InterleavedSamples;


LibwavpackException::LibwavpackException(const std::string &value,
		const std::string &name, const std::string &error_msg)
	: msg_ {}
{
	std::ostringstream msg;
	msg << "libwavpack: Function " << name  << " returned unexpected value '"
		<< value << "'";

	if (not error_msg.empty())
	{
		msg << ", error message: '" << error_msg << "'";
	}

	msg_ = msg.str();
}


char const * LibwavpackException::what() const noexcept
{
	return msg_.c_str();
}


/**
 * \brief Private libwavpack-based implementation of WavpackOpenFile.
 */
class WavpackOpenFile::Impl final
{

public:

	/**
	 * \brief The filename to open.
	 *
	 * \param[in] filename The filename to open
	 */
	Impl(const std::string &filename);

	Impl(const Impl &impl) = delete;

	/**
	 * \brief Destructor
	 */
	~Impl() noexcept;

	/**
	 * \brief Returns TRUE if file is lossless.
	 *
	 * \return TRUE iff file contains losslessly compressed data, otherwise
	 * FALSE
	 */
	bool is_lossless() const;

	/**
	 * \brief Returns TRUE if original file was WAV.
	 *
	 * \return TRUE iff the original file had WAV format, otherwise FALSE.
	 */
	bool has_wav_format() const;

	/**
	 * \brief Returns TRUE if original file had float samples.
	 *
	 * \return TRUE iff the original file contained float samples
	 */
	bool has_float_samples() const;

	/**
	 * \brief Returns the Wavpack version that built this file.
	 *
	 * \return Number of the Wavpack version used for encoding this file
	 */
	uint8_t version() const;

	/**
	 * \brief Returns the bits per sample.
	 *
	 * \return Number of bits per sample
	 */
	int bits_per_sample() const;

	/**
	 * \brief Returns the number of channels.
	 *
	 * \return Number of channels
	 */
	int num_channels() const;

	/**
	 * \brief Returns the sampling rate as number of samples per second.
	 *
	 * \return Number of samples per second
	 */
	int64_t samples_per_second() const;

	/**
	 * \brief Returns the bytes per sample (and channel, hence it is 2 for
	 * CDDA).
	 *
	 * \return Number of bytes per sample
	 */
	int bytes_per_sample() const;

	/**
	 * \brief Returns the total number of 32bit PCM samples this file contains.
	 *
	 * \return Total number of 32bit PCM samples in this file
	 */
	int64_t total_pcm_samples() const;

	/**
	 * \brief Returns TRUE iff the channel ordering is left/right, otherwise
	 * FALSE.
	 *
	 * \return TRUE iff the channel ordering is left/right, otherwise FALSE.
	 */
	bool channel_order() const;

	/**
	 * \brief Returns channel mask (expect 3 to indicate stereo for CDDA).
	 *
	 * \return The channel mask of the wavepack file
	 */
	int channel_mask() const;

	/**
	 * \brief Indicates whether the core audio file needs channel reorder.
	 *
	 * The core audio file may have a different channel ordering than the
	 * Wavepack container.
	 *
	 * \return TRUE if the channels have to be reordered, otherwise FALSE
	 */
	bool needs_channel_reorder() const;

	/**
	 * \brief Read the specified number of 32 bit PCM samples.
	 *
	 * Note that each
	 * 16 bit sample will be stored as a single int32, so buffer will require
	 * a size of pcm_samples_to_read * number_of_channels to store the requested
	 * number of samples.
	 *
	 * \param[in] pcm_samples_to_read Number of 32 bit samples to read
	 * \param[in,out] buffer The buffer to read the samples to
	 *
	 * \return Number of 32 bit PCM samples actually read
	 */
	int64_t read_pcm_samples(const int64_t pcm_samples_to_read,
		std::vector<int32_t> &buffer) const;

	// Delete copy assignment operator
	WavpackOpenFile::Impl& operator = (WavpackOpenFile::Impl &file) = delete;


private:

	/**
	 * \brief Internal WavpackContext.
	 */
	WavpackContext *context_;
};


// WAVPACK_CDDA_t


WAVPACK_CDDA_t::~WAVPACK_CDDA_t() noexcept = default;


bool WAVPACK_CDDA_t::lossless() const
{
	return true; // Always require losless compression
}


int WAVPACK_CDDA_t::bytes_per_sample() const
{
	return 2; // Always require 16bit per sample and channel
}


bool WAVPACK_CDDA_t::wav_format_only() const
{
	return true;
}


bool WAVPACK_CDDA_t::floats_ok() const
{
	return false;
}


int WAVPACK_CDDA_t::at_least_version() const
{
	return 1;
}


int WAVPACK_CDDA_t::at_most_version() const
{
	return 5;
}


// WavpackOpenFile


WavpackOpenFile::Impl::Impl(const std::string &filename)
	: context_()
{
	int   flags = OPEN_WVC | OPEN_NO_CHECKSUM ;

	char *error = nullptr;
	context_ = WavpackOpenFileInput(filename.c_str(), error, flags, 0);

	if (!context_)
	{
		ARCS_LOG_ERROR << "Wavpack file could not be opened";

		std::string error_msg;
		if (error)
		{
			error_msg = std::string(error);
			free(error);
			error = nullptr;
		}

		throw LibwavpackException("NULL", "WavpackOpenFileInput", error_msg);
	}
}


WavpackOpenFile::Impl::~Impl() noexcept
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
				"WavpackCloseFile could not close audio file. Possible leak!";
		}
	}
}


bool WavpackOpenFile::Impl::is_lossless() const
{
	return MODE_LOSSLESS & WavpackGetMode(context_);
}


bool WavpackOpenFile::Impl::has_wav_format() const
{
	return WP_FORMAT_WAV == WavpackGetFileFormat(context_);
}


bool WavpackOpenFile::Impl::has_float_samples() const
{
	return MODE_FLOAT & WavpackGetMode(context_);
}


uint8_t WavpackOpenFile::Impl::version() const
{
	return WavpackGetVersion(context_);
}


int WavpackOpenFile::Impl::bits_per_sample() const
{
	return WavpackGetBitsPerSample(context_);
}


int WavpackOpenFile::Impl::num_channels() const
{
	return WavpackGetNumChannels(context_);
}


int64_t WavpackOpenFile::Impl::samples_per_second() const
{
	return WavpackGetSampleRate(context_);
}


int WavpackOpenFile::Impl::bytes_per_sample() const
{
	return WavpackGetBytesPerSample(context_);
}


int64_t WavpackOpenFile::Impl::total_pcm_samples() const
{
	int64_t total_pcm_samples = WavpackGetNumSamples64(context_);

	if (total_pcm_samples == -1)
	{
		throw LibwavpackException("-1", "WavpackGetNumSamples64",
				"Could not determine total number of samples");
	}

	return total_pcm_samples;
}


bool WavpackOpenFile::Impl::channel_order() const
{
	const int num_channels = WavpackGetNumChannels(context_);

	if (num_channels != 2)
	{
		std::ostringstream msg;
		msg << "Expected 2 channels but got " << num_channels
			<< ", input does not seem to be CDDA compliant (stereo)";

		throw InvalidAudioException(msg.str());
	}

	std::vector<unsigned char> identities(
			static_cast<unsigned int>(num_channels + 1)); // +1 for \0

	WavpackGetChannelIdentities(context_, identities.data());

	if (identities.empty() or identities.size() < 2)
	{
		std::ostringstream msg;
		msg << "Expected 2 channels but got not enough identities ("
			<< identities.size()
			<< "), input does not seem to be CDDA compliant (stereo)";

		throw InvalidAudioException(msg.str());
	}

	if (identities[0] == 1 && identities[1] == 2)
	{
		ARCS_LOG_INFO << "Channel order: left/right";
		return true;
	}

	ARCS_LOG_DEBUG << "Channel order: channel 0 = '" << identities[0]
		<< "', channel 1 = '" << identities[1] << "'";

	return false;
}


int WavpackOpenFile::Impl::channel_mask() const
{
	// stereo == 3
	return WavpackGetChannelMask(context_);
}


bool WavpackOpenFile::Impl::needs_channel_reorder() const
{
	// Channel layout of the core audio file

	unsigned char* reorder_str = nullptr;
	auto channel_layout = WavpackGetChannelLayout(context_, reorder_str);

	if (!reorder_str)
	{
		return false;
	}

	unsigned int channel_count = 0;

	while (channel_layout > 0)
	{
		channel_count += channel_layout & 1;
		channel_layout >>= 1;
	}

	ARCS_LOG_DEBUG << "Number of channels declared in layout: "
		<< channel_count;
	ARCS_LOG_DEBUG << "Channel layout:  " << channel_layout
		<< " (uint32_t))";
	ARCS_LOG_DEBUG << "Channel reorder: " << reorder_str;

	return true;
}


int64_t WavpackOpenFile::Impl::read_pcm_samples(
		const int64_t pcm_samples_to_read,
		std::vector<int32_t> &buffer) const
{
	const auto samples_to_read =
		static_cast<uint64_t>(pcm_samples_to_read * CDDA::NUMBER_OF_CHANNELS);

	if (buffer.size() < samples_to_read)
	{
		std::ostringstream msg;
		msg << "Buffer size " << buffer.size()
			<< " is too small for the requested "
			<< std::to_string(pcm_samples_to_read)
			<< " samples. At least size of "
			<< (pcm_samples_to_read * CDDA::NUMBER_OF_CHANNELS)
			<< " integers is required.";

		throw std::invalid_argument(msg.str());
	}

	const auto samples_read = WavpackUnpackSamples(context_, &(buffer[0]),
			pcm_samples_to_read);

	ARCS_LOG_DEBUG << "    Read " << samples_read << " PCM samples (32 bit)";

	return samples_read;
}


// WavpackOpenFile


WavpackOpenFile::WavpackOpenFile(const std::string &filename)
	: impl_(std::make_unique<WavpackOpenFile::Impl>(filename))
{
	// empty
}


WavpackOpenFile::~WavpackOpenFile() noexcept = default;


bool WavpackOpenFile::is_lossless() const
{
	return impl_->is_lossless();
}


bool WavpackOpenFile::has_wav_format() const
{
	return impl_->has_wav_format();
}


bool WavpackOpenFile::has_float_samples() const
{
	return impl_->has_float_samples();
}


uint8_t WavpackOpenFile::version() const
{
	return impl_->version();
}


int WavpackOpenFile::bits_per_sample() const
{
	return impl_->bits_per_sample();
}


int WavpackOpenFile::num_channels() const
{
	return impl_->num_channels();
}


int64_t WavpackOpenFile::samples_per_second() const
{
	return impl_->samples_per_second();
}


int WavpackOpenFile::bytes_per_sample() const
{
	return impl_->bytes_per_sample();
}


int64_t WavpackOpenFile::total_pcm_samples() const
{
	return impl_->total_pcm_samples();
}


bool WavpackOpenFile::channel_order() const
{
	return impl_->channel_order();
}


int WavpackOpenFile::channel_mask() const
{
	return impl_->channel_mask();
}


bool WavpackOpenFile::needs_channel_reorder() const
{
	return impl_->needs_channel_reorder();
}


int64_t WavpackOpenFile::read_pcm_samples(const int64_t pcm_samples_to_read,
		std::vector<int32_t> &buffer) const
{
	return impl_->read_pcm_samples(pcm_samples_to_read, buffer);
}



// WavpackValidatingHandler


WavpackValidatingHandler::WavpackValidatingHandler(
		std::unique_ptr<WAVPACK_CDDA_t> valid_values)
	: DefaultValidator()
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
	if (not this->assert_true("Test: Bits per sample",
		CDDAValidator::bits_per_sample(file.bits_per_sample()),
		"Number of bits per sample does not conform to CDDA"))
	{
		ARCS_LOG_ERROR << this->last_error();
		return false;
	}

	if (not this->assert_true("Test: Channels",
		CDDAValidator::num_channels(file.num_channels()),
		"Number of channels does not conform to CDDA"))
	{
		ARCS_LOG_ERROR << this->last_error();
		return false;
	}

	if (file.channel_mask() < 0)
	{
		ARCS_LOG_ERROR << "Negative channel mask"; // FIXME Use error stack
		return false;
	}
	if (not this->assert_equals("Test: Channel Mask",
		file.channel_mask(), 3,
		"Channel mask does not conform to CDDA"))
	{
		ARCS_LOG_ERROR << this->last_error();
		return false;
	}

	if (not this->assert_true("Test: No channel reordering",
		not file.needs_channel_reorder(),
		"Channel reordering required, but not implemented yet"))
	{
		ARCS_LOG_ERROR << this->last_error();
		return false;
	}

	if (not this->assert_true("Test: Samples per second",
		CDDAValidator::samples_per_second(file.samples_per_second()),
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


AudioValidator::codec_set_type WavpackValidatingHandler::do_codecs() const
{
	return { Codec::WAVPACK };
}


// WavpackAudioReaderImpl


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

	audiosize->set_total_samples(file.total_pcm_samples());

	return audiosize;
}


void WavpackAudioReaderImpl::do_process_file(const std::string &filename)
{
	this->signal_startinput();

	// Validation

	ARCS_LOG_DEBUG << "Start validating Wavpack file: " << filename;
	WavpackOpenFile file(filename);

	if (not validate_handler_)
	{
		ARCS_LOG_WARNING
			<< "No validation handler configured, cannot validate file.";
	} else
	{
		if (not perform_validations(file))
		{
			ARCS_LOG_ERROR << "Validation failed";
			return;
		}

		ARCS_LOG_DEBUG << "Completed validation of Wavpack file";
	}

	const int64_t total_samples { file.total_pcm_samples() };


	// Notify about correct size

	{
		AudioSize size;
		size.set_total_samples(total_samples);
		this->signal_updateaudiosize(size);
	}


	// Samples reading loop

	{
		using sample_t = int32_t;

		InterleavedSamples<sample_t> sequence(file.channel_order());

		std::vector<sample_t> buffer;
		using buffersize_t = typename decltype(buffer)::size_type;

		buffer.resize(static_cast<buffersize_t>(this->samples_per_read()));

		// Request Half the Number of Samples in a Block in one Read.
		// Thus a Sequence will Have Exactly the Size of a Block.
		const int64_t wv_samples_to_read = this->samples_per_read() / 2;

		int64_t wv_samples_read = 0;
		for (int64_t i = total_samples; i > 0; i -= wv_samples_to_read)
		{
			ARCS_LOG_DEBUG << "READ SEQUENCE, remaining samples " << i;

			wv_samples_read = file.read_pcm_samples(wv_samples_to_read, buffer);

			if (wv_samples_read != wv_samples_to_read)
			{
				// Chunk is Smaller than Declared.
				// This is only Allowed for the Last Chunk.
				// The Last Chunk must have size total_samples % wv_sample_count
				// what is Precisely the Value i has After the Last Loop Run.

				if (wv_samples_read != i)
				{
					std::ostringstream msg;
					msg << "    Read unexpected number of samples: "
						<< wv_samples_read
						<< ", but expected "
						<< wv_samples_to_read;

					throw FileReadException(msg.str());
				}

				buffer.resize(static_cast<buffersize_t>(
						wv_samples_read * CDDA::NUMBER_OF_CHANNELS));
			}

			ARCS_LOG_DEBUG << "    Size: " << buffer.size()
					<< " integers, add to current block";

			sequence.wrap_int_buffer(buffer.data(), buffer.size());
			// Note: we use the Number of 16-bit-samples _per_channel_, not
			// the total number of 16 bit samples in the chunk.

			this->signal_appendsamples(sequence.begin(), sequence.end());
		}
	}

	this->signal_endinput();
}


std::unique_ptr<FileReaderDescriptor> WavpackAudioReaderImpl::do_descriptor()
	const
{
	return std::make_unique<DescriptorWavpack>();
}


void WavpackAudioReaderImpl::register_validate_handler(
		std::unique_ptr<WavpackValidatingHandler> validator)
{
	validate_handler_ = std::move(validator);
}


bool WavpackAudioReaderImpl::perform_validations(const WavpackOpenFile &file)
{
	return  validate_handler_->validate_format(file)
		and validate_handler_->validate_mode(file)
		and validate_handler_->validate_cdda(file)
		and validate_handler_->validate_version(file);
}

} // namespace wavpack
} // namespace details

/// @}


// DescriptorWavpack


DescriptorWavpack::~DescriptorWavpack() noexcept = default;


std::string DescriptorWavpack::do_name() const
{
	return "Wavpack";
}


LibInfo  DescriptorWavpack::do_libraries() const
{
	using details::find_lib;
	using details::libarcsdec_libs;

	return { { "libwavpack", find_lib(libarcsdec_libs(), "libwavpack") } };
}


bool DescriptorWavpack::do_accepts_bytes(
		const std::vector<unsigned char> &bytes, const uint64_t &offset) const
{
	return  bytes.size() >= 4
		and offset       == 0
		and bytes [0]    == 0x77  // w
		and bytes [1]    == 0x76  // v
		and bytes [2]    == 0x70  // p
		and bytes [3]    == 0x6B  // k
		;
}


bool DescriptorWavpack::do_accepts(Codec codec) const
{
	return codec == Codec::WAVPACK;
}


std::set<Codec> DescriptorWavpack::do_codecs() const
{
	return { Codec::WAVPACK };
}


bool DescriptorWavpack::do_accepts(Format format) const
{
	return format == Format::WV;
}


std::set<Format> DescriptorWavpack::do_formats() const
{
	return { Format::WV };
}


std::unique_ptr<FileReader> DescriptorWavpack::do_create_reader() const
{
	using details::wavpack::WAVPACK_CDDA_t;
	using details::wavpack::WavpackAudioReaderImpl;
	using details::wavpack::WavpackValidatingHandler;

	auto valid = std::make_unique<WAVPACK_CDDA_t>();
	auto handler = std::make_unique<WavpackValidatingHandler>(std::move(valid));

	auto impl = std::make_unique<WavpackAudioReaderImpl>();
	impl->register_validate_handler(std::move(handler));

	return std::make_unique<AudioReader>(std::move(impl));
}


std::unique_ptr<FileReaderDescriptor> DescriptorWavpack::do_clone() const
{
	return std::make_unique<DescriptorWavpack>();
}


// Add this descriptor to the audio descriptor registry

namespace {

const auto d = RegisterAudioDescriptor<DescriptorWavpack>();

} // namespace

} // namespace v_1_0_0

} // namespace arcsdec

