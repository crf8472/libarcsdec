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
#include <memory>
#include <regex>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp> // for SampleInputIterator
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif


namespace arcsdec
{
inline namespace v_1_0_0
{

/**
 * \internal
 * \brief Implementation details.
 */
namespace details
{

/* -NO_DOXYGEN-
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


/* -NO_DOXYGEN-
 * \brief Case insensitive comparable string.
 */
using ci_string = std::basic_string<char, ci_char_traits>;


/**
 * \internal
 * \defgroup libinfoImpl API for implementing info functions about descriptors
 *
 * \ingroup descriptors
 *
 * \brief API for implementing info functions about descriptors.
 *
 * \warning
 * This API is currently *nix-only. It uses only dlopen and operates only on
 * sonames.
 *
 * @{
 */

/**
 * \brief Escape a character in a string with another string.
 *
 * \param[in,out] input Input string to modify
 * \param[in]     c     Character to escape
 * \param[in]     seq   Escape string
 */
void escape(std::string &input, const char c, const std::string &seq);


/**
 * \brief Service: construct soname search pattern from library name.
 *
 * The library name should be the first part of the soname without any
 * suffices, e.g. 'libfoo', 'libFLAC++' but not 'libwavpack.so.4' or 'quux.dll'.
 *
 * \warning
 * This function is *nix-specific. It constructs a search pattern for shared
 * objects.
 *
 * \param[in] libname The library name to turn into a pattern
 *
 * \return A regex matching concrete sonames for this library
 */
std::regex libname_pattern(const std::string &libname);


/**
 * \brief Service: load runtime dependencies of an object.
 *
 * If \c object_name is empty, the runtime dependencies of the main executable
 * are loaded.
 *
 * \warning
 * This function is *nix-specific. It inspects binaries with dlopen.
 *
 * \param[in] object_name Name of the object to get dependencies for.
 *
 * \return List of runtime dependencies of an object
 */
std::vector<std::string> runtime_deps(const std::string &object_name);


/**
 * \brief Find shared object in the list of libarcsdec runtime dependencies.
 *
 * List \c list is a list of sonames, it can be created by using runtime_deps.
 * The \c name is the same format as the input for libname_pattern.
 *
 * \param[in] list List of library so filepaths
 * \param[in] name Name of the lib (e.g. libFLAC++, libavformat etc.)
 *
 * \return Filepath for the object or empty string.
 */
const std::string& find_lib(const std::vector<std::string> &list,
		const std::string &name);


/**
 * \brief Comprehensive list of libarcsdec runtime dependency libraries.
 *
 * \return Comprehensive list of libarcsdec runtime dependency libraries.
 */
const std::vector<std::string>& libarcsdec_libs();

/// @}

} // namespace details


/**
 * \defgroup descriptors API for selecting FileReaders
 *
 * \brief API for selecting \link FileReader FileReaders\endlink for given
 * input files.
 *
 * Abstract class FileReaderSelection provides the API for the mechanism to
 * check a specified input file for a matching FileReaderDescriptor. If a
 * matching FileReaderDescriptor is found, an instance of this descriptor is
 * returned which is then used to create the concrete FileReader instance.
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
 * \brief List of supported file formats for metadata and audio.
 *
 * These are only the tested formats, in fact other file formats are
 * supported if an appropriate FileReader exists.
 *
 * The intention is to support inspecting the capabilities of
 * \link FileReader FileReaders\endlink.
 */
enum class Format : unsigned
{
	UNKNOWN,  // 0
	CUE,
	CDRDAO,
	// ... add more metadata formats here
	WAV, // Audio formats from here on (is_audio_format relies on that)
	FLAC,
	APE,
	CAF,
	M4A,
	OGG,
	WV,
	AIFF
	// ... add more audio formats here
};


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
 *
 * The intention is to support inspecting the capabilities of
 * \link FileReader FileReaders\endlink.
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
	WAVPACK,
	MONKEY,
	ALAC
};


/**
 * \brief Return the name of the codec.
 *
 * \param[in] codec The codec to get the name for
 *
 * \return Name of the codec.
 */
std::string name(Codec codec);


/**
 * \internal
 * \brief Adds inequality operator to classes defining equality operator==.
 */
template <typename T>
struct Comparable
{
	virtual ~Comparable() = default;

