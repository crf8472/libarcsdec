#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#define __LIBARCSDEC_AUDIOREADER_HPP__

/**
 * \file
 *
 * \brief Implement AudioReaders.
 */

#ifndef __LIBARCSDEC_DESCRIPTOR_HPP__
#include "descriptor.hpp"  // for Codec, FileReaderDescriptor, ...
#endif
#ifndef __LIBARCSDEC_SAMPLEPROC_HPP__
#include "sampleproc.hpp"  // for SampleProcessor, SampleProvider
#endif

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>  // for AudioSize, SampleInputIterator
#endif

#include <cstddef>    // for size_t
#include <cstdint>    // for uint16_t, uint32_t, int16_t, int32_t
#include <memory>     // for unique_ptr
#include <set>        // for set
#include <stdexcept>  // for logic_error
#include <string>     // for string
#include <vector>     // for vector


namespace arcsdec
{
inline namespace v_1_0_0
{

using arcstk::AudioSize;


/**
 * \defgroup audioreader API for implementing AudioReaders
 *
 * \brief API for implementing \link AudioReader AudioReaders\endlink.
 *
 * Class AudioReader provides an interface for reading audio files.
 *
 * The AudioReader provides two actual operations on the input file: it can
 * either analyze the file via \c acquire_size() or actually process the file
 * via \c process_file(), which yields the actual calculation results. An
 * AudioReader is a SampleProvider and hence a SampleProcessor can be
 * attached to it.
 *
 * An AudioReader internally holds a concrete instance of AudioReaderImpl.
 * AudioReaderImpl can be subclassed to implement the capabilities of an
 * AudioReader.
 *
 * The concrete reading of a given audio file is implemented by the subclasses
 * of AudioReaderImpl. An AudioReaderImpl can be set to a block size, that may
 * or may not refer to a buffer size, depending on the actual implementation.
 *
 * CDDAValidator provides a uniform implementation of checking sample size,
 * sampling rate and number of channels of an input audio file for CDDA
 * conformity.
 *
 * AudioValidator wraps a CDDAValidator with error tracking for easy
 * registering validation functionality to an AudioReaderImpl. It provides
 * default implementations for CDDA testing. Subclasses are supposed to add
 * validation for the concrete format and codec.
 *
 * Validation failures are reported as InvalidAudioException.
 *
 * BigEndianBytes and LittleEndianBytes decode short sequences of single chars
 * to integers.
 *
 * @{
 */


/**
 * \brief Maximum number of PCM 32 bit samples to read from a file.
 *
 * This is equivalent to the product of the maximal lba block address the
 * redbook standard accepts (449.999 frames) and the number of samples per lba
 * frame (588).
 *
 * The numerical value is 264.599.412.
 */
extern const int32_t MAX_SAMPLES_TO_READ;
// FIXME But we have to expect more samples than redbook allows!


/**
 * \brief Abstract base class for AudioReader implementations.
 *
 * Concrete subclasses of AudioReaderImpl implement AudioReaders for a concrete
 * FileReaderDescriptor.
 *
 * \note
 * Instances of subclasses are non-copyable but movable.
 */
class AudioReaderImpl : public SampleProvider
{
public:

	/**
	 * \brief Default constructor.
	 */
	AudioReaderImpl();

	/**
	 * \brief Provides implementation for acquire_size() of a AudioReader.
	 *
	 * \param[in] filename The filename of the file to process
	 *
	 * \return A CalcContext for the specified file
	 *
	 * \throw FileReadException If the file could not be read
	 */
	std::unique_ptr<AudioSize> acquire_size(const std::string& filename);

	/**
	 * \brief Provides implementation for process_file() of some AudioReader.
	 *
	 * \param[in] filename The filename of the file to process
	 *
	 * \return The checksums of this file
	 *
	 * \throw FileReadException If the file could not be read
	 */
	void process_file(const std::string& filename);

	/**
	 * \brief Set the number of samples to read in one read operation.
	 *
	 * The default is BLOCKSIZE::DEFAULT.
	 *
	 * \param[in] samples_per_read Number of samples to read/buffer at once.
	 */
	void set_samples_per_read(const int64_t samples_per_read);

	/**
	 * \brief Return the number of samples to read in one read operation.
	 *
	 * \return Number of samples per read operation.
	 */
	int64_t samples_per_read() const;

	/**
	 * \brief Create a descriptor for this AudioReader implementation.
	 *
	 * \return Descriptor for this implementation.
	 */
	std::unique_ptr<FileReaderDescriptor> descriptor() const;

protected:

	// Avoid -Weffc++ firing
	AudioReaderImpl(const AudioReaderImpl&) = delete;
	AudioReaderImpl& operator = (const AudioReaderImpl&) = delete;

	AudioReaderImpl(AudioReaderImpl&&) noexcept;
	AudioReaderImpl& operator = (AudioReaderImpl&&) noexcept;

	/**
	 * \brief Default implementation of attach_processor().
	 */
	void attach_processor_impl(SampleProcessor& processor);

	/**
	 * \brief Use the internal SampleProcessor.
	 */
	SampleProcessor* use_processor();

private:

	// SampleProvider

	void do_signal_startinput() override;

	void do_signal_appendsamples(SampleInputIterator begin,
			SampleInputIterator end) override;

	void do_signal_updateaudiosize(const AudioSize& size) override;

	void do_signal_endinput() override;

	void do_attach_processor(SampleProcessor& processor) override;

	const SampleProcessor* do_processor() const override;

	/**
	 * \brief Provides implementation for \c acquire_size() of an AudioReader.
	 *
	 * \param[in] filename The filename of the file to process
	 *
	 * \return A CalcContext for the specified file
	 *
	 * \throw FileReadException If the file could not be read
	 */
	virtual std::unique_ptr<AudioSize> do_acquire_size(
		const std::string& filename)
	= 0;

	/**
	 * \brief Provides implementation for process_file() of some AudioReader.
	 *
	 * \param[in] filename The filename of the file to process
	 *
	 * \return The checksums of this file
	 *
	 * \throw FileReadException If the file could not be read
	 */
	virtual void do_process_file(const std::string& filename)
	= 0;

	virtual std::unique_ptr<FileReaderDescriptor> do_descriptor() const
	= 0;

	/**
	 * \brief Internal pointer to the SampleProcessor.
	 */
	SampleProcessor* processor_;

	/**
	 * \brief Buffer size as total number of PCM 32 bit samples.
	 */
	int64_t samples_per_read_;
};


/**
 * \brief Read audio files and provide the decoded samples.
 *
 * A AudioReader can process an audio file and return the processing results.
 *
 * \note
 * Instances of this class are non-copyable but movable.
 */
class AudioReader final : public FileReader
{
public:

	/**
	 * \brief Constructor with a concrete implementation and a SampleProcessor.
	 *
	 * \param[in] impl AudioReader implementation to use
	 * \param[in] proc SampleProcessor to use
	 */
	AudioReader(std::unique_ptr<AudioReaderImpl> impl, SampleProcessor& proc);

	/**
	 * \brief Constructor with a concrete implementation
	 *
	 * \param[in] impl The implementation of this instance
	 */
	explicit AudioReader(std::unique_ptr<AudioReaderImpl> impl);

	AudioReader(AudioReader&&) noexcept;
	AudioReader& operator = (AudioReader&&) noexcept;

	/**
	 * \brief Default destructor.
	 */
	~AudioReader() noexcept final;

	/**
	 * \brief Set the number of samples to read in one read operation.
	 *
	 * The default is BLOCKSIZE::DEFAULT.
	 *
	 * \param[in] samples_per_read The number of 32 bit PCM samples per read
	 */
	void set_samples_per_read(const int64_t samples_per_read);

	/**
	 * \brief Return the number of samples to read in one read operation.
	 *
	 * \return Number of samples per read operation.
	 */
	int64_t samples_per_read() const;

	/**
	 * \brief Register a SampleProcessor instance to pass the read samples to.
	 *
	 * \param[in] processor SampleProcessor to use
	 */
	void set_processor(SampleProcessor& processor);

	/**
	 * \brief Acquire the AudioSize of a file.
	 *
	 * Acquiring the AudioSize includes validation.
	 *
	 * \param[in] filename The filename of the file to process
	 *
	 * \return AudioSize for the specified file
	 *
	 * \throw FileReadException If the file could not be read
	 */
	std::unique_ptr<AudioSize> acquire_size(const std::string& filename) const;

	/**
	 * \brief Process the file and return ARCSs v1 and v2 for all tracks.
	 *
	 * \param[in] filename The filename of the file to process
	 *
	 * \return The checksums of this file
	 *
	 * \throw FileReadException If the file could not be read
	 */
	void process_file(const std::string& filename);

private:

	class Impl;

	/**
	 * \brief Private implementation of this AudioReader.
	 */
	std::unique_ptr<AudioReader::Impl> impl_;

	std::unique_ptr<FileReaderDescriptor> do_descriptor() const final;
};


/**
 * \brief Reports validation failure on audio data.
 */
class InvalidAudioException final : public std::logic_error
{
public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] what_arg What argument
	 */
	explicit InvalidAudioException(const std::string& what_arg);

	/**
	 * \brief Constructor.
	 *
	 * \param[in] what_arg What argument
	 */
	explicit InvalidAudioException(const char* what_arg);
};


/**
 * \brief Abstract base class for validation handlers for
 * \link AudioReaderImpl AudioReaderImpls\endlink.
 *
 * Implements a class that just provides some assert methods that get a label, a
 * current value, a proper value and an error message. The validating handler
 * keeps an error list and can also return the latest error or the complete
 * list of errors. Subclasses may decide to throw exceptions by implementing
 * \c on_failure().
 *
 * It also provides default assertions for validating against CDDA,
 * thereby delegating to CDDAValidator.
 *
 * Subclasses must implement \c do_codecs() to provide the list of supported
 * audio codecs, i.e. codecs that are actively validated. If the subclass does
 * not validate any aspects of the codec, it should return an empty set.
 *
 * \note
 * Instances of subclasses are non-copyable but movable.
 *
 * \see DefaultValidator
 */
class AudioValidator
{
public:

