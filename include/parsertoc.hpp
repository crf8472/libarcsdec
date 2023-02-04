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

#ifndef __LIBARCSDEC_DESCRIPTORS_HPP__
#include "descriptors.hpp"// for Codec, Format, FileReaderDescriptor, FileReader
#endif


namespace arcsdec
{
inline namespace v_1_0_0
{

/**
 * \internal \defgroup parsertoc Metadata: CDRDAO/TOC
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
	 * \brief Constructor.
	 */
	DescriptorToc()
		: FileReaderDescriptor { { "toc" } }
	{ /* empty */ }

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~DescriptorToc() noexcept override;

	/**
	 * \brief Returns "CDRDAO/TOC".
	 *
	 * \return "CDRDAO/TOC"
	 */
	std::string do_name() const override;

	/**
	 * \brief Always returns TRUE since CDRDAO/TOC cannot be recognized by a
	 * certain byte sequence in a certain offset.
	 *
	 * \param[in] bytes  (ignored)
	 * \param[in] offset (ignored)
	 *
	 * \return TRUE
	 */
	bool do_accepts_bytes(const std::vector<unsigned char> &bytes,
			const uint64_t &offset) const override;

	std::set<Format> define_formats() const override;

	bool do_accepts_codec(Codec /* codec */) const override { return false; };

	std::set<Codec> do_codecs() const override { return {}; };

	LibInfo do_libraries() const override;

	std::unique_ptr<FileReader> do_create_reader() const override;

	std::unique_ptr<FileReaderDescriptor> do_clone() const override;
};

/// @}

} // namespace v_1_0_0
} // namespace arcsdec

#endif

