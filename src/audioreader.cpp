/**
 * \file
 *
 * \brief Implements API for implementing AudioReaders.
 */


#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"
#endif

#include <cstdint>
#include <memory>
#include <sstream>       // for ostringstream
#include <stdexcept>     // for logic_error
#include <string>

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif


namespace arcsdec
{

inline namespace v_1_0_0
{

using arcstk::CDDA;


// MAX_SAMPLES_TO_READ


const int32_t MAX_SAMPLES_TO_READ =
	CDDA::MAX_BLOCK_ADDRESS * CDDA::SAMPLES_PER_FRAME;


// LittleEndianBytes


int16_t LittleEndianBytes::to_int16(const char &b1, const char &b2)
{
	uint16_t val =
			static_cast<uint16_t>(b2 & 0xFF) << 8 |
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

	return static_cast<int16_t>(val);
	// separate explicit cast to avoid -Wsign-conversion firing
}


uint16_t LittleEndianBytes::to_uint16(const char &b1, const char &b2)
{
	return to_int16(b1, b2) & 0xFFFF;
}


int32_t LittleEndianBytes::to_int32(const char &b1,
		const char &b2,
		const char &b3,
		const char &b4)
{
	uint32_t val =
			static_cast<uint32_t>(b4 & 0xFF) << 24 |
			static_cast<uint32_t>(b3 & 0xFF) << 16 |
			static_cast<uint32_t>(b2 & 0xFF) <<  8 |
			static_cast<uint32_t>(b1 & 0xFF);

	// NOTE Same note as in to_int16() applies
	return static_cast<int32_t>(val);
	// separate explicit cast to avoid -Wsign-conversion firing
}


uint32_t LittleEndianBytes::to_uint32(const char &b1,
		const char &b2,
		const char &b3,
		const char &b4)
{
	return static_cast<uint32_t>(to_int32(b1, b2, b3, b4));
}


// BigEndianBytes


int32_t BigEndianBytes::to_int32(const char &b1,
		const char &b2,
		const char &b3,
		const char &b4)
{
	return LittleEndianBytes::to_int32(b4, b3, b2, b1);
	/*
	uint32_t val =
			static_cast<uint32_t>(b1 & 0xFF) << 24 |
			static_cast<uint32_t>(b2 & 0xFF) << 16 |
			static_cast<uint32_t>(b3 & 0xFF) <<  8 |
			static_cast<uint32_t>(b4 & 0xFF);

	// NOTE Same note as in to_int16() applies
	return static_cast<int32_t>(val);
	// separate cast to avoid -Wsign-conversion firing
	*/
}


uint32_t BigEndianBytes::to_uint32(const char &b1,
		const char &b2,
		const char &b3,
		const char &b4)
{
	//return be_bytes_to_int32(b1, b2, b3, b4) & 0xFFFFFFFF;
	return static_cast<uint32_t>(BigEndianBytes::to_int32(b1, b2, b3, b4));
}


// CDDAValidator


bool CDDAValidator::bits_per_sample(const int &bits_per_sample)
{
	return CDDA::BITS_PER_SAMPLE == bits_per_sample;
}


bool CDDAValidator::num_channels(const int &num_channels)
{
	return CDDA::NUMBER_OF_CHANNELS == num_channels;
}


bool CDDAValidator::samples_per_second(const int &samples_per_second)
{
	return CDDA::SAMPLES_PER_SECOND == samples_per_second;
}


// InvalidAudioException


InvalidAudioException::InvalidAudioException(const std::string &what_arg)
	: std::logic_error { what_arg }
{
	// empty
}


InvalidAudioException::InvalidAudioException(const char *what_arg)
	: std::logic_error { what_arg }
{
	// empty
}


// AudioValidator


AudioValidator::AudioValidator()
	: errors_()
{
  // empty
}


AudioValidator::~AudioValidator() noexcept = default;


AudioValidator::codec_set_type AudioValidator::codecs() const
{
	return this->do_codecs();
}


bool AudioValidator::bits_per_sample(const int bits_per_sample)
{
	if (not this->assert_true("Test (CDDA): Bits per sample",
			CDDAValidator::bits_per_sample(bits_per_sample),
			"Number of bits per sample does not conform to CDDA"))
	{
		this->on_failure();
		return false;
	}

	return true;
}


bool AudioValidator::samples_per_second(const int sps)
{
	if (not this->assert_true("Test (CDDA): Sample Rate",
			CDDAValidator::samples_per_second(sps),
			"Number of samples per second does not conform to CDDA"))
	{
		this->on_failure();
		return false;
	}

	return true;
}


bool AudioValidator::num_channels(const int num_channels)
{
	if (not this->assert_true("Test (CDDA): Channels",
			CDDAValidator::num_channels(num_channels),
			"Number of channels does not conform to CDDA"))
	{
		this->on_failure();
		return false;
	}

	return true;
}


void AudioValidator::error(const std::string &msg)
{
	errors_.push_back(msg);
}


const std::string& AudioValidator::last_error() const
{
	return errors_.back();
}


bool AudioValidator::has_errors() const
{
	return not errors_.empty();
}


const AudioValidator::error_list_type&
	AudioValidator::get_errors() const
{
	return errors_;
}


bool AudioValidator::assert_equals(
		const std::string &label,
		int value,
		int proper_value,
		const std::string error_msg)
{
	ARCS_LOG(DEBUG1) << label
		<< "  ["
		<< (value == proper_value ? "yes" : "no")
		<< "]";

	if (value != proper_value)
	{
		std::ostringstream msg;

		msg << error_msg;
		msg << ". Expected " << std::to_string(proper_value)
			<< " but is " << std::to_string(value);

		errors_.push_back(msg.str());
		return false;
	}

	return true;
}


bool AudioValidator::assert_equals_u(
		const std::string &label,
		uint32_t value,
		uint32_t proper_value,
		const std::string error_msg)
{
	ARCS_LOG(DEBUG1) << label
		<< "  ["
		<< (value == proper_value ? "yes" : "no")
		<< "]";

	if (value != proper_value)
	{
		std::ostringstream msg;

		msg << error_msg;
		msg << ". Expected " << std::to_string(proper_value)
			<< " but is " << std::to_string(value);

		errors_.push_back(msg.str());
		return false;
	}

	return true;
}


bool AudioValidator::assert_at_least(
		const std::string &label,
		int value,
		int proper_value,
		const std::string error_msg)
{
	ARCS_LOG(DEBUG1) << label
		<< "  ["
		<< (value >= proper_value ? "yes" : "no")
		<< "]";

	if (value < proper_value)
	{
		std::ostringstream msg;

		msg << error_msg;
		msg << ". Expected at least " << std::to_string(proper_value)
			<< " but is only " << std::to_string(value);

		errors_.push_back(msg.str());
		return false;
	}

	return true;
}


bool AudioValidator::assert_at_most(
		const std::string &label,
		int value,
		int proper_value,
		const std::string error_msg)
{
	ARCS_LOG(DEBUG1) << label
		<< "  ["
		<< (value <= proper_value ? "yes" : "no")
		<< "]";

	if (value > proper_value)
	{
		std::ostringstream msg;

		msg << error_msg;
		msg << ". Expected at most " << std::to_string(proper_value)
			<< " but is in fact " << std::to_string(value);

		errors_.push_back(msg.str());
		return false;
	}

	return true;
}


bool AudioValidator::assert_true(
		const std::string &label,
		bool value,
		const std::string error_msg)
{
	ARCS_LOG(DEBUG1) << label << "  [" << (value ? "yes" : "no") << "]";

	if (not value)
	{
		std::ostringstream msg;

		msg << error_msg;
		msg << ". Expected true but is false";

		errors_.push_back(msg.str());
		return false;
	}

	return true;
}


void AudioValidator::log_error_stack() const
{
	for (const auto& error : this->get_errors())
	{
		ARCS_LOG_ERROR << error;
	}

	ARCS_LOG_ERROR << "=> Validation failed";
}


// DefaultValidator


void DefaultValidator::on_failure()
{
	throw InvalidAudioException(this->last_error());
}


// AudioReaderImpl


AudioReaderImpl::AudioReaderImpl()
	: samples_per_read_ { BLOCKSIZE::DEFAULT }
{
	// empty
}


AudioReaderImpl::AudioReaderImpl(AudioReaderImpl &&) noexcept = default;


AudioReaderImpl& AudioReaderImpl::operator = (AudioReaderImpl &&) noexcept
= default;


void AudioReaderImpl::set_samples_per_read(const std::size_t samples_per_read)
{
	samples_per_read_ = samples_per_read;
}


std::size_t AudioReaderImpl::samples_per_read() const
{
	return samples_per_read_;
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


std::unique_ptr<FileReaderDescriptor> AudioReaderImpl::descriptor() const
{
	return this->do_descriptor();
}


// Audioreader::Impl


/**
 * \brief Private implementation of AudioReader.
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
	Impl& operator = (const Impl &rhs) = delete;

	/**
	 * Set the number of samples to read in one read operation.
	 *
	 * The default is BLOCKSIZE::DEFAULT.
	 */
	void set_samples_per_read(const std::size_t samples_per_read);

	/**
	 * Return the number of samples to read in one read operation.
	 *
	 * \return Number of samples per read operation.
	 */
	std::size_t samples_per_read() const;

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
	 * \brief Create a descriptor for this AudioReader implementation.
	 *
	 * \return Descriptor for this implementation.
	 */
	std::unique_ptr<FileReaderDescriptor> descriptor() const;

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
	readerimpl_->attach_processor(processor);
}


AudioReader::Impl::Impl(std::unique_ptr<AudioReaderImpl> readerimpl)
	: readerimpl_(std::move(readerimpl))
{
	// empty
}


void AudioReader::Impl::set_samples_per_read(const std::size_t samples_per_read)
{
	readerimpl_->set_samples_per_read(samples_per_read);
}


std::size_t AudioReader::Impl::samples_per_read() const
{
	return readerimpl_->samples_per_read();
}


std::unique_ptr<AudioSize> AudioReader::Impl::acquire_size(
		const std::string &filename) const
{
	ARCS_LOG_DEBUG << "Try to acquire audio file size of '" << filename << "'";

	auto audiosize = readerimpl_->acquire_size(filename);

	ARCS_LOG_DEBUG << "Audio file size of '" << filename
		<< "' successfully acquired (== "
		<< std::to_string(audiosize->leadout_frame())
		<< " LBA frames)";

	return audiosize;
}


void AudioReader::Impl::process_file(const std::string &filename)
{
	ARCS_LOG_DEBUG << "Try to process audio file '" << filename << "'";

	readerimpl_->process_file(filename);

	ARCS_LOG_DEBUG << "Sucessfully processed audio file '" << filename << "'";
}


std::unique_ptr<FileReaderDescriptor> AudioReader::Impl::descriptor() const
{
	return readerimpl_->descriptor();
}


void AudioReader::Impl::set_processor(SampleProcessor &processor)
{
	readerimpl_->attach_processor(processor);
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


AudioReader::AudioReader(AudioReader &&) noexcept = default;


AudioReader& AudioReader::operator = (AudioReader &&) noexcept = default;


AudioReader::~AudioReader() noexcept = default;


void AudioReader::set_samples_per_read(const std::size_t samples_per_read)
{
	impl_->set_samples_per_read(samples_per_read);
}


std::size_t AudioReader::samples_per_read() const
{
	return impl_->samples_per_read();
}


std::unique_ptr<AudioSize> AudioReader::acquire_size(
	const std::string &filename) const
{
	auto size = impl_->acquire_size(filename);

	if (size->total_samples() > MAX_SAMPLES_TO_READ)
	{
		ARCS_LOG_WARNING << "File seems to contain "
			<< size->total_samples()
			<< " but redbook defines a maximum of "
			<< MAX_SAMPLES_TO_READ
			<< ". File does not seem to be a compact disc image";
	}

	return size;
}


void AudioReader::process_file(const std::string &filename)
{
	impl_->process_file(filename);
}


void AudioReader::set_processor(SampleProcessor &processor)
{
	impl_->set_processor(processor);
}


std::unique_ptr<FileReaderDescriptor> AudioReader::do_descriptor() const
{
	return impl_->descriptor();
}

} // namespace v_1_0_0

} // namespace arcsdec