	/**
	 * \brief Type of the error list.
	 */
	using error_list_type = std::vector<std::string>;

	/**
	 * \brief Type of the codec set.
	 */
	using codec_set_type = std::set<Codec>;

	/**
	 * \brief Empty constructor.
	 */
	AudioValidator();

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~AudioValidator() noexcept;

	/**
	 * \brief Returns the codec to validate.
	 */
	codec_set_type codecs() const;

	/**
	 * \brief CDDA validation of the sample size.
	 *
	 * The number of bits per sample must conform to
	 * CDDAValidator::bits_per_sample().
	 *
	 * Calls \c on_failure() when validation fails.
	 *
	 * \param[in] bits_per_sample The sample size to validate
	 *
	 * \return TRUE if the sample size equals CDDAValidator::bits_per_sample()
	 * otherwise FALSE
	 */
	void validate_bits_per_sample(const int bits_per_sample);

	/**
	 * \brief CDDA validation of the sampling rate (must be 44.100).
	 *
	 * The number of samples per second must conform to
	 * CDDAValidator::samples_per_second().
	 *
	 * Calls \c on_failure() when validation fails.
	 *
	 * \param[in] samples_per_second The sampling rate to test
	 *
	 * \return TRUE if the sampling rate equals
	 * CDDAValidator::samples_per_second() otherwise FALSE
	 */
	void validate_samples_per_second(const int samples_per_second);

