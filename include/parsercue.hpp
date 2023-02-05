/**
 * \file
 *
 * \brief Parser for Cue sheets
 */


#ifndef __LIBARCSDEC_PARSERCUE_HPP__
#define __LIBARCSDEC_PARSERCUE_HPP__

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
 * \internal \defgroup parsercue Metadata: CueSheet
 *
 * \brief A MetadataParser for CueSheet files
 *
 * @{
 */


/**
 * \brief Libcue-based parser for Cuesheets.
 */
class DescriptorCue : public FileReaderDescriptor
{

public:

	/**
	 * \brief Constructor.
	 */
	DescriptorCue()
		: FileReaderDescriptor { { "cue" } }
	{ /* empty */ }

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~DescriptorCue() noexcept override;


private:

	std::string do_id() const override;

	/**
	 * \brief Returns "CueSheet".
	 *
	 * \return "CueSheet"
	 */
	std::string do_name() const override;

	/**
	 * \brief Always returns TRUE since CueSheets cannot be recognized by a
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

