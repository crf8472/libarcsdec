#ifndef LIBARCSDEC_PARSERTOC_HPP_
#error "Do not include parsertoc_details.hpp, include parsertoc.hpp instead"
#endif
#ifndef LIBARCSDEC_PARSERTOC_DETAILS_HPP_
#define LIBARCSDEC_PARSERTOC_DETAILS_HPP_

/**
 * \internal
 *
 * \file
 *
 * \brief Implementation details of parsertoc.hpp.
 */

#include <memory>   // for unique_ptr
#include <string>   // for string

#ifndef LIBARCSDEC_DESCRIPTOR_HPP_
#include "descriptor.hpp"        // for FileReaderDescriptor
#endif
#ifndef LIBARCSDEC_METAPARSER_HPP_
#include "metaparser.hpp"        // for MetaparserImpl
#endif


namespace arcsdec
{
                                                  /** \cond NAMESPACE_v_1_0_0 */
inline namespace v_1_0_0
{
                                                                 /** \endcond */
namespace read
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

using arcsdec::read::FileReaderDescriptor;
using arcsdec::read::MetadataParserImpl;

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
} // namespace read
                                                  /** \cond NAMESPACE_v_1_0_0 */
} // namespace v_1_0_0
                                                                 /** \endcond */
} // namespace arcsdec

#endif

