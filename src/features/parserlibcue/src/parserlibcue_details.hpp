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

#include <cstdint>  // for uintmax_t
#include <memory>   // for unique_ptr
#include <optional> // for optional
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
 * \brief Get file size of a file.
 *
 * \param[in] filepath Filepath of the file
 *
 * \return Size of the file
 */
std::uintmax_t file_size_or_throw(const std::string &filepath);
// TODO This may be provided for other features

/**
 * \brief Load a file in text mode that is not bigger than \c max_size.
 *
 * \param[in] filepath Filepath to load
 * \param[in] max_size Maximal file size in bytes
 *
 * \return Content of the file on success
 *
 * \throws runtime_error On failure
 */
std::optional<std::string> file_content(const std::string &filepath,
		const std::uintmax_t max_size);
// TODO This may be provided for other features

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
 * \brief Convert a CdPtr (libcue) to a ToC (libarcstk).
 *
 * \param[in] cd CdPtr to convert
 *
 * \return ToC representing information from CdPtr
 */
ToC convert(const CdPtr& cd);

/**
 * \brief Implementation for libcue-based reading of CueSheets.
 */
class LibcueParserImpl final : public MetadataParserImpl
{
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

