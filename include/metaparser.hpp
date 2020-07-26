/**
 * \file
 *
 * \brief An interface for parsing TOC informations
 */


#ifndef __LIBARCSDEC_METAPARSER_HPP__
#define __LIBARCSDEC_METAPARSER_HPP__

#include <memory>
#include <string>

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>
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
 * \internal
 * \defgroup metaparser API for parsing metadata/toc files
 *
 * \brief Interface for implementing and creating
 * \link MetadataParser MetadataParsers\endlink.
 *
 * The interface for reading audio files is provided by class MetadataParser
 * that internally holds a concrete instance of MetadataParserImpl.
 *
 * The MetadataParser provides the parse operation on the input file that yields
 * a TOC instance. The information represented by the TOC instance undergoes
 * some basic consistency checks while constructing the TOC object.
 *
 * The concrete reading of a given metadata file is implemented by the
 * subclasses of MetadataParserImpl.
 *
 * @{
 */


/**
 * \brief Implementation of a MetadataParser.
 */
class MetadataParserImpl
{

public:

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
 * \brief Interface for MetadataParsers
 */
class MetadataParser : public FileReader
{
public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] impl The concrete implementation of the MetadataParser
	 */
	MetadataParser(std::unique_ptr<MetadataParserImpl> impl);

	/**
	 * \brief Default destructor.
	 */
	virtual ~MetadataParser() noexcept override;

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


/**
 * \brief Selects and builds
 * @link MetadataParser MetadataParsers @endlink for given inputs.
 */
class MetadataParserSelection : public FileReaderSelection
{
public:

	/**
	 * \brief Constructor.
	 */
	MetadataParserSelection();

	/**
	 * \brief Virtual default destructor.
	 */
	~MetadataParserSelection() noexcept override;

	/**
	 * \brief Create a MetadataParser for the specified file.
	 *
	 * \param[in] metafilename The filename to create MetadataParser for
	 *
	 * \return A MetadataParser for the specified file
	 *
	 * \throw FileReadException If the file could not be read
	 */
	std::unique_ptr<MetadataParser> for_file(const std::string &metafilename)
		const;

protected:

	/**
	 * \brief Turns a FileReader to a MetadataParser.
	 *
	 * \param[in] file_reader_uptr The FileReader to cast
	 *
	 * \return MetadataParser or nullptr
	 */
	std::unique_ptr<MetadataParser> safe_cast(
		std::unique_ptr<FileReader> file_reader_uptr) const;
};

/// @}

} // namespace v_1_0_0

} // namespace arcsdec

#endif

