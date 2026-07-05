#ifndef LIBARCSDEC_PARSERCUE_HPP_
#error "Do not include parsercue_details.hpp, include parsercue.hpp instead"
#endif
#ifndef LIBARCSDEC_PARSERCUE_DETAILS_HPP_
#define LIBARCSDEC_PARSERCUE_DETAILS_HPP_

/**
 * \internal
 *
 * \file
 *
 * \brief Implementation details of parsercue.hpp.
 */

#include <cstdint>  // for int32_t
#include <memory>   // for unique_ptr
#include <string>   // for string

#ifndef LIBARCSDEC_DESCRIPTOR_HPP_
#include "descriptor.hpp"       // for FileReaderDescriptor
#endif
#ifndef LIBARCSDEC_METAPARSER_HPP_
#include "metaparser.hpp"       // for MetaparserImpl
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
 * \brief Implementation details of parsercue.
 */
namespace cuesheet
{

using arcstk::ToC;


/**
 * \internal
 *
 * \defgroup parsercueImpl Implementation details of Cuesheet parsing
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

	ToC do_parse(const std::string& filename) final;

	std::unique_ptr<FileReaderDescriptor> do_descriptor() const final;
};

/// @}


} // namespace cuesheet
} // namespace details
} // namespace read
                                                  /** \cond NAMESPACE_v_1_0_0 */
} // namespace v_1_0_0
                                                                 /** \endcond */
} // namespace arcsdec

#endif

