#ifndef __LIBARCSDEC_PARSERCUE_HPP__
#define __LIBARCSDEC_PARSERCUE_HPP__

/**
 * \file
 *
 * \brief Parser for Cuesheets.
 */

#ifndef __LIBARCSDEC_DESCRIPTOR_HPP__
#include "descriptor.hpp"// for Codec, Format, FileReaderDescriptor, FileReader
#endif

#include <memory>   // for unique_ptr
#include <set>      // for set
#include <string>   // for string


namespace arcsdec
{
inline namespace v_1_0_0
{

/**
 * \internal
 *
 * \defgroup parsercue Metadata: Cuesheet
 *
 * \brief A MetadataParser for Cuesheet files
 *
 * @{
 */

/**
 * \brief Parser for Cuesheets.
 */
class DescriptorCuesheet final : public FileReaderDescriptor
{

public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~DescriptorCuesheet() noexcept final;


private:

	std::string do_id() const final;

	/**
	 * \brief Returns "Cuesheet".
	 *
	 * \return "Cuesheet"
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

