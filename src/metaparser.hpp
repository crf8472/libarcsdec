/**
 * \file metaparser.hpp An interface for parsing TOC informations
 *
 */


#ifndef __LIBARCSDEC_METAPARSER_HPP__
#define __LIBARCSDEC_METAPARSER_HPP__

#include <memory>
#include <string>

#ifndef __LIBARCS_CALCULATE_HPP__
#include <arcs/calculate.hpp>
#endif

#ifndef __LIBARCSDEC_FILEFORMATS_HPP__
#include "fileformats.hpp"
#endif


namespace arcs
{

/**
 * \internal \defgroup metaparser Level 0 API: Reading and Validating Metadata Files
 *
 * \brief Interface for implementing and creating MetadataParsers
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
 * Implementation of a MetadataParser
 */
class MetadataParserImpl
{

public:

	/**
	 * Virtual default destructor
	 */
	virtual ~MetadataParserImpl() noexcept;

	/**
	 * Parses a metadata file
	 *
	 * \param[in] filename The file to parse
	 *
	 * \return The TOC information represented by the file
	 *
	 * \throw FileReadException      If the file could not be read
	 * \throw MetadataParseException If the metadata could not be parsed
	 */
	std::unique_ptr<TOC> parse(const std::string &filename);


protected:

	/**
	 * Implements parse()
	 *
	 * \param[in] filename The file to parse
	 *
	 * \return The TOC information represented by the file
	 *
	 * \throw FileReadException      If the file could not be read
	 * \throw MetadataParseException If the metadata could not be parsed
	 */
	virtual std::unique_ptr<TOC> do_parse(const std::string &filename) = 0;
};


/**
 * Interface for MetadataParsers
 */
class MetadataParser : public FileReader
{

public:

	/**
	 * Constructor
	 *
	 * \param[in] impl The concrete implementation of the MetadataParser
	 */
	MetadataParser(std::unique_ptr<MetadataParserImpl> impl);

	/**
	 * Default destructor
	 */
	virtual ~MetadataParser() noexcept;

	/**
	 * Parses a metadata file
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
	 * Implementation of a MetadataParser
	 */
	std::unique_ptr<MetadataParserImpl> impl_;
};


/**
 * Reports an error when encountering unexpected content while parsing a
 * metadata file.
 */
class MetadataParseException final : public std::runtime_error
{

public:

	/**
	 * Constructor
	 *
	 * \param[in] what_arg What argument
	 */
	explicit MetadataParseException(const std::string &what_arg);
};


/**
 * A builder class for MetadataParsers
 */
class MetadataParserCreator : public FileReaderCreator
{

public:

	/**
	 * Constructor
	 */
	MetadataParserCreator();

	/**
	 * Virtual default destructor
	 */
	virtual ~MetadataParserCreator() noexcept;

	/**
	 * Create a MetadataParser for the specified file
	 *
	 * \param[in] metafilename The filename to create MetadataParser for
	 *
	 * \return A MetadataParser for the specified file
	 *
	 * \throw FileReadException If the file could not be read
	 */
	std::unique_ptr<MetadataParser> create_metadata_parser(
			const std::string &metafilename) const;
};

/// @}

} // namespace arcs

#endif