	/**
	 * \brief Inequality.
	 *
	 * \param[in] lhs Left hand side of the comparison
	 * \param[in] rhs Right hand side of the comparison
	 *
	 * \return TRUE iff not \c lhs == \c rhs, otherwise FALSE
	 */
	friend bool operator != (const T &lhs, const T &rhs) noexcept
	{
		return !(lhs == rhs);
	}
};


class FileReaderDescriptor;


/**
 * \brief Abstract base class for \link FileReader FileReaders\endlink.
 *
 * \note
 * Instances of this class are non-copyable and non-movable but provide
 * protected special members for move that can be used in subclasses.
 *
 * \see AudioReader
 * \see MetadataParser
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
	 *
	 * \return Descriptor for this FileReader instance
	 */
	std::unique_ptr<FileReaderDescriptor> descriptor() const;

protected:

	FileReader(FileReader &&rhs) noexcept = default;
	FileReader& operator = (FileReader &&rhs) noexcept = default;

private:

	/**
	 * \brief Implements FileReader::descriptor().
	 */
	virtual std::unique_ptr<FileReaderDescriptor> do_descriptor() const
	= 0;
};


namespace details
{

/**
 * \brief Downcast a FileReader to a specialized ReaderType.
 *
 * The operation is safe: if the cast fails, the input pointer is returned
 * unaltered as second element of the pair, together with a nullptr as casting
 * result. If the cast succeeds, the casted pointer is returned together with
 * a nullptr as second element.
 */
template<typename ReaderType>
auto cast_reader(std::unique_ptr<FileReader> file_reader) noexcept
	-> std::pair<std::unique_ptr<ReaderType>, std::unique_ptr<FileReader>>
{
	if (!file_reader)
	{
		return std::make_pair(nullptr, std::move(file_reader));
	}

	// Create ReaderType manually by downcasting and reassignment

	FileReader *file_reader_rptr = file_reader.get();
	ReaderType *reader_type_rptr = nullptr;

	// Dry run:
	// Casting succeeds iff the FileReader created is in fact a ReaderType.
	// If not, the file is not supported by file_reader, so bail out.
	try
	{
		reader_type_rptr = dynamic_cast<ReaderType*>(file_reader_rptr);

	} catch (...) // std::bad_cast is possible
	{
		ARCS_LOG_WARNING <<
				"Failed to safely cast FileReader pointer to ReaderType";

		return std::make_pair(nullptr, std::move(file_reader));
	}

	if (!reader_type_rptr)
	{
		ARCS_LOG_WARNING <<
				"Casting FileReader pointer to ReaderType resulted in nullptr";

		return std::make_pair(nullptr, std::move(file_reader));
	}

	auto readertype_uptr = std::make_unique<ReaderType>(nullptr);

	// file_reader was left unmodified until now

	readertype_uptr.reset(dynamic_cast<ReaderType*>(file_reader.release()));
	// release() + reset() are both 'noexcept'

	return std::make_pair(std::move(readertype_uptr), nullptr);
}


/**
 * \brief Worker: Read \c length bytes from file \c filename starting at
 * position \c offset.
 *
 * \param[in] filename Name of the file to read from
 * \param[in] offset   0-based byte offset to start
 * \param[in] length   Number of bytes to read
 *
 * \return Byte sequence read from file
 *
 * \throw FileReadException If the specified number of bytes could not be
 * read from the specified file and position
 *
 * \throw InputFormatException On unspecified error
 */
std::vector<unsigned char> read_bytes(const std::string &filename,
	const uint32_t &offset, const uint32_t &length);


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
		const std::string &delimiter);

} // namespace details


/**
 * \brief Reports an error concerning the input file format.
 *
 * This exception can be thrown when the input format could not be determined
 * or nor FileReader could be acquired.
 */
class InputFormatException final : public std::runtime_error
{
public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] what_arg What argument
	 */
	explicit InputFormatException(const std::string &what_arg)
		: std::runtime_error(what_arg)
	{ /* empty */ };
};


