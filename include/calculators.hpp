#ifndef LIBARCSDEC_CALCULATORS_HPP__
#define LIBARCSDEC_CALCULATORS_HPP__

/**
 * \file
 *
 * \brief Calculate AccurateRip Checksums and IDs.
 */

#include <memory>   // for unique_ptr
#include <string>   // for string
#include <utility>  // for pair
#include <vector>   // for vector

#ifndef LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>    // for Checksums, ChecksumSet,...
#endif
#ifndef LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>   // for ARId
#endif
#ifndef LIBARCSTK_METADATA_HPP__
#include <arcstk/metadata.hpp>     // for ToC
#endif

#ifndef LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"           // for CreateReader, FileReaders, FormatList,
#endif                             // FileReaderProvider, FileReaderSelector


namespace arcsdec
{
                                                  /** \cond NAMESPACE_v_1_0_0 */
inline namespace v_1_0_0
{
                                                                 /** \endcond */
namespace read // forward declarations
{
class AudioReader;
class MetadataParser;
} // namespace read


/**
 * \brief Format-independent calculators for AccurateRip checksums and ids.
 */
namespace calc
{

using arcstk::ARId;
using arcstk::Algorithm;
using arcstk::AudioSize;
using arcstk::ChecksumSet;
using arcstk::Checksums;
using arcstk::ChecksumtypeSet;
using arcstk::Points;
using arcstk::Settings;
using arcstk::ToC;

using arcsdec::read::AudioReader;
using arcsdec::read::MetadataParser;
using arcsdec::select::FileReaderProvider;

/**
 * \defgroup calculators Calculators for AccurateRip Checksums and IDs
 *
 * \brief Calculate AccurateRip checksums and IDs.
 *
 * Calculators provide calculation results, thereby processing data provided by
 * FileReader instances. When passed filenames, calculators determine
 * autonomously the required FileReader types for reading those files, perform
 * the read process and their respective calculation task and provide the result
 * to the caller. The caller is not responsible for any format or codec related
 * task.
 *
 * This module defines four calculators providing different kinds of
 * information:
 *
 * <table>
 *		<tr>
 *			<td>ARCSCalculator is a calculator for the ARCSs for each audio
 *				track of a given audio-/metadata file pair.</td>
 *		</tr>
 *		<tr>
 *			<td>ARIdCalculator is a calculator for the AccurateRip id of a given
 *				audio-/metadata file pair.</td>
 *		</tr>
 *		<tr>
 *			<td>ToCParser is a format independent parser for metadata files.
 *			</td>
 *		</tr>
 *		<tr>
 *			<td>AudioInfo is a format independent reader for metadata of audio
 *			files that currently provides the amount of samples.</td>
 *		</tr>
 * </table>
 *
 * @{
 */


/**
 * \brief Format-independent parser for audio metadata.
 */
class AudioInfo final : public FileReaderProvider<AudioReader>
{
public:

	/**
	 * \brief Parse the size of the audio data from the audio file.
	 *
	 * \param[in] audiofilename Name of the audiodatafile
	 *
	 * \return The size of the audio data
	 */
	std::unique_ptr<arcstk::AudioSize> size(const std::string& audiofilename)
		const;
};


/**
 * \brief Format-independent parser for CD ToC metadata files.
 */
class ToCParser final : public FileReaderProvider<MetadataParser>
{
public:

	/**
	 * \brief Parse the metadata file to a ToC object.
	 *
	 * \param[in] metafilename Name of the metadatafile
	 *
	 * \return The parsed ToC
	 */
	std::unique_ptr<ToC> parse(const std::string& metafilename) const;
};


/**
 * \brief Calculate ARCSs for input audio files.
 */
class ARCSCalculator final : public FileReaderProvider<AudioReader>
{
public:

	/**
	 * \brief Constructor
	 *
	 * \param[in] type The Checksum type to calculate.
	 */
	explicit ARCSCalculator(const ChecksumtypeSet& type);

	/**
	 * \brief Constructor.
	 *
	 * Uses ARCS1 and ARCS2 as default checksum types.
	 */
	ARCSCalculator();

	/**
	 * \brief Calculate ARCS values for an audio file, using the given ToC.
	 *
	 * The ToC is supposed to contain the offsets of all tracks represented
	 * in the audio file. It is not required to be <tt>complete()</tt>.
	 *
	 * Any audio file names in the ToC are ignored in favor of \c audiofilename.
	 *
	 * The result will contain ARCS v1 and v2 for all tracks specified in the
	 * ToC. A version of the ToC is returned that is ensured to be complete.
	 *
	 * \param[in] audiofilename Name of the audiofile
	 * \param[in] toc           Offsets for the audiofile
	 *
	 * \return AccurateRip checksums of all tracks in the Toc and completed ToC
	 */
	std::pair<Checksums, ToC> calculate(const std::string& audiofilename,
			const ToC& toc);

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
	 * \param[in] audiofilenames            Names of the audiofiles
	 * \param[in] first_file_is_first_track Process first file as first track
	 * \param[in] last_file_is_last_track   Process last file as last track
	 *
	 * \return AccurateRip checksums of the input files
	 */
	Checksums calculate(const std::vector<std::string>& audiofilenames,
			const bool first_file_is_first_track,
			const bool last_file_is_last_track);

	/**
	 * \brief Calculate a single ARCS for an audio file.
	 *
	 * The flags \c is_first_track and \c is_last_track control whether the
	 * track is processed as first or last track of an album. Since the
	 * AccurateRip algorithms process the first and last file in a special way,
	 * it is required to flag them accordingly.
	 *
	 * \param[in] audiofilename     Name of the audiofile
	 * \param[in] is_first_track    Iff TRUE, file is treated as first track
	 * \param[in] is_last_track     Iff TRUE, file is treated as last track
	 *
	 * \return The AccurateRip checksum of this track
	 */
	ChecksumSet calculate(const std::string& audiofilename,
		const bool is_first_track, const bool is_last_track);
	// NOTE This is not really useful except for testing

	/**
	 * \brief Calculate Checksums of a single audiofile.
	 *
	 * If \c leadout is zero() the leadout will be updated by reading it from
	 * the audiofile, otherwise the returned leadout will just be identical to
	 * the leadout passed.
	 *
	 * \param[in] audiofilename Name of audio file to process
	 * \param[in] settings      Settings for calculations
	 * \param[in] req_types     Requested checksum types
	 * \param[in] leadout       Leadout
	 * \param[in] offsets       Offsets
	 *
	 * \return Calculated checksums and updated Leadout
	 */
	std::pair<Checksums, AudioSize> calculate(const std::string& audiofilename,
			const Settings& settings, const ChecksumtypeSet& req_types,
			const AudioSize& leadout, const Points& offsets);

	/**
	 * \brief Return checksum::types calculated by this instance.
	 *
	 * \return The set of checksum::types to calculate
	 */
	ChecksumtypeSet types() const;

	/**
	 * \brief Set checksum::type for the instance to calculate.
	 *
	 * \param[in] type The checksum::type to calculate
	 */
	void set_types(const ChecksumtypeSet& type);

	/**
	 * \brief Size of the read buffer.
	 *
	 * \return Preferred size of the read buffer
	 */
	int64_t read_buffer_size() const;

	/**
	 * \brief Set the preferred size of the read buffer.
	 *
	 * This determines the number of samples to read in one read operation.
	 *
	 * The Audioreader is not forced to respect it, but it is a strong hint.
	 *
	 * \param[in] total_samples Number of PCM 32 bit samples to read at once
	 */
	void set_read_buffer_size(const int64_t total_samples); // TODO AudioSize?

private:

	/**
	 * \brief Convert the flags for first and last track to a Context.
	 *
	 * \param[in] first_file_is_first_track Treat first file as track 1
	 * \param[in] last_file_is_last_track   Treat last file as last track
	 *
	 * \return Context equivalent to the input flags
	 */
	arcstk::Context to_context(
		const bool first_file_is_first_track,
		const bool last_file_is_last_track) const;

	/**
	 * \brief Internal checksum type.
	 */
	ChecksumtypeSet types_;

	/**
	 * \brief Size of the read buffer (in number of samples).
	 */
	int64_t read_buffer_size_;
};


/**
 * \brief Calculate AccurateRip ID of an album.
 *
 * When instantiated, the default_selection() for AudioReaders is active. To
 * modify this behaviour, replace the default AudioInfo by a custom one.
 */
class ARIdCalculator final : public FileReaderProvider<MetadataParser>
{
public:

	/**
	 * \brief Constructor.
	 */
	ARIdCalculator();

	/**
	 * \brief Calculate ARId using the specified metadata and audio file.
	 *
	 * \param[in] metafilename  Name of the metadata file
	 * \param[in] audiofilename Name of the audiofile
	 *
	 * \return The AccurateRip id for this medium
	 */
	ARId calculate(const std::string& metafilename,
			const std::string& audiofilename) const;

	/**
	 * \brief Calculate ARId from ToC while taking leadout from audio file.
	 *
	 * Iff the ToC is complete(), the audiofilename parameter is completely
	 * ignored.
	 *
	 * \param[in] toc           ToC of the audio data
	 * \param[in] audiofilename Name of the audiofile
	 *
	 * \return The AccurateRip id for this medium
	 */
	ARId calculate(const ToC& toc,
			const std::string& audiofilename) const;

	/**
	 * \brief AudioInfo used by this instance.
	 *
	 * \return AudioInfo used by this instance
	 */
	const AudioInfo* audio() const;

	/**
	 * \brief Set the AudioInfo used by this instance.
	 *
	 * \param[in] audio_info AudioInfo to be used by this instance
	 */
	void set_audio(const AudioInfo& audio_info);

private:

	/**
	 * \brief Internal worker to determine the AudioSize if required.
	 */
	AudioInfo audio_;
};

/// @}

} // namespace calc
                                                  /** \cond NAMESPACE_v_1_0_0 */
} // namespace v_1_0_0
                                                                 /** \endcond */
} // namespace arcsdec

#endif

