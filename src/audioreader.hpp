#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#define __LIBARCSDEC_AUDIOREADER_HPP__

/**
 * \file audioreader.hpp Toolkit for reading and validating audio files
 */

#include <functional>
#include <memory>
#include <string>
#include <vector>

#ifndef __LIBARCS_CALCULATE_HPP__
#include <arcs/calculate.hpp>
#endif

#ifndef __LIBARCSDEC_FILEFORMATS_HPP__
#include "fileformats.hpp"
#endif


/**
 * Namespace for readers
 */
namespace arcs
{

/**
 * \internal \defgroup audioreader Level 0 API: Reading and Validating Audio Files
 *
 * \brief Interface for implementing and creating AudioReaders
 *
 * The interface for reading audio files is provided by class AudioReader that
 * internally holds a concrete instance of AudioReaderImpl.
 *
 * The AudioReader provides two actual operations on the input file: it can
 * either analyze the file to provide a CalcContext for it or actually process
 * the file, which yields the actual ARCSs. An AudioReader can be attached a
 * pre-configured \ref Calculation to perform the actual calculation.
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
 * the SAMPLE_FORMAT along to the requirements of \ref Calculation.
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
 * AudioReaderCreator is a convenience class to create an AudioReader for any
 * specified audio input file.
 *
 * @{
 */


// ByteConverter


/**
 * Service class providing methods to convert short byte sequences to
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
	 * Virtual default destructor
	 */
	virtual ~ByteConverter() noexcept;

	/**
	 * Service method: Interpret a single byte as a 8 bit unsigned integer with
	 * storage as-is.
	 *
	 * \param[in] b Input byte
	 *
	 * \return The byte as 8 bit unsigned integer
	 */
	uint8_t byte_to_uint8(const char &b) const;

	/**
	 * Service method: Interpret 2 bytes as a 16 bit (signed) integer
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
	 * Service method: Interpret 2 bytes as a 16 bit unsigned integer
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
	 * Service method: Interpret 4 bytes as a 32 bit (signed) integer
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
	 * Service method: Interpret 4 bytes as a 32 bit unsigned integer
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
	 * Service method: Interpret 4 bytes as a 32 bit (signed) integer
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
	 * Service method: Interpret 4 bytes as a 32 bit unsigned integer
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
 * Supported sample formats. The sample format represents two basic
 * informations: the physical size of a sample in bits and whether the samples
 * are arranged in a planar or interleaved layout.
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
 * Validates values against the reference values in CDDA_t.
 *
 * This just encapsulate the comparisons for reuse.
 */
class CDDAValidator
{

public:

	/**
	 * Destructor stub
	 */
	virtual ~CDDAValidator() noexcept;

	/**
	 * Return TRUE iff the number of bits per sample conforms to CDDA
	 *
	 * \param[in] bits_per_sample The actual number of bits per sample
	 *
	 * \return TRUE iff the number of bits per sample is 16 (conforming to CDDA)
	 * otherwise FALSE
	 */
	bool bits_per_sample(const uint32_t &bits_per_sample);

	/**
	 * Return TRUE iff the number of channels conforms to CDDA
	 *
	 * \param[in] num_channels The actual number of channels
	 *
	 * \return TRUE iff the number of channels is 2 (conforming to CDDA)
	 * otherwise FALSE
	 */
	bool num_channels(const uint32_t &num_channels);

	/**
	 * Return TRUE if the sample rate conforms to CDDA
	 *
	 * \param[in] samples_per_second The actual number of samples per second
	 *
	 * \return TRUE iff the number of samples per second is 44100
	 * (conforming to CDDA) otherwise FALSE
	 */
	bool samples_per_second(const uint32_t &samples_per_second);
};


/**
 * Base class for event handlers to be used by instances of AudioReaderImpl.
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
	 * Default constructor
	 */
	ReaderValidatingHandler();

	// make class non-copyable (1/2)
	ReaderValidatingHandler(const ReaderValidatingHandler &) = delete;

	// TODO move constructor

	/**
	 * Virtual default destructor
	 */
	virtual ~ReaderValidatingHandler() noexcept;

	/**
	 * Add an error to the internal error list
	 *
	 * \param[in] msg The error message to be added to the error list
	 */
	void error(const std::string &msg);

	/**
	 * Returns TRUE iff there are any errors occurred so far
	 *
	 * \return TRUE iff there are errors in the internal error list
	 */
	bool has_errors();

