#ifndef LIBARCSDEC_PARSERLIBCUE_HPP_
#error "Do not include parserlibcue_details.hpp, include parserlibcue.hpp instead"
#endif
#ifndef LIBARCSDEC_PARSERLIBCUE_DETAILS_HPP_
#define LIBARCSDEC_PARSERLIBCUE_DETAILS_HPP_

/**
 * \internal
 *
 * \file
 *
 * \brief Implementation details of parserlibcue.hpp.
 */

extern "C" {
#include <libcue/libcue.h>  // for Cd, cd_delete
}

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
namespace read::details // NOLINT(modernize-concat-nested-namespaces)
{

/**
 * \internal
 *
 * \brief Implementation details of parserlibcue.
 */
namespace libcue
{

using arcstk::ToC;


/**
 * \internal
 *
 * \defgroup parserlibcueImpl Implementation
 *
 * \ingroup parserlibcue
 *
 * @{
 */

/**
 * \brief Functor for freeing Cd* instances.
 */
struct Free_Cd final
{
	void operator()(::Cd* cd) const
	{
		if (cd)
		{
			::cd_delete(cd);
			cd = nullptr;
		}
	}
};

/**
 * \brief A unique_ptr for Cd using Free_Cd as a custom deleter.
 */
using CdPtr = std::unique_ptr<::Cd, Free_Cd>;

/**
 * \brief Implementation for libcue-based reading of CueSheets.
 */
class LibcueParserImpl final : public MetadataParserImpl
{
	/**
	 * \brief Convert a CdPtr (libcue) to a ToC (libarcstk).
	 *
	 * \param[in] cd CdPtr to convert
	 *
	 * \return ToC representing information from CdPtr
	 */
	ToC convert(const CdPtr& cd) const;

	/**
	 * \brief Parse Cuesheet file to a ToC using libcue.
	 *
	 * \param[in] filename Name of the file to read.
	 *
	 * \return ToC of the parsed Cuesheet
	 *
	 * \throw runtime_error          If the file could not be read
	 * \throw MetadataParseException If the file is zero or parsing failed
	 */
	ToC parse_worker(const std::string& filename) const;

	// MetadataParserImpl

	ToC do_parse(const std::string& filename) final;

	std::unique_ptr<FileReaderDescriptor> do_descriptor() const final;
};

/// @}


} // namespace libcue
} // namespace read::details
                                                  /** \cond NAMESPACE_v_1_0_0 */
} // namespace v_1_0_0
                                                                 /** \endcond */
} // namespace arcsdec

#endif

