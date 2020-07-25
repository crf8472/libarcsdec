/**
 * \file
 *
 * \brief Parser for CUE sheets
 */


#ifndef __LIBARCSDEC_PARSERCUE_HPP__
#define __LIBARCSDEC_PARSERCUE_HPP__

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#ifndef __LIBARCSDEC_DESCRIPTORS_HPP__
#include "descriptors.hpp"
#endif
#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include "metaparser.hpp"
#endif


namespace arcsdec
{

inline namespace v_1_0_0
{

/**
 * \internal \defgroup parsercue Metadata: CUESheet
 *
 * \brief A MetadataParser for CUESheet files
 *
 * @{
 */


/**
 * \brief Libcue-based parser for CUEsheets.
 */
class DescriptorCUE : public FileReaderDescriptor
{

public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~DescriptorCUE() noexcept override;

	/**
	 * \brief Returns "CUESheet".
	 *
	 * \return "CUESheet"
	 */
	std::string do_name() const override;

	LibInfo do_libraries() const override;

	/**
	 * \brief Always returns TRUE since CUESheets cannot be recognized by a
	 * certain byte sequence in a certain offset.
	 *
	 * \param[in] bytes  (ignored)
	 * \param[in] offset (ignored)
	 *
	 * \return TRUE
	 */
	bool do_accepts_bytes(const std::vector<char> &bytes,
			const uint64_t &offset) const override;

	/**
	 * \brief Returns TRUE if the suffix matches a CUE sheet suffix.
	 *
	 * \return TRUE iff suffix is case-insensitively equal to suffix otherwise
	 * FALSE
	 */
	//bool do_accepts_suffix(const std::string &suffix) const override;

	bool do_accepts(FileFormat format) const override;

	std::set<FileFormat> do_formats() const override;

	std::unique_ptr<FileReader> do_create_reader() const override;

	std::unique_ptr<FileReaderDescriptor> do_clone() const override;
};

/// @}

} // namespace v_1_0_0

} // namespace arcsdec

#endif

