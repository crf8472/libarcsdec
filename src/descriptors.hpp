#ifndef __LIBARCSDEC_DESCRIPTORS_HPP__
#define __LIBARCSDEC_DESCRIPTORS_HPP__

/**
 * \file descriptors.hpp Toolkit for selecting file readers
 */

#include <cstdint>
#include <functional>  // for std::function
#include <limits>
#include <list>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp> // for PCMForwardIterator
#endif


namespace arcsdec
{

inline namespace v_1_0_0
{

/// \defgroup descriptors API for creating a FileReader for a specified file

/**
 * \brief Framework for creating specialized FileReaders for a specified file.
 *
 * Abstract class FileReaderSelection implements the generic mechanism to check
 * a specified input file for a matching FileReaderDescriptor. If a matching
 * FileReaderDescriptor is found, an instance of this descriptor is returned
 * which is then used to create the concrete FileReader instance.
 *
 * A FileReaderSelection holds a a list of tests to perform on the input file
 * and a list of supported FileReaderDescriptors. Internally, it uses an
 * instance of FileReaderSelector to select a concrete FileReaderDescriptor.
 * FileReaderSelector performs the selection obeying a certain selection policy.
 * The default FileReaderSelector just selects the first FileReaderDescriptor in
 * the list of supported FileReaderDescriptors that passes all tests in the
 * provided list of tests.
 *
 * A FileTest implements a single test. It may or may not open the file to test.
 *
 * The \ref AudioReader and \ref MetadataParser APIs are built on this API.
 */

/// @{


// forward declaration for FileReader
class FileReaderDescriptor;


enum class FileFormat : uint32_t
{
	UNKNOWN = 0,

	// Audio Container
	RIFFWAV = 1,
	FLAC    = 2,
	APE     = 3,
	ALAC    = 4,
	WMA     = 5,
	M4A     = 6,
	OGG     = 7,
	AIFF    = 8,
	CAF     = 9,
	RAW     = 10,
	WV      = 11,

	ANY_AUDIO    = std::numeric_limits<uint32_t>::max() - 1001,
	ANY_METADATA = std::numeric_limits<uint32_t>::max() - 1000,

	// Metadata
	CUE     = ANY_METADATA + 1,
	TOC     = ANY_METADATA + 2
};


/**
 * \brief Returns TRUE if \c format is an audio FileFormat, otherwise FALSE.
 *
 * \param[in] format The FileFormat to test
 *
 * \return TRUE iff \c format is an audio format, otherwise FALSE.
 */
bool is_audio_format(FileFormat format);


/**
 * \brief Abstract base class for all FileReaders, acts as tag.
 *
 * This class does not define any interface but ensures a common base type for
 * all readers. Therefore, all readers can be built and provided by the same
 * creation framework.
 */
class FileReader
{

// TODO FileReader should inform about libs in its implementation

public:

	/**
	 * \brief Default constructor.
	 */
	FileReader();

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~FileReader() noexcept;

	/**
	 * \brief Get a descriptor for this FileReader.
	 */
	std::unique_ptr<FileReaderDescriptor> descriptor() const;


private:

	virtual std::unique_ptr<FileReaderDescriptor> do_descriptor() const
	= 0;
};


/**
 * \brief Reports an error while reading a file.
 *
 * This exception can be thrown when the file does not exist or is not readable
 * or another IO related error occurrs while reading the file content.
 *
 * A FileReadException may optionally carry the byte position of the error. A
 * negative value indicates that no position is known.
 */
class FileReadException final : public std::runtime_error
{

public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] what_arg What argument
	 */
	explicit FileReadException(const std::string &what_arg);

	/**
	 * \brief Constructor.
	 *
	 * The byte position marks the error. This implies that byte_pos - 1 bytes
	 * have been read without error.
	 *
	 * \param[in] what_arg What argument
	 * \param[in] byte_pos Byte position of the error
	 */
	FileReadException(const std::string &what_arg, const int64_t &byte_pos);

	/**
	 * \brief Byte position on which the error occurred or a negative value if
	 * the position is not known.
	 *
	 * This entails that byte_pos - 1 bytes have been read without error.
	 *
	 * \return Byte position on which the error occurred.
	 */
	int64_t byte_pos() const;


private:

	/**
	 * \brief Internal byte position.
	 */
	int64_t byte_pos_;
};


/**
 * \brief Abstract base class for the properties of a FileReader.
 *
 * A FileReaderDescriptor provides information that can be used to decide
 * whether a a given file can be read by readers specified by this descriptor.
 * It can create an opaque reader that can read the file.
 *
 * A FileReaderDescriptor is just an abstract test whether a specific reader
 * should be created or not. It may or may not be a specific combination of a
 * codec and a file container format. For instance, "FLAC" can be a
 * FileReaderDescriptor, representing FLAC data in a FLAC container. Another way
 * to implement a FileReaderDescriptor would be "ffmpeg-readable audio file".
 * Therefore, a FileReaderDescriptor does not to distinguish between container
 * format and codec, but just should provide the heuristics for deciding whether
 * the corresponding FileReader can read the file in question.
 */
class FileReaderDescriptor
{

public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~FileReaderDescriptor() noexcept;

