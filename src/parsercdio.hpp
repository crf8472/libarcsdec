#ifndef __LIBARCSDEC_PARSERCDIO_HPP__
#define __LIBARCSDEC_PARSERCDIO_HPP__

/**
 * \file parsercdio.hpp Parser for CD device
 */

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
 * \internal \defgroup parsercdio Metadata: physical CD
 *
 * \brief A MetadataParser for physical CDs
 *
 * @{
 */


/**
 * \brief Libcdio-based parser for physical CDs and certain images
 */
class DescriptorCdio : public FileReaderDescriptor
{

public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~DescriptorCdio() noexcept;

	/**
	 * \brief Returns "physical device".
	 *
	 * \return "physical device"
	 */
	std::string do_name() const;

	/**
	 * \brief Always returns TRUE since CDs cannot be recognized by a certain
	 * byte sequence in a certain offset.
	 *
	 * \param[in] bytes  (ignored)
	 * \param[in] offset (ignored)
	 *
	 * \return TRUE
	 */
	bool do_accepts_bytes(const std::vector<char> &bytes,
			const uint64_t &offset) const;

	/**
	 * \brief Returns FALSE since CD devices do not have suffices
	 *
	 * \return FALSE
	 */
	bool do_accepts_suffix(const std::string &suffix) const;

	std::unique_ptr<FileReader> do_create_reader() const override;

	std::unique_ptr<FileReaderDescriptor> do_clone() const override;
};

/// @}

} // namespace v_1_0_0

} // namespace arcsdec

#endif

