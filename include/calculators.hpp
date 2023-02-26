#ifndef __LIBARCSTK_CALCULATORS_HPP__
#define __LIBARCSTK_CALCULATORS_HPP__

/**
 * \file
 *
 * \brief A high-level API for calculating ARCSs and IDs.
 */

#include <memory>   // for unique_ptr
#include <string>   // for string
#include <utility>  // for pair
#include <vector>   // for vector

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>   // for ARId, TOC
#endif
#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>    // for Checksums, ChecksumSet
#endif

#ifndef __LIBARCSDEC_DESCRIPTOR_HPP__
#include "descriptor.hpp"          // for FileReaderDescriptor
#endif
#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"           // for FileReaders, FileReaderSelector
#endif


/**
 * \brief Main namespace for libarcsdec.
 */
namespace arcsdec
{

/**
 * \brief API version 1.0.0.
 */
inline namespace v_1_0_0
{

/**
 * \defgroup calculators Calculators for AccurateRip Checksums and IDs
 *
 * \brief Calculators for AccurateRip checksums and IDs.
 *
 * ARIdCalculator is a calculator for the AccurateRip id of a given medium
 * description. ARCSCalculator is a calculator for the ARCSs for each audio
 * track of a given file. TOCParser is a format independent parser for
 * TOC files.
 *
 * @{
 */

using arcstk::TOC;
using arcstk::ARId;
using arcstk::Checksums;
using arcstk::ChecksumSet;


/**
 * \brief Format-independent parser for CD TOC metadata files.
 */
class TOCParser final
{
public:

	/**
	 * \brief Constructor.
	 */
	TOCParser();

	/**
	 * \brief Destructor.
	 */
	~TOCParser() noexcept; // required for completeness of Impl

	/**
	 * \brief Parse the metadata file to a TOC object.
	 *
	 * \param[in] metafilename Name of the metadatafile
	 *
	 * \return The parsed TOC
	 */
	std::unique_ptr<TOC> parse(const std::string &metafilename) const;

	/**
	 * \brief Set the list of supported formats.
	 *
	 * \param[in] formats The list of supported formats.
	 */
	void set_formats(const FormatList *formats);

	/**
	 * \brief List of supported formats.
	 *
	 * \return List of supported formats.
	 */
	const FormatList& formats() const;

	/**
	 * \brief Set the FileReaders for this instance.
	 *
	 * \param[in] readders The set of available FileReaderDescriptors to use
	 */
	void set_descriptorset(const FileReaders *readers);

	/**
	 * \brief Get the MetadataParserSelection used by this instance.
	 *
	 * \return The MetadataParserSelection used by this instance
	 */
	const FileReaders& descriptorset() const;

	/**
	 * \brief Set the selection for this instance.
	 *
	 * \param[in] selection The selection to use
	 */
	void set_selection(const FileReaderSelection *selection);

	/**
	 * \brief Get the selection used by this instance.
	 *
	 * \return The selection used by this instance
	 */
	const FileReaderSelection& selection() const;

private:

	// Forward declaration for private implementation.
	class Impl;

	/**
	 * \brief Internal implementation instance.
	 */
	std::unique_ptr<Impl> impl_;
};


/**
 * \brief Calculate AccurateRip ID of an album.
 */
class ARIdCalculator final
{
public:

	/**
	 * \brief Constructor.
	 */
	ARIdCalculator();

	/**
	 * \brief Destructor.
	 */
	~ARIdCalculator() noexcept; // required for completeness of Impl

	/**
	 * \brief Calculate ARId using the specified metadata file.
	 *
	 * \param[in] metafilename Name of the metadata file
	 *
	 * \return The AccurateRip id for this medium
	 */
	std::unique_ptr<ARId> calculate(const std::string &metafilename);

	/**
	 * \brief Calculate ARId using the specified metadata file and the specified
	 * audio file.
	 *
	 * \param[in] audiofilename Name of the audiofile
	 * \param[in] metafilename  Name of the metadata file
	 *
	 * \return The AccurateRip id for this medium
	 */
	std::unique_ptr<ARId> calculate(const std::string &audiofilename,
			const std::string &metafilename);

	/**
	 * \brief Set the list of supported formats.
	 *
	 * \param[in] formats The list of supported formats.
	 */
	void set_formats(const FormatList *formats);

	/**
	 * \brief List of supported formats.
	 *
	 * \return List of supported formats.
	 */
	const FormatList& formats() const;

	/**
	 * \brief Set the \c FileReaderDescriptor s for this instance.
	 *
	 * \param[in] descriptors The \c FileReaderDescriptor s of this instance.
	 */
	void set_descriptorset(const FileReaders *descriptors);

	/**
	 * \brief The \c FileReaderDescriptor s of this instance.
	 *
	 * \return The \c FileReaderDescriptor s of this instance.
	 */
	const FileReaders& descriptorset() const;

	/**
	 * \brief Set the metadata parser selection for this instance.
	 *
	 * \param[in] selection The metadata parser selection to use
	 */
	void set_toc_selection(const FileReaderSelection *selection);

	/**
	 * \brief Get the metadata parser selection used by this instance.
	 *
	 * \return The metadata parser selection used by this instance
	 */
	const FileReaderSelection& toc_selection() const;

	/**
	 * \brief Set the audioreader selection for this instance.
	 *
	 * \param[in] selection The audioreader selection to use
	 */
	void set_audio_selection(const FileReaderSelection *selection);

	/**
	 * \brief Get the audioreader selection used by this instance.
	 *
	 * \return The audioreader selection used by this instance
	 */
	const FileReaderSelection& audio_selection() const;

private:

	// Forward declaration for private implementation.
	class Impl;

	/**
	 * \brief Internal implementation instance.
	 */
	std::unique_ptr<Impl> impl_;
};


/**
 * \brief Calculate ARCSs for input audio files.
 *
 * Note that ARCSCalculator does not perform any lookups in the filesystem. This
 * part is completely delegated to the \link FileReader FileReaders\endlink.
 */
class ARCSCalculator final
{
public:

	/**
	 * \brief Constructor.
	 *
	 * Uses ARCS1 and ARCS2 as default checksum types.
	 */
	ARCSCalculator();

	/**
	 * \brief Constructor
	 *
	 * \param[in] type The Checksum type to calculate.
	 */
	ARCSCalculator(const arcstk::checksum::type type);

	/**
	 * \brief Destructor.
	 */
	~ARCSCalculator() noexcept; // required for completeness of Impl

	/**
	 * \brief Calculate ARCS values for an audio file, using the given TOC.
	 *
	 * The TOC is supposed to contain the offsets of all tracks represented
	 * in the audio file. It is not required to be <tt>complete()</tt>.
	 *
	 * Any audio file names in the TOC are ignored in favor of \c audiofilename.
	 *
	 * The result will contain ARCS v1 and v2 for all tracks specified in the
	 * TOC.
	 *
	 * \param[in] audiofilename Name of the audiofile
	 * \param[in] toc           Offsets for the audiofile
	 *
	 * \return AccurateRip checksums of all tracks specified in the TOC
	 */
	std::pair<Checksums, ARId> calculate(const std::string &audiofilename,
			const TOC &toc);

	/**
	 * \brief Calculate ARCSs for audio files.
	 *
	 * It can be specified that the sequence of audiofiles forms an album by
	 * passing <tt>TRUE</tt> for both boolean parameters.
	 *
	 * The ARCSs in the result will have the same order as the input files,
	 * so for any index <tt>i: 0 <= i < audiofilenames.size()</tt>,
	 * \c result[i] will be the result for <tt>audiofilenames[i]</tt>. The
	 * result will have the same size as audiofilenames.
	 *
	 * Note that in this use case, it is not offered to compute the ARId of the
	 * album since the exact offsets are missing.
	 *
	 * \param[in] audiofilenames       Names of the audiofiles
	 * \param[in] first_file_with_skip Process first file as first track
	 * \param[in] last_file_with_skip  Process last file as last track
	 *
	 * \return AccurateRip checksums of the input files
	 */
	Checksums calculate(const std::vector<std::string> &audiofilenames,
			const bool &first_file_with_skip,
			const bool &last_file_with_skip);

	/**
	 * \brief Calculate a single ARCS for an audio file.
	 *
	 * The flags \c skip_front and \c skip_back control whether the track is
	 * processed as first or last track of an album. If \c skip_front is set to
	 * <tt>TRUE</tt>, the track is processed as first track of an album, meaning
	 * the first 2939 samples are skipped in the calculation according to the
	 * ARCS checksum definition. If \c skip_back is set to <tt>TRUE</tt>, the
	 * track is processed as the last track of an album, meaning that the last
	 * 5 frames of the input are skipped in the calculation.
	 *
	 * \param[in] audiofilename  Name  of the audiofile
	 * \param[in] skip_front     Skip front samples of first track
	 * \param[in] skip_back      Skip back samples of last track
	 *
	 * \return The AccurateRip checksum of this track
	 */
	ChecksumSet calculate(const std::string &audiofilename,
		const bool &skip_front, const bool &skip_back);

	/**
	 * \brief Set the list of supported formats.
	 *
	 * \param[in] formats The list of supported formats.
	 */
	void set_formats(const FormatList *formats);

	/**
	 * \brief List of supported formats.
	 *
	 * \return List of supported formats.
	 */
	const FormatList& formats() const;

	/**
	 * \brief Set the \c FileReaderDescriptor s for this instance.
	 *
	 * \param[in] descriptors The \c FileReaderDescriptor s of this instance.
	 */
	void set_descriptorset(const FileReaders *descriptors);

	/**
	 * \brief The \c FileReaderDescriptor s of this instance.
	 *
	 * \return The \c FileReaderDescriptor s of this instance.
	 */
	const FileReaders& descriptorset() const;

	/**
	 * \brief Set the AudioReaderSelection for this instance.
	 *
	 * \param[in] selection The AudioReaderSelection to use
	 */
	void set_selection(const FileReaderSelection *selection);

	/**
	 * \brief Get the AudioReaderSelection used by this instance.
	 *
	 * \return The AudioReaderSelection used by this instance
	 */
	const FileReaderSelection& selection() const;

	/**
	 * \brief Set checksum::type for the instance to calculate.
	 *
	 * \param[in] type The checksum::type to calculate
	 */
	void set_type(const arcstk::checksum::type type);

	/**
	 * \brief Return checksum::type calculated by this instance.
	 *
	 * \return The checksum::type to calculate
	 */
	arcstk::checksum::type type() const;

private:

	// Forward declaration for private implementation.
	class Impl;

	/**
	 * \brief Internal implementation instance.
	 */
	std::unique_ptr<Impl> impl_;
};

/// @}

} // namespace v_1_0_0

} // namespace arcsdec

#endif