	/**
	 * \brief Name of this FileReaderDescriptor.
	 *
	 * \return A human-readable name of this FileReaderDescriptor
	 */
	std::string name() const;

	/**
	 * \brief Check whether this descriptor matches the given input bytes.
	 *
	 * \param[in] bytes  Sequence of consecutive bytes in the file
	 * \param[in] offset Offset of the first byte of this sequence
	 *
	 * \return TRUE iff the descriptor accepts the sequence, otherwise FALSE
	 */
	bool accepts_bytes(const std::vector<char> &bytes,
			const uint64_t &offset) const;

	/**
	 * \brief Check whether this descriptor is known to have the specified
	 * suffix.
	 *
	 * \param[in] suffix The suffix of the filename to test for
	 *
	 * \return TRUE iff the suffix matches the suffix of the descriptor
	 */
	bool accepts_suffix(const std::string &suffix) const;

	/**
	 * \brief Check for acceptance of the specified format
	 *
	 * \param[in] format The format to check for
	 *
	 * \return TRUE if \c format is accepted, otherwise FALSE
	 */
	bool accepts(FileFormat format) const;

	/**
	 * \brief @link FileFormat FileFormats @endlink accepted by the FileReader.
	 *
	 * \return @link FileFormat FileFormats @endlink accepted by the FileReader
	 */
	std::set<FileFormat> formats() const;

	/**
	 * \brief Create an opaque reader for the specified file.
	 *
	 * \return A FileReader that can read this FileReaderDescriptor
	 */
	std::unique_ptr<FileReader> create_reader() const;

	/**
	 * \brief Clone this instance.
	 *
	 * Method clone() allows to duplicate an instance without knowing its
	 * static type.
	 *
	 * \return A deep copy of the instance
	 */
	std::unique_ptr<FileReaderDescriptor> clone() const;

	/**
	 * \brief Equality.
	 *
	 * \param[in] rhs The right hand side of the comparison
	 *
	 * \return TRUE iff the right hand side is equal to the left hand side,
	 * otherwise false
	 */
	bool operator == (const FileReaderDescriptor &rhs) const;

	/**
	 * \brief Inequality.
	 *
	 * \param[in] rhs The right hand side of the comparison
	 *
	 * \return TRUE iff the right hand side is not equal to the left hand side,
	 * otherwise false
	 */
	bool operator != (const FileReaderDescriptor &rhs) const;


private:

	/**
	 * \brief Implements FileReaderDescriptor::name().
	 *
	 * \return A human-readable name of this FileReaderDescriptor
	 */
	virtual std::string do_name() const
	= 0;

	/**
	 * \brief Implements FileReaderDescriptor::accepts_bytes().
	 *
	 * \param[in] bytes  Sequence of consecutive bytes in the file
	 * \param[in] offset Offset of the first byte of this sequence
	 *
	 * \return TRUE iff the descriptor accepts the bytes, otherwise FALSE
	 */
	virtual bool do_accepts_bytes(const std::vector<char> &bytes,
			const uint64_t &offset) const
	= 0;

	/**
	 * \brief Implements FileReaderDescriptor::accepts_suffix().
	 *
	 * \param[in] suffix The suffix of the filename to test for
	 *
	 * \return TRUE iff the suffix matches the suffix of the descriptor
	 */
	virtual bool do_accepts_suffix(const std::string &suffix) const
	= 0;

	/**
	 * \brief Check for acceptance of the specified format
	 *
	 * \param[in] format The format to check for
	 *
	 * \return TRUE if \c format is accepted, otherwise FALSE
	 */
	virtual bool do_accepts(FileFormat format) const
	= 0;

	/**
	 * \brief @link FileFormat FileFormats @endlink accepted by the FileReader.
	 *
	 * \return @link FileFormat FileFormats @endlink accepted by the FileReader
	 */
	virtual std::set<FileFormat> do_formats() const
	= 0;

	/**
	 * \brief Implements FileReaderDescriptor::create_reader().
	 *
	 * \return A FileReader that can read this FileReaderDescriptor
	 */
	virtual std::unique_ptr<FileReader> do_create_reader() const
	= 0;

