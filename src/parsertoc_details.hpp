#ifndef __LIBARCSDEC_PARSERTOC_HPP__
#error "Do not include parsertoc_details.hpp, include parsertoc.hpp instead"
#endif

/**
 * \file
 *
 * \brief Internal APIs for libcdio-based CDRDAO/TOC reader.
 */

#ifndef __LIBARCSDEC_PARSERTOC_DETAILS_HPP__
#define __LIBARCSDEC_PARSERTOC_DETAILS_HPP__

#include <cstdint>  // for uint16_t, int32_t
#include <memory>   // for unique_ptr
#include <string>   // for string
#include <tuple>    // for tuple
#include <vector>   // for vector

//extern "C" {
#include <cdio++/cdio.hpp>  // for
//}

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
namespace cdrdao
{

using arcstk::TOC;


/**
 * \internal \defgroup parserTocImpl Implementation details of CDRDAO/TOC parsing
 *
 * \ingroup parsertoc
 *
 * @{
 */


/**
 * \brief Implementation for libcdio-based reading of CDRDAO/TOC files.
 */
class TocParserImpl final : public MetadataParserImpl
{
public:

private:

	std::unique_ptr<TOC> do_parse(const std::string &filename) override;

	std::unique_ptr<FileReaderDescriptor> do_descriptor() const override;
};

/// @}


} // namespace cdrdao
} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

#endif

