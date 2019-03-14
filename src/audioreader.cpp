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
#include <sstream>

#ifndef __LIBARCSDEC_AUDIOBUFFER_HPP__
#include "audiobuffer.hpp"
#endif
#ifndef __LIBARCSDEC_AUDIOFORMATS_HPP__
#include "audioformats.hpp"
#endif

#ifndef __LIBARCS_LOGGING_HPP__
#include <arcs/logging.hpp>
#endif


namespace arcs
{

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


// SampleBuffer


SampleBuffer::SampleBuffer()
	: BlockAccumulator(BLOCKSIZE::DEFAULT)
	, call_update_audiosize_()
{
	this->init();
}


SampleBuffer::SampleBuffer(const uint32_t samples_per_block)
	: BlockAccumulator(samples_per_block)
	, call_update_audiosize_()
{
	this->init();
}


SampleBuffer::~SampleBuffer() noexcept = default;


void SampleBuffer::reset()
{
	this->init();
}


void SampleBuffer::notify_total_samples(const uint32_t sample_count)
{
	ARCS_LOG_DEBUG << "Total samples updated to: " << sample_count;

	AudioSize size;
	size.set_sample_count(sample_count);
	call_update_audiosize_(size); // call registered method
}


void SampleBuffer::register_processor(Calculation &calc)
{
	// Set Calculation instance as consumer for blocks
	this->register_block_consumer(
		std::bind(&Calculation::update, &calc,
			std::placeholders::_1, std::placeholders::_2));

	// Inform Calculation instance when total number of samples is set/updated
	call_update_audiosize_ =
		std::bind(&Calculation::update_audiosize, &calc, std::placeholders::_1);
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


// PCMBlockReader


PCMBlockReader::PCMBlockReader()
	: consume_()
{
	// empty
}


PCMBlockReader::PCMBlockReader(const uint32_t &samples_per_block)
	: BlockCreator(samples_per_block)
	, consume_()
{
	// empty
}


PCMBlockReader::~PCMBlockReader() noexcept = default;


void PCMBlockReader::register_block_consumer(const std::function<void(
			PCMForwardIterator begin, PCMForwardIterator end
		)> &consume)
{
	consume_ = consume;
}


uint64_t PCMBlockReader::read_blocks(std::ifstream &in,
		const uint64_t &total_pcm_bytes)
{
	std::vector<uint32_t> samples(this->samples_per_block());

	uint32_t bytes_per_block =
			this->samples_per_block() * CDDA.BYTES_PER_SAMPLE;

	uint32_t estimated_blocks = total_pcm_bytes / bytes_per_block
				+ (total_pcm_bytes % bytes_per_block ? 1 : 0);

	ARCS_LOG_DEBUG << "START READING " << total_pcm_bytes
		<< " bytes in " << std::to_string(estimated_blocks) << " blocks with "
		<< bytes_per_block << " bytes per block";

	uint32_t samples_todo = total_pcm_bytes / CDDA.BYTES_PER_SAMPLE;
	uint32_t total_bytes_read   = 0;
	uint32_t total_blocks_read  = 0;

	uint32_t read_bytes = this->samples_per_block() * sizeof(uint32_t);
	// FIXME Use sample type!

	while (total_bytes_read < total_pcm_bytes)
	{
		// Adjust buffer size for last buffer, if necessary

		if (samples_todo < this->samples_per_block())
		{
			// Avoid trailing zeros in buffer
			samples.resize(samples_todo);

			//read_bytes = samples_todo * sizeof(samples.front());
			read_bytes = samples_todo * sizeof(uint32_t); // FIXME Use sample type!
		}

		// Actually read the bytes

		try
		{
			in.read(reinterpret_cast<char*>(samples.data()), read_bytes);
		}
		catch (const std::ifstream::failure& f)
		{
			total_bytes_read += in.gcount();

			ARCS_LOG_ERROR << "Failed to read from file: " << f.what();

			throw FileReadException(f.what(), total_bytes_read + 1);
		}
		total_bytes_read += read_bytes;
		samples_todo     -= samples.size();

		// Logging + Statistics

		++total_blocks_read;

		ARCS_LOG_DEBUG << "READ BLOCK " << total_blocks_read
			<< "/" << estimated_blocks;
		ARCS_LOG(LOG_DEBUG1) << "Size: " << read_bytes << " bytes";
		ARCS_LOG(LOG_DEBUG1) << "      " << samples.size()
				<< " Stereo PCM samples (32 bit)";

		this->consume_(samples.begin(), samples.end());
	}

	ARCS_LOG_DEBUG << "END READING after " << total_blocks_read << " blocks";

	ARCS_LOG(LOG_DEBUG1) << "Read "
		<< std::to_string(total_bytes_read / CDDA.BYTES_PER_SAMPLE)
		<< " samples / "
		<< total_bytes_read << " bytes";

	return total_bytes_read;
}


// AudioReaderImpl


AudioReaderImpl::AudioReaderImpl() = default;


AudioReaderImpl::~AudioReaderImpl() noexcept = default;


std::unique_ptr<AudioSize> AudioReaderImpl::acquire_size(
		const std::string &filename)
{
	return this->do_acquire_size(filename);
}


Checksums AudioReaderImpl::process_file(const std::string &filename)
{
	return this->do_process_file(filename);
}


void AudioReaderImpl::set_samples_per_block(const uint32_t &samples_per_block)

{
	this->do_set_samples_per_block(samples_per_block);
}


uint32_t AudioReaderImpl::samples_per_block() const
{
	return this->do_get_samples_per_block();
}


void AudioReaderImpl::set_calc(std::unique_ptr<Calculation> calc)
{
	this->do_set_calc(std::move(calc));
}


const Calculation& AudioReaderImpl::calc() const
{
	return this->do_get_calc();
}


// AudioReader


AudioReader::AudioReader(std::unique_ptr<AudioReaderImpl> impl)
	:impl_(std::move(impl))
{
	// empty
}


AudioReader::~AudioReader() noexcept = default;


std::unique_ptr<AudioSize> AudioReader::acquire_size(
	const std::string &filename) const
{
	return impl_->acquire_size(filename);
}


Checksums AudioReader::process_file(const std::string &filename)
{
	return impl_->process_file(filename);
}


void AudioReader::set_samples_per_block(const uint32_t &samples_per_block)
{
	impl_->set_samples_per_block(samples_per_block);
}


uint32_t AudioReader::samples_per_block() const
{
	return impl_->samples_per_block();
}


void AudioReader::set_calc(std::unique_ptr<Calculation> calc)
{
	impl_->set_calc(std::move(calc));
}


const Calculation& AudioReader::calc()
{
	return impl_->calc();
}


// AudioReaderCreator


AudioReaderCreator::AudioReaderCreator()
{
	// Provide tests

	std::unique_ptr<FileFormatTestBytes> test =
		std::make_unique<FileFormatTestBytes>(0, 24);
	// 24 is a sufficient number of bytes for recognition (at the moment)
	// Note: RIFF/WAV needs only the first 12 bytes for identification and PCM
	// format is encoded in byte 20 + 21, but 24 is such a beautiful number...

	this->register_test(std::move(test));


	// Provide FileFormats

	// The constructor of AudioReaderCreator automagically introduces the
	// knowledge about what formats are available. This knowledge is
	// provided by the instance FileFormatsAudio that is populated at
	// buildtime based on the configuration of the build system.

	FileFormatsAudio compiled_supported_audio_formats;

	// We move all the actual formats to not access FileFormatsAudio
	// beyond this particular block

	for (auto& f : compiled_supported_audio_formats)
	{
		this->register_format(std::move(f));
	}
}


AudioReaderCreator::~AudioReaderCreator() noexcept = default;


std::unique_ptr<AudioReader> AudioReaderCreator::create_audio_reader(
	const std::string &filename)
{
	FileReader* file_reader_rptr = nullptr;

	// Create FileReader

	auto file_reader_uptr = FileReaderCreator::create_reader(filename);

	if (not file_reader_uptr)
	{
		ARCS_LOG_ERROR << "FileReader could not be created";
		return nullptr;
	}

	// Create AudioReader manually by (safe) downcasting and reassignment

	auto audio_reader_uptr = std::make_unique<AudioReader>(nullptr);

	file_reader_rptr = file_reader_uptr.release();

	try
	{
		audio_reader_uptr.reset(dynamic_cast<AudioReader*>(file_reader_rptr));

		// Creation is correct iff the FileReader created is in fact an
		// AudioReader. If not, the file is not a supported audio file, so bail
		// out.

	} catch (const std::bad_cast& e)
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

} // namespace arcs

