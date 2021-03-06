#ifndef __LIBARCSDEC_METAPARSER_HPP__
#define __LIBARCSDEC_METAPARSER_HPP__

/**
 * \file
 *
 * \brief API for implementing MetadataParsers
 */

#include <memory>     // for unique_ptr
#include <stdexcept>  // for runtime_error
#include <string>     // for string

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>
#endif

#ifndef __LIBARCSDEC_DESCRIPTORS_HPP__
#include "descriptors.hpp"
#endif

namespace arcsdec
{
inline namespace v_1_0_0
{

using arcstk::TOC;

/**
 * \defgroup metaparser API for implementing MetadataParsers
 *
 * \brief API for implementing \link MetadataParser MetadataParsers\endlink.
 *
 * Class MetadataParser provides an interface for parsing TOC files.
 *
 * The MetadataParser provides function \c parse() to parse the input file to
 * a arcstk::TOC instance. The TOC object is constructed using arcstk::make_toc.
 *
 * A MetadataParser internally holds a concrete instance of MetadataParserImpl.
 * MetadataParserImpl can be subclassed to implement the capabilities of a
 * MetadataParser.
 *
 * The concrete reading of a given TOC file is implemented by the subclasses
 * of MetadataParserImpl.
 *
 * A parse error is reported by a MetadataParseException.
 *
 * CreateMetadataParser creates a MetadataParser from a selection of readers and
 * a given filename.
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
	 * \return The TOC information represented by the file
	 *
	 * \throw FileReadException      If the file could not be read
	 * \throw MetadataParseException If the metadata could not be parsed
	 */
	std::unique_ptr<TOC> parse(const std::string &filename);

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
	 * \return The TOC information represented by the file
	 *
	 * \throw FileReadException      If the file could not be read
	 * \throw MetadataParseException If the metadata could not be parsed
	 */
	virtual std::unique_ptr<TOC> do_parse(const std::string &filename)
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
 * \brief Parse metadata files and provide the content as a TOC instance.
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
	 * \return The TOC information represented by the file
	 *
	 * \throw FileReadException      If the file could not be read
	 * \throw MetadataParseException If the metadata could not be parsed
	 */
	std::unique_ptr<TOC> parse(const std::string &filename);

private:

	/**
	 * \brief Implementation of a MetadataParser.
	 */
	std::unique_ptr<MetadataParserImpl> impl_;

	std::unique_ptr<FileReaderDescriptor> do_descriptor() const override;
};


/**
 * \brief Functor for safe creation of a MetadataParser.
 */
struct CreateMetadataParser final : public details::CreateReader<MetadataParser>
{
	// empty
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

} // namespace details


/**
 * \brief Service method: Cast a value of some integral type to an integral
 * type of smaller range.
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
auto cast_or_throw(const T value) -> S
{
	if (value < std::numeric_limits<S>::min())
	{
		std::ostringstream msg;
		msg << "Value " << value << " too small to cast";

		throw std::out_of_range(msg.str());
	}

	if (value > std::numeric_limits<S>::max())
	{
		std::ostringstream msg;
		msg << "Value " <<  value << " too big to cast";

		throw std::out_of_range(msg.str());
	}

	return static_cast<S>(value);
}

/// @}

} // namespace v_1_0_0
} // namespace arcsdec

#endif

