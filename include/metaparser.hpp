#ifndef __LIBARCSDEC_METAPARSER_HPP__
#define __LIBARCSDEC_METAPARSER_HPP__

/**
 * \file
 *
 * \brief API for implementing MetadataParsers
 */

#include <limits>       // for numeric_limits
#include <memory>       // for unique_ptr
#include <ostream>      // for ostringstream
#include <stdexcept>    // for out_of_range, runtime_error
#include <string>       // for string
#include <type_traits>  // for is_signed, is_unsigned

#ifndef __LIBARCSTK_METADATA_HPP__
#include <arcstk/metadata.hpp>    // for ToC
#endif

#ifndef __LIBARCSDEC_DESCRIPTOR_HPP__
#include "descriptor.hpp"  // for FileReader, FileReaderDescriptor
#endif

namespace arcsdec
{
inline namespace v_1_0_0
{

using arcstk::ToC;

/**
 * \defgroup metaparser API for implementing MetadataParsers
 *
 * \brief API for implementing \link MetadataParser MetadataParsers\endlink.
 *
 * Class MetadataParser provides an interface for parsing ToC files.
 *
 * The MetadataParser provides function \c parse() to parse the input file to
 * a arcstk::ToC instance. The ToC object is constructed using arcstk::make_toc.
 *
 * A MetadataParser internally holds a concrete instance of MetadataParserImpl.
 * MetadataParserImpl can be subclassed to implement the capabilities of a
 * MetadataParser.
 *
 * The concrete reading of a given ToC file is implemented by the subclasses
 * of MetadataParserImpl.
 *
 * A parse error is reported by a MetadataParseException.
 *
 * @{
 */


/**
 * \brief Abstract base class for MetadataParser implementations.
 *
 * Concrete subclasses of MetadataParserImpl implement MetadataParsers for a
 * concrete FileReaderDescriptor.
 *
 * \note
 * Instances of subclasses are non-copyable but movable.
 */
class MetadataParserImpl
{
public:

	/**
	 * \brief Default constructor.
	 */
	MetadataParserImpl();

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~MetadataParserImpl() noexcept;

	/**
	 * \brief Parses a metadata file.
	 *
	 * \param[in] filename The file to parse
	 *
	 * \return The ToC information represented by the file
	 *
	 * \throw FileReadException      If the file could not be read
	 * \throw MetadataParseException If the metadata could not be parsed
	 */
	std::unique_ptr<ToC> parse(const std::string &filename);

	/**
	 * \brief Create a descriptor for this MetadataParser implementation.
	 *
	 * \return Descriptor for this implementation.
	 */
	std::unique_ptr<FileReaderDescriptor> descriptor() const;

protected:

	MetadataParserImpl(MetadataParserImpl &&) noexcept;
	MetadataParserImpl& operator = (MetadataParserImpl &&) noexcept;

private:

	/**
	 * \brief Implements parse().
	 *
	 * \param[in] filename The file to parse
	 *
	 * \return The ToC information represented by the file
	 *
	 * \throw FileReadException      If the file could not be read
	 * \throw MetadataParseException If the metadata could not be parsed
	 */
	virtual std::unique_ptr<ToC> do_parse(const std::string &filename)
	= 0;

	/**
	 * \brief Provides implementation for \c descriptor() of a MetadataParser.
	 *
	 * \return A FileReaderDescriptor for this MetadataParser
	 */
	virtual std::unique_ptr<FileReaderDescriptor> do_descriptor() const
	= 0;
};


/**
 * \brief Parse metadata files and provide the content as a ToC instance.
 *
 * \note
 * Instances of this class are non-copyable but movable.
 */
class MetadataParser final : public FileReader
{
public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] impl The concrete implementation of the MetadataParser
	 */
	MetadataParser(std::unique_ptr<MetadataParserImpl> impl);

	MetadataParser(MetadataParser &&) noexcept;
	MetadataParser& operator = (MetadataParser &&) noexcept;

	/**
	 * \brief Parses a metadata file.
	 *
	 * \param[in] filename The file to parse
	 *
	 * \return The ToC information represented by the file
	 *
	 * \throw FileReadException      If the file could not be read
	 * \throw MetadataParseException If the metadata could not be parsed
	 */
	std::unique_ptr<ToC> parse(const std::string &filename);

private:

	/**
	 * \brief Implementation of a MetadataParser.
	 */
	std::unique_ptr<MetadataParserImpl> impl_;

	std::unique_ptr<FileReaderDescriptor> do_descriptor() const override;
};


/**
 * \brief Reports unexpected content while parsing a metadata file.
 */
class MetadataParseException final : public std::runtime_error
{
public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] what_arg What argument
	 */
	explicit MetadataParseException(const std::string &what_arg);
};


namespace details
{

/**
 * \brief Returns TRUE if types S and T are either both signed or both unsigned.
 *
 * \tparam S Left type
 * \tparam T Right type
 */
template <typename S, typename T>
struct signedness : public std::integral_constant<bool,
	(std::is_signed<S>::value && std::is_signed<T>::value)
	|| (std::is_unsigned<S>::value && std::is_unsigned<T>::value)>
{
	// empty
};


/**
 * \brief Service method: Cast a value of some integral type safely to an
 * integral type of smaller range.
 *
 * The types must either both be signed or both be unsigned.
 *
 * If the input type is within the range of the target type, the cast is
 * performed, otherwise an exception is thrown.
 *
 * \param[in] value The value to convert
 *
 * \throw out_of_range If \c value is out of the range of target type
 *
 * \return The numerical value
 */
template <typename S, typename T,
		 std::enable_if_t<details::signedness<S, T>::value, int> = 0>
inline auto cast_or_throw(const T value) -> S
{
	auto throw_with_message = [](const T val, const std::string &msg)
	{
		std::ostringstream stream;
		stream << "Value " << val << " " << msg;
		throw std::out_of_range(stream.str());
	};

	if (value < std::numeric_limits<S>::min())
	{
		throw_with_message(value, "is too small to cast");
	}

	if (value > std::numeric_limits<S>::max())
	{
		throw_with_message(value, "is too big to cast");
	}

	return static_cast<S>(value);
}


/**
 * \brief Convert MSF time to CDDA frames.
 */
long msf_to_frames(const int m, const int s, const int f);


/**
 * \brief Convert CDDA frames to MSF frames.
 */
void frames_to_msg(long frames, int* m, int* s, int* f);

} // namespace details

/// @}

} // namespace v_1_0_0
} // namespace arcsdec

#endif

