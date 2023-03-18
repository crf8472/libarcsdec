/**
 * \file
 *
 * \brief Parser for metadata files in cdrdao toc format.
 */


#ifndef __LIBARCSDEC_PARSERTOC_HPP__
#define __LIBARCSDEC_PARSERTOC_HPP__

#include <cstdint>  // for uint64_t
#include <memory>   // for unique_ptr
#include <set>      // for set
#include <string>   // for string
#include <vector>   // for vector

#ifndef __LIBARCSDEC_DESCRIPTOR_HPP__
#include "descriptor.hpp"// for Codec, Format, FileReaderDescriptor, FileReader
#endif


namespace arcsdec
{
inline namespace v_1_0_0
{

/**
 * \internal
 * \defgroup parsertoc Metadata: CDRDAO/TOC
 *
 * \brief A MetadataParser for CDRDAO/TOC files
 *
 * @{
 */


/**
 * \brief Libcdio-based parser for CDRDAO's .toc files.
 */
class DescriptorToc : public FileReaderDescriptor
{

public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~DescriptorToc() noexcept override;


private:

	std::string do_id() const override;

	/**
	 * \brief Returns "CDRDAO/TOC".
	 *
	 * \return "CDRDAO/TOC"
	 */
	std::string do_name() const override;

	bool do_accepts_codec(Codec /* codec */) const override;

	std::set<Format> define_formats() const override;

	LibInfo do_libraries() const override;

	std::unique_ptr<FileReader> do_create_reader() const override;

	std::unique_ptr<FileReaderDescriptor> do_clone() const override;
};

/// @}

} // namespace v_1_0_0
} // namespace arcsdec

#endif

