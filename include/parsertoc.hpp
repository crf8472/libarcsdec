#ifndef __LIBARCSDEC_PARSERTOC_HPP__
#define __LIBARCSDEC_PARSERTOC_HPP__

/**
 * \file
 *
 * \brief Parser for cdrdao's toc files, implemented with libcdio.
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
 * \brief A MetadataParser for CDRDAO/TOC files, implemented with libcdio.
 */
class DescriptorToc final : public FileReaderDescriptor
{
public:

	/**
	 * \brief Default destructor.
	 */
	~DescriptorToc() noexcept final;


private:

	std::string do_id() const final;

	/**
	 * \brief Returns "CDRDAO/TOC".
	 *
	 * \return "CDRDAO/TOC"
	 */
	std::string do_name() const final;

	InputType do_input_type() const final;

	bool do_accepts_codec(Codec /* codec */) const final;

	std::set<Format> define_formats() const final;

	LibInfo do_libraries() const final;

	std::unique_ptr<FileReader> do_create_reader() const final;

	std::unique_ptr<FileReaderDescriptor> do_clone() const final;
};

} // namespace v_1_0_0
} // namespace arcsdec

#endif

