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
#include <libcue/libcue.h>  // for Cd
}

#include <cstdint>  // for uint16_t, int32_t
#include <memory>   // for unique_ptr
#include <optional> // for optional
#include <string>   // for string
#include <tuple>    // for tuple
#include <vector>   // for vector

#ifndef LIBARCSDEC_DESCRIPTOR_HPP_
#include "descriptor.hpp"       // for FileReaderDescriptor
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
	void operator()(::Cd* cd) const;
};


/**
 * \brief A unique_ptr for Cd using Free_Cd as a custom deleter.
 */
using CdPtr = std::unique_ptr<::Cd, Free_Cd>;


/**
 * \brief Construction functor for CdPtr instances.
 */
// struct Make_CdPtr final
// {
// 	CdPtr operator()(const std::string& filename) const;
// };


/**
 * \brief Close FILE instances.
 */
// struct Close_FILEPtr final
// {
// 	void operator()(FILE* f) const;
// };


/**
 * \brief A unique_ptr for FILE using Close_FILEPtr as a custom deleter.
 */
//using FILEPtr = std::unique_ptr<FILE, Close_FILEPtr>;


/**
 * \brief Open \c filename and return a handle.
 *
 * \param[in] filename Name of the file to open
 *
 * \return Handle to \c filename
 */
//FILEPtr safe_open_for_read(const std::string& filename);


/**
 * \brief Convert a CdPtr (libcue) to a ToC (libarcstk).
 *
 * \param[in] cd CdPtr to convert
 *
 * \return ToC representing information from CdPtr
 */
ToC convert(const CdPtr& cd);


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
std::optional<std::vector<char>> file_content(const std::string &filepath,
		const std::uintmax_t max_size);


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