/**
 * \brief Reports an error while reading a file.
 *
 * This exception can be thrown when the file does not exist or is not readable
 * or another IO related error occurrs while reading the file content.
 *
 * A FileReadException may optionally report the byte position of the error. A
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
	 * The byte position marks the byte on which the first error occurred.
	 *
	 * \param[in] what_arg What argument
	 * \param[in] byte_pos Byte position of the error
	 */
	FileReadException(const std::string &what_arg, const int64_t &byte_pos);

	/**
	 * \brief Byte position on which the error occurred.
	 *
	 * This entails that <tt>byte_pos - 1</tt> bytes have been read without
	 * error.
	 *
	 * A negative return value if the position is not known.
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


// forward declaration for operator ==
class FileReaderDescriptor;

bool operator == (const FileReaderDescriptor &lhs,
			const FileReaderDescriptor &rhs);

/**
 * \brief Abstract base class for the properties of a FileReader.
 *
 * A FileReaderDescriptor provides all required information to decide
 * whether a a given file can be read by readers conforming to this descriptor.
 * It can create an opaque reader that can read the file.
 *
 * FileReaderDescriptors are supposed to be stateless.
 *
 * \note
 * Instances of this class are non-copyable and non-movable but provide
 * protected special members for copy and move that can be used in subclasses.
 */
class FileReaderDescriptor : public Comparable<FileReaderDescriptor>
{
private:

	/**
	 * \brief List of case-insensitive accepted suffices.
	 */
	std::set<details::ci_string> suffices_;

public:

	friend bool operator == (const FileReaderDescriptor &lhs,
			const FileReaderDescriptor &rhs);

	/**
	 * \brief Empty constructor.
	 */
	FileReaderDescriptor();

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~FileReaderDescriptor() noexcept;

	/**
	 * \brief Name of this FileReaderDescriptor type.
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
	bool accepts_bytes(const std::vector<unsigned char> &bytes,
			const uint64_t &offset) const;

	/**
	 * \brief Check whether this descriptor accepts the specified filename.
	 *
	 * \param[in] filename The filename to test for
	 *
	 * \return TRUE iff the descriptor accepts the filename, otherwise FALSE
	 */
	bool accepts_name(const std::string &filename) const;

	/**
	 * \brief Check for acceptance of the specified format.
	 *
	 * \param[in] format The format to check for
	 *
	 * \return TRUE if \c format is accepted, otherwise FALSE
	 */
	bool accepts(Format format) const;

	/**
	 * \brief Check for acceptance of the specified format.
	 *
	 * \param[in] codec The Codec to check for
	 *
	 * \return TRUE if \c codec is accepted, otherwise FALSE
	 */
	bool accepts(Codec codec) const;

	/**
	 * \brief \link Format Formats\endlink accepted by the FileReader.
	 *
	 * \return \link Format Formats\endlink accepted by the FileReader
	 */
	std::set<Format> formats() const;

	/**
	 * \brief \link Codec Codecs\endlink accepted by the FileReader.
	 *
	 * \return \link Codec Codecs\endlink accepted by the FileReader
	 */
	std::set<Codec> codecs() const;

	/**
	 * \brief Names of the underlying libraries.
	 *
	 * Each library is represented by its name and the filepath of the
	 * concrete module loaded at runtime.
	 *
	 * \return Names of the underlying libraries
	 */
	LibInfo libraries() const;

	/**
	 * \brief Create an opaque reader for the tested file.
	 *
	 * \return A FileReader that can read the tested file
	 */
	std::unique_ptr<FileReader> create_reader() const;

	/**
	 * \brief Clone this instance.
	 *
	 * Duplicates the instance.
	 *
	 * \return A deep copy of the instance
	 */
	std::unique_ptr<FileReaderDescriptor> clone() const;

protected:

	/**
	 * \brief Constructor for accepting a set of suffices.
	 *
	 * Intended to implement a delegate constructor in subclasses.
	 */
	FileReaderDescriptor(const decltype( suffices_ ) suffices)
		: suffices_ { suffices } { /* empty */ }
	// TODO suffices_ vary with subclass not with instance, should be static

	FileReaderDescriptor(const FileReaderDescriptor &) = default;
	FileReaderDescriptor& operator = (const FileReaderDescriptor &) = default;