	/**
	 * \brief Implements FileReaderDescriptor::clone().
	 *
	 * \return A deep copy of the instance
	 */
	virtual std::unique_ptr<FileReaderDescriptor> do_clone() const
	= 0;

	/**
	 * \brief Implements FileReaderDescriptor::operator ==.
	 */
	virtual bool do_operator_equals(const FileReaderDescriptor &rhs) const;
};


/**
 * \brief A test whether a given FileReaderDescriptor matches certain criteria
 */
class FileTest
{

public:

	/**
	 * \brief Constructor.
	 */
	FileTest();

	/**
	 * \brief Constructor.
	 *
	 * \param[in] filename The filename to test
	 */
	explicit FileTest(const std::string &filename);

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~FileTest() noexcept;

	/**
	 * \brief Set the filename to test.
	 *
	 * \param[in] filename The filename to test
	 */
	void set_filename(const std::string &filename);

	/**
	 * \brief Returns the filename tested by this test.
	 *
	 * \return Filename to test
	 */
	const std::string& filename() const;

	/**
	 * \brief Test a given descriptor instance for matching the criteria of this
	 * test.
	 *
	 * \param[in] desc The FileReaderDescriptor to test
	 *
	 * \return TRUE iff the descriptor matches the criteria of this test
	 */
	bool matches(const FileReaderDescriptor &desc) const;


private:

	/**
	 * \brief Implements FileTest::matches().
	 *
	 * \param[in] desc The FileReaderDescriptor to test
	 *
	 * \return TRUE iff the descriptor matches the criteria of this selector
	 */
	virtual bool do_matches(const FileReaderDescriptor &desc) const
	= 0;

	/**
	 * \brief Filename tested by this test.
	 */
	std::string filename_;
};


/**
 * \brief A byte sequence from a file along with its offset and length.
 *
 * FileTestBytes just represent a part of a file. This part can be tested
 * for compliance to a certain FileReaderDescriptor.
 */
class FileTestBytes final : public FileTest
{

public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] offset The offset in bytes where this sequence starts
	 * \param[in] length Number of bytes in the sequence
	 */
	FileTestBytes(const uint64_t &offset, const uint32_t &length);


private:

	bool do_matches(const FileReaderDescriptor &desc) const override;

	/**
	 * \brief Read length bytes from position offset in file filename.
	 *
	 * \param[in] filename Filename to test
	 * \param[in] offset   The offset in bytes where this sequence starts
	 * \param[in] length   Number of bytes in the sequence
	 *
	 * \return The actual byte sequence
	 *
	 * \throw FileReadException If the specified number of bytes could not be
	 * read from the specified file and position
	 */
	std::vector<char> read_bytes(const std::string &filename,
		const uint64_t &offset, const uint32_t &length) const;

	/**
	 * \brief Byte offset of the byte sequence in the file.
	 */
	uint64_t offset_;

	/**
	 * \brief Number of bytes to read from the start position.
	 */
	uint16_t length_;
};


/**
 * \brief Test file for compliance with a certain filename suffix
 */
class FileTestSuffix final : public FileTest
{

private:

	bool do_matches(const FileReaderDescriptor &desc) const override;

	/**
	 * \brief Provides the suffix of a given filename.
	 *
	 * The suffix is the part of filename following the last occurrence of ".".
	 * If filename does not contain the character ".", the entire filename is
	 * returned.
	 *
	 * \param[in] filename The filename to check
	 *
	 * \return The relevant suffix or the entire filename
	 */
	std::string get_suffix(const std::string &filename) const;
};


/**
 * \brief A selection mechanism for a FileReaderSelection.
 *
 * A FileReaderSelector applies some FileTests to decide whether a given
 * FileReaderDescriptor matches or not.
 *
 * The default FileReaderSelector selects just the first descriptor in the
 * descriptor list passed that passes all tests. Subclassing FileReaderSelector
 * can implement different selection policies.
 */
class FileReaderSelector {

public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~FileReaderSelector() noexcept;

	/**
	 * \brief Selects the descriptor with the lowest index position from
	 * descriptors that passes all tests.
	 *
	 * \param[in] tests Set of tests to perform
	 * \param[in] descs Set of descriptors to select from
	 *
	 * \return A FileReaderDescriptor
	 */
	std::unique_ptr<FileReaderDescriptor> select(
			const std::set<std::unique_ptr<FileTest>> &tests,
			const std::list<std::unique_ptr<FileReaderDescriptor>> &descs)
		const;


private:

