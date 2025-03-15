#ifndef __LIBARCSDEC_PARSERTOC_HPP__
#error "Do not include parsertoc_details.hpp, include parsertoc.hpp instead"
#endif
#ifndef __LIBARCSDEC_PARSERTOC_DETAILS_HPP__
#define __LIBARCSDEC_PARSERTOC_DETAILS_HPP__

/**
 * \internal
 *
 * \file
 *
 * \brief Implementation details of parsertoc.hpp.
 */

#ifndef __LIBARCSDEC_DESCRIPTOR_HPP__
#include "descriptor.hpp"        // for FileReaderDescriptor
#endif
#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include "metaparser.hpp"        // for MetaparserImpl
#endif

#include <memory>   // for unique_ptr
#include <string>   // for string


namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{

/**
 * \internal
 *
 * \brief Implementation details of parsertoc.
 */
namespace cdrtoc
{

using arcstk::ToC;


/**
 * \internal
 *
 * \defgroup parserTocImpl Implementation details of CDRDAO/TOC parsing
 *
 * \ingroup parsertoc
 *
 * @{
 */

/**
 * \brief Implementation for parsing of CDRDAO/TOC files.
 */
class TocParserImpl final : public MetadataParserImpl
{
	std::unique_ptr<ToC> do_parse(const std::string& filename) final;

	std::unique_ptr<FileReaderDescriptor> do_descriptor() const final;
};

/// @}


} // namespace cdrtoc
} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

#endif

