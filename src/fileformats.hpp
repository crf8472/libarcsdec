/**
 * \file fileformats.hpp Toolkit for selecting file readers by file format
 *
 * This file provides the interface of the \ref fileformats API.
 *
 */

#ifndef __LIBARCSDEC_FILEFORMATS_HPP__
#define __LIBARCSDEC_FILEFORMATS_HPP__

#include <cstdint>
#include <list>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>


namespace arcs
{
/// \defgroup fileformats Level 0 API: Create a FileReader for a Given File

/**
 * \brief Framework for creating specialized FileReaders for a specified file.
 *
 * Abstract class FileReaderCreator implements the generic mechanism to check a
 * specified input file for a matching FileFormat. If a matching FileFormat is
 * found, an instance of this format is returned which is then used to create
 * the concrete FileReader instance.
 *
 * A FileReaderCreator holds a a list of tests to perform on the input file and
 * a list of supported FileFormats. Internally, it uses an instance of
 * FileFormatSelector to select a concrete FileFormat. FileFormatSelector
 * performs the selection obeying a certain selection policy. The default
 * FileFormatSelector just selects the first FileFormat in the list of supported
 * FileFormats that passes all tests in the provided list of tests.
 *
 * A FileFormatTest implements a single test. It may or may not open the file to
 * test.
 *
 * The \ref audioreader and \ref metaparser are built on this API.
 */

/// @{

/**
 * Abstract base class for all file readers, audio readers as well as metadata
 * readers.
 *
 * This class does not define any interface but ensures a common base type for
 * all readers. Therefore, all readers can be built and provided by the same
 * creation framework.
 */
class FileReader
{

public:

	/**
	 * Default constructor
	 */
	FileReader();

	/**
	 * Virtual default destructor
	 */
	virtual ~FileReader() noexcept;
};


/**
 * Reports an error while reading a file.
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
	 * Constructor
	 *
	 * \param[in] what_arg What argument
	 */
	explicit FileReadException(const std::string &what_arg);

	/**
	 * Constructor
	 *
	 * The byte position marks the error. This implies that byte_pos - 1 bytes
	 * have been read without error.
	 *
	 * \param[in] what_arg What argument
	 * \param[in] byte_pos Byte position of the error
	 */
	FileReadException(const std::string &what_arg, const int64_t &byte_pos);

	/**
	 * Byte position on which the error occurred or a negative value if the
	 * position is not known.
	 *
	 * This entails that byte_pos - 1 bytes have been read without error.
	 *
	 * \return Byte position on which the error occurred.
	 */
	int64_t byte_pos() const;


private:

	/**
	 * Internal byte position.
	 */
	int64_t byte_pos_;
};


/**
 * Abstract base class to represent a file format.
 *
 * A FileFormat provides information that can be used to decide whether a
 * a given file matches this format. It can create an opaque reader that can
 * read files matching this format.
 *
 * A FileFormat is just an abstract test whether a specific reader should be
 * created or not. It may or may not be a specific combination of a codec and a
 * file container format. For instance, "FLAC" can be a FileFormat, representing
 * FLAC data in a FLAC container. Another way to implement a FileFormat would be
 * "ffmpeg-readable audio file". Therefore, a FileFormat does not to distinguish
 * between container format and codec, but just should provide the heuristics
 * for deciding whether the corresponding FileReader can read the file in
 * question.
 */
class FileFormat
{

public:

	/**
	 * Virtual default destructor
	 */
	virtual ~FileFormat() noexcept;

	/**
	 * Name of this FileFormat
	 *
	 * \return A human-readable name of this FileFormat
	 */
	std::string name() const;

	/**
	 * Check whether this format matches the given input bytes.
	 *
	 * \param[in] bytes  Sequence of consecutive bytes in the file
	 * \param[in] offset Offset of the first byte of this sequence
	 *
	 * \return TRUE iff the byte sequence matches the format, otherwise FALSE
	 */
	bool can_have_bytes(const std::vector<char> &bytes,
			const uint64_t &offset) const;

	/**
	 * Check whether this format is known to have the specified suffix.
	 *
	 * \param[in] suffix The suffix of the filename to test for
	 *
	 * \return TRUE iff the suffix matches the suffix of the format
	 */
	bool can_have_suffix(const std::string &suffix) const;

	/**
	 * Create an opaque reader for the specified file
	 *
	 * \return A FileReader that can read this FileFormat
	 */
	std::unique_ptr<FileReader> create_reader() const;

	/**
	 * Clone this instance.
	 *
	 * Method clone() allows to duplicate an instance without knowing its
	 * static type.
	 *
	 * \return A deep copy of the instance
	 */
	std::unique_ptr<FileFormat> clone() const;

