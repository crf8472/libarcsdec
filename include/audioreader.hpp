#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#define __LIBARCSDEC_AUDIOREADER_HPP__

/**
 * \file
 *
 * \brief API for reading and validating audio files
 */

#include <functional>
#include <memory>
#include <string>
#include <vector>

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>
#endif

#ifndef __LIBARCSDEC_DESCRIPTORS_HPP__
#include "descriptors.hpp"
#endif
#ifndef __LIBARCSDEC_SAMPLEPROC_HPP__
#include "sampleproc.hpp"
#endif

namespace arcsdec
{
inline namespace v_1_0_0
{

/**
 * \internal
 * \defgroup audioreader API for reading and validating audio files
 *
 * \brief Interface for implementing and creating AudioReaders
 *
 * The interface for reading audio files is provided by class AudioReader that
 * internally holds a concrete instance of AudioReaderImpl.
 *
 * The AudioReader provides two actual operations on the input file: it can
 * either analyze the file to provide a CalcContext for it or actually process
 * the file, which yields the actual ARCSs. An AudioReader can be attached a
 * pre-configured Calculation to perform the actual calculation.
 *
 * The concrete reading of a given audio file is implemented by the subclasses
 * of AudioReaderImpl. An AudioReaderImpl can be set to a block size, that may
 * or may not refer to a buffer size, depending on the actual implementation.
 *
 * To avoid reimplementing things, there are some tools provided in this API to
 * ease up the implementation of an AudioReaderImpl:
 *
 * A subclass of SampleSequence can be used (and actually re-used) for
 * transferring the audio samples to a SampleBuffer. SampleSequences hides the
 * concrete SAMPLE_FORMAT to the buffer (including whether the sample order is
 * planar or interleaved). Putting a SampleSequence to a SampleBuffer normalizes
 * the SAMPLE_FORMAT along to the requirements of Calculation.
 *
 * SampleBuffer buffers SampleSequences and flushes an entire block of samples
 * to a registered ARCScalc instance once the buffer is full.
 *
 * CDDAValidator provides a uniform implementation of checking core values from
 * an input file for CDDA conformity.
 *
 * ReaderValidatingHandler wraps a CDDAValidator with error tracking for easy
 * registering to an AudioReaderImpl.
 *
 * PCMBlockReader reads raw PCM samples from an input stream.
 *
 * ByteConverter converts single sequences of chars to integer representations.
 *
 * AudioReaderSelection is a convenience class to create an AudioReader for any
 * specified audio input file.
 *
 * @{
 */

using arcstk::AudioSize;

/**
 * \brief Service class providing methods to convert short byte sequences to
 * integers.
 *
 * ByteConverter can be used to parse integers from an input stream of bytes.
 * It provides methods to interpret short sequences of 2 or 4 bytes as a 16 bit
 * or 32 bit integer number.
 */
class ByteConverter
{

public:

	/**
	 * \brief Virtual default destructor
	 */
	virtual ~ByteConverter() noexcept;

	/**
	 * \brief Service method: Interpret a single byte as a 8 bit unsigned
	 * integer with storage as-is.
	 *
	 * \param[in] b Input byte
	 *
	 * \return The byte as 8 bit unsigned integer
	 */
	uint8_t byte_to_uint8(const char &b) const;

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
	int16_t le_bytes_to_int16(const char &b1, const char &b2) const;

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
	uint16_t le_bytes_to_uint16(const char &b1, const char &b2) const;

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
	int32_t le_bytes_to_int32(const char &b1,
			const char &b2,
			const char &b3,
			const char &b4) const;

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
	uint32_t le_bytes_to_uint32(const char &b1,
			const char &b2,
			const char &b3,
			const char &b4) const;

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
	int32_t be_bytes_to_int32(const char &b1,
			const char &b2,
			const char &b3,
			const char &b4) const;

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
	uint32_t be_bytes_to_uint32(const char &b1,
			const char &b2,
			const char &b3,
			const char &b4) const;
};


/**
 * \brief Supported sample formats.
 *
 * The sample format represents two basic informations: the physical size of a
 * sample in bits and whether the samples are arranged in a planar or
 * interleaved layout.
 */
enum class SAMPLE_FORMAT : uint8_t
{
	UNKNOWN = 0,
	S16     = 1,
	S16P    = 2,
	S32     = 3,
	S32P    = 4
};


/**
 * \brief Verifies the CDDA conformity of values.
 *
 * This just encapsulate the comparisons for reuse.
 */
class CDDAValidator
{

public:

	/**
	 * \brief Virtual destructor.
	 */
	virtual ~CDDAValidator() noexcept;

	/**
	 * \brief Return TRUE iff the number of bits per sample conforms to CDDA.
	 *
	 * \param[in] bits_per_sample The actual number of bits per sample
	 *
	 * \return TRUE iff the number of bits per sample is 16 (conforming to CDDA)
	 * otherwise FALSE
	 */
	bool bits_per_sample(const int &bits_per_sample);

	/**
	 * \brief Return TRUE iff the number of channels conforms to CDDA.
	 *
	 * \param[in] num_channels The actual number of channels
	 *
	 * \return TRUE iff the number of channels is 2 (conforming to CDDA)
	 * otherwise FALSE
	 */
	bool num_channels(const int &num_channels);

	/**
	 * \brief Return TRUE if the sample rate conforms to CDDA.
	 *
	 * \param[in] samples_per_second The actual number of samples per second
	 *
	 * \return TRUE iff the number of samples per second is 44100
	 * (conforming to CDDA) otherwise FALSE
	 */
	bool samples_per_second(const int &samples_per_second);
};


/**
 * \brief Base class for validation handlers for
 * @link AudioReaderImpl AudioReaderImpls @endlink.
 *
 * Implements a class that just provides some assert methods that get a label, a
 * current value, a proper value and an error message. The validating handler
 * keeps an error list and can also return the latest error or the complete
 * list of errors. Subclasses may decide to throw exceptions accordingly.
 */
class ReaderValidatingHandler
{

public:

	/**
	 * \brief Default constructor.
	 */
	ReaderValidatingHandler();

	// make class non-copyable (1/2)
	ReaderValidatingHandler(const ReaderValidatingHandler &) = delete;

	// TODO move constructor

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~ReaderValidatingHandler() noexcept;

	/**
	 * \brief Add an error to the internal error list.
	 *
	 * \param[in] msg The error message to be added to the error list
	 */
	void error(const std::string &msg);

	/**
	 * \brief Returns TRUE iff there are any errors occurred so far.
	 *
	 * \return TRUE iff there are errors in the internal error list
	 */
	bool has_errors();

	/**
	 * \brief Returns the current error list.
	 *
	 * \return Current list of errors
	 */
	std::vector<std::string> get_errors(); // TODO type leak, list?

	/**
	 * \brief Returns the last error that occurred.
	 *
	 * \return The last error that occurred
	 */
	std::string last_error();

	// make class non-copyable (2/2)
	ReaderValidatingHandler& operator = (const ReaderValidatingHandler &)
		= delete;

	// TODO move assignment


protected:

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
		const std::string &label,
		int value,
		int proper_value,
		const std::string error_msg);

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
		const std::string &label,
		uint32_t value,
		uint32_t proper_value,
		const std::string error_msg);

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
		const std::string &label,
		int value,
		int proper_value,
		const std::string error_msg);

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
		const std::string &label,
		int value,
		int proper_value,
		const std::string error_msg);

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
		const std::string &label,
		bool value,
		const std::string error_msg);


