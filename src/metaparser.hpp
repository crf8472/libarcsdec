/**
 * \file metaparser.hpp An interface for parsing TOC informations
 *
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


namespace arcs
{

inline namespace v_1_0_0
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
 * \brief Implementation of a MetadataParser
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


private:

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
	virtual std::unique_ptr<TOC> do_parse(const std::string &filename)
	= 0;
};


/**
 * \brief Interface for MetadataParsers
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
 * \brief Reports unexpected content while parsing a metadata file.
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
 * \brief Selects and builds @link MetadataParser MetadataParsers @endlink for given inputs.
 */
class MetadataParserSelection : public FileReaderSelection
{

public:

	/**
	 * Constructor
	 */
	MetadataParserSelection();

	/**
	 * Virtual default destructor
	 */
	~MetadataParserSelection() noexcept override;

	/**
	 * Create a MetadataParser for the specified file
	 *
	 * \param[in] metafilename The filename to create MetadataParser for
	 *
	 * \return A MetadataParser for the specified file
	 *
	 * \throw FileReadException If the file could not be read
	 */
	std::unique_ptr<MetadataParser> for_file(const std::string &metafilename)
		const;

	/**
	 * Return the MetadataParser specified by its name.
	 *
	 * If the selection does not contain a MetadataParser with the specified name,
	 * \c nullptr will be returned.
	 *
	 * \param[in] name The name of the MetadataParser.
	 *
	 * \return A MetadataParser with the specified name
	 */
	std::unique_ptr<MetadataParser> by_name(const std::string &name)
		const;


protected:

	/**
	 * Turns a FileReader to a MetadataParser.
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

} // namespace arcs

#endif

