#ifndef __LIBARCSDEC_PARSERLIBCUE_HPP__
#define __LIBARCSDEC_PARSERLIBCUE_HPP__

/**
 * \file
 *
 * \brief Parser for Cuesheets, implemented with libcue.
 */

#ifndef __LIBARCSDEC_DESCRIPTOR_HPP__
#include "descriptor.hpp"// for Codec, Format, FileReaderDescriptor, FileReader
#endif

#include <cstdint>  // for uint64_t
#include <memory>   // for unique_ptr
#include <set>      // for set
#include <string>   // for string
#include <vector>   // for vector


namespace arcsdec
{
inline namespace v_1_0_0
{

/**
 * \internal
 *
 * \defgroup parserlibcue Metadata: CueSheet
 *
 * \brief A MetadataParser for CueSheet files
 *
 * @{
 */


/**
 * \brief Libcue-based parser for Cuesheets.
 */
class DescriptorCue final : public FileReaderDescriptor
{

public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~DescriptorCue() noexcept final;


private:

	std::string do_id() const final;

	/**
	 * \brief Returns "CueSheet".
	 *
	 * \return "CueSheet"
	 */
	std::string do_name() const final;

	InputType do_input_type() const final;

	bool do_accepts_codec(Codec /* codec */) const final;

	std::set<Format> define_formats() const final;

	LibInfo do_libraries() const final;

	std::unique_ptr<FileReader> do_create_reader() const final;

	std::unique_ptr<FileReaderDescriptor> do_clone() const final;
};

/// @}

} // namespace v_1_0_0
} // namespace arcsdec

#endif