private:

	/**
	 * \brief Result: List of errors in the validated input
	 */
	std::vector<std::string> errors_;
};


/**
 * \brief Abstract base class for AudioReader implementations.
 *
 * Concrete subclasses of AudioReaderImpl implement AudioReaders for a concrete
 * FileReaderDescriptor.
 */
class AudioReaderImpl : public virtual SampleProvider
{

public:

	/**
	 * \brief Default constructor.
	 */
	AudioReaderImpl();

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~AudioReaderImpl() noexcept;

	// make class non-copyable (1/2)
	AudioReaderImpl(const AudioReaderImpl &) = delete;

	// TODO Move constructor

	/**
	 * \brief Provides implementation for acquire_size() of a AudioReader.
	 *
	 * \param[in] filename The filename of the file to process
	 *
	 * \return A CalcContext for the specified file
	 *
	 * \throw FileReadException If the file could not be read
	 */
	std::unique_ptr<AudioSize> acquire_size(const std::string &filename);

	/**
	 * \brief Provides implementation for process_file() of some AudioReader.
	 *
	 * \param[in] filename The filename of the file to process
	 *
	 * \return The checksums of this file
	 *
	 * \throw FileReadException If the file could not be read
	 */
	void process_file(const std::string &filename);

	/**
	 * \brief Returns TRUE if the number of samples to read at once is
	 * configurable.
	 *
	 * \return TRUE if the number of samples to read at once is configurable.
	 */
	bool configurable_read_buffer() const;

	/**
	 * \brief Set the number of samples to read in one read operation.
	 *
	 * The default is BLOCKSIZE::DEFAULT.
	 */
	void set_samples_per_read(const int64_t &samples_per_read);

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

	// make class non-copyable (2/2)
	AudioReaderImpl& operator = (const AudioReaderImpl &) = delete;

	// TODO move assignment


private:

	/**
	 * \brief Returns TRUE if the number of samples to read at once is
	 * configurable.
	 *
	 * \return TRUE if the number of samples to read at once is configurable.
	 */
	virtual bool do_configurable_read_buffer() const;

	/**
	 * \brief Set the number of samples to read in one read operation.
	 *
	 * The default is BLOCKSIZE::DEFAULT.
	 */
	virtual void do_set_samples_per_read(const int64_t &samples_per_read);

