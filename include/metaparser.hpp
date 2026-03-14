#ifndef LIBARCSDEC_METAPARSER_HPP_
#define LIBARCSDEC_METAPARSER_HPP_

/**
 * \file
 *
 * \brief Implement \link arcsdec::read::MetadataParser MetadataParsers\endlink.
 */

#include <memory>       // for unique_ptr
#include <string>       // for string

#ifndef LIBARCSTK_METADATA_HPP_
#include <arcstk/metadata.hpp>    // for ToC
#endif

#ifndef LIBARCSDEC_DESCRIPTOR_HPP_
#include "descriptor.hpp"  // for FileReader, FileReaderDescriptor
#endif


namespace arcsdec
{
                                                  /** \cond NAMESPACE_v_1_0_0 */
inline namespace v_1_0_0
{
                                                                 /** \endcond */
namespace read
{

/**
 * \defgroup metaparser Implement MetadataParsers
 *
 * \brief Implement \link MetadataParser MetadataParsers\endlink.
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


using arcstk::ToC;


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
	std::unique_ptr<ToC> parse(const std::string& filename);

	/**
	 * \brief Create a descriptor for this MetadataParser implementation.
	 *
	 * \return Descriptor for this implementation.
	 */
	std::unique_ptr<FileReaderDescriptor> descriptor() const;

protected:

	MetadataParserImpl(MetadataParserImpl&&) noexcept;
	MetadataParserImpl& operator = (MetadataParserImpl&&) noexcept;

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
	virtual std::unique_ptr<ToC> do_parse(const std::string& filename)
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

	MetadataParser(MetadataParser&&) noexcept;
	MetadataParser& operator = (MetadataParser&&) noexcept;

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
	std::unique_ptr<ToC> parse(const std::string& filename);

private:

	/**
	 * \brief Implementation of a MetadataParser.
	 */
	std::unique_ptr<MetadataParserImpl> impl_;

	std::unique_ptr<FileReaderDescriptor> do_descriptor() const final;
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
	explicit MetadataParseException(const std::string& what_arg);
};

/// @}

} // namespace read
                                                  /** \cond NAMESPACE_v_1_0_0 */
} // namespace v_1_0_0
                                                                 /** \endcond */
} // namespace arcsdec

#endif

