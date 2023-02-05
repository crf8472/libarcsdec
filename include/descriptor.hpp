#ifndef __LIBARCSDEC_DESCRIPTOR_HPP__
#define __LIBARCSDEC_DESCRIPTOR_HPP__

/**
 * \file
 *
 * \brief Toolkit for selecting file readers
 */

#include <cctype>      // for toupper
#include <cstddef>     // for size_t
#include <cstdint>     // for uint32_t, uint64_t, int64_t
#include <list>        // for list
#include <memory>      // for unique_ptr, make_unique
#include <set>         // for set
#include <stdexcept>   // for runtime_error
#include <string>      // for basic_string, string, char_traits
#include <utility>     // for pair
#include <vector>      // for vector

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>   // for ARCS_LOG_WARNING, ARCS_LOG_DEBUG
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

} // namespace details


/**
 * \defgroup descriptor API for abstract FileReaders
 *
 * \brief API for describing opaque \link FileReader FileReaders\endlink for
 * given input files.
 *
 * A Format represents a file format. A Codec represents an audio codec.
 * \link Format Formats\endlink and \link Codec Codecs\endlink can be
 * transferred to their names by name().
 *
 * A FileReader is an abstract base for either reading metadata/TOC files or
 * audio files. A FileReaderDescriptor contains metainformation about the
 * FileReader and can help to determine whether its FileReader is the right one
 * for reading a particular file.
 *
 * An InputFormatException indicates any error concerning the input file format.
 * A FileReadException indicates problems while actually reading the file.
 *
 * Helpers for implementing own \link FileReader FileReaders\endlink are
 * get_suffix() which returns the filename suffix and read_bytes() which reads
 * a specified amount of bytes from a specified position in the file.
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
 * \brief Entry of a \c LibInfo.
 */
using LibInfoEntry = std::pair<std::string, std::string>;


/**
 * \brief Create an entry for a LibInfo.
 *
 * \param[in] libname Name of a library.
 */
LibInfoEntry libinfo_entry(const std::string &libname);


/**
 * \brief Represents a list of pairs of a library name and an additional string.
 */
using LibInfo = std::list<LibInfoEntry>;



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
	 * \brief Id of this FileReaderDescriptor type.
	 *
	 * The id can be used as key in a FileReaderRegistry.
	 *
	 * \return A human-readable id of this FileReaderDescriptor
	 */
	std::string id() const;

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
	 * \brief \link Format Formats\endlink accepted by the FileReader.
	 *
	 * \return \link Format Formats\endlink accepted by the FileReader
	 */
	std::set<Format> formats() const;

	/**
	 * \brief Check for acceptance of the specified format.
	 *
	 * \param[in] codec The Codec to check for
	 *
	 * \return TRUE if \c codec is accepted, otherwise FALSE
	 */
	bool accepts(Codec codec) const;

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
	 * \brief Implements FileReaderDescriptor::id().
	 *
	 * \return A human-readable id of this FileReaderDescriptor
	 */
	virtual std::string do_id() const
	= 0;

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
	virtual bool do_accepts_format(Format format) const;

	/**
	 * \brief \link Format Formats\endlink accepted by the FileReader.
	 *
	 * \return \link Format Formats\endlink accepted by the FileReader
	 */
	virtual std::set<Format> do_formats() const;

	/**
	 * \brief Create the list of \link Format Formats\endlink accepted by the FileReader.
	 *
	 * While the default implementation of do_formats() buffers the list,
	 * define_formats() actually creates it.
	 *
	 * \return The list of \link Format Formats\endlink accepted by the FileReader
	 */
	virtual std::set<Format> define_formats() const;

	/**
	 * \brief Implements FileReaderDescriptor::accepts().
	 *
	 * \param[in] codec The codec to check for
	 *
	 * \return TRUE if \c codec is accepted, otherwise FALSE
	 */
	virtual bool do_accepts_codec(Codec codec) const;

	/**
	 * \brief \link Codec Codecs\endlink accepted by the FileReader.
	 *
	 * \return \link Codec Codecs\endlink accepted by the FileReader
	 */
	virtual std::set<Codec> do_codecs() const;

	/**
	 * \brief Create the list of \link Codec Codecs\endlink accepted by the FileReader.
	 *
	 * While the default implementation of do_codecs() buffers the list,
	 * define_codecs() actually creates it.
	 *
	 * \return The list of \link Codec Codecs\endlink accepted by the FileReader
	 */
	virtual std::set<Codec> define_codecs() const;

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

/// @}

} // namespace v_1_0_0
} // namespace arcsdec

#endif

