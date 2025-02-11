#ifndef __LIBARCSDEC_PARSERCUE_HPP__
#error "Do not include parsercue_details.hpp, include parsercue.hpp instead"
#endif

/**
 * \file
 *
 * \brief Internal APIs for Cuesheet reader
 */

#ifndef __LIBARCSDEC_PARSERCUE_DETAILS_HPP__
#define __LIBARCSDEC_PARSERCUE_DETAILS_HPP__

#include <cstdint>  // for uint16_t, int32_t
#include <memory>   // for unique_ptr
#include <string>   // for string
#include <tuple>    // for tuple
#include <vector>   // for vector

#ifndef __LIBARCSDEC_DESCRIPTOR_HPP__
#include "descriptor.hpp"       // for FileReaderDescriptor
#endif
#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include "metaparser.hpp"        // for MetaparserImpl
#endif

namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{

/**
 * \brief Namespace for implementation details of parsercue
 */
namespace cuesheet
{

using arcstk::ToC;


/**
 * \internal \defgroup parserCueImpl Implementation details of Cuesheet parsing
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
 * \brief Implementation for reading Cuesheets.
 */
class CuesheetParserImpl final : public MetadataParserImpl
{
private:

	/**
	 * \brief Return Cuesheet data.
	 *
	 * \param[in] filename Name of the file to read.
	 *
	 * \return The CueInfo of the parsed Cuesheet
	 *
	 * \throw FileReadException If the file could not be read
	 */
	//Cuesheet parse_worker(const std::string& filename) const;

	std::unique_ptr<ToC> do_parse(const std::string& filename) final;

	std::unique_ptr<FileReaderDescriptor> do_descriptor() const final;
};

/// @}


} // namespace cuesheet
} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

#endif

