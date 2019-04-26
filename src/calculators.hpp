#ifndef __LIBARCSTK_CALCULATORS_HPP__
#define __LIBARCSTK_CALCULATORS_HPP__

/**
 * \file calculators.hpp A high-level API for calculating ARCSs and IDs.
 */

#include <memory>
#include <set>
#include <string>

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>
#endif
#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>
#endif

#ifndef __LIBARCSDEC_DESCRIPTORS_HPP__
#include "descriptors.hpp"
#endif
#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"
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
 * track of a given file.
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
class TOCParser
{

public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~TOCParser() noexcept;

	/**
	 * \brief Parse the metadata file to a TOC object.
	 *
	 * \param[in] metafilename Name of the metadatafile
	 *
	 * \return The parsed TOC
	 */
	TOC parse(const std::string &metafilename);
};


/**
 * \brief Calculate AccurateRip ID of an album.
 */
class ARIdCalculator
{

public:

	/**
	 * \brief Constructor.
	 */
	ARIdCalculator();

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~ARIdCalculator() noexcept;

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
 * part is completely delegated to the <tt>FileReader</tt>s.
 */
class ARCSCalculator
{

public:

	/**
	 * \brief Constructor.
	 */
	ARCSCalculator();

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~ARCSCalculator() noexcept;

	/**
	 * \brief Calculate ARCS values for the given audio file, using the metadata 
	 * from the given TOC.
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
	 * \brief Calculate ARCSs for the given audio files.
	 *
	 * It can be specified that the sequence of audiofiles forms an album by
	 * passing <tt>true</tt> for both boolean parameters.
	 *
	 * The ARCSs in the result will have the same order as the input files,
	 * so for any index i: 0 <= i < audiofilenames.size(), result[i] will be the
	 * result for audiofilenames[i]. The result will have the same size as
	 * audiofilenames.
	 *
	 * Note that in this use case, it is not offered to compute the ARId of the
	 * album since the exact offsets are missing. Calculating the offsets from
	 * the actual tracks may be a feature but it is in no way guaranteed that
	 * this would lead to a correct result (e.g. if the silence was not appended
	 * to the end of the previous track).
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
	 * \brief Calculate a single ARCS for the given audio file.
	 *
	 * The flags skip_front and skip_back control whether the track is processed
	 * as first or last track of an album. If \c skip_front is set to TRUE, the
	 * track is processed as first track of an album, meaning the first 2939
	 * samples are skipped in the calculation. If \c skip_back
	 * is set to TRUE, the track is processed as the last track of an album,
	 * meaning that the last 5 frames of are skipped in the calculation.
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
	 * \brief Set the AudioReaderSelection for this instance.
	 *
	 * \param[in] selection The AudioReaderSelection to use
	 */
	void set_selection(std::unique_ptr<AudioReaderSelection> selection);

	/**
	 * \brief Get the AudioReaderSelection used by this instance.
	 *
	 * \return The AudioReaderSelection used by this instance
	 */
	const AudioReaderSelection& selection() const;


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