	/**
	 * \brief Implements FileReaderSelector::select().
	 *
	 * \param[in] tests Set of tests to perform
	 * \param[in] descs List of descriptors to select from
	 *
	 * \return A FileReaderDescriptor
	 */
	virtual std::unique_ptr<FileReaderDescriptor> do_select(
			const std::set<std::unique_ptr<FileTest>> &tests,
			const std::list<std::unique_ptr<FileReaderDescriptor>> &descs)
		const;

	/**
	 * \brief Test whether a descriptor matches the criteria of this selector.
	 *
	 * \param[in] tests Set of tests to perform
	 * \param[in] desc  The descriptor to check
	 *
	 * \return TRUE iff the descriptor matches the criteria of this selector
	 */
	virtual bool matches(
		const std::set<std::unique_ptr<FileTest>> &tests,
		const std::unique_ptr<FileReaderDescriptor> &desc) const;
};


/**
 * \brief Abstract builder class for creating readers for given files.
 */
class FileReaderSelection
{

public:

	/**
	 * \brief Constructor.
	 */
	FileReaderSelection();

	// class is non-copy-constructible
	FileReaderSelection(const FileReaderSelection &) = delete;

	// TODO Move constructor

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~FileReaderSelection() noexcept;

	/**
	 * \brief Add a descriptor to the list of descriptors for which a FileReader
	 * can be created.
	 *
	 * \param[in] desc The FileReaderDescriptor to support
	 */
	void add_descriptor(std::unique_ptr<FileReaderDescriptor> desc);

	/**
	 * \brief Remove all descriptors that qualify as equivalent to the given
	 * descriptor by '==' from the list of descriptors.
	 *
	 * \param[in] desc The FileReaderDescriptor to be removed
	 *
	 * \return Number of descriptor instances removed.
	 */
	int remove_descriptor(const FileReaderDescriptor * desc);

	/**
	 * \brief Register a test for a FileReaderDescriptor for the specified
	 * filename.
	 *
	 * \param[in] testobj The test to be registered
	 */
	void register_test(std::unique_ptr<FileTest> testobj);

	/**
	 * \brief Remove all tests that qualify as equivalent to the given test by
	 * '==' from the list of test.
	 *
	 * \param[in] testobj The FileTest to be removed
	 *
	 * \return Number of test instances removed.
	 */
	int unregister_test(const FileTest * testobj);

	/**
	 * \brief Removes all tests registered to this instance.
	 */
	void remove_all_tests();

	/**
	 * \brief Set the FileReaderSelector for this instance.
	 *
	 * \param[in] selector The FileReaderSelector for this instance
	 */
	void set_selector(std::unique_ptr<FileReaderSelector> selector);

	/**
	 * \brief Return the FileReaderSelector of this instance.
	 *
	 * \return The FileReaderSelector of this instance
	 */
	const FileReaderSelector& selector() const;

	/**
	 * \brief Determine a matching FileReaderDescriptor for the specified file.
	 *
	 * \param[in] filename Name of the file to determine a descriptor for
	 *
	 * \return A FileReaderDescriptor for the specified file
	 */
	std::unique_ptr<FileReaderDescriptor> descriptor(
			const std::string &filename) const;

	/**
	 * \brief Create an opaque FileReader for the given file.
	 *
	 * Will return nullptr if the file does not exist or cannot be read or the
	 * filename is empty.
	 *
	 * \param[in] filename Name of the file to create the reader for
	 *
	 * \return A FileReader for the specified file
	 */
	std::unique_ptr<FileReader> for_file(const std::string &filename) const;

	/**
	 * \brief Return the FileReader specified by its name.
	 *
	 * If the selection does not contain a FileReader with the specified name,
	 * \c nullptr will be returned.
	 *
	 * \param[in] name The name of the FileReader.
	 *
	 * \return A FileReader with the specified name
	 */
	std::unique_ptr<FileReader> by_name(const std::string &name) const;

	/**
	 * \brief Traverse all available descriptors and apply the specified
	 * function \c func on each of them.
	 *
	 * \param[in] func Function to apply to each descriptor.
	 */
	void traverse_descriptors(
			std::function<void(const FileReaderDescriptor &)> func) const;

	/**
	 * \brief Reset this instance to its initial state, removing all tests and
	 * descriptors.
	 */
	void reset();

	// class is non-copy-assignable
	FileReaderSelection& operator = (const FileReaderSelection &) = delete;

	// TODO Move assignment


private:

	// forward declaration
	class Impl;

	/**
	 * \brief Private implementation of this FileReaderSelection.
	 */
	std::unique_ptr<FileReaderSelection::Impl> impl_;
};

/// @}

} // namespace v_1_0_0

} // namespace arcsdec

#endif