	/**
	 * Equality
	 *
	 * \param[in] rhs The right hand side of the comparison
	 *
	 * \return TRUE iff the right hand side is equal to the left hand side,
	 * otherwise false
	 */
	bool operator == (const FileFormat &rhs) const;

	/**
	 * Inequality
	 *
	 * \param[in] rhs The right hand side of the comparison
	 *
	 * \return TRUE iff the right hand side is not equal to the left hand side,
	 * otherwise false
	 */
	bool operator != (const FileFormat &rhs) const;


private:

	/**
	 * Implements FileFormat::name()
	 *
	 * \return A human-readable name of this FileFormat
	 */
	virtual std::string do_name() const
	= 0;

	/**
	 * Implements FileFormat::can_have_bytes()
	 *
	 * \param[in] bytes  Sequence of consecutive bytes in the file
	 * \param[in] offset Offset of the first byte of this sequence
	 *
	 * \return TRUE iff the byte sequence matches the format, otherwise FALSE
	 */
	virtual bool do_can_have_bytes(const std::vector<char> &bytes,
			const uint64_t &offset) const
	= 0;

	/**
	 * Implements FileFormat::can_have_suffix()
	 *
	 * \param[in] suffix The suffix of the filename to test for
	 *
	 * \return TRUE iff the suffix matches the suffix of the format
	 */
	virtual bool do_can_have_suffix(const std::string &suffix) const
	= 0;

	/**
	 * Implements FileFormat::create_reader()
	 *
	 * \return A FileReader that can read this FileFormat
	 */
	virtual std::unique_ptr<FileReader> do_create_reader() const
	= 0;

	/**
	 * Implements FileFormat::clone()
	 *
	 * \return A deep copy of the instance
	 */
	virtual std::unique_ptr<FileFormat> do_clone() const
	= 0;
};


/**
 * Provides a test whether a given FileFormat matches certain test criteria
 */
class FileFormatTest
{

public:

	/**
	 * Virtual default destructor
	 */
	virtual ~FileFormatTest() noexcept;

	/**
	 * Sets the name or name with path of the file to test
	 *
	 * \param[in] filepath Sets the filepath of the file to test
	 */
	void set_filename(const std::string &filepath);

	/**
	 * Returns the name of the file to test
	 *
	 * \return Name of the file to test
	 */
	std::string filename() const;

	/**
	 * Test a given format instance for matching the criteria of this test
	 *
	 * \param[in] format The FileFormat to test
	 *
	 * \return TRUE iff the format matches the criteria of this test
	 */
	bool matches(const FileFormat &format) const;


private:

	/**
	 * Implements FileFormatTest::set_filename()
	 *
	 * \param[in] filepath Sets the filepath of the file to test
	 */
	virtual void do_set_filename(const std::string &filepath)
	= 0;

	/**
	 * Implements FileFormatTest::filename()
	 *
	 * \return Name of the file to test
	 */
	virtual std::string do_filename() const
	= 0;

	/**
	 * Implements FileFormatTest::matches()
	 *
	 * \param[in] format The FileFormat to test
	 *
	 * \return TRUE iff the format matches the criteria of this selector
	 */
	virtual bool do_matches(const FileFormat &format) const
	= 0;
};


/**
 * Represents a byte sequence from a file along with its offset and length.
 *
 * FileFormatTestBytes just represent a part of a file. This part can be tested
 * for compliance to a certain FileFormat.
 */
class FileFormatTestBytes final : public FileFormatTest
{

public:

	/**
	 * Constructor
	 *
	 * \param[in] offset The offset in bytes where this sequence starts
	 * \param[in] length Number of bytes in the sequence
	 */
	FileFormatTestBytes(const uint64_t &offset, const uint32_t &length);


private:

	void do_set_filename(const std::string &filepath) override;

	std::string do_filename() const override;

	bool do_matches(const FileFormat &format) const override;

	/**
	 * Read length bytes from position offset in file filename.
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
	 * Byte offset of the byte sequence in the file
	 */
	uint64_t offset_;

	/**
	 * Number of bytes to read from the start position
	 */
	uint16_t length_;

	/**
	 * Name of the file to test
	 */
	std::string filename_;
};


/**
 * Test file for compliance with a certain filename suffix
 */
class FileFormatTestSuffix final : public FileFormatTest
{

public:

	/**
	 * Empty constructor
	 */
	FileFormatTestSuffix();

	/**
	 * Empty constructor
	 *
	 * \param[in] filename The filename to test
	 */
	FileFormatTestSuffix(const std::string &filename);


private:

	void do_set_filename(const std::string &filepath) override;

	std::string do_filename() const override;

	bool do_matches(const FileFormat &format) const override;