	FileReaderDescriptor(FileReaderDescriptor &&) noexcept = default;
	FileReaderDescriptor& operator = (FileReaderDescriptor &&) noexcept
		= default;

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
	virtual bool do_accepts_bytes(const std::vector<unsigned char> &bytes,
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
	virtual bool do_accepts_name(const std::string &name) const;

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
	 * \brief Implements FileReaderDescriptor::accepts().
	 *
	 * \param[in] codec The codec to check for
	 *
	 * \return TRUE if \c codec is accepted, otherwise FALSE
	 */
	virtual bool do_accepts(Codec codec) const
	= 0;

	/**
	 * \brief \link Format Formats\endlink accepted by the FileReader.
	 *
	 * \return \link Format Formats\endlink accepted by the FileReader
	 */
	virtual std::set<Format> do_formats() const
	= 0;

	/**
	 * \brief \link Codec Codecs\endlink accepted by the FileReader.
	 *
	 * \return \link Codec Codecs\endlink accepted by the FileReader
	 */
	virtual std::set<Codec> do_codecs() const
	= 0;

	/**
	 * \brief Implements FileReaderDescriptor::libraries().
	 *
	 * \return Name of the underlying libraries along with their versions
	 */
	virtual LibInfo do_libraries() const
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
};


class FileTest;
bool operator == (const FileTest &lhs, const FileTest &rhs);

/**
 * \brief A test whether a given FileReaderDescriptor matches a criterion.
 *
 * FileTest instances are polymorphically comparable to support their use
 * in containers.
 *
 * \note
 * Instances of this class are non-copyable and non-movable but provide
 * protected special members for copy and move that can be used in subclasses.
 */
class FileTest : public Comparable<FileTest>
{
public:

	friend bool operator == (const FileTest &lhs, const FileTest &rhs);

	/**
	 * \brief Default constructor.
	 */
	FileTest();

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~FileTest() noexcept;

	/**
	 * \brief Short description of this test.
	 *
	 * \returns Description of this test
	 */
	std::string description() const;

	/**
	 * \brief Perform test for a given pair of descriptor instance and filename.
	 *
	 * \param[in] desc     The FileReaderDescriptor to match
	 * \param[in] filename The filename to test
	 *
	 * \return TRUE iff the descriptor matches the criterion of this test
	 */
	bool passes(const FileReaderDescriptor &desc, const std::string &filename)
		const;

protected:

	FileTest(const FileTest &) = default;
	FileTest& operator = (const FileTest &) = default;

	FileTest(FileTest &&) noexcept = default;
	FileTest& operator = (FileTest &&) noexcept = default;

	/**
	 * \brief TRUE if instance equals \c rhs.
	 *
	 * \param[in] rhs Right hand side of the comparison
	 *
	 * \returns TRUE if instance equals \c rhs.
	 */
	virtual bool equals(const FileTest &rhs) const
	= 0;

private:

	/**
	 * \brief Implements FileTest::description().
	 */
	virtual std::string do_description() const
	= 0;

	/**
	 * \brief Implements FileTest::passes().
	 */
	virtual bool do_passes(const FileReaderDescriptor &desc,
			const std::string &filename) const
	= 0;
};


/**
 * \brief Test for matching a byte sequence from a file.
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
	FileTestBytes(const uint32_t &offset, const uint32_t &length);

	/**
	 * \brief Offset.
	 *
	 * \return Offset
	 */
	uint32_t offset() const;

	/**
	 * \brief Length.
	 *
	 * \return Length
	 */
	uint32_t length() const;

protected:

	bool equals(const FileTest &) const override;

private:

	std::string do_description() const override;

	bool do_passes(const FileReaderDescriptor &desc,
			const std::string &filename) const override;

	/**
	 * \brief Byte offset of the byte sequence in the file.
	 */
	uint32_t offset_;

	/**
	 * \brief Number of bytes to read from the start position.
	 */
	uint32_t length_;
};


/**
 * \brief Test for matching an actual filename.
 */
class FileTestName final : public FileTest
{
private:

	std::string do_description() const override;

	bool do_passes(const FileReaderDescriptor &desc,
			const std::string &filename) const override;

protected:

	bool equals(const FileTest &) const override;
};


/**
 * \brief A selection mechanism for a FileReaderSelection.
 *
 * A FileReaderSelector applies FileTests to FileReaderDescriptors to select
 * a descriptor with a certain test result.
 *
 * It implements two different decisions. Implementing matches() defines which
 * descriptors are candidates to be selected. Implementing select() defines
 * which of the matching candidates is concretely selected.
 *
 * \note
 * Instances of this class are non-copyable and non-movable but provide
 * protected special members for copy and move that can be used in subclasses.
 *
 * \see DefaultSelector
 */
class FileReaderSelector
{
public:

