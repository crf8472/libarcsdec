/**
 * \file parsercue.hpp Parser for CUE sheets
 *
 */


#ifndef __LIBARCSDEC_PARSERCUE_HPP__
#define __LIBARCSDEC_PARSERCUE_HPP__

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#ifndef __LIBARCSDEC_FILEFORMATS_HPP__
#include "fileformats.hpp"
#endif
#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include "metaparser.hpp"
#endif


namespace arcs
{

/**
 * \internal \defgroup parsercue Metadata: CUESheet
 *
 * \brief A MetadataParser for CUESheet files
 *
 * @{
 */


/**
 * Represents the CUE file format.
 */
class FileReaderDescriptorCUE : public FileReaderDescriptor
{

public:

	/**
	 * Virtual default destructor
	 */
	virtual ~FileReaderDescriptorCUE() noexcept;

	/**
	 * Returns "CUESheet"
	 *
	 * \return "CUESheet"
	 */
	std::string do_name() const;

	/**
	 * Always returns TRUE since CUESheets cannot be recognized by a certain
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
	 * Returns TRUE if the suffix matches a CUE sheet suffix
	 *
	 * \return TRUE iff suffix is case-insensitively equal to suffix otherwise
	 * FALSE
	 */
	bool do_accepts_suffix(const std::string &suffix) const;

	// Override
	std::unique_ptr<FileReader> do_create_reader() const;

	// Override
	std::unique_ptr<FileReaderDescriptor> do_clone() const;
};

/// @}

} // namespace arcs

#endif