	/**
	 * Provides the suffix of a given filename.
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

	/**
	 * Name of the file to test
	 */
	std::string filename_;
};


/**
 * Represents a selection mechanism for a FileFormatCreator.
 *
 * A FileFormatSelector applies some FileFormatTests to decide whether a given
 * FileFormat matches or not.
 *
 * The default FileFormatSelector selects just the first format in the format
 * list passed that passes all tests. Subclassing FileFormatSelector can
 * implement different selection policies (like e.g. "best").
 */
class FileFormatSelector
{

public:

	/**
	 * Virtual default destructor
	 */
	virtual ~FileFormatSelector() noexcept;

	/**
	 * Selects the format with the lowest index position from formats that
	 * passes all tests.
	 *
	 * \param[in] tests   Set of tests to perform
	 * \param[in] formats Set of formats to select from
	 *
	 * \return A FileFormat
	 */
	std::unique_ptr<FileFormat> select(
			const std::set<std::unique_ptr<FileFormatTest>> &tests,
			const std::list<std::unique_ptr<FileFormat>> &formats) const;


private:

	/**
	 * Implements FileFormatSelector::select()
	 *
	 * \param[in] tests   Set of tests to perform
	 * \param[in] formats List of formats to select from
	 *
	 * \return A FileFormat
	 */
	virtual std::unique_ptr<FileFormat> do_select(
			const std::set<std::unique_ptr<FileFormatTest>> &tests,
			const std::list<std::unique_ptr<FileFormat>> &formats) const;

	/**
	 * Test whether a format matches the criteria of this selector
	 *
	 * \param[in] tests  Set of tests to perform
	 * \param[in] format The format to check
	 *
	 * \return TRUE iff the format matches the criteria of this selector
	 */
	virtual bool matches(
		const std::set<std::unique_ptr<FileFormatTest>> &tests,
		const std::unique_ptr<FileFormat> &format) const;
};


/**
 * Abstract builder class for creating readers for given files.
 */
class FileReaderCreator
{

public:

	/**
	 * Constructor
	 */
	FileReaderCreator();

	// class is non-copy-constructible
	FileReaderCreator(const FileReaderCreator &) = delete; // TODO why?

	/**
	 * Virtual default destructor
	 */
	virtual ~FileReaderCreator() noexcept;

	/**
	 * Add a format to the list of formats for which a FileReader can be created
	 *
	 * \param[in] format The FileFormat to support
	 */
	void register_format(std::unique_ptr<FileFormat> format);

	/**
	 * Remove all formats that qualify as equivalent to the given format by
	 * '==' from the list of formats.
	 *
	 * \param[in] format The FileFormat to be removed
	 *
	 * \return Number of format instances removed.
	 */
	int remove_format(FileFormat const * format); // TODO const ptr?

	/**
	 * Register a test for a FileFormat for the specified filename.
	 *
	 * \param[in] format_test The test to be registered
	 */
	void register_test(std::unique_ptr<FileFormatTest> format_test);

	/**
	 * Remove all tests that qualify as equivalent to the given test by
	 * '==' from the list of test.
	 *
	 * \param[in] format_test The FileFormatTest to be removed
	 *
	 * \return Number of test instances removed.
	 */
	int remove_test(FileFormatTest const * format_test); // TODO const ptr?

	/**
	 * Set the FileFormatSelector for this instance
	 *
	 * \param[in] selector The FileFormatSelector for this instance
	 */
	void set_selector(std::unique_ptr<FileFormatSelector> selector);

	/**
	 * Return the FileFormatSelector of this instance
	 *
	 * \return The FileFormatSelector of this instance
	 */
	const FileFormatSelector& selector() const;

	/**
	 * Determine a matching FileFormat for the specified file.
	 *
	 * \param[in] filename Name of the file to determine a FileFormat for
	 *
	 * \return A FileFormat for the specified file
	 */
	std::unique_ptr<FileFormat> get_format(const std::string &filename);

	/**
	 * Create an opaque FileReader for the given file.
	 *
	 * Will return nullptr if the file does not exist or cannot be read or the
	 * filename is empty.
	 *
	 * \param[in] filename Name of the file to create the reader for
	 *
	 * \return A FileReader for the specified file
	 */
	std::unique_ptr<FileReader> create_reader(const std::string &filename);

	/**
	 * Reset this instance to its initial state, removing all tests and
	 * formats.
	 */
	void reset();

	// class is non-copy-assignable
	FileReaderCreator& operator = (const FileReaderCreator &) = delete;
	// TODO why?


private:

	// forward declaration
	class Impl;

	/**
	 * Private implementation of this FileReaderFactory
	 */
	std::unique_ptr<FileReaderCreator::Impl> impl_;
};

/// @}

} // namespace arcs

#endif