	/**
	 * \brief Default constructor.
	 */
	FileReaderSelector();

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~FileReaderSelector() noexcept;

	/**
	 * \brief Decide whether a descriptor matches the given set of tests.
	 *
	 * This defines the set of selection candidates.
	 *
	 * \param[in] filename Name of the file to perform the tests on
	 * \param[in] tests    Set of tests to perform
	 * \param[in] desc     The descriptor to check
	 *
	 * \return TRUE iff the descriptor matches the given set of tests
	 */
	bool matches(
		const std::string &filename,
		const std::set<std::unique_ptr<FileTest>> &tests,
		const std::unique_ptr<FileReaderDescriptor> &desc) const;

	/**
	 * \brief Selects a descriptor using tests.
	 *
	 * This defines the selection of a concrete selection candidate.
	 *
	 * The concrete implementation is supposed to use \c matches() to
	 * decide whether a descriptor is matched.
	 *
	 * \param[in] filename Name of the file to select a descriptor for
	 * \param[in] tests    Set of tests to perform
	 * \param[in] descs    Set of descriptors to select from
	 *
	 * \return A FileReaderDescriptor
	 */
	std::unique_ptr<FileReaderDescriptor> select(
			const std::string &filename,
			const std::set<std::unique_ptr<FileTest>> &tests,
			const std::set<std::unique_ptr<FileReaderDescriptor>> &descs)
		const;

protected:

	FileReaderSelector(const FileReaderSelector &rhs) = default;
	FileReaderSelector& operator = (const FileReaderSelector &rhs) = default;

	FileReaderSelector(FileReaderSelector &&rhs) = default;
	FileReaderSelector& operator = (FileReaderSelector &&rhs) = default;

private:

	/**
	 * \brief Implements FileReaderSelector::matches().
	 */
	virtual bool do_matches(
		const std::string &filename,
		const std::set<std::unique_ptr<FileTest>> &tests,
		const std::unique_ptr<FileReaderDescriptor> &desc) const
	= 0;

	/**
	 * \brief Implements FileReaderSelector::select().
	 */
	virtual std::unique_ptr<FileReaderDescriptor> do_select(
			const std::string &filename,
			const std::set<std::unique_ptr<FileTest>> &tests,
			const std::set<std::unique_ptr<FileReaderDescriptor>> &descs)
		const
	= 0;
};


/**
 * \brief Default selector.
 *
 * Selects the first descriptor from the descriptor list that passes all tests.
 *
 * Note that if no tests are passed, each FileReaderDescriptor matches!
 * This means that whatever is first descriptor in the sequence of descriptors
 * will be matched and create the FileReader.
 */
class DefaultSelector final : public FileReaderSelector
{
private:

	std::unique_ptr<FileReaderDescriptor> do_select(
			const std::string &filename,
			const std::set<std::unique_ptr<FileTest>> &tests,
			const std::set<std::unique_ptr<FileReaderDescriptor>> &descs)
		const override;

	bool do_matches(
		const std::string &filename,
		const std::set<std::unique_ptr<FileTest>> &tests,
		const std::unique_ptr<FileReaderDescriptor> &desc) const override;
};


/**
 * \brief Traversable selection of available FileReader descriptors.
 *
 * Default constructor initializes the selection with a DefaultSelector.
 *
 * \note
 * Instances of this class are non-copyable but movable.
 */
class FileReaderSelection final
{
public:

	/**
	 * \brief Constructor.
	 */
	FileReaderSelection();

	FileReaderSelection(FileReaderSelection &&) noexcept = default;
	FileReaderSelection& operator = (FileReaderSelection &&) noexcept = default;

	/**
	 * \brief Default destructor.
	 */
	~FileReaderSelection() noexcept; // Required by pimpl

	/**
	 * \brief Add a descriptor to the list of descriptors for which a FileReader
	 * can be created.
	 *
	 * \param[in] desc The FileReaderDescriptor to support
	 */
	void add_descriptor(std::unique_ptr<FileReaderDescriptor> desc);

	/**
	 * \brief Remove all matching descriptors.
	 *
	 * Removes all descriptors from the selection that qualify as equivalent to
	 * \c desc by testing equality with '=='.
	 *
	 * \param[in] desc The FileReaderDescriptor to be removed
	 *
	 * \return The FileReaderDescriptor removed or nullptr
	 */
	std::unique_ptr<FileReaderDescriptor> remove_descriptor(
			const std::unique_ptr<FileReaderDescriptor> &desc);