	/**
	 * Returns the current error list
	 *
	 * \return Current list of errors
	 */
	std::vector<std::string> get_errors(); // TODO type leak, list?

	/**
	 * Returns the last error that occurred
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
	 * Returns TRUE iff value == proper_value. Always prints the label. Iff
	 * the comparison is not TRUE, error_msg is printed and a new error
	 * is added to the error list.
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
		const uint32_t &value,
		const uint32_t &proper_value,
		const std::string error_msg);

	/**
	 * Returns TRUE iff value >= proper_value. Always prints the label. Iff
	 * the comparison is not TRUE, error_msg is printed and a new error
	 * is added to the error list.
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
		const uint32_t &value,
		const uint32_t &proper_value,
		const std::string error_msg);

	/**
	 * Returns TRUE iff value <= proper_value. Always prints the label. Iff
	 * the comparison is not TRUE, error_msg is printed and a new error
	 * is added to the error list.
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
		const uint32_t &value,
		const uint32_t &proper_value,
		const std::string error_msg);

	/**
	 * Returns TRUE iff value is true. Always prints the label. Iff
	 * the comparison is not TRUE, error_msg is printed and a new error
	 * is added to the error list.
	 *
	 * \param[in] label Label to log for this test
	 * \param[in] value Value to be checked for being TRUE
	 * \param[in] error_msg Message to log in case value is not TRUE
	 *
	 * \return TRUE if value is TRUE, otherwise FALSE
	 */
	bool assert_true(
		const std::string &label,
		const bool &value,
		const std::string error_msg);


private:

	/**
	 * Result: List of errors in the validated stream
	 */
	std::vector<std::string> errors_;
};


/**
 * Abstract base class for AudioReader implementations.
 *
 * Concrete subclasses of AudioReaderImpl implement AudioReaders for a concrete
 * FileFormat.
 */
class AudioReaderImpl
{

public:

	/**
	 * Default constructor
	 */
	AudioReaderImpl();

	/**
	 * Virtual default destructor
	 */
	virtual ~AudioReaderImpl() noexcept;

	// make class non-copyable (1/2)
	AudioReaderImpl(const AudioReaderImpl &) = delete;

	// TODO Move constructor

	/**
	 * Provides implementation for acquire_size() of a AudioReader
	 *
	 * \param[in] filename The filename of the file to process
	 *
	 * \return A CalcContext for the specified file
	 *
	 * \throw FileReadException If the file could not be read
	 */
	std::unique_ptr<AudioSize> acquire_size(const std::string &filename);

	/**
	 * Provides implementation for process_file() of some AudioReader
	 *
	 * \param[in] filename The filename of the file to process
	 *
	 * \return The checksums of this file
	 *
	 * \throw FileReadException If the file could not be read
	 */
	Checksums process_file(const std::string &filename);

	/**
	 * Set the number of samples to be accumulated in one block to be processed.
	 * The parameter is silently clipped to appropriate bounds if necessary.
	 *
	 * Setting the samples per block to a certain value does not guarantee that
	 * this number of samples is read in a block at once. It only clips the
	 * number of samples to be accumulated in memory.
	 *
	 * \param[in] samples_per_block The number of samples in one block
	 */
	void set_samples_per_block(const uint32_t &samples_per_block);

	/**
	 * Return the number of samples to be put in one block.
	 *
	 * \return The current number of samples to put in one block
	 */
	uint32_t samples_per_block() const;

	/**
	 * Sets the \ref Calculation instance of this instance
	 *
	 * \param[in] calc The \ref Calculation for this instance
	 */
	void set_calc(std::unique_ptr<Calculation> calc);

	/**
	 * Returns the \ref Calculation instance of this instance
	 *
	 * \return The \ref Calculation of this instance
	 */
	const Calculation& calc() const;

	// make class non-copyable (2/2)
	AudioReaderImpl& operator = (const AudioReaderImpl &) = delete;

	// TODO move assignment


private:

	/**
	 * Provides implementation for \c acquire_size() of an \ref AudioReader
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
	 * Provides implementation for process_file() of some AudioReader
	 *
	 * \param[in] filename The filename of the file to process
	 *
	 * \return The checksums of this file
	 *
	 * \throw FileReadException If the file could not be read
	 */
	virtual Checksums do_process_file(const std::string &filename)
	= 0;