	/**
	 * \brief CDDA validation for stereo (must be 2).
	 *
	 * The number of channels must conform to CDDAValidator::num_channels().
	 *
	 * Calls \c on_failure() when validation fails.
	 *
	 * \param[in] num_channels The number of channels to test
	 *
	 * \return TRUE if num_channels is equal to CDDAValidator::num_channels()
	 * otherwise FALSE
	 */
	void validate_num_channels(const int num_channels);

	/**
	 * \brief Add an error to the internal error list.
	 *
	 * \param[in] msg The error message to be added to the error list
	 */
	void error(const std::string& msg);

	/**
	 * \brief Returns the last error that occurred.
	 *
	 * \return The last error that occurred
	 */
	const std::string& last_error() const;

	/**
	 * \brief Returns TRUE iff there are any errors occurred so far.
	 *
	 * \return TRUE iff there are errors in the internal error list
	 */
	bool has_errors() const;

	/**
	 * \brief Returns the current error list.
	 *
	 * \return Current list of errors
	 */
	const error_list_type& get_errors() const;

protected:

	AudioValidator(AudioValidator&&) noexcept = default;
	AudioValidator& operator = (AudioValidator&&) noexcept
		= default;

	/**
	 * \brief Call on_failure() iff condition is TRUE.
	 *
	 * \param[in] condition Condition to check for TRUE
	 */
	void fail_if(const bool condition);