	/**
	 * \brief Removes all descriptors in this instance.
	 */
	void remove_all_descriptors();

	/**
	 * \brief Register a test.
	 *
	 * \param[in] testobj The test to be registered
	 */
	void register_test(std::unique_ptr<FileTest> testobj);

	/**
	 * \brief Remove all matching tests.
	 *
	 * Removes all tests from the selection that qualify as equivalent to
	 * \c test by testing equality with '=='.
	 *
	 * \param[in] test The FileTest to be removed
	 *
	 * \return Number of test instances removed.
	 */
	std::unique_ptr<FileTest> unregister_test(
			const std::unique_ptr<FileTest> &test);

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
	std::unique_ptr<FileReaderDescriptor> select_descriptor(
			const std::string &filename) const;

	/**
	 * \brief Create an opaque FileReader for the given file.
	 *
	 * Will return \c nullptr if the file cannot be read or the filename is
	 * empty. The FileReader returned is selected by \c select_descriptor().
	 *
	 * \param[in] filename Name of the file to create the reader for
	 *
	 * \return A FileReader for the specified file
	 */
	std::unique_ptr<FileReader> for_file(const std::string &filename) const;

	/**
	 * \brief Traverse all available descriptors and apply the specified
	 * function \c func on each of them.
	 *
	 * This enables listing or querying the set of added descriptors.
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

	/**
	 * \brief Number of registered tests.
	 *
	 * \return The number of registered tests in this selection.
	 */
	std::size_t total_tests() const;

	/**
	 * \brief TRUE if this selection has no tests registered.
	 *
	 * \return TRUE if this selection has no tests registered.
	 */
	bool no_tests() const;

private:

	// forward declaration
	class Impl;

	/**
	 * \brief Private implementation of this FileReaderSelection.
	 */
	std::unique_ptr<FileReaderSelection::Impl> impl_;
};


/**
 * \brief Instantiate FileReaderDescriptor.
 *
 * \tparam T    The type to instantiate
 * \tparam Args The constructor arguments
 */
template <class T, typename... Args>
std::unique_ptr<FileReaderDescriptor> make_descriptor(Args&&... args)
{
	static_assert(std::is_convertible<T*, FileReaderDescriptor*>::value,
			"Cannot convert type to FileReaderDescriptor");

	return std::make_unique<T>(std::forward<Args>(args)...);
}


/**
 * \internal
 * \brief Function pointer to function returning std::unique_ptr<T>.
 *
 * \tparam T The type the return unique_ptr holds
 */
template <class T>
using FunctionReturningUniquePtr = std::unique_ptr<T>(*)();


/**
 * \brief A global singleton registry holding all compiled-in FileDescriptors.
 *
 * \note
 * Instances of this class are non-copyable and non-movable but provide
 * protected special members for copy and move that can be used in subclasses.
 *
 * \note
 * This class is non-final but does not support polymorphic deletion.
 */
class FileReaderRegistry
{
public:

	/**
	 * \brief Default constructor.
	 */
	FileReaderRegistry();

	/**
	 * \brief Create a reader for the specified audio file.
	 *
	 * \param[in] filename Name of the input audio file
	 *
	 * \return Opaque FileReader for the input audio file
	 */
	static std::unique_ptr<FileReader> for_audio_file(
			const std::string &filename)
	{
		return audio_selection()->for_file(filename);
	}

	/**
	 * \brief Create a reader for the specified metadata file.
	 *
	 * \param[in] filename Name of the input metadata file
	 *
	 * \return Opaque FileReader for the input metadata file
	 */
	static std::unique_ptr<FileReader> for_toc_file(const std::string &filename)
	{
		return toc_selection()->for_file(filename);
	}

	/**
	 * \brief Return the internal audio descriptor selection.
	 *
	 * \return the internal audio descriptor selection.
	 */
	static FileReaderSelection* audio_selection()
	{
		if (!audio_selection_)
		{
			audio_selection_ = std::make_unique<FileReaderSelection>();

			audio_selection_->register_test(
					std::make_unique<FileTestBytes>(0, 44));
			// Why 44? => Enough for WAVE and every other metadata format.
			// We want to recognize container format, codec and CDDA format.
			// Consider RIFFWAVE/PCM: the first 12 bytes identify the container
			// format ('RIFF' + size + 'WAVE'), PCM format is encoded in bytes
			// 20+21, but validating CDDA requires to read the entire format
			// chunk (up to and including byte 36). Bytes 37-40 are the data
			// subchunk id and 41-44 the data subchunk size. This length is also
			// sufficient to identify all other formats currently supported.
		}

		return audio_selection_.get();
	}

