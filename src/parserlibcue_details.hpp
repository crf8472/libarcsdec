#ifndef __LIBARCSDEC_PARSERLIBCUE_HPP__
#error "Do not include parserlibcue_details.hpp, include parserlibcue.hpp instead"
#endif

/**
 * \file
 *
 * \brief Internal APIs for libcue-based CueSheet reader
 */

#ifndef __LIBARCSDEC_PARSERLIBCUE_DETAILS_HPP__
#define __LIBARCSDEC_PARSERLIBCUE_DETAILS_HPP__

#include <cstdint>  // for uint16_t, int32_t
#include <memory>   // for unique_ptr
#include <string>   // for string
#include <tuple>    // for tuple
#include <vector>   // for vector

extern "C" {
#include <libcue/libcue.h>  // for Cd
}

#ifndef __LIBARCSDEC_DESCRIPTOR_HPP__
#include "descriptor.hpp"       // for FileReaderDescriptor
#endif
#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include "metaparser.hpp"        // for MetaparserImpl
#endif

#ifndef __LIBARCSTK_METADATA_HPP__
#include <arcstk/metadata.hpp>   // for ToC
#endif

namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{
namespace libcue
{

using arcstk::ToC;


/**
 * \internal \defgroup parserCueImpl Implementation details of CueSheet parsing
 *
 * \ingroup parserlibcue
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
 * \brief Type for raw Cue data.
 */
using CueInfo = std::tuple<uint16_t, // track count
	std::vector<lba_type>,           // offsets
	std::vector<lba_type>,           // lengths
	std::vector<std::string>>;       // filenames


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
struct Make_CdPtr final
{
	CdPtr operator()(const std::string& filename) const;
};


/**
 * \brief Represents an opened Cuesheet file.
 *
 * Instances of this class are non-copyable but movable.
 */
class CueOpenFile final
{
public:

	/**
	 * \brief Open Cuesheet with the given name.
	 *
	 * \param[in] filename The Cuesheet file to read
	 *
	 * \throw FileReadException      If the CueSheet file could not be read
	 * \throw MetadataParseException If the Cue data could not be parsed
	 */
	explicit CueOpenFile(const std::string& filename);

	CueOpenFile(CueOpenFile&& file) noexcept;
	CueOpenFile& operator = (CueOpenFile&& file) noexcept;

	/**
	 * \brief Returns all ToC information from the file.
	 *
	 * \return CueInfo representing the ToC information
	 */
	CueInfo info() const;

private:

	/**
	 * \brief Internal libcue-based representation.
	 */
	CdPtr cd_info_;
};


/**
 * \brief Implementation for libcue-based reading of CueSheets.
 */
class CueParserImpl final : public MetadataParserImpl
{
private:

	/**
	 * \brief Return Cue data.
	 *
	 * \param[in] filename Name of the file to read.
	 *
	 * \return The CueInfo of the parsed Cuesheet
	 *
	 * \throw FileReadException If the file could not be read
	 */
	CueInfo parse_worker(const std::string& filename) const;

	std::unique_ptr<ToC> do_parse(const std::string& filename) override;

	std::unique_ptr<FileReaderDescriptor> do_descriptor() const override;
};

/// @}


} // namespace libcue
} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

#endif

