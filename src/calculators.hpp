#ifndef __LIBARCS_CALCULATORS_HPP__
#define __LIBARCS_CALCULATORS_HPP__

/**
 * \file calculators.hpp A high-level API for calculating ARCSs of files.
 */

#include <memory>
#include <set>
#include <string>

#ifndef __LIBARCS_IDENTIFIER_HPP__
#include <arcs/identifier.hpp>
#endif
#ifndef __LIBARCS_CALCULATE_HPP__
#include <arcs/calculate.hpp>
#endif


namespace arcs
{

/**
 * \defgroup calc Level 2 API : Calculators for AccurateRip Checksums and IDs
 *
 * \brief Calculators for the AccurateRip ID and the AccurateRip checksums of
 * a medium
 *
 * ARIdCalculator is a calculator for the AccurateRip id of a given medium
 * description. ARCSCalculator is a calculator for the ARCSs for each audio
 * track of a given file.
 *
 * @{
 */


/**
 * Parser for metadata files, e.g. files containing compact disc TOC data.
 */
class TOCParser
{

public:

	/**
	 * Virtual default destructor
	 */
	virtual ~TOCParser() noexcept;

	/**
	 * Parse the metadata file to a TOC object.
	 *
	 * \param[in] metafilename Name of the metadatafile
	 *
	 * \return The parsed TOC
	 */
	TOC parse(const std::string &metafilename);
};


/**
 * Compute AccurateRip identifiers from a metadata file and a corresponding
 * audio data representation of the file.
 */
class ARIdCalculator
{

public:

	/**
	 * Constructor
	 */
	ARIdCalculator();

	/**
	 * Virtual default destructor
	 */
	virtual ~ARIdCalculator() noexcept;

	/**
	 * Calculate ARId using the specified metadata file
	 *
	 * \param[in] metafilename Name of the metadata file
	 *
	 * \return The AccurateRip id for this medium
	 */
	std::unique_ptr<ARId> calculate(const std::string &metafilename);

	/**
	 * Calculate ARId using the specified metadata file and the specified
	 * audio file
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
	 * Internal implementation instance
	 */
	std::unique_ptr<Impl> impl_;
};


/**
 * Compute ARCSs for input audio files.
 */
class ARCSCalculator
{

public:

	/**
	 * Constructor
	 */
	ARCSCalculator();

	/**
	 * Virtual default destructor
	 */
	virtual ~ARCSCalculator() noexcept;

	/**
	 * Calculate ARCS values for the given audio file, using the metadata from
	 * the given TOC.
	 *
	 * The TOC is supposed to contain the offsets of all tracks represented
	 * in the audio file. The result will contain ARCS v1 and v2 for all tracks
	 * specified in the TOC.
	 *
	 * \param[in] audiofilename Name of the audiofile
	 * \param[in] toc           Metadata for the audiofile
	 *
	 * \return AccurateRip checksums of all tracks specified in the TOC
	 */
	std::pair<Checksums, ARId> calculate(const std::string &audiofilename,
			const TOC &toc);
	// multiple tracks with tracknos (by TOC)

	/**
	 * Calculate an ARCS for the given audio file
	 *
	 * The flags skip_front and skip_back control whether the track is processed
	 * as first or last track of an album. If skip_front is set to TRUE, the
	 * first 2939 samples are skipped in the calculation. If skip_back
	 * is set to TRUE, the last 5 frames of are skipped in the calculation.
	 *
	 * \param[in] audiofilename  Name  of the audiofile
	 * \param[in] skip_front     Skip front samples of first track
	 * \param[in] skip_back      Skip back samples of last track
	 *
	 * \return The AccurateRip checksum of this track
	 */
	std::unique_ptr<ChecksumSet> calculate(const std::string &audiofilename,
		const bool &skip_front, const bool &skip_back);
	// single track without trackno

	/**
	 * Calculate ARCSs for the given audio files.
	 *
	 * The ARCSs in the result will have the same order as the input files,
	 * so for any index i: 0 <= i < audiofilenames.size(), result[i] will be the
	 * result for audiofilenames[i]. The result will have the same size as
	 * audiofilenames.
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
	// multiple tracks with tracknos (by default) (no id)


private:

	// Forward declaration for private implementation.
	class Impl;

	/**
	 * Internal implementation instance
	 */
	std::unique_ptr<Impl> impl_;
};


/**
 * Wrapper for \ref ARCSCalculator to calculate album metadata from a sequence
 * of single results that are known to form an album.
 */
class ProfileCalculator
{

public:

	/**
	 * Constructor
	 */
	ProfileCalculator();

	/**
	 * Virtual default destructor
	 */
	virtual ~ProfileCalculator() noexcept;

	/**
	 * Compute the result values of the CD image represented by the specified
	 * files.
	 *
	 * \param[in] audiofilenames Name of the audio files
	 * \param[in] metafilename   Name of the metadata file
	 *
	 * \return Checksums, Id and TOC of the image represented by the input files
	 */
	std::tuple<Checksums, ARId, std::unique_ptr<TOC>> calculate(
			const std::vector<std::string> &audiofilenames,
			const std::string &metafilename) const;

	/**
	 * Compute the result values of the CD image represented by the specified
	 * file.
	 *
	 * \param[in] metafilename   Name of the metadata file
	 * \param[in] searchpath     Name of the searchpath for audio files
	 *
	 * \return Checksums, Id and TOC of the image represented by the input files
	 */
	std::tuple<Checksums, ARId, std::unique_ptr<TOC>> calculate(
			const std::string &metafilename, const std::string &searchpath)
			const;


private:

	// Forward declaration for private implementation.
	class Impl;

	/**
	 * Internal implementation instance
	 */
	std::unique_ptr<Impl> impl_;
};


/// @}

} // namespace arcs

#endif

