#ifndef LIBARCSDEC_PARSERLIBCUE_HPP__
#define LIBARCSDEC_PARSERLIBCUE_HPP__

/**
 * \file
 *
 * \brief Parser for Cuesheets, implemented with libcue.
 */

#include <memory>   // for unique_ptr
#include <set>      // for set
#include <string>   // for string

#ifndef LIBARCSDEC_DESCRIPTOR_HPP__
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
 * \defgroup parserlibcue Features based on libcue
 *
 * \ingroup metaparser
 *
 * @{
 */

/**
 * \internal
 *
 * \brief A MetadataParser for Cuesheet files, implemented with libcue 2.
 */
class DescriptorCue final : public FileReaderDescriptor
{
public:

	/**
	 * \brief Default destructor.
	 */
	~DescriptorCue() noexcept final;


private:

	std::string do_id() const final;

	/**
	 * \brief Returns "Libcue".
	 *
	 * \return "Libcue"
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

