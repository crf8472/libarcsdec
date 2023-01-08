#ifndef __LIBARCSDEC_PARSERCUE_HPP__
#error "Do not include parsercue_details.hpp, include parsercue.hpp instead"
#endif

/**
 * \file
 *
 * \brief Internal APIs for libcue-based CUESheet reader
 */

#ifndef __LIBARCSDEC_PARSERCUE_DETAILS_HPP__
#define __LIBARCSDEC_PARSERCUE_DETAILS_HPP__

#include <cstdint>  // for uint16_t, int32_t
#include <memory>   // for unique_ptr
#include <string>   // for string
#include <tuple>    // for tuple
#include <vector>   // for vector

extern "C" {
#include <libcue/libcue.h>  // for Cd
}

#ifndef __LIBARCSDEC_DESCRIPTORS_HPP__
#include "descriptors.hpp"       // for FileReaderDescriptor
#endif
#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include "metaparser.hpp"        // for MetaparserImpl
#endif

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp> // for TOC
#endif

namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{
namespace libcue
{

using arcstk::TOC;


/**
 * \internal \defgroup parserCueImpl Implementation details of CUESheet parsing
 *
 * \ingroup parsercue
 *
 * @{
 */


/**
 * \brief Type for amounts of lba frames.
 *
 * This type is a signed integral type.
 */
using lba_type = int32_t;


/**
 * \brief Type for raw CUE data.
 */
using CueInfo = std::tuple<uint16_t, // track count
	std::vector<lba_type>,           // offsets
	std::vector<lba_type>,           // lengths
	std::vector<std::string>>;       // filenames


/**
 * \brief Functor for freeing Cd* instances.
 */
struct FreeCd final
{
	void operator()(::Cd* cd) const;
};


/**
 * \brief A unique_ptr for Cd using FreeCd as a custom deleter.
 */
using CdPtr = std::unique_ptr<::Cd, FreeCd>;


/**
 * \brief Represents an opened CUEsheet file.
 *
 * Instances of this class are non-copyable but movable.
 */
class CueOpenFile final
{
public:

	/**
	 * \brief Open CUEsheet with the given name.
	 *
	 * \param[in] filename The CUEsheet file to read
	 *
	 * \throw FileReadException      If the CUESheet file could not be read
	 * \throw MetadataParseException If the CUE data could not be parsed
	 */
	explicit CueOpenFile(const std::string &filename);

	CueOpenFile(CueOpenFile &&file) noexcept;
	CueOpenFile& operator = (CueOpenFile &&file) noexcept;

	/**
	 * \brief Returns all TOC information from the file.
	 *
	 * \return CueInfo representing the TOC information
	 */
	CueInfo parse_info();

private:

	/**
	 * \brief Internal libcue-based representation.
	 */
	CdPtr cd_info_;
};


/**
 * \brief Implementation for libcue-based reading of CUESheets.
 */
class CueParserImpl final : public MetadataParserImpl
{
public:

	/**
	 * \brief Return CUE data.
	 *
	 * \param[in] filename Name of the file to read.
	 *
	 * \return The CueInfo of the parsed CUEsheet
	 *
	 * \throw FileReadException If the file could not be read
	 */
	CueInfo parse_worker(const std::string &filename);

private:

	std::unique_ptr<TOC> do_parse(const std::string &filename) override;

	std::unique_ptr<FileReaderDescriptor> do_descriptor() const override;
};

/// @}


} // namespace libcue
} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

#endif

