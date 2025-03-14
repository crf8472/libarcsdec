#ifndef __LIBARCSDEC_CALCULATORS_HPP__
#define __LIBARCSDEC_CALCULATORS_HPP__

/**
 * \file
 *
 * \brief Calculate AccurateRip Checksums and IDs.
 */

#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"           // for CreateReader, FileReaders, FormatList,
#endif                             // FileReaderSelector

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>    // for Checksums, ChecksumSet,...
#endif
#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>   // for ARId
#endif
#ifndef __LIBARCSTK_METADATA_HPP__
#include <arcstk/metadata.hpp>     // for ToC
#endif

#include <memory>   // for unique_ptr
#include <string>   // for string
#include <utility>  // for pair
#include <vector>   // for vector


/**
 * \brief APIs of libarcsdec.
 */
namespace arcsdec
{

/**
 * \brief libarcsdec API version 1.0.0.
 */
inline namespace v_1_0_0
{

// required by interface
class AudioReader;
class MetadataParser;

using arcstk::ARId;
using arcstk::Algorithm;
using arcstk::AudioSize;
using arcstk::ChecksumSet;
using arcstk::Checksums;
using arcstk::ChecksumtypeSet;
using arcstk::Points;
using arcstk::Settings;
using arcstk::ToC;


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
 * \brief Provide the default FileReaderSelection for the specified ReaderType.
 *
 * \return The default FileReaderSelection.
 */
template <class ReaderType>
const FileReaderSelection* default_selection();


// Deactivate -Weffc++ for the following two classes
//
// -Weffc++ will warn about ReaderAndFormatHolder and SelectionPerformer
// not having declared copy constructor and copy assignment operator although
// they have pointer type members. But this is intended + exactly what we want.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"

/**
 * \brief Interface for a class that holds formats and readers.
 *
 * The default constructor initializes Formats and FileReaderDescriptors with
 * their respective sets from FileReaderRegistry.
 */
class ReaderAndFormatHolder
{
public:

	/**
	 * \brief Constructor.
	 *
	 * Initializes the formats and readers from FileReaderRegistry.
	 */
	ReaderAndFormatHolder();

	/**
	 * \brief Pure virtual default descriptor.
	 */
	virtual ~ReaderAndFormatHolder() noexcept
	= 0;

	/**
	 * \brief Set the list of formats supported by this instance.
	 *
	 * \param[in] formats The list of supported formats.
	 */
	void set_formats(const FormatList* formats);

	/**
	 * \brief List of formats supported by this instance.
	 *
	 * \return List of supported formats.
	 */
	const FormatList* formats() const;

	/**
	 * \brief Set the FileReaders for this instance.
	 *
	 * \param[in] readers The set of available FileReaderDescriptors to use
	 */
	void set_readers(const FileReaders* readers);

	/**
	 * \brief Get the FileReaders used by this instance.
	 *
	 * \return The MetadataParserSelection used by this instance
	 */
	const FileReaders* readers() const;

private:

	/**
	 * \brief Internal list of supported formats
	 */
	const FormatList* formats_;

	/**
	 * \brief Internal list of available
	 * \link FileReaderDescriptor FileReaderDescriptors\endlink
	 */
	const FileReaders* descriptors_;
};


/**
 * \brief Interface for a class that performs a selection.
 *
 * The default Constructor initializes the selection by the default selection
 * for AudioReaders as provided by FileReaderRegistry.
 */
template <class ReaderType>
class SelectionPerformer
{
public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] selection The selection to use
	 */
	explicit SelectionPerformer(const FileReaderSelection* selection)
		: selection_ { selection }
		, create_    { /* default */ }
	{
		/* empty */
	}

	/**
	 * \brief Constructor.
	 *
	 * Initializes the instance with the default_selection() for the ReaderType.
	 */
	SelectionPerformer()
		: SelectionPerformer(default_selection<ReaderType>())
	{
		/* empty */
	}

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~SelectionPerformer() noexcept = default;

	/**
	 * \brief Set the selection to be used for selecting AudioReaders.
	 *
	 * \param[in] selection Selection for AudioReaders
	 */
	void set_selection(const FileReaderSelection* selection)
	{
		selection_ = selection;
	}

	/**
	 * \brief Get the selection to be used for selecting AudioReaders.
	 *
	 * \return Selection for AudioReaders
	 */
	const FileReaderSelection* selection() const
	{
		return selection_;
	}

	/**
	 * \brief Create a FileReader capable of reading \c filename.
	 *
	 * \param[in] filename The file to read
	 * \param[in] f        Available FileReader and FileFormat types
	 *
	 * \return A FileReader for the input file
	 */
	std::unique_ptr<ReaderType> file_reader(const std::string& filename,
			const ReaderAndFormatHolder* f) const
	{
		return this->create_(filename, *this->selection(), *f->formats(),
				*f->readers());
	}

private:

	/**
	 * \brief Internal selection for \link FileReaders FileReaders\endlink
	 */
	const FileReaderSelection* selection_;

	/**
	 * \brief Internal FileReader creator.
	 */
	details::CreateReader<ReaderType> create_;
};


// Re-activate -Weffc++ for all what follows
#pragma GCC diagnostic pop


/**
 * \brief Abstract base class for classes that create opaque readers.
 *
 * A subclass must specify the ReaderType and can then easily use create()
 * to create an appropriate FileReader by just specifying the filename.
 */
template <class ReaderType>
class FileReaderProvider : public ReaderAndFormatHolder
						 , public SelectionPerformer<ReaderType>
{
protected:

	/**
	 * \brief Create a FileReader capable of reading \c filename.
	 *
	 * \param[in] filename The file to read
	 *
	 * \return A FileReader for the input file
	 */
	std::unique_ptr<ReaderType> create(const std::string& filename) const
	{
		return this->file_reader(filename, this);
	}
};


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
 *
 * Note that ARCSCalculator does not perform any lookups in the filesystem. This
 * part is completely delegated to the \link FileReader FileReaders\endlink.
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
	 * ToC.
	 *
	 * \param[in] audiofilename Name of the audiofile
	 * \param[in] toc           Offsets for the audiofile
	 *
	 * \return AccurateRip checksums of all tracks specified in the ToC
	 */
	std::pair<Checksums, ARId> calculate(const std::string& audiofilename,
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
	 * \param[in] audiofilename Name of audio file to process
	 * \param[in] settings      Settings for calculations
	 * \param[in] types         Requested checksum types
	 * \param[in,out] leadout   Leadout
	 * \param[in] offsets       Offsets
	 */
	Checksums calculate(const std::string& audiofilename,
			const Settings& settings, const ChecksumtypeSet& types,
			std::unique_ptr<AudioSize>& leadout, const Points& offsets);

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
	std::unique_ptr<ARId> calculate(const std::string& metafilename,
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
	std::unique_ptr<ARId> calculate(const ToC& toc,
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
	 * \param[in] audio AudioInfo to be used by this instance
	 */
	void set_audio(const AudioInfo& audio);

private:

	/**
	 * \brief Internal worker to determine the AudioSize if required.
	 */
	AudioInfo audio_;
};

/// @}

} // namespace v_1_0_0
} // namespace arcsdec

#endif

