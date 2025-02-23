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
	if (!ctx) { return; }

	ctx = ::WavpackCloseFile(ctx);

	if (ctx)
	{
		ARCS_LOG_ERROR <<
			"WavpackCloseFile could not close audio file. Possible leak!";
		return;
	}

	ARCS_LOG_DEBUG << "Audio file closed.";
}


WavpackContextPtr get_context(const std::string& filename) noexcept
{
	const int flags = OPEN_WVC | OPEN_NO_CHECKSUM ;
	char*     error = nullptr;

	auto ctxp = WavpackContextPtr {
		::WavpackOpenFileInput(filename.c_str(), error, flags, 0) };

	if (error)
	{
		const auto error_msg = std::string { error };

		::free(error);
		error = nullptr;

		ARCS_LOG_ERROR << "Error while opening Wavpack file: " << error_msg;
	}

	return ctxp;
}


// LibwavpackException


LibwavpackException::LibwavpackException(const std::string& value,
		const std::string& name, const std::string& error_msg)
	: msg_ { /* empty */ }
{
	auto msg = std::ostringstream{};

	msg << "libwavpack: Function " << name  << " returned unexpected value '"
		<< value << "'";

	if (!error_msg.empty())
	{
		msg << ", error message: '" << error_msg << "'";
	}

	msg_ = msg.str();
}


char const * LibwavpackException::what() const noexcept
{
	return msg_.c_str();
}


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


WavpackOpenFile::WavpackOpenFile(const std::string& filename)
	: context_ { get_context(filename) }
{
	// empty
}


WavpackOpenFile::~WavpackOpenFile() noexcept = default;


bool WavpackOpenFile::is_lossless() const
{
	return MODE_LOSSLESS & ::WavpackGetMode(context_.get());
}


bool WavpackOpenFile::has_wav_format() const
{
	return WP_FORMAT_WAV == ::WavpackGetFileFormat(context_.get());
}


bool WavpackOpenFile::has_float_samples() const
{
	return MODE_FLOAT & ::WavpackGetMode(context_.get());
}


uint8_t WavpackOpenFile::version() const
{
	return ::WavpackGetVersion(context_.get());
}


int WavpackOpenFile::bits_per_sample() const
{
	return ::WavpackGetBitsPerSample(context_.get());
}


int WavpackOpenFile::num_channels() const
{
	return ::WavpackGetNumChannels(context_.get());
}


int64_t WavpackOpenFile::samples_per_second() const
{
	return ::WavpackGetSampleRate(context_.get());
}


int64_t WavpackOpenFile::total_pcm_samples() const
{
	const auto total_pcm_samples = int64_t {
		::WavpackGetNumSamples64(context_.get()) };

	if (total_pcm_samples == -1)
	{
		throw LibwavpackException("-1", "WavpackGetNumSamples64",
				"Could not determine total number of samples");
	}

	return total_pcm_samples;
}


bool WavpackOpenFile::channel_order() const
{
	const auto num_channels = int { ::WavpackGetNumChannels(context_.get()) };

	// It is already ensured by validate_cdda that num_channels == 2.
	// No retest required.

	auto identities = ByteSequence( // deliberately parentheses
			static_cast<unsigned int>(num_channels + 1) ); // +1 for \0

	::WavpackGetChannelIdentities(context_.get(), identities.data());

	if (identities.empty() || identities.size() < 2)
	{
		auto msg = std::ostringstream{};
		msg << "Expected 2 channels but got not enough identities (== "
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


int WavpackOpenFile::channel_mask() const
{
	// stereo == 3
	return ::WavpackGetChannelMask(context_.get());
}


bool WavpackOpenFile::needs_channel_reorder() const
{
	// Channel layout of the core audio file

	unsigned char* reorder = nullptr;
	auto channel_layout = ::WavpackGetChannelLayout(context_.get(), reorder);

	if (!reorder)
	{
		return false;
	}

	auto channel_count = unsigned { 0 };

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


int64_t WavpackOpenFile::read_pcm_samples(
		const int64_t pcm_samples_to_read,
		std::vector<int32_t>& buffer) const
{
	const auto samples_to_read =
		static_cast<uint64_t>(pcm_samples_to_read * CDDA::NUMBER_OF_CHANNELS);

	if (buffer.size() < samples_to_read)
	{
		auto msg = std::ostringstream{};
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


bool WavpackOpenFile::success() const
{
	return context_ != nullptr;
}


// WavpackValidatingHandler


WavpackValidatingHandler::WavpackValidatingHandler(
		std::unique_ptr<WAVPACK_CDDA_t> valid_values)
	: DefaultValidator { /* default */ }
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
	validate_bits_per_sample(file.bits_per_sample());

	validate_num_channels(file.num_channels());

	validate_samples_per_second(file.samples_per_second());


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
	const auto file = WavpackOpenFile { filename };

	if (!file.success())
	{
		ARCS_LOG_ERROR << "File could not be opened, bail out";
		return nullptr;
	}

	const auto size = to_audiosize(file.total_pcm_samples(), UNIT::SAMPLES);

	return std::make_unique<AudioSize>(size);
}


void WavpackAudioReaderImpl::do_process_file(const std::string& filename)
{
	this->signal_startinput();

	const auto file = WavpackOpenFile { filename };

	if (!file.success())
	{
		ARCS_LOG_ERROR << "File could not be opened, bail out";
		this->signal_endinput();
		return;
	}

	// Validation

	ARCS_LOG_DEBUG << "Start validating Wavpack file: " << filename;

	if (!validate_handler_)
	{
		ARCS_LOG_WARNING
			<< "No validation handler configured, cannot validate file.";
	} else
	{
		if (!perform_validations(file))
		{
			ARCS_LOG_ERROR << "Validation failed";
			this->signal_endinput();
			return;
		}

		ARCS_LOG_DEBUG << "Completed validation of Wavpack file";
	}


	// Notify about correct size

	const int64_t total_samples { file.total_pcm_samples() };

	{
		const auto size = to_audiosize(file.total_pcm_samples(), UNIT::SAMPLES);
		this->signal_updateaudiosize(size);
	}


	// Samples reading loop

	{
		using sample_t = int32_t;
		using std::cbegin;
		using std::cend;

		auto sequence = InterleavedSamples<sample_t> { file.channel_order() };
		auto buffer   = std::vector<sample_t>{};
		buffer.resize(this->samples_per_read());

		// Request Half the Number of Samples in a Block in one Read.
		// Thus a Sequence will Have Exactly the Size of a Block.
		const auto wv_samples_to_read = int64_t {
			static_cast<int64_t>(this->samples_per_read() / 2) };

		auto wv_samples_read = int64_t { 0 };
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
					auto msg = std::ostringstream{};
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

			this->signal_appendsamples(cbegin(sequence), cend(sequence));
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
		&&  validate_handler_->validate_mode(file)
		&&  validate_handler_->validate_cdda(file)
		&&  validate_handler_->validate_version(file);
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

const auto d = RegisterDescriptor<DescriptorWavpack>{};

} // namespace

} // namespace v_1_0_0

} // namespace arcsdec

