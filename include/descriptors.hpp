#ifndef __LIBARCSDEC_DESCRIPTORS_HPP__
#define __LIBARCSDEC_DESCRIPTORS_HPP__

/**
 * \file
 *
 * \brief Toolkit for selecting file readers
 */

#include <cctype>
#include <cstdint>
#include <functional>  // for std::function
#include <limits>
#include <list>
#include <memory>
#include <regex>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp> // for SampleInputIterator
#endif


namespace arcsdec
{

inline namespace v_1_0_0
{

namespace details
{

/**
 * \brief Traits for case insensitive string comparison.
 *
 * Thanks to Herb Sutter: http://www.gotw.ca/gotw/029.htm
 */
struct ci_char_traits : public std::char_traits<char>
{
	static bool eq(char c1, char c2) { return toupper(c1) == toupper(c2); }

	static bool ne(char c1, char c2) { return toupper(c1) != toupper(c2); }

	static bool lt(char c1, char c2) { return toupper(c1)  < toupper(c2); }

	static int compare(const char* s1, const char* s2, size_t n)
	{
        while(n-- != 0)
		{
			if( toupper(*s1) < toupper(*s2) ) { return -1; }
			if( toupper(*s1) > toupper(*s2) ) { return  1; }

			++s1;
			++s2;
		}

		return 0;
	}

	static const char* find(const char* s, int n, char a)
	{
		while(n-- > 0 && toupper(*s) != toupper(a)) { ++s; }

		return s;
	}
};


/**
 * \brief Case insensitive comparable string.
 */
using ci_string = std::basic_string<char, ci_char_traits>;


/**
 * \brief Load runtime dependencies of \c object_name.
 *
 * If \c object_name is empty, the runtime dependencies of the main executable
 * are loaded.
 *
 * \param[in] object_name Name of the object to get dependencies for.
 */
std::vector<std::string> list_libs(const std::string &object_name);


/**
 * \brief Escape a character in a string with another string.
 *
 * \param[in,out] input Input string to modify
 * \param[in]     c     Character to escape
 * \param[in]     seq   Escape string
 */
void escape(std::string &input, const char c, const std::string &seq);


/**
 * \brief Construct search pattern from library name.
 *
 * \param[in] libname The library name to turn into a pattern
 *
 * \return A regex matching concrete sonames for this library
 */
std::regex libname_pattern(const std::string &libname);


/**
 * \brief Find a lib in the list of libarcsdec runtime dependencies.
 *
 * \param[in] list List of library so filepaths
 * \param[in] name Name of the lib (e.g. libFLAC++, libavformat etc.)
 *
 * \return Filepath for the lib or empty string.
 */
const std::string& find_lib(const std::vector<std::string> &list,
		const std::string &name);


/**
 * \brief List runtime dependencies of libarcsdec.
 *
 * \return List of runtime dependencies of libarcsdec
 */
std::vector<std::string> acquire_libarcsdec_libs();


/**
 * \brief Global list of libarcsdec runtime dependency libraries.
 */
const std::vector<std::string>& libarcsdec_libs();

} // namespace details


/**
 * \defgroup descriptors API for creating a FileReader for a specified file
 *
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
 *
 * @{
 */


/**
 * \brief List of supported file formats.
 *
 * These are only the tested containers, in fact other lossless codecs are
 * supported if an appropriate FileReader exists.
 */
enum class Format : unsigned
{
	UNKNOWN,  // 0
	CUE,
	CDRDAO,
	// ... add more metadata formats here
	WAVE, // Audio formats from here on (is_audio_format relies on that)
	FLAC,
	APE,
	CAF,
	M4A,
	OGG,
	WV,
	AIFF,
	WMA
	// ... add more audio formats here
};
// TODO Raw


/**
 * \brief Return the name of the format.
 *
 * \param[in] format The format to get the name for
 *
 * \return Name of the format.
 */
std::string name(Format format);


/**
 * \brief Returns TRUE if \c format is an audio format, otherwise FALSE.
 *
 * \param[in] format The format to test
 *
 * \return TRUE iff \c format is an audio format, otherwise FALSE.
 */
bool is_audio_format(Format format);


/**
 * \brief List of supported audio codecs.
 *
 * These are only the tested codecs, in fact other lossless codecs are supported
 * if an appropriate FileReader exists.
 */
enum class Codec : unsigned
{
	UNKNOWN,  // 0
	PCM_S16BE,
	PCM_S16BE_PLANAR,
	PCM_S16LE,
	PCM_S16LE_PLANAR,
	PCM_S32BE,
	PCM_S32BE_PLANAR,
	PCM_S32LE,
	PCM_S32LE_PLANAR,
	FLAC,
	WAVEPACK,
	MONKEY,
	ALAC,
	WMALOSSLESS
};
// TODO Raw


/**
 * \brief Return the name of the codec.
 *
 * \param[in] codec The codec to get the name for
 *
 * \return Name of the codec.
 */
std::string name(Codec codec);


// forward declaration for FileReader
class FileReaderDescriptor;

/**
 * \brief Abstract base class for all FileReaders, acts as tag.
 *
 * This class does not define any interface but ensures a common base type for
 * all readers. Therefore, all readers can be built and provided by the same
 * creation framework.
 */
class FileReader
{
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
 * \brief Represents a list of pairs of a library name and an additional string.
 */
using LibInfo = std::vector<std::pair<std::string, std::string>>;


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
private:

	/**
	 * \brief List of case-insensitive accepted suffices.
	 */
	std::vector<details::ci_string> suffices_;

public:

	/**
	 * \brief Default Constructor.
	 */
	FileReaderDescriptor()
		: suffices_ { } { /* empty */ }

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
	 * \brief Check whether this descriptor accepts the specified filename.
	 *
	 * \param[in] filename The filename to test for
	 *
	 * \return TRUE iff the descriptor accepts the filename
	 */
	bool accepts_name(const std::string &filename) const;

	bool accepts(Codec codec) const;

	std::set<Codec> codecs() const;

	/**
	 * \brief Check for acceptance of the specified format
	 *
	 * \param[in] format The format to check for
	 *
	 * \return TRUE if \c format is accepted, otherwise FALSE
	 */
	bool accepts(Format format) const;

	/**
	 * \brief \link Format Formats\endlink accepted by the FileReader.
	 *
	 * \return \link Format Formats\endlink accepted by the FileReader
	 */
	std::set<Format> formats() const;

	/**
	 * \brief Name of the underlying library.
	 *
	 * \return Name of the underlying library
	 */
	LibInfo libraries() const;

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

protected:

	/**
	 * \brief Constructor for accepting suffices.
	 */
	FileReaderDescriptor(const decltype( suffices_ ) suffices)
		: suffices_ { suffices } { /* empty */ }

	/**
	 * \brief Worker: Provides the suffix of a given filename.
	 *
	 * The suffix is the part of filename following the last occurrence of
	 * \c delimiter. If filename does not contain the delimiter, the entire
	 * filename is returned as suffix.
	 *
	 * \param[in] filename  The filename to check
	 * \param[in] delimiter The delimiter to separate the suffix from the base
	 *
	 * \return The relevant suffix or the entire filename
	 */
	std::string get_suffix(const std::string &filename,
			const std::string &delimiter) const;
	// TODO Use std::filesystem of C++17

	/**
	 * \brief Worker: Returns TRUE if filename has specified suffix, otherwise
	 * FALSE.
	 *
	 * \param[in] name   Filename to test
	 * \param[in] suffix Suffix to test \c name for
	 *
	 * \return TRUE if file has specified suffix, otherwise FALSE
	 */
	bool has_suffix(const std::string &name,
			const details::ci_string &suffix) const;

private:

	/**
	 * \brief Implements FileReaderDescriptor::name().
	 *
	 * \return A human-readable name of this FileReaderDescriptor
	 */
	virtual std::string do_name() const
	= 0;

	/**
	 * \brief Implements FileReaderDescriptor::libraries().
	 *
	 * \return Name of the underlying libraries along with their versions
	 */
	virtual LibInfo do_libraries() const
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
	 * \brief Implements FileReaderDescriptor::accepts_name().
	 *
	 * The default implementation tries to match the suffix of the filename
	 * against the predefined suffices of this descriptor type.
	 *
	 * \param[in] name The filename to test
	 *
	 * \return TRUE iff the filename is accepted by this descriptor
	 */
	virtual bool do_accepts_name(const std::string &suffix) const;

	/**
	 * \brief Implements FileReaderDescriptor::accepts().
	 *
	 * \param[in] format The format to check for
	 *
	 * \return TRUE if \c format is accepted, otherwise FALSE
	 */
	virtual bool do_accepts(Codec codec) const
	= 0;

	/**
	 * \brief \link Codec Codecs\endlink accepted by the FileReader.
	 *
	 * \return \link Codec Codecs\endlink accepted by the FileReader
	 */
	virtual std::set<Codec> do_codecs() const
	= 0;

	/**
	 * \brief Implements FileReaderDescriptor::accepts().
	 *
	 * \param[in] format The format to check for
	 *
	 * \return TRUE if \c format is accepted, otherwise FALSE
	 */
	virtual bool do_accepts(Format format) const
	= 0;

	/**
	 * \brief \link Format Formats\endlink accepted by the FileReader.
	 *
	 * \return \link Format Formats\endlink accepted by the FileReader
	 */
	virtual std::set<Format> do_formats() const
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
		const uint32_t &offset, const uint32_t &length) const;

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
 * \brief Test an actual filename for compliance.
 */
class FileTestName final : public FileTest
{
private:

	bool do_matches(const FileReaderDescriptor &desc) const override;
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
class FileReaderSelector
{
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

	/**
	 * \brief Number of descriptors.
	 *
	 * \return The number of descriptors in this selection.
	 */
	std::size_t size() const;

	/**
	 * \brief TRUE if this selection contains no descriptors.
	 *
	 * \return TRUE if this selection contains no descriptors.
	 */
	bool empty() const;

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