	/**
	 * \brief Return the number of samples to read in one read operation.
	 *
	 * \return Number of samples per read operation.
	 */
	virtual int64_t do_samples_per_read() const;

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
		const std::string &filename)
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
	virtual void do_process_file(const std::string &filename)
	= 0;

	virtual std::unique_ptr<FileReaderDescriptor> do_descriptor() const
	= 0;
};


/**
 * \brief AudioReaderImpl with configurable read buffer size
 */
class BufferedAudioReaderImpl : public AudioReaderImpl
{

public:

	/**
	 * \brief Constructor.
	 */
	BufferedAudioReaderImpl();

	/**
	 * \brief Constructor with read size
	 */
	explicit BufferedAudioReaderImpl(const int64_t samples_per_read);

	/**
	 * \brief Destructor
	 */
	~BufferedAudioReaderImpl() noexcept override;

	// TODO Copy + move constructors

	// TODO Copy + move assignment


private:

	bool do_configurable_read_buffer() const final;

	void do_set_samples_per_read(const int64_t &samples_per_read) final;

	int64_t do_samples_per_read() const final;

	/**
	 * \brief Number of samples to be read in one block
	 */
	int64_t samples_per_read_;
};


/**
 * \brief Opaque instance to read audio files and provide the decoded samples.
 *
 * A AudioReader can process an audio file and return its checksums including
 * the ARCSs v1 and v2 for all tracks.
 */
class AudioReader : public FileReader
{

public:

	/**
	 * \brief Constructor with a concrete implementation and a SampleProcessor.
	 *
	 * \param[in] impl AudioReader implementation to use
	 * \param[in] proc SampleProcessor to use
	 */
	AudioReader(std::unique_ptr<AudioReaderImpl> impl, SampleProcessor &proc);

	/**
	 * \brief Constructor with a concrete implementation
	 *
	 * \param[in] impl The implementation of this instance
	 */
	explicit AudioReader(std::unique_ptr<AudioReaderImpl> impl);

	// make class non-copyable (1/2)
	AudioReader(const AudioReader &) = delete;

	// TODO move constructor

	/**
	 * \brief Default destructor.
	 */
	~AudioReader() noexcept override;

	/**
	 * \brief Check if the read buffer size can be specified.
	 *
	 * Returns TRUE if it can be specified which amount the reader should read
	 * in one pass.
	 *
	 * \return TRUE if this reader has a configurable buffer, otherwise FALSE
	 */
	bool configurable_read_buffer() const;

	/**
	 * \brief Set the number of samples to read in one read operation.
	 *
	 * The default is BLOCKSIZE::DEFAULT.
	 *
	 * \param[in] samples_per_read The number of 32 bit PCM samples per read
	 */
	void set_samples_per_read(const int64_t &samples_per_read);

	/**
	 * \brief Return the number of samples to read in one read operation.
	 *
	 * \return Number of samples per read operation.
	 */
	int64_t samples_per_read() const;

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
	std::unique_ptr<AudioSize> acquire_size(const std::string &filename) const;

	/**
	 * \brief Process the file and return ARCSs v1 and v2 for all tracks.
	 *
	 * \param[in] filename The filename of the file to process
	 *
	 * \return The checksums of this file
	 *
	 * \throw FileReadException If the file could not be read
	 */
	void process_file(const std::string &filename);

	/**
	 * \brief Register a SampleProcessor instance to pass the read samples to.
	 *
	 * \param[in] processor SampleProcessor to use
	 *
	 * \todo This should be part of a SampleProvider interface
	 */
	void set_processor(SampleProcessor &processor);

	// make class non-copyable (2/2)
	AudioReader& operator = (const AudioReader &) = delete;

	// TODO move assignment


private:

	class Impl;

	/**
	 * \brief Private implementation of this AudioReader.
	 */
	std::unique_ptr<AudioReader::Impl> impl_;

	std::unique_ptr<FileReaderDescriptor> do_descriptor() const override;
};


struct CreateAudioReader
{
	std::unique_ptr<AudioReader> operator()(const FileReaderSelection &s,
			const std::string &filename) const;
};


/**
 * \brief Selects and builds AudioReader instances for given inputs.
 */
class AudioReaderSelection : public FileReaderSelection
{

public:

	/**
	 * \brief Constructor.
	 */
	AudioReaderSelection();

	/**
	 * \brief Virtual default destructor.
	 */
	~AudioReaderSelection() noexcept override;

	/**
	 * \brief Create an AudioReader for the specified file.
	 *
	 * \param[in] filename The filename to create an AudioReader for
	 *
	 * \return An AudioReader for the given file
	 */
	std::unique_ptr<AudioReader> for_file(const std::string &filename) const;


protected:

	/**
	 * \brief Turns a FileReader to an AudioReader.
	 *
	 * \param[in] filereader The FileReader to cast
	 *
	 * \return AudioReader or nullptr
	 */
	std::unique_ptr<AudioReader> safe_cast(
			std::unique_ptr<FileReader> filereader) const;
};

/// @}

} // namespace v_1_0_0

} // namespace arcsdec

#endif