	/**
	 * \brief Returns TRUE iff value == proper_value.
	 *
	 * Always prints the label. Iff the comparison is not TRUE, error_msg is
	 * printed and a new error is added to the error list.
	 *
	 * \param[in] label Label to log for this test
	 * \param[in] value Value to be checked
	 * \param[in] proper_value Value to check against
	 * \param[in] error_msg Message to log in case value is not equal to
	 * proper_value
	 *
	 * \return TRUE if value is equal to proper_value, otherwise FALSE
	 */
	bool assert_equals(
		const std::string& label,
		int value,
		int proper_value,
		const std::string& error_msg);

	/**
	 * \brief Returns TRUE iff value == proper_value.
	 *
	 * Always prints the label. Iff the comparison is not TRUE, error_msg is
	 * printed and a new error is added to the error list.
	 *
	 * \param[in] label Label to log for this test
	 * \param[in] value Value to be checked
	 * \param[in] proper_value Value to check against
	 * \param[in] error_msg Message to log in case value is not equal to
	 * proper_value
	 *
	 * \return TRUE if value is equal to proper_value, otherwise FALSE
	 */
	bool assert_equals_u(
		const std::string& label,
		uint32_t value,
		uint32_t proper_value,
		const std::string& error_msg);

	/**
	 * \brief Returns TRUE iff value >= proper_value.
	 *
	 * Always prints the label. Iff the comparison is not TRUE, error_msg is
	 * printed and a new error is added to the error list.
	 *
	 * \param[in] label Label to log for this test
	 * \param[in] value Value to be checked
	 * \param[in] proper_value Value to check against
	 * \param[in] error_msg Message to log in case value is smaller than
	 * proper_value
	 *
	 * \return TRUE if value is not smaller than proper_value, otherwise FALSE
	 */
	bool assert_at_least(
		const std::string& label,
		int value,
		int proper_value,
		const std::string& error_msg);

	/**
	 * \brief Returns TRUE iff value <= proper_value.
	 *
	 * Always prints the label. Iff the comparison is not TRUE, error_msg is
	 * printed and a new error is added to the error list.
	 *
	 * \param[in] label Label to log for this test
	 * \param[in] value Value to be checked
	 * \param[in] proper_value Value to check against
	 * \param[in] error_msg Message to log in case value is bigger than
	 * proper_value
	 *
	 * \return TRUE if value is not bigger than proper_value, otherwise FALSE
	 */
	bool assert_at_most(
		const std::string& label,
		int value,
		int proper_value,
		const std::string& error_msg);

	/**
	 * \brief Returns TRUE iff value is true.
	 *
	 * Always prints the label. Iff the comparison is not TRUE, error_msg is
	 * printed and a new error is added to the error list.
	 *
	 * \param[in] label Label to log for this test
	 * \param[in] value Value to be checked for being TRUE
	 * \param[in] error_msg Message to log in case value is not TRUE
	 *
	 * \return TRUE if value is TRUE, otherwise FALSE
	 */
	bool assert_true(
		const std::string& label,
		bool value,
		const std::string& error_msg);

	/**
	 * \brief Logs every error on the error stack with ARCS_LOG_ERROR.
	 *
	 * Leaves the error stack unmodified.
	 */
	void log_error_stack() const;

	/**
	 * \brief Hook called when one of the predefined validations fails.
	 */
	virtual void on_failure()
	= 0;

private:

	/**
	 * \brief Implements \c codecs().
	 *
	 * \return List of supported codecs
	 */
	virtual codec_set_type do_codecs() const
	= 0;

	/**
	 * \brief Result: List of errors in the validated input
	 */
	error_list_type errors_;
};


/**
 * \brief Abstract base implementation of AudioValidator.
 *
 * Implements \c on_failure to throw an InvalidAudioException with the message
 * of the last error. The implementation of \c do_codecs() is left to
 * subclasses.
 */
class DefaultValidator : public AudioValidator
{
protected:

	void on_failure() override;
};


/**
 * \brief Service: verify the CDDA conformity of values.
 */
struct CDDAValidator final
{
	/**
	 * \brief Return TRUE iff the number of bits per sample conforms to CDDA.
	 *
	 * \param[in] bits_per_sample The actual number of bits per sample
	 *
	 * \return TRUE iff the number of bits per sample is 16 (conforming to CDDA)
	 * otherwise FALSE
	 */
	static bool bits_per_sample(const int& bits_per_sample);

	/**
	 * \brief Return TRUE iff the number of channels conforms to CDDA.
	 *
	 * \param[in] num_channels The actual number of channels
	 *
	 * \return TRUE iff the number of channels is 2 (conforming to CDDA)
	 * otherwise FALSE
	 */
	static bool num_channels(const int& num_channels);