	/**
	 * \brief Return the internal metadata descriptor selection.
	 *
	 * \return the internal metadata descriptor selection.
	 */
	static FileReaderSelection* toc_selection()
	{
		if (!toc_selection_)
		{
			toc_selection_ = std::make_unique<FileReaderSelection>();

			toc_selection_->register_test(std::make_unique<FileTestName>());
		}

		return toc_selection_.get();
	}

protected:

	~FileReaderRegistry() noexcept = default;

	FileReaderRegistry(const FileReaderRegistry &) = default;
	FileReaderRegistry& operator = (const FileReaderRegistry &) = default;

	FileReaderRegistry(FileReaderRegistry &&) noexcept = default;
	FileReaderRegistry& operator = (FileReaderRegistry &&) noexcept = default;

	/**
	 * \brief Instantiate the FileReader with the given name.
	 *
	 * The string parameter is ignored.
	 *
	 * \param[in] create Function pointer to create the instance
	 *
	 * \return Instance returned by \c create
	 */
	static std::unique_ptr<FileReaderDescriptor> call(
			FunctionReturningUniquePtr<FileReaderDescriptor> create)
	{
		return create();
	}

private:

	static std::unique_ptr<FileReaderSelection> audio_selection_;

	static std::unique_ptr<FileReaderSelection> toc_selection_;
};


namespace details
{

/**
 * \brief Functor to safely create a unique_ptr to a downcasted FileReader.
 *
 * It will either provide a valid FileReader of the requested type or will
 * throw. It will never silently fail nor provide a nullptr.
 *
 * \warning
 * This class is non-abstract and non-final and does not support polymorphic
 * deletion. It is intended to be used for private inheritance to stateless
 * subclasses.
 *
 * \tparam ReaderType Concrete type of the required FileReader
 *
 * \see CreateAudioReader
 * \see CreateMetadataParser
 */
template <class ReaderType>
struct CreateReader
{
	/**
	 * \brief Return a unique_ptr to an instance of the specified \c ReaderType.
	 *
	 * \param[in] selection The FileReaderSelection to choose from
	 * \param[in] filename  The name of the file to choose a FileReader
	 */
	auto operator()(const FileReaderSelection &selection,
			const std::string &filename) const -> std::unique_ptr<ReaderType>
	{
		ARCS_LOG_DEBUG << "Recognize format of input file '" << filename << "'";

		auto file_reader = selection.for_file(filename);

		if (!file_reader)
		{
			throw InputFormatException("Could not identify file format: '"
					+ filename + "'");
		}

		auto rc = details::cast_reader<ReaderType>(std::move(file_reader));

		if (!rc.first)
		{
			throw InputFormatException("Could not acquire reader for file: "
					+ filename);
		}

		return std::move(rc.first);
		// XXX Prevents RVO, but omitting the move causes compile error.
	}

protected:

	/**
	 * \brief Default constructor.
	 */
	~CreateReader() noexcept = default;
};

} // namespace details


/**
 * \brief Register a FileReaderDescriptor type for audio input.
 *
 * \tparam D The descriptor type to register
 */
template <class D>
class RegisterAudioDescriptor final : private FileReaderRegistry
{
public:

	/**
	 * \brief Register a descriptor
	 */
	RegisterAudioDescriptor()
	{
		audio_selection()->add_descriptor(call(&make_descriptor<D>));
	}
};


/**
 * \brief Register a FileReaderDescriptor type for TOC/metadata input.
 *
 * \tparam D The descriptor type to register
 */
template <class D>
class RegisterMetadataDescriptor final : private FileReaderRegistry
{
public:

	/**
	 * \brief Register a descriptor
	 */
	RegisterMetadataDescriptor()
	{
		toc_selection()->add_descriptor(call(&make_descriptor<D>));
	}
};

/// @}

} // namespace v_1_0_0

} // namespace arcsdec

#endif

