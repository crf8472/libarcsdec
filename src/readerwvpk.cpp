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

#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"  // for AudioReaderImpl, InvalidAudioException
#endif
#ifndef __LIBARCSDEC_LIBINSPECT_HPP__
#include "libinspect.hpp"   // for first_libname_match
#endif
#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"    // for RegisterDescriptor
#endif

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>     // for CDDA
#endif
#ifndef __LIBARCSTK_METADATA_HPP__
#include <arcstk/metadata.hpp>       // for AudioSize
#endif
#ifndef __LIBARCSTK_SAMPLES_HPP__
#include <arcstk/samples.hpp>        // for InterleavedSamples
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp> // for ARCS_LOG_ERROR, _WARNING, _INFO, _DEBUG
#endif

extern "C" {
#include <wavpack/wavpack.h>
}

#include <cstdint>   // for uint8_t, uint64_t, int32_t, int64_t
#include <cstdlib>   // for size_t, free
#include <memory>    // for unique_ptr
#include <set>       // for set
#include <sstream>   // for ostringstream
#include <stdexcept> // for invalid_argument
#include <string>    // for string, to_string
#include <utility>   // for make_unique, move
#include <vector>    // for vector


namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{
namespace wavpack
{

using arcstk::AudioSize;
using arcstk::CDDA;
using arcstk::InterleavedSamples;


void Free_WavpackContext::operator()(::WavpackContext* ctx) const
{
	if (ctx)
	{
		ctx = ::WavpackCloseFile(ctx);

		if (!ctx)
		{
			ARCS_LOG_DEBUG << "Audio file closed.";
		} else
		{
			ARCS_LOG_ERROR <<
				"WavpackCloseFile could not close audio file. Possible leak!";
		}
	}
}


ContextPtr Make_ContextPtr::operator()(const std::string& filename) const
{
	int   flags = OPEN_WVC | OPEN_NO_CHECKSUM ;
	char* error = nullptr;

	ContextPtr ctxp {::WavpackOpenFileInput(filename.c_str(), error, flags, 0)};

	if (!ctxp)
	{
		ARCS_LOG_ERROR << "Wavpack file could not be opened";

		std::string error_msg;
		if (error)
		{
			error_msg = std::string(error);
			::free(error);
			error = nullptr;
		}

		throw LibwavpackException("NULL", "WavpackOpenFileInput", error_msg);
	}

	return ctxp;
}


// LibwavpackException


LibwavpackException::LibwavpackException(const std::string& value,
		const std::string& name, const std::string& error_msg)
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
	Impl(const std::string& filename);

	Impl(const Impl& impl) = delete;

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
		std::vector<int32_t>& buffer) const;

	// Delete copy assignment operator
	WavpackOpenFile::Impl& operator = (WavpackOpenFile::Impl& file) = delete;


private:

	/**
	 * \brief Internal WavpackContext.
	 */
	ContextPtr context_;
};


// WAVPACK_CDDA_t


WAVPACK_CDDA_t::~WAVPACK_CDDA_t() noexcept = default;


