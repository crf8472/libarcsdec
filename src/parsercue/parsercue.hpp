#ifndef LIBARCSDEC_PARSERCUE_HPP_
#define LIBARCSDEC_PARSERCUE_HPP_

/**
 * \file
 *
 * \brief Parser for Cuesheets.
 */

#include <memory>   // for unique_ptr
#include <set>      // for set
#include <string>   // for string

#ifndef LIBARCSDEC_DESCRIPTOR_HPP_
#include "descriptor.hpp"// for Codec, Format, FileReaderDescriptor, FileReader
#endif


namespace arcsdec
{
                                                  /** \cond NAMESPACE_v_1_0_0 */
inline namespace v_1_0_0
{
                                                                 /** \endcond */
namespace read
{

/**
 * \internal
 *
 * \defgroup parsercue libarcsdec's Cuesheet implementation
 *
 * \ingroup metaparser
 *
 * @{
 */

/**
 * \internal
 *
 * \brief A MetadataParser for Cuesheet files.
 */
class DescriptorCuesheet final : public FileReaderDescriptor
{
public:

	/**
	 * \brief Default destructor.
	 */
	~DescriptorCuesheet() noexcept final;


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

} // namespace read
                                                  /** \cond NAMESPACE_v_1_0_0 */
} // namespace v_1_0_0
                                                                 /** \endcond */
} // namespace arcsdec

#endif