	/**
	 * \brief Return TRUE if the sample rate conforms to CDDA.
	 *
	 * \param[in] samples_per_second The actual number of samples per second
	 *
	 * \return TRUE iff the number of samples per second is 44100
	 * (conforming to CDDA) otherwise FALSE
	 */
	static bool samples_per_second(const int& samples_per_second);
};


/**
 * \brief Service: interpret sequences of 2 or 4 little-endian bytes as integer.
 */
struct LittleEndianBytes final
{
	/**
	 * \brief Service method: Interpret 2 bytes as a 16 bit (signed) integer
	 * with little endian storage, which means that the bits of b2 become the
	 * most significant bits of the result.
	 *
	 * \param[in] b1
	 *     First input byte, provides least significant bits of the result
	 * \param[in] b2
	 *     Second input byte, provides most significant bits of the result
	 *
	 * \return The bytes as 16 bit (signed) integer
	 */
	static int16_t to_int16(const char& b1, const char& b2);

	/**
	 * \brief Service method: Interpret 2 bytes as a 16 bit unsigned integer
	 * with little endian storage, which means that the bits of b2 become the
	 * most significant bits of the result.
	 *
	 * \param[in] b1
	 *     First input byte, provides least significant bits of the result
	 * \param[in] b2
	 *     Second input byte, provides most significant bits of the result
	 *
	 * \return The bytes as 16 bit unsigned integer
	 */
	static uint16_t to_uint16(const char& b1, const char& b2);

	/**
	 * \brief Service method: Interpret 4 bytes as a 32 bit (signed) integer
	 * with little endian storage, which means that the bits of b4 become the
	 * most significant bits of the result.
	 *
	 * \param[in] b1
	 *     First input byte, provides least significant bits of the result
	 * \param[in] b2
	 *     Second input byte
	 * \param[in] b3
	 *     Third input byte
	 * \param[in] b4
	 *     Fourth input byte, provides most significant bits of the result
	 *
	 * \return The bytes as 32 bit (signed) integer
	 */
	static int32_t to_int32(const char& b1,
			const char& b2,
			const char& b3,
			const char& b4);

	/**
	 * \brief Service method: Interpret 4 bytes as a 32 bit unsigned integer
	 * with little endian storage, which means that the bits of b4 become the
	 * most significant bits of the result.
	 *
	 * \param[in] b1
	 *     First input byte, provides least significant bits of the result
	 * \param[in] b2
	 *     Second input byte
	 * \param[in] b3
	 *     Third input byte
	 * \param[in] b4
	 *     Fourth input byte, provides most significant bits of the result
	 *
	 * \return The bytes as 32 bit unsigned integer
	 */
	static uint32_t to_uint32(const char& b1,
			const char& b2,
			const char& b3,
			const char& b4);
};


/**
 * \brief Service: interpret sequences of 2 or 4 big endian bytes as integer.
 */
struct BigEndianBytes final
{
	/**
	 * \brief Service method: Interpret 4 bytes as a 32 bit (signed) integer
	 * with big endian storage, which means that the bits of b1 become the
	 * most significant bits of the result.
	 *
	 * \param[in] b1
	 *     First input byte, provides most significant bits of the result
	 * \param[in]
	 *     b2 Second input byte
	 * \param[in]
	 *     b3 Third input byte
	 * \param[in] b4
	 *     Fourth input byte, provides least significant bits of the result
	 *
	 * \return The bytes as 32 bit (signed) integer
	 */
	static int32_t to_int32(const char& b1,
			const char& b2,
			const char& b3,
			const char& b4);

	/**
	 * \brief Service method: Interpret 4 bytes as a 32 bit unsigned integer
	 * with big endian storage, which means that the bits of b1 become the
	 * most significant bits of the result.
	 *
	 * \param[in] b1
	 *     First input byte, provides most significant bits of the result
	 * \param[in] b2
	 *     Second input byte
	 * \param[in] b3
	 *     Third input byte
	 * \param[in] b4
	 *     Fourth input byte, provides least significant bits of the result
	 *
	 * \return The bytes as 32 bit unsigned integer
	 */
	static uint32_t to_uint32(const char& b1,
			const char& b2,
			const char& b3,
			const char& b4);
};

/// @}

} // namespace v_1_0_0

} // namespace arcsdec

#endif