	/**
	 * Implementation of set_samples_per_block()
	 *
	 * Set the number of samples to be accumulated in one block to be processed.
	 * The parameter is silently clipped to appropriate bounds if necessary.
	 *
	 * Setting the samples per block to a certain value does not guarantee that
	 * this number of samples is read in a block at once. It only clips the
	 * number of samples to be accumulated in memory.
	 *
	 * \param[in] samples_per_block The number of samples in one block
	 */
	virtual void do_set_samples_per_block(const uint32_t &samples_per_block)
	= 0;

	/**
	 * Implementation of get_samples_per_block()
	 *
	 * Return the number of samples to be put in one block.
	 *
	 * \return The current number of samples to put in one block
	 */
	virtual uint32_t do_get_samples_per_block() const
	= 0;

	/**
	 * Implementation of set_calc()
	 *
	 * Sets the \ref Calculation instance of this instance
	 *
	 * \param[in] calc The \ref Calculation for this instance
	 */
	virtual void do_set_calc(std::unique_ptr<Calculation> calc)
	= 0;

	/**
	 * Implementation of get_calc()
	 *
	 * Returns the \ref Calculation instance of this instance
	 *
	 * \return The \ref Calculation of this instance
	 */
	virtual const Calculation& do_get_calc() const
	= 0;
};


/**
 * Represents a reader to a concrete combination of a lossless audio
 * format and a file format.
 *
 * A AudioReader can process an audio file and return its checksums including
 * the ARCSs v1 and v2 for all tracks.
 */
class AudioReader : public FileReader//, public virtual SampleProvider
{

public:

	/**
	 * Constructor with a concrete implementation
	 *
	 * \param[in] impl The implementation of this instance
	 */
	explicit AudioReader(std::unique_ptr<AudioReaderImpl> impl);

	// make class non-copyable (1/2)
	AudioReader(const AudioReader &) = delete;

	// TODO move constructor

	/**
	 * Default destructor
	 */
	~AudioReader() noexcept override;

	/**
	 * Acquire the \ref AudioSize of a file.
	 *
	 * Acquiring the \ref AudioSize includes validation.
	 *
	 * \param[in] filename The filename of the file to process
	 *
	 * \return \ref AudioSize for the specified file
	 *
	 * \throw FileReadException If the file could not be read
	 */
	std::unique_ptr<AudioSize> acquire_size(
		const std::string &filename) const;

	/**
	 * Process the file and return ARCSs v1 and v2 for all tracks.
	 *
	 * \param[in] filename The filename of the file to process
	 *
	 * \return The checksums of this file
	 *
	 * \throw FileReadException If the file could not be read
	 */
	Checksums process_file(const std::string &filename);

	/**
	 * Set the number of samples to be accumulated in one block to be processed.
	 * The parameter is silently clipped to appropriate bounds if necessary.
	 *
	 * Setting the samples per block to a certain value does not guarantee that
	 * this number of samples is read in a block at once. It only clips the
	 * number of samples to be accumulated in memory.
	 *
	 * \param[in] samples_per_block The number of samples in one block
	 */
	void set_samples_per_block(const uint32_t &samples_per_block);

	/**
	 * Return the number of samples to be put in one block.
	 *
	 * \return The current number of samples to put in one block
	 */
	uint32_t samples_per_block() const;

	/**
	 * Set the calculator instance of this reader
	 *
	 * \param[in] calc The \ref Calculation for this instance
	 */
	void set_calc(std::unique_ptr<Calculation> calc);

	/**
	 * Get the calculator instance of this reader
	 *
	 * \return The \ref Calculation of this instance
	 */
	const Calculation& calc();

	// make class non-copyable (2/2)
	AudioReader& operator = (const AudioReader &) = delete;

	// TODO move assignment


private:

	/**
	 * Private implementation of this AudioReader
	 */
	std::unique_ptr<AudioReaderImpl> impl_;
};


/**
 * A builder class for AudioReaders. Selects a matching FileFormat for a given
 * file and creates an appropriate AudioReader for the specified file.
 */
class AudioReaderCreator : public FileReaderCreator
{

public:

	/**
	 * Constructor
	 */
	AudioReaderCreator();

	/**
	 * Virtual default destructor
	 */
	~AudioReaderCreator() noexcept override;

	/**
	 * Create an AudioReader for the specified file
	 *
	 * \param[in] filename The filename to create an AudioReader for
	 *
	 * \return An AudioReader for the given file
	 */
	std::unique_ptr<AudioReader> create_audio_reader(
			const std::string &filename) const;
};

} // namespace arcs

/// @}

#endif

