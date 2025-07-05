#ifndef __LIBARCSDEC_DESCRIPTOR_HPP__
#define __LIBARCSDEC_DESCRIPTOR_HPP__

/**
 * \file
 *
 * \brief Recognize file formats and select compatible file readers.
 */

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>   // for ARCS_LOG_WARNING, ARCS_LOG_DEBUG
#endif

#include <algorithm>   // for transform
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


namespace arcsdec
{
inline namespace v_1_0_0
{

/**
 * \internal
 *
 * \brief Implementation details of API 1.0.0.
 */
namespace details
{

/* -NO_DOXYGEN-
 * \brief Traits for case insensitive string comparison.
 *
 * Thanks to Herb Sutter: http://www.gotw.ca/gotw/029.htm
 */
struct ci_char_traits final : public std::char_traits<char>
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
 * \defgroup descriptor Interfaces for abstract FileReaders
 *
 * \brief API for describing \link FileReader FileReaders\endlink for several
 * file \link Format Formats\endlink.
 *
 * A Format represents a file format. A Codec represents an audio codec. For
 * \link Format Formats\endlink as well as for \link Codec Codecs\endlink, their
 * respective names can be achieved by functions name().
 *
 * A file can be matched against a Format by a FormatMatcher. A FormatMatcher
 * is defined with some referential Bytes of a file or a SuffixSet for
 * filenames.
 *
 * Bytes denote a reference sequence of bytes. They consist of an offset,
 * denoting their required start position in the file and a
 * \link ByteSeq ByteSequence\endlink which is a sequence of concrete byte
 * values that may or may not contain wildcards.
 *
 * A FormatMatcher matches Bytes passed or a concrete filename suffix for
 * matching its reference information. The interpretation of the match of one
 * or both of these input informations is in the responsibility of the caller.
 * This is a base mechanism for checking a file for a certain format and codec.
 *
 * A FileReader is an abstract base for either reading metadata/ToC files or
 * audio files. Any concrete FileReader implements a reading capability for at
 * least one Format and at least one Codec. The FileReader interface defines
 * only the capability to return a FileReaderDescriptor. (The definition of
 * other capabilities is left to subclasses, as there are AudioReader and
 * MetadataParser.)
 *
 * A FileReaderDescriptor contains metainformation about some concrete
 * FileReader and can inform about whether this respective FileReader has
 * the capability to read a particular file in question.
 * \link FileReaderDescriptor FileReaderDescriptors\endlink either accept or
 * do not accept any particular \link Format Formats\endlink. Any
 * FileReaderDescriptor can create the concrete FileReader it describes. Any
 * concrete FileReader can return its specific FileReaderDescriptor,
 * accordingly.
 *
 * An InputFormatException indicates any error concerning the input file format.
 * A FileReadException indicates problems while actually reading the file.
 *
 * There are some helpers for implementing own
 * \link FileReaderDescriptor FileReaderDescriptors\endlink. Function
 * read_bytes() reads a specified amount of bytes from a specified position in
 * the file. The function returns a \link ByteSeq ByteSequence\endlink that is
 * compatible with the input for file format checks. Function get_suffix()
 * returns the suffix of a given filename. This suffix can be matched
 * case-insensitive against a set of suffices by ci_match_suffix().
 *
 * The \ref AudioReader and \ref MetadataParser APIs are built on this API.
 *
 * @{
 */


/**
 * \brief List of supported file formats for metadata and audio.
 *
 * Format::UNKNOWN represents a Format that was checked but could not be
 * recognized.
 *
 * The intention is to support inspecting the capabilities of
 * \link FileReader FileReaders\endlink.
 */
enum class Format : unsigned
{
	UNKNOWN,  // 0, guaranteed to be first
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
 * \brief Name of the \c format.
 *
 * \param[in] format The Format to get the name for
 *
 * \return Name of the Format.
 */
std::string name(Format format);


/**
 * \brief A supported audio codec.
 *
 * The supported codecs are only the tested codecs, in fact other lossless
 * codecs can be silently supported if an appropriate FileReader accepts a
 * Format that supports this Codec and accepts Codec::UNKNOWN. However, the best
 * practise is to explicitly support a Codec by an explicit check for it.
 *
 * Codec::UNKNOWN represents a codec that was checked but could not be
 * recognized. Codec::NONE represents the information that no codec is expected,
 * supported or available.
 *
 * The intention is to support inspecting the capabilities of
 * \link FileReader FileReaders\endlink.
 */
enum class Codec : unsigned
{
	UNKNOWN,  // 0, guaranteed to be first
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
	ALAC,
	NONE // Guaranteed to be last
};


/**
 * \brief Name of the \c codec.
 *
 * \param[in] codec The Codec to get the name for
 *
 * \return Name of the Codec.
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
	friend bool operator != (const T& lhs, const T& rhs) noexcept
	{
		return !(lhs == rhs);
	}
};


class ByteSeq;

bool operator == (const ByteSeq& lhs, const ByteSeq& rhs);

void swap(ByteSeq& lhs, ByteSeq& rhs);

/**
 * \brief Byte sequence with or without wildcards.
 */
class ByteSeq final : public Comparable<ByteSeq>
{
public:

	/**
	 * \brief Type to represent an actual byte.
	 */
	using byte_type = unsigned char;

private:

	/**
	 * \brief Internal byte sequence.
	 */
	std::vector<byte_type> sequence_;

	/**
	 * \brief Type of the internal sequence.
	 */
	using sequence_type = decltype( sequence_ );

	/**
	 * \brief List of wildcard positions in the sequence.
	 */
	std::set<sequence_type::size_type> wildcards_;

public:

	friend bool operator == (const ByteSeq& lhs, const ByteSeq& rhs);

	/**
	 * \brief Numerical maximum of the \c byte_type.
	 */
	constexpr static byte_type max_byte_value {
		std::numeric_limits<byte_type>::max() };

	/**
	 * \brief Constructor using a sequence of byte values and wildcards.
	 *
	 * \param[in] values List of byte values with or without wildcards.
	 */
	ByteSeq(std::initializer_list<unsigned> values);

	/**
	 * \brief Constructor for an empty sequence of specified length.
	 *
	 * \param[in] length Actual length for a yet empty sequence
	 */
	ByteSeq(sequence_type::size_type length);

	/**
	 * \brief TRUE if byte on position \c i of the sequence has value \c b.
	 *
	 * \param[in] i Position index
	 * \param[in] b Byte value to check for
	 *
	 * \return TRUE if byte on position \c i of the sequence has value \c b,
	 * otherwise FALSE.
	 */
	bool matches(sequence_type::size_type i, byte_type b) const;

	/**
	 * \brief TRUE if byte on position \c i of the sequence is a wildcard.
	 *
	 * \param[in] i Position index
	 *
	 * \return TRUE if byte on position \c i of the sequence is a wildcard,
	 * otherwise FALSE.
	 */
	bool is_wildcard(sequence_type::size_type i) const;

	/**
	 * \brief Swap this instance with another.
	 *
	 * \param[in] rhs The other instance to swap with
	 *
	 * \return Instance after swapping
	 */
	ByteSeq& swap(ByteSeq& rhs) ; //TODO noexcept possible when C++17


	// Wrappers for functions delegated to the sequence_type

	using const_reference = sequence_type::const_reference;
	using reference = sequence_type::reference;
	using size_type = sequence_type::size_type;

	size_type size() const;
	bool empty() const;

	const_reference operator[](size_type i) const;
	reference operator[](size_type i);

	sequence_type::iterator begin();
	sequence_type::iterator end();

	sequence_type::const_iterator begin() const;
	sequence_type::const_iterator end() const;

	sequence_type::const_iterator cbegin() const;
	sequence_type::const_iterator cend() const;

	byte_type* data(); // forces contiguous memory layout
};



/**
 * \brief A sequence of bytes.
 *
 * As read from a file.
 */
using ByteSequence = ByteSeq;


class Bytes;

bool operator == (const Bytes& lhs, const Bytes& rhs);

void swap(Bytes& lhs, Bytes& rhs);


/**
 * \brief A sequence of bytes read from a specific offset in a file.
 *
 * The sequence can contain wildcards.
 */
class Bytes final : public Comparable<Bytes>
{
public:

	friend bool operator == (const Bytes& lhs, const Bytes& rhs);

	/**
	 * \brief Wildcard for a single byte.
	 */
	static constexpr unsigned int any = ByteSequence::max_byte_value + 1;

	/**
	 * \brief Constructor.
	 *
	 * Initiates an empty internal ByteSequence with an offset() of 0.
	 */
	Bytes();

	/**
	 * \brief Constructor.
	 *
	 * \param[in] offset  0-based offset where \c bytes occured in the file
	 * \param[in] bytes   Sequence of bytes
	 */
	Bytes(const uint32_t offset, const ByteSequence& bytes);

	/**
	 * \brief Match a byte sequence with this instance.
	 *
	 * The match is tried starting on positin offset() on this instance and
	 * ends on the end of the shorter sequence, either \c bytes or this
	 * instance.
	 *
	 * \param[in] bytes  Byte sequence to be matched
	 *
	 * \return TRUE iff this instance matches \c bytes, otherwise FALSE
	 */
	bool match(const Bytes& bytes) const;

	/**
	 * \brief Match a byte sequence starting with \c offset with this instance.
	 *
	 * The match is tried starting on positin \c offset on this instance and
	 * ends on the end of the shorter sequence, either \c bytes or this
	 * instance.
	 *
	 * \note Parameter \c offset does not refer to the original file position
	 * but to the start of the reference ByteSequence. It does therfore not
	 * indicate where in the file the sequence \c bytes is located but whether
	 * \c bytes is the very start of the reference ByteSequence or only a
	 * shifted part of it.
	 *
	 * \param[in] bytes  ByteSequence from a file to be matched
	 * \param[in] offset Offset to start of ByteSequence of this instance
	 *
	 * \return TRUE iff this instance matches \c bytes, otherwise FALSE
	 */
	bool match(const ByteSequence& bytes, const uint32_t& offset) const;

	/**
	 * \brief Match a byte sequence with offset 0 with this instance.
	 *
	 * This is equivalent to <tt>match(bytes, 0)</tt>.
	 *
	 * \param[in] bytes ByteSequence to be matched
	 *
	 * \return TRUE iff this instance matches \c bytes, otherwise FALSE
	 */
	bool match(const ByteSequence& bytes) const;

	/**
	 * \brief Offset of this instance.
	 *
	 * \return Offset of this instance.
	 */
	uint32_t offset() const;

	/**
	 * \brief ByteSequence of this instance.
	 *
	 * \return ByteSequence of this instance.
	 */
	ByteSequence sequence() const;

	/**
	 * \brief Total number of bytes contained.
	 *
	 * \return Total number of bytes.
	 */
	ByteSequence::size_type size() const;

	/**
	 * \brief Read access to a single byte.
	 *
	 * \param[in] i Index position
	 *
	 * \return Read a single byte.
	 */
	ByteSequence::const_reference operator[](ByteSequence::size_type i) const;

	/**
	 * \brief Swap this instance with another.
	 *
	 * \param[in] rhs The other instance to swap with
	 *
	 * \return Instance after swapping
	 */
	Bytes& swap(Bytes& rhs) ; //TODO noexcept possible when C++17

private:

	/**
	 * \brief Returns the internal ByteSequence.
	 *
	 * \return The internal ByteSequence.
	 */
	const ByteSequence& ref_bytes() const;

	/**
	 * \brief Offset of the internal ByteSequence in the file.
	 */
	uint32_t offset_;

	/**
	 * \brief Internal ByteSequence.
	 */
	ByteSequence seq_;
};


/**
 * \brief A set of filename suffices.
 *
 * An ordered set of case-insensitive strings. Defines a \c value_type and
 * is iterable as required by \c std::find_if.
 */
using SuffixSet = std::set<details::ci_string>;


namespace details
{

/**
 * \brief Worker: default implementation for checking a filename.
 *
 * Returns TRUE if the suffix of the filename equals one of the internal
 * suffices. The check is done case-insensitive.
 *
 * \param[in] suffices Set of filename suffices
 * \param[in] filename Input filename to check
 *
 * \return TRUE if the filename suffix matches one of the internal suffices
 */
bool ci_match_suffix(const SuffixSet& suffices,
		const std::string& filename);


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
std::string get_suffix(const std::string& filename,
		const std::string& delimiter);


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
Bytes read_bytes(const std::string& filename,
	const uint32_t& offset, const uint32_t& length);

} // namespace details


/**
 * \brief Interface for matchers.
 *
 * A Matcher is a check for a certain file format or audio codec.
 */
class Matcher
{
public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~Matcher() noexcept;

	/**
	 * \brief Name of this matcher.
	 *
	 * A printable name that hints about what this matcher refers to.
	 *
	 * \return Name of this matcher.
	 */
	std::string name() const;

	/**
	 * \brief Match byte sequence located at a specific offset in the file.
	 *
	 * \param[in] bytes   Bytes of a file to check.
	 *
	 * \return TRUE if \c bytes occur in the target
	 */
	bool matches(const Bytes& bytes) const;

	/**
	 * \brief Match filename.
	 *
	 * \param[in] filename Name of the file to check.
	 *
	 * \return TRUE if \c filename matches what is described by this instance
	 */
	bool matches(const std::string& filename) const;

	/**
	 * \brief Format matched by this matcher.
	 *
	 * \return Format matched by this matcher.
	 */
	Format format() const;

	/**
	 * \brief Codecs supported by this matcher.
	 *
	 * \return Codecs supported by this matcher
	 */
	std::set<Codec> codecs() const;

	/**
	 * \brief Reference bytes to be matched.
	 *
	 * \return Reference bytes this matcher tries to match.
	 */
	Bytes reference_bytes() const;

	/**
	 * \brief Create a deep copy.
	 *
	 * \return Deep copy of this instance.
	 */
	std::unique_ptr<Matcher> clone() const;

private:

	virtual std::string do_name() const
	= 0;

	virtual bool do_matches(const Bytes& bytes) const
	= 0;

	virtual bool do_matches(const std::string& filename) const
	= 0;

	virtual Format do_format() const
	= 0;

	virtual std::set<Codec> do_codecs() const
	= 0;

	virtual Bytes do_reference_bytes() const
	= 0;

	virtual std::unique_ptr<Matcher> do_clone() const
	= 0;
};


/**
 * \brief Matcher for file formats.
 *
 * Matches a specific Format. A FormatMatcher can be implemented by
 * constructing a FormatMatcher with the specified format as its template
 * parameter along with filename suffices and a byte sequence to match.
 *
 * \tparam F The Format matched by this matcher.
 */
template <enum Format F>
class FormatMatcher final : public Matcher
                          , public Comparable<FormatMatcher<F>>
{
public:

	friend bool operator == (const FormatMatcher& lhs,
			const FormatMatcher& rhs)
	{
		return lhs.suffices_ == rhs.suffices_ && lhs.bytes_ == rhs.bytes_
			&& lhs.codecs_ == rhs.codecs_;
	}


	/**
	 * \brief Constructor with reference suffices and bytes.
	 *
	 * \param[in] suffices  Suffices accepted by this Format
	 * \param[in] bytes     A byte sequence accepted by this Format
	 * \param[in] codecs    Codecs supported for this Format
	 */
	FormatMatcher(const SuffixSet& suffices, const Bytes& bytes,
			const std::set<Codec>& codecs)
		: suffices_ { suffices }
		, bytes_    { bytes }
		, codecs_   { codecs }
	{ /* empty */ };


	/**
	 * \brief Constructor with reference suffices.
	 *
	 * \param[in] suffices  Suffices accepted by this Format
	 * \param[in] codecs    Codecs supported for this Format
	 */
	FormatMatcher(const SuffixSet& suffices, const std::set<Codec>& codecs)
		: FormatMatcher(suffices, { /* empty byte sequence */ }, codecs)
	{ /* empty */ };


	/**
	 * \brief Virtual default destructor.
	 */
	inline virtual ~FormatMatcher() noexcept = default;


private:

	inline std::string do_name() const final
	{
		using arcsdec::name;
		return name(F);
	}

	inline bool do_matches(const Bytes& bytes) const final
	{
		return bytes_.match(bytes);
	}

	inline bool do_matches(const std::string& filename) const final
	{
		return details::ci_match_suffix(suffices_, filename);
	}

	inline Format do_format() const final
	{
		return F;
	}

	inline std::set<Codec> do_codecs() const final
	{
		return codecs_;
	}

	inline Bytes do_reference_bytes() const final
	{
		return bytes_;
	}

	inline std::unique_ptr<Matcher> do_clone() const final
	{
		return
			std::make_unique<FormatMatcher<F>>(suffices_, bytes_, codecs_);
	}

	/**
	 * \brief Internal set of supported suffices.
	 */
	SuffixSet suffices_;

	/**
	 * \brief Internal reference byte sequence.
	 */
	Bytes bytes_;

	/**
	 * \brief Internal set of codecs supported for this Format.
	 */
	std::set<Codec> codecs_;
};


/**
 * \brief Entry of a LibInfo.
 *
 * An entry for a LibInfo consists of the library name and an additional string.
 * The additional string can be used for information about the library.
 */
using LibInfoEntry = std::pair<std::string, std::string>;

/**
 * \brief Represents a list of pairs of a library name and an additional string.
 */
using LibInfo = std::list<LibInfoEntry>;

/**
 * \brief Create a LibInfoEntry for \c libname, lookup filepath of library.
 *
 * The second part will contain the concrete filepath for the library named
 * \c libname. The libarcsdec binary is inspected to lookup this information.
 *
 * \param[in] libname Name of a library.
 *
 * \return A LibInfoEntry that contains the concrete filepath for the library.
 */
LibInfoEntry libinfo_entry_filepath(const std::string& libname);



/**
 * \brief Reports an error concerning the input file format.
 *
 * This exception reports that the input format could not be determined
 * or no FileReader could be acquired.
 */
class InputFormatException final : public std::runtime_error
{
public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] what_arg What argument
	 */
	explicit InputFormatException(const std::string& what_arg);
};


/**
 * \brief Reports an error while reading a file.
 *
 * This exception reports that the file does not exist or is not readable
 * or another IO related error occurred while reading the file content.
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
	explicit FileReadException(const std::string& what_arg);

	/**
	 * \brief Constructor.
	 *
	 * The byte position marks the byte on which the first error occurred.
	 *
	 * \param[in] what_arg What argument
	 * \param[in] byte_pos Byte position of the error
	 */
	FileReadException(const std::string& what_arg, const int64_t& byte_pos);

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


class FileReaderDescriptor;

/**
 * \brief Abstract base class for \link FileReader FileReaders\endlink.
 *
 * \see AudioReader
 * \see MetadataParser
 */
class FileReader
{
public:

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

private:

	/**
	 * \brief Implements FileReader::descriptor().
	 */
	virtual std::unique_ptr<FileReaderDescriptor> do_descriptor() const
	= 0;
};


bool operator == (const FileReaderDescriptor& lhs,
			const FileReaderDescriptor& rhs);


/**
 * \brief Input filetype.
 */
enum class InputType: unsigned
{
	AUDIO,
	TOC
};


/**
 * \brief Abstract base class for the properties of a FileReader.
 *
 * A FileReaderDescriptor provides all required information to decide
 * whether a a given file can be read by readers specified and created by this
 * descriptor. It can create an opaque reader that can read the successfully
 * matched file.
 *
 * \warning
 * FileReaderDescriptors are supposed to be stateless. If a subclass adds a
 * state, the equality operator will not work as expected.
 */
class FileReaderDescriptor : public Comparable<FileReaderDescriptor>
{
public:

	friend bool operator == (const FileReaderDescriptor& lhs,
			const FileReaderDescriptor& rhs);

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
	 * \brief Check for acceptance of the specified format.
	 *
	 * A descriptor accepts a Format iff it is contained in formats().
	 *
	 * \param[in] format The format to check for
	 *
	 * \return TRUE if \c format is accepted, otherwise FALSE
	 */
	bool accepts(const Format format) const;

	/**
	 * \brief Check for acceptance of the specified codec.
	 *
	 * A descriptor accepts a Codec iff it is contained in codecs().
	 *
	 * \warning
	 * This does not entail that accepts(f, codec) is TRUE for every
	 * accepted Format f! The set of formats for which this codec is
	 * accepted may be restricted to a subset of the accepted formats().
	 *
	 * \param[in] codec The codec to check for
	 *
	 * \return TRUE if \c codec is accepted, otherwise FALSE
	 */
	bool accepts(const Codec codec) const;

	/**
	 * \brief Check for acceptance of the specified Format and Codec pair.
	 *
	 * A specified Codec may be accepted on its own, but not together with
	 * the specified Format.
	 *
	 * \param[in] format The Format to check for
	 * \param[in] codec  The Codec to check for
	 *
	 * \return TRUE if \c format and \c codec are accepted, otherwise FALSE
	 */
	bool accepts(const Format format, const Codec codec) const;

	/**
	 * \brief Set of accepted formats.
	 *
	 * \return The set of accepted formats.
	 */
	const std::set<Format> formats() const;

	/**
	 * \brief Set of accepted codecs.
	 *
	 * \return The set of accepted codecs.
	 */
	const std::set<Codec> codecs() const;

	// TODO List accepted format/codec pairs

	/**
	 * \brief Names of the libraries the reader used to implement the reader.
	 *
	 * Each library is represented by its name and the filepath of the
	 * concrete binary object loaded at runtime.
	 *
	 * \return Names of the underlying libraries
	 */
	LibInfo libraries() const;

	/**
	 * \brief Returns TRUE iff this descriptor is for an AudioReader, otherwise
	 * FALSE.
	 *
	 * \return InputType of this Descriptor
	 */
	InputType input_type() const;

	/**
	 * \brief Create an opaque reader for the tested file.
	 *
	 * \return A FileReader that can read the tested file
	 */
	std::unique_ptr<FileReader> create_reader() const;

	/**
	 * \brief Clone this instance.
	 *
	 * Provides a deep copy of the instance.
	 *
	 * \return A deep copy of the instance
	 */
	std::unique_ptr<FileReaderDescriptor> clone() const;

private:

	virtual std::string do_id() const
	= 0;

	virtual std::string do_name() const
	= 0;


	virtual InputType do_input_type() const;

	virtual bool do_accepts_format(const Format f) const;

	virtual bool do_accepts_codec(const Codec c) const;

	virtual bool do_accepts_format_and_codec(const Format f, const Codec c)
		const;

	virtual const std::set<Format> do_formats() const;

	virtual const std::set<Codec> do_codecs() const;

	virtual std::set<Format> define_formats() const;

	virtual std::set<Codec> define_codecs() const;


	virtual LibInfo do_libraries() const
	= 0;

	virtual std::unique_ptr<FileReader> do_create_reader() const
	= 0;

	virtual std::unique_ptr<FileReaderDescriptor> do_clone() const
	= 0;
};

/// @}

} // namespace v_1_0_0
} // namespace arcsdec

#endif

