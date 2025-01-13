#ifndef __LIBARCSDEC_CALCULATORS_HPP__
#define __LIBARCSDEC_CALCULATORS_HPP__

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
#include "selection.hpp"           // for CreateReader, FileReaders, FormatList,
#endif                             // FileReaderSelector


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

// required by interface
class AudioReader;
class MetadataParser;

using arcstk::TOC;
using arcstk::Algorithm;
using arcstk::ARId;
using arcstk::Checksums;
using arcstk::ChecksumSet;


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
	virtual ~ReaderAndFormatHolder() noexcept = 0;

	/**
	 * \brief Set the list of formats supported by this instance.
	 *
	 * \param[in] formats The list of supported formats.
	 */
	void set_formats(const FormatList *formats);

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
	void set_readers(const FileReaders *readers);

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
	const FormatList *formats_;

	/**
	 * \brief Internal list of available \link FileReaderDescriptor FileReaderDescriptors\endlink
	 */
	const FileReaders *descriptors_;
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
	 * Initializes the instance with the default_selection() for the ReaderType.
	 */
	inline SelectionPerformer()
		: selection_ { default_selection<ReaderType>() }
		, create_    { /* default */ }
	{
		/* empty */
	}

	/**
	 * \brief Constructor.
	 *
	 * \param[in] selection The selection to use
	 */
	inline SelectionPerformer(const FileReaderSelection* selection)
		: selection_ { selection }
		, create_    { /* default */ }
	{
		/* empty */
	}

	/**
	 * \brief Virtual default destructor.
	 */
	inline virtual ~SelectionPerformer() noexcept
	{
		/* empty */
	}

	/**
	 * \brief Set the selection to be used for selecting AudioReaders.
	 *
	 * \param[in] selection Selection for AudioReaders
	 */
	inline void set_selection(const FileReaderSelection *selection)
	{
		selection_ = selection;
	}

	/**
	 * \brief Get the selection to be used for selecting AudioReaders.
	 *
	 * \return Selection for AudioReaders
	 */
	inline const FileReaderSelection* selection() const
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
	inline std::unique_ptr<ReaderType> file_reader(const std::string &filename,
			const ReaderAndFormatHolder* f) const
	{
		return this->create_(filename, *this->selection(), *f->formats(),
				*f->readers());
	}

private:

	/**
	 * \brief Internal selection for \link FileReaders FileReaders\endlink
	 */
	const FileReaderSelection *selection_;

	/**
	 * \brief Internal FileReader creator.
	 */
	details::CreateReader<ReaderType> create_;
};


// Re-activate -Weffc++ for all what follows
#pragma GCC diagnostic pop


/**
 * \brief Base class for classes that create opaque readers.
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
	inline std::unique_ptr<ReaderType> create(const std::string &filename) const
	{
		return this->file_reader(filename, this);
	}
};


/**
 * \brief Format-independent parser for CD TOC metadata files.
 */
class TOCParser final : public FileReaderProvider<MetadataParser>
{
public:

	/**
	 * \brief Parse the metadata file to a TOC object.
	 *
	 * \param[in] metafilename Name of the metadatafile
	 *
	 * \return The parsed TOC
	 */
	std::unique_ptr<TOC> parse(const std::string &metafilename) const;
};


/**
 * \brief Set of checksum types.
 */
using ChecksumTypeset = std::unordered_set<arcstk::checksum::type>;


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
	explicit ARCSCalculator(const ChecksumTypeset& type);

	/**
	 * \brief Constructor.
	 *
	 * Uses ARCS1 and ARCS2 as default checksum types.
	 */
	ARCSCalculator();

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
	 * \brief Set checksum::type for the instance to calculate.
	 *
	 * \param[in] type The checksum::type to calculate
	 */
	void set_types(const ChecksumTypeset& type);

	/**
	 * \brief Return checksum::types calculated by this instance.
	 *
	 * \return The set of checksum::types to calculate
	 */
	ChecksumTypeset types() const;

private:

	/**
	 * \brief Worker method: calculating the ARCS of a single audiofile.
	 *
	 * \param[in] audiofilename Name of the audiofile
	 *
	 * \return The AccurateRip checksum of this track
	 */
	ChecksumSet calculate_track(const std::string &audiofilename,
		const bool &skip_front, const bool &skip_back);

	/**
	 * \brief Internal checksum type.
	 */
	ChecksumTypeset types_;
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
	std::unique_ptr<arcstk::AudioSize> size(const std::string &audiofilename)
		const;
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
	 * \brief Calculate ARId using the specified metadata file.
	 *
	 * \param[in] metafilename Name of the metadata file
	 *
	 * \return The AccurateRip id for this medium
	 */
	std::unique_ptr<ARId> calculate(const std::string &metafilename) const;

	/**
	 * \brief Calculate ARId using the specified metadata and audio files.
	 *
	 * \param[in] metafilename  Name of the metadata file
	 * \param[in] audiofilename Name of the audiofile
	 *
	 * \return The AccurateRip id for this medium
	 */
	std::unique_ptr<ARId> calculate(const std::string &metafilename,
			const std::string &audiofilename) const;

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
	 * \brief Worker: calculate ID from TOC while taking leadout from audio
	 * file.
	 *
	 * \param[in] toc           TOC of the image
	 * \param[in] audiofilename Name of the image audiofile
	 *
	 * \return The AccurateRip id for this medium
	 */
	std::unique_ptr<ARId> calculate(const TOC &toc,
			const std::string &audiofilename) const;

	/**
	 * \brief Internal worker to determine the AudioSize if required.
	 */
	AudioInfo audio_;
};

/// @}

} // namespace v_1_0_0
} // namespace arcsdec

#endif

