/**
 * \file audioreader.cpp Implements interface and toolkit for reading and validating audio files
 *
 */


#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"
#endif

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <stdexcept> // for logic_error
#include <sstream>

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __LIBARCSDEC_AUDIOFORMATS_HPP__
#include "audioformats.hpp"
#endif


namespace arcsdec
{

inline namespace v_1_0_0
{

using arcs::CDDA;
using arcs::LOG_DEBUG1;


// ByteConverter


ByteConverter::~ByteConverter() noexcept = default;


uint8_t ByteConverter::byte_to_uint8(const char &b) const
{
	return b & 0xFF;
}


int16_t ByteConverter::le_bytes_to_int16(const char &b1,
		const char &b2) const
{
	return	static_cast<uint16_t>(b2 & 0xFF) << 8 |
			static_cast<uint16_t>(b1 & 0xFF);

	// NOTE: This should also work without static_cast.
	// Explanation:
	// Operator & (bitwise AND) does usual arithmetic conversions, which means
	// if both of the operands are signed integral types (which is the case),
	// the operand with lower conversion rank will be converted to the type of
	// the operand with higher conversion rank.
	// Since 0xFF is a (signed) int, b1/b2 will be converted to (signed) int.
	// This entails that the result of the expression (b2 & 0xFF) is also int.
	// Since the length of int in bits is more than 8, the following left-shift
	// will succeed. Type int being signed does not affect the intended result
	// of the left-shift. Therefore static_cast should not be necessary.
	// Source: n4296:
	// - 5 (expr) 10.5.2, p.89
	// - 5.8 (expr.shift), p.126
	// - 5.11 (expr.bit.and), p.128
	// But...
	// ...I do not completely understand whether there can occurr a situation
	// in which the static_cast to uint16_t will in fact prevent unwanted
	// effects. The result of the bitwise & is guaranteed to fit in 16 bits,
	// since the value will be representable as int. Therefore it should be
	// safe to rely on the implicit cast when returning the result.
	// In case the cast is free of effect, the optimizer should remove it, but
	// it is unsatisfying to not be sure what's going on.
}


uint16_t ByteConverter::le_bytes_to_uint16(const char &b1,
		const char &b2) const
{
	return le_bytes_to_int16(b1, b2) & 0xFFFF;
}


int32_t ByteConverter::le_bytes_to_int32(const char &b1,
		const char &b2,
		const char &b3,
		const char &b4) const
{
	return	static_cast<uint32_t>(b4 & 0xFF) << 24 |
			static_cast<uint32_t>(b3 & 0xFF) << 16 |
			static_cast<uint32_t>(b2 & 0xFF) <<  8 |
			static_cast<uint32_t>(b1 & 0xFF);

	// NOTE Same note as in le_bytes_to_int16() applies
}


uint32_t ByteConverter::le_bytes_to_uint32(const char &b1,
		const char &b2,
		const char &b3,
		const char &b4) const
{
	return le_bytes_to_int32(b1, b2, b3, b4) & 0xFFFFFFFF;
}


int32_t ByteConverter::be_bytes_to_int32(const char &b1,
		const char &b2,
		const char &b3,
		const char &b4) const
{
	return	static_cast<uint32_t>(b1 & 0xFF) << 24 |
			static_cast<uint32_t>(b2 & 0xFF) << 16 |
			static_cast<uint32_t>(b3 & 0xFF) <<  8 |
			static_cast<uint32_t>(b4 & 0xFF);

	// NOTE Same note as in le_bytes_to_int16() applies
}


uint32_t ByteConverter::be_bytes_to_uint32(const char &b1,
		const char &b2,
		const char &b3,
		const char &b4) const
{
	return be_bytes_to_int32(b1, b2, b3, b4) & 0xFFFFFFFF;
}


// CDDAValidator


CDDAValidator::~CDDAValidator() noexcept = default;


bool CDDAValidator::bits_per_sample(const uint32_t &bits_per_sample)
{
	return CDDA.BITS_PER_SAMPLE == bits_per_sample;
}


bool CDDAValidator::num_channels(const uint32_t &num_channels)
{
	return CDDA.NUMBER_OF_CHANNELS == num_channels;
}


bool CDDAValidator::samples_per_second(const uint32_t &samples_per_second)
{
	return CDDA.SAMPLES_PER_SECOND == samples_per_second;
}


// ReaderValidatingHandler


ReaderValidatingHandler::ReaderValidatingHandler()
	: errors_()
{
  // empty
}


ReaderValidatingHandler::~ReaderValidatingHandler() noexcept = default;


void ReaderValidatingHandler::error(const std::string &msg)
{
	errors_.push_back(msg);
}


bool ReaderValidatingHandler::has_errors()
{
	return not errors_.empty();
}


std::vector<std::string> ReaderValidatingHandler::get_errors()
{
	return errors_;
}


std::string ReaderValidatingHandler::last_error()
{
	return errors_.back();
}


bool ReaderValidatingHandler::assert_equals(const std::string &label,
	const uint32_t &value, const uint32_t &proper_value,
	const std::string error_msg)
{
	ARCS_LOG(LOG_DEBUG1) << label
		<< "  ["
		<< (value == proper_value ? "yes" : "no")
		<< "]";

	if (value != proper_value)
	{
		std::stringstream ss;

		ss << error_msg;
		ss << ". Expected " << std::to_string(proper_value)
			<< " but is " << std::to_string(value);

		errors_.push_back(ss.str());
		return false;
	}

	return true;
}


bool ReaderValidatingHandler::assert_at_least(
		const std::string &label,
		const uint32_t &value,
		const uint32_t &proper_value,
		const std::string error_msg)
{
	ARCS_LOG(LOG_DEBUG1) << label
		<< "  ["
		<< (value >= proper_value ? "yes" : "no")
		<< "]";

	if (value < proper_value)
	{
		std::stringstream ss;

		ss << error_msg;
		ss << ". Expected at least " << std::to_string(proper_value)
			<< " but is only " << std::to_string(value);

		errors_.push_back(ss.str());
		return false;
	}

	return true;
}


bool ReaderValidatingHandler::assert_at_most(
		const std::string &label,
		const uint32_t &value,
		const uint32_t &proper_value,
		const std::string error_msg)
{
	ARCS_LOG(LOG_DEBUG1) << label
		<< "  ["
		<< (value <= proper_value ? "yes" : "no")
		<< "]";

	if (value > proper_value)
	{
		std::stringstream ss;

		ss << error_msg;
		ss << ". Expected at most " << std::to_string(proper_value)
			<< " but is in fact " << std::to_string(value);

		errors_.push_back(ss.str());
		return false;
	}

	return true;
}


bool ReaderValidatingHandler::assert_true(
		const std::string &label,
		const bool &value,
		const std::string error_msg)
{
	ARCS_LOG(LOG_DEBUG1) << label << "  [" << (value ? "yes" : "no") << "]";

	if (not value)
	{
		std::stringstream ss;

		ss << error_msg;
		ss << ". Expected true but is false";

		errors_.push_back(ss.str());
		return false;
	}

	return true;
}


// AudioReaderImpl


AudioReaderImpl::AudioReaderImpl() = default;


AudioReaderImpl::~AudioReaderImpl() noexcept = default;


bool AudioReaderImpl::configurable_read_buffer() const
{
	return this->do_configurable_read_buffer();
}


void AudioReaderImpl::set_samples_per_read(const uint32_t &samples_per_read)
{
	this->do_set_samples_per_read(samples_per_read);
}


uint32_t AudioReaderImpl::samples_per_read() const
{
	return this->do_samples_per_read();
}


std::unique_ptr<AudioSize> AudioReaderImpl::acquire_size(
		const std::string &filename)
{
	return this->do_acquire_size(filename);
}


void AudioReaderImpl::process_file(const std::string &filename)
{
	this->do_process_file(filename);
}


bool AudioReaderImpl::do_configurable_read_buffer() const
{
	return false;
}


void AudioReaderImpl::do_set_samples_per_read(
		const uint32_t &/* samples_per_read */)
{
	throw std::logic_error(
		"Try to set read buffer size on an AudioReader that has "
		"no configurable read buffer");
}


uint32_t AudioReaderImpl::do_samples_per_read() const
{
	return 0;
}


// BufferedAudioReaderImpl


BufferedAudioReaderImpl::BufferedAudioReaderImpl()
	: samples_per_read_(BLOCKSIZE.DEFAULT)
{
	// empty
}


BufferedAudioReaderImpl::BufferedAudioReaderImpl(
		const uint32_t samples_per_read)
	: samples_per_read_(samples_per_read)
{
	// empty
}


BufferedAudioReaderImpl::~BufferedAudioReaderImpl() noexcept = default;


bool BufferedAudioReaderImpl::do_configurable_read_buffer() const
{
	return true;
}


void BufferedAudioReaderImpl::do_set_samples_per_read(
		const uint32_t &samples_per_read)
{
	samples_per_read_ = samples_per_read;

	ARCS_LOG_DEBUG << "Set read buffer size: " << samples_per_read
		<< " samples";
}


uint32_t BufferedAudioReaderImpl::do_samples_per_read() const
{
	return samples_per_read_;
}


// Audioreader::Impl


/**
 * Private implementation of AudioReader
 */
class AudioReader::Impl final
{

public:

	/**
	 * Construct an AudioReader::Impl
	 *
	 * \param[in] readerimpl The AudioReaderImpl to use
	 * \param[in] processor  The SampleProcessor to use
	 */
	Impl(std::unique_ptr<AudioReaderImpl> readerimpl,
			SampleProcessor &processor);

	/**
	 * Construct an incomplete AudioReader::Impl (without SampleProcessor).
	 *
	 * \param[in] readerimpl The AudioReaderImpl to use
	 */
	explicit Impl(std::unique_ptr<AudioReaderImpl> readerimpl);

	Impl(const Impl &rhs) = delete;

	// TODO Move constructor

	/**
	 *
	 * \return TRUE if the size of input read at once is configurable
	 */
	bool configurable_read_buffer() const;

	/**
	 * Set the number of samples to read in one read operation.
	 *
	 * The default is BLOCKSIZE.DEFAULT.
	 */
	void set_samples_per_read(const uint32_t &samples_per_read);

	/**
	 * Return the number of samples to read in one read operation.
	 *
	 * \return Number of samples per read operation.
	 */
	uint32_t samples_per_read() const;

	/**
	 *
	 * \param[in] filename Audiofile to get size from
	 */
	std::unique_ptr<AudioSize> acquire_size(const std::string &filename) const;

	/**
	 *
	 * \param[in] filename Audiofile to process
	 */
	void process_file(const std::string &filename);

	/**
	 *
	 * \return The reader interface implementation
	 */
	const AudioReaderImpl& readerimpl();

	/**
	 *
	 * \param[in] processor The SampleProcessor to use
	 */
	void set_processor(SampleProcessor &processor);

	/**
	 *
	 * \return The SampleProcessor the reader uses
	 */
	const SampleProcessor* sampleprocessor();

	Impl& operator = (const Impl &rhs) = delete;

	// TODO Move assignment


private:

	/**
	 * \brief Internal AudioReaderImpl instance.
	 */
	std::unique_ptr<AudioReaderImpl> readerimpl_;
};


AudioReader::Impl::Impl(std::unique_ptr<AudioReaderImpl> readerimpl,
			SampleProcessor &processor)
	: readerimpl_(std::move(readerimpl))
{
	readerimpl_->register_processor(processor);
}


AudioReader::Impl::Impl(std::unique_ptr<AudioReaderImpl> readerimpl)
	: readerimpl_(std::move(readerimpl))
{
	// empty
}


bool AudioReader::Impl::configurable_read_buffer() const
{
	return readerimpl_->configurable_read_buffer();
}


void AudioReader::Impl::set_samples_per_read(const uint32_t &samples_per_read)
{
	readerimpl_->set_samples_per_read(samples_per_read);
}


uint32_t AudioReader::Impl::samples_per_read() const
{
	return readerimpl_->samples_per_read();
}


std::unique_ptr<AudioSize> AudioReader::Impl::acquire_size(
		const std::string &filename) const
{
	return readerimpl_->acquire_size(filename);
}


void AudioReader::Impl::process_file(const std::string &filename)
{
	readerimpl_->process_file(filename);
}


const AudioReaderImpl& AudioReader::Impl::readerimpl()
{
	return *readerimpl_;
}


void AudioReader::Impl::set_processor(SampleProcessor &processor)
{
	readerimpl_->register_processor(processor);
}


const SampleProcessor* AudioReader::Impl::sampleprocessor()
{
	return readerimpl_->processor();
}


// AudioReader


AudioReader::AudioReader(std::unique_ptr<AudioReaderImpl> impl,
			SampleProcessor &proc)
	: impl_(std::make_unique<AudioReader::Impl>(std::move(impl), proc))
{
	// empty
}


AudioReader::AudioReader(std::unique_ptr<AudioReaderImpl> impl)
	: impl_(std::make_unique<AudioReader::Impl>(std::move(impl)))
{
	// empty
}


AudioReader::~AudioReader() noexcept = default;


bool AudioReader::configurable_read_buffer() const
{
	return impl_->configurable_read_buffer();
}


void AudioReader::set_samples_per_read(const uint32_t &samples_per_read)
{
	impl_->set_samples_per_read(samples_per_read);
}


uint32_t AudioReader::samples_per_read() const
{
	return impl_->samples_per_read();
}


std::unique_ptr<AudioSize> AudioReader::acquire_size(
	const std::string &filename) const
{
	return impl_->acquire_size(filename);
}


void AudioReader::process_file(const std::string &filename)
{
	impl_->process_file(filename);
}


void AudioReader::set_processor(SampleProcessor &processor)
{
	impl_->set_processor(processor);
}


// AudioReaderSelection


AudioReaderSelection::AudioReaderSelection()
{
	std::unique_ptr<FileTestBytes> test =
		std::make_unique<FileTestBytes>(0, 24);
	// 24 is a sufficient number of bytes for recognition (at the moment)
	// Note: RIFF/WAV needs only the first 12 bytes for identification and PCM
	// format is encoded in byte 20 + 21, but 24 is such a beautiful number...

	this->register_test(std::move(test));


	// Provide FileReaderDescriptors

	// The constructor of AudioReaderSelection automagically introduces the
	// knowledge about what formats are available. This knowledge is
	// provided by the instance FileReaderDescriptorsAudio that is populated at
	// buildtime based on the configuration of the build system.

	FileReaderDescriptorsAudio compiled_supported_audio_formats;

	// We move all the actual formats to not access FileReaderDescriptorsAudio
	// beyond this particular block

	for (auto& f : compiled_supported_audio_formats)
	{
		this->add_descriptor(std::move(f));
	}
}


AudioReaderSelection::~AudioReaderSelection() noexcept = default;


std::unique_ptr<AudioReader> AudioReaderSelection::for_file(
	const std::string &filename) const
{
	return this->safe_cast(std::move(
				FileReaderSelection::for_file(filename)));
}


std::unique_ptr<AudioReader> AudioReaderSelection::by_name(
	const std::string &name) const
{
	return this->safe_cast(std::move(FileReaderSelection::by_name(name)));
}


std::unique_ptr<AudioReader> AudioReaderSelection::safe_cast(
		std::unique_ptr<FileReader> file_reader_uptr) const
{
	if (not file_reader_uptr)
	{
		return nullptr;
	}

	// Create AudioReader manually by (safe) downcasting and reassignment

	auto audio_reader_uptr = std::make_unique<AudioReader>(nullptr);

	FileReader* file_reader_rptr = nullptr;
	file_reader_rptr = file_reader_uptr.release();
	// This is definitively NOT nice since file_reader_rptr is now an
	// owning raw pointer. We will fix that with the following reset() to
	// a unique_ptr. If thereby something goes wrong, we immediately destroy
	// the raw pointer accurately.

	try
	{
		audio_reader_uptr.reset(dynamic_cast<AudioReader*>(file_reader_rptr));

		// Creation is correct iff the FileReader created is in fact an
		// AudioReader. If not, the file is not a supported audio file, so bail
		// out.

	} catch (...) // std::bad_cast is possible, but we play it safe
	{
		if (file_reader_rptr)
		{
			delete file_reader_rptr;
		}

		ARCS_LOG_ERROR <<
				"FileReader created, but failed to turn it to an AudioReader";

		return nullptr;
	}

	return audio_reader_uptr;
}

} // namespace v_1_0_0

} // namespace arcsdec