bool WAVPACK_CDDA_t::lossless() const
{
	return true; // Always require losless compression
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


WavpackOpenFile::Impl::Impl(const std::string& filename)
	: context_ { nullptr }
{
	static const Make_ContextPtr make_context;

	context_ = std::move(make_context(filename));
}


WavpackOpenFile::Impl::~Impl() noexcept
{
	// empty
}


bool WavpackOpenFile::Impl::is_lossless() const
{
	return MODE_LOSSLESS & ::WavpackGetMode(context_.get());
}


bool WavpackOpenFile::Impl::has_wav_format() const
{
	return WP_FORMAT_WAV == ::WavpackGetFileFormat(context_.get());
}


bool WavpackOpenFile::Impl::has_float_samples() const
{
	return MODE_FLOAT & ::WavpackGetMode(context_.get());
}


uint8_t WavpackOpenFile::Impl::version() const
{
	return ::WavpackGetVersion(context_.get());
}


int WavpackOpenFile::Impl::bits_per_sample() const
{
	return ::WavpackGetBitsPerSample(context_.get());
}


int WavpackOpenFile::Impl::num_channels() const
{
	return ::WavpackGetNumChannels(context_.get());
}


int64_t WavpackOpenFile::Impl::samples_per_second() const
{
	return ::WavpackGetSampleRate(context_.get());
}


int64_t WavpackOpenFile::Impl::total_pcm_samples() const
{
	int64_t total_pcm_samples = ::WavpackGetNumSamples64(context_.get());

	if (total_pcm_samples == -1)
	{
		throw LibwavpackException("-1", "WavpackGetNumSamples64",
				"Could not determine total number of samples");
	}

	return total_pcm_samples;
}


bool WavpackOpenFile::Impl::channel_order() const
{
	const int num_channels = ::WavpackGetNumChannels(context_.get());

	// It is already ensured by validate_cdda that num_channels == 2.
	// No retest required.

	ByteSequence identities(
			static_cast<unsigned int>(num_channels + 1)); // +1 for \0

	::WavpackGetChannelIdentities(context_.get(), identities.data());

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
	return ::WavpackGetChannelMask(context_.get());
}


bool WavpackOpenFile::Impl::needs_channel_reorder() const
{
	// Channel layout of the core audio file

	unsigned char* reorder = nullptr;
	auto channel_layout = ::WavpackGetChannelLayout(context_.get(), reorder);

	if (!reorder)
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
	ARCS_LOG_DEBUG << "Channel reorder: " << reorder;

	return true;
}


int64_t WavpackOpenFile::Impl::read_pcm_samples(
		const int64_t pcm_samples_to_read,
		std::vector<int32_t>& buffer) const
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

	const auto samples_read = ::WavpackUnpackSamples(context_.get(),
			&(buffer[0]), pcm_samples_to_read);

	ARCS_LOG_DEBUG << "    Read " << samples_read << " PCM samples (32 bit)";

	return samples_read;
}


// WavpackOpenFile


WavpackOpenFile::WavpackOpenFile(const std::string& filename)
	: impl_ { std::make_unique<WavpackOpenFile::Impl>(filename) }
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
		std::vector<int32_t>& buffer) const
{
	return impl_->read_pcm_samples(pcm_samples_to_read, buffer);
}



// WavpackValidatingHandler


WavpackValidatingHandler::WavpackValidatingHandler(
		std::unique_ptr<WAVPACK_CDDA_t> valid_values)
	: DefaultValidator {}
	, valid_ { std::move(valid_values) }
{
	// empty
}


WavpackValidatingHandler::~WavpackValidatingHandler() noexcept = default;


bool WavpackValidatingHandler::validate_format(const WavpackOpenFile& file)
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


bool WavpackValidatingHandler::validate_mode(const WavpackOpenFile& file)
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


bool WavpackValidatingHandler::validate_cdda(const WavpackOpenFile& file)
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

	return true;
}


bool WavpackValidatingHandler::validate_version(const WavpackOpenFile& file)
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


std::unique_ptr<AudioSize> WavpackAudioReaderImpl::do_acquire_size(
	const std::string& filename)
{
	std::unique_ptr<AudioSize> audiosize = std::make_unique<AudioSize>();

	WavpackOpenFile file(filename);

	audiosize->set_samples(file.total_pcm_samples());

	return audiosize;
}


void WavpackAudioReaderImpl::do_process_file(const std::string& filename)
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
		size.set_samples(total_samples);
		this->signal_updateaudiosize(size);
	}


	// Samples reading loop

	{
		using sample_t = int32_t;

		InterleavedSamples<sample_t> sequence(file.channel_order());
		std::vector<sample_t>        buffer;
		buffer.resize(this->samples_per_read());

		// Request Half the Number of Samples in a Block in one Read.
		// Thus a Sequence will Have Exactly the Size of a Block.
		const int64_t wv_samples_to_read =
			static_cast<int64_t>(this->samples_per_read() / 2);

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

				buffer.resize(static_cast<std::size_t>(
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


bool WavpackAudioReaderImpl::perform_validations(const WavpackOpenFile& file)
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


std::string DescriptorWavpack::do_id() const
{
	return "wavpack";
}


std::string DescriptorWavpack::do_name() const
{
	return "Wavpack";
}


std::set<Format> DescriptorWavpack::define_formats() const
{
	return { Format::WV };
}


std::set<Codec> DescriptorWavpack::define_codecs() const
{
	return { Codec::WAVPACK };
}


LibInfo  DescriptorWavpack::do_libraries() const
{
	return { libinfo_entry_filepath("libwavpack") };
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

const auto d = RegisterDescriptor<DescriptorWavpack>();

} // namespace

} // namespace v_1_0_0

} // namespace arcsdec

