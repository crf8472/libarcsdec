#ifndef __LIBARCSDEC_READERFLAC_HPP__
#error "Do not include readerflac_details.hpp, include readerflac.hpp instead"
#endif
#ifndef __LIBARCSDEC_READERFLAC_DETAILS_HPP__
#define __LIBARCSDEC_READERFLAC_DETAILS_HPP__

/**
 * \internal
 *
 * \file
 *
 * \brief Implementation details of readerflac.hpp.
 */

#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"    // for AudioReaderImpl, DefaultValidator
#endif

#ifndef __LIBARCSTK_SAMPLES_HPP__
#include <arcstk/samples.hpp> // for SampleSequence
#endif

#include <FLAC++/decoder.h>		// for FLAC::Decoder::File,
								// FLAC__StreamDecoderWriteStatus,
								// FLAC__StreamDecoderErrorStatus
#include <FLAC++/metadata.h>	// for FLAC::Metadata::StreamInfo,
								// FLAC__StreamMetadata
								// for FLAC__int32
								// for FLAC__Frame

#include <memory>   // for unique_ptr
#include <string>   // for string


namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{

/**
 * \internal
 *
 * \brief Implementation details of readerflac.
 */
namespace flac
{

using arcstk::SampleSequence;

/**
 * \internal
 *
 * \defgroup readerflacImpl Implementation
 *
 * \ingroup readerflac
 *
 * @{
 */

/**
 * \brief Interface: Error handler for FLAC files.
 */
class FlacMetadataHandler
{
	virtual void do_validate(const FLAC::Metadata::StreamInfo& streaminfo)
	= 0;

	virtual void do_cuesheet(const FLAC::Metadata::CueSheet& cuesheet)
	= 0;

public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~FlacMetadataHandler() noexcept;

	/**
	 * \brief To be called manually by the AudioReaderImpl to trigger
	 * validation.
	 *
	 * Validates stream for CDDA compliance.
	 *
	 * \param[in] streaminfo The Streaminfo object from FLAC++
	 *
	 * \throw InvalidMetadataException If validation fails
	 */
	void validate(const FLAC::Metadata::StreamInfo& streaminfo);

	/**
	 * \brief To be called manually by the AudioReaderImpl to handle CueSheet
	 * data.
	 *
	 * \param[in] cuesheet CueSheet data found in FLAC file
	 */
	void cuesheet(const FLAC::Metadata::CueSheet& cuesheet);
};

/**
 * \brief Interface: Error handler for FLAC files.
 */
class FlacErrorHandler
{
	virtual void do_error(::FLAC__StreamDecoderErrorStatus status)
	= 0;

public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~FlacErrorHandler() noexcept;

	/**
	 * \brief To be called manually by the AudioReaderImpl to signal a decoder
	 * error.
	 *
	 * \param[in] status Error status signalled by the decoder
	 */
	void error(::FLAC__StreamDecoderErrorStatus status);
};

/**
 * \brief Provides an implementation of the FLAC__metadata_callback handler that
 * validates the audio data for conforming to CDDA.
 *
 * Each call of the callback can call one of the handler methods.
 *
 * It is not the responsibility of the FlacMetadataHandler to inform the
 * internal \c Calculation about the total number of samples or bytes. This is
 * done within the implementation of metadata_callback().
 */
class FlacDefaultMetadataHandler final  : public FlacMetadataHandler
										, public DefaultValidator
{
public:

	/**
	 * \brief Default constructor.
	 */
	FlacDefaultMetadataHandler();

	// class is non-copyable
	FlacDefaultMetadataHandler(const FlacDefaultMetadataHandler&)
	= delete;
	FlacDefaultMetadataHandler& operator = (const FlacDefaultMetadataHandler&)
	= delete;

	// class is movable
	FlacDefaultMetadataHandler(FlacDefaultMetadataHandler&&) noexcept;
	FlacDefaultMetadataHandler& operator = (FlacDefaultMetadataHandler&&)
		noexcept;

private:

	void do_validate(const FLAC::Metadata::StreamInfo& streaminfo) final;

	void do_cuesheet(const FLAC::Metadata::CueSheet& cuesheet) final;

	codec_set_type do_codecs() const final;

	/**
	 * \brief To be called manually by the AudioReaderImpl to trigger
	 * validation.
	 *
	 * Validates it for CDDA compliance.
	 *
	 * \param[in] streaminfo The Streaminfo object from FLAC++
	 *
	 * \return TRUE if metadata indicates CDDA conformity, otherwise FALSE
	 */
	bool validate_streaminfo(const FLAC::Metadata::StreamInfo& streaminfo);
};


/**
 * \brief Default error handler for FLAC files.
 */
class FlacDefaultErrorHandler final : public FlacErrorHandler
{
	void do_error(::FLAC__StreamDecoderErrorStatus status) final;
};


/**
 * \brief File reader implementation for files in CDDA/Flac format, i.e.
 * containing 44.100 Hz/16 bit Stereo PCM data samples.
 *
 * This class provides the PCM sample data as a succession of blocks of 32 bit
 * PCM samples to its \c Calculation. The first block starts with the very
 * first PCM sample in the file. The streaminfo metadata block is validated to
 * conform to CDDA.
 */
class FlacAudioReaderImpl final : public AudioReaderImpl, FLAC::Decoder::File
{
public:

	/**
	 * \brief Default constructor.
	 */
	FlacAudioReaderImpl();

	// class is non-copyable
	FlacAudioReaderImpl(const FlacAudioReaderImpl&) = delete;
	FlacAudioReaderImpl& operator = (const FlacAudioReaderImpl&) = delete;

	/**
	 * \brief Pass frames to internal handler.
	 *
	 * \param[in] frame  The frame describing object as defined by FLAC
	 * \param[in] buffer The sample buffer
	 *
	 * \return Decoder status info
	 */
	::FLAC__StreamDecoderWriteStatus write_callback(
			const ::FLAC__Frame* frame,
			const ::FLAC__int32* const buffer[]) final;

	/**
	 * \brief Pass metadata by type to internal handler.
	 *
	 * Passes the the total number of samples in the stream to the
	 * sample_count() method since this number is required by process_file() in
	 * any configuration setup.
	 *
	 * \param[in] metadata The StreamMetadata from FLAC
	 */
	void metadata_callback(const ::FLAC__StreamMetadata* metadata) final;

	/**
	 * \brief Log the decoder's error status.
	 *
	 * \param[in] status The StreamDecoderErrorStatus
	 */
	void error_callback(::FLAC__StreamDecoderErrorStatus status) final;

	/**
	 * \brief Register a metadata handler to this instance.
	 *
	 * \param[in] hndlr Set the metadata handler of this instance
	 */
	void register_metadata_handler(std::unique_ptr<FlacMetadataHandler> hndlr);

	/**
	 * \brief Register an error handler to this instance.
	 *
	 * \param[in] hndlr Set the error handler of this instance
	 */
	void register_error_handler(std::unique_ptr<FlacErrorHandler> hndlr);

private:

	std::unique_ptr<AudioSize> do_acquire_size(const std::string& filename)
		final;

	void do_process_file(const std::string& filename) final;

	std::unique_ptr<FileReaderDescriptor> do_descriptor() const final;

	/**
	 * \brief Internal SampleSequence instance.
	 */
	SampleSequence<::FLAC__int32, true> smplseq_;

	/**
	 * \brief Handles each metadata block.
	 */
	std::unique_ptr<FlacMetadataHandler> metadata_handler_;

	/**
	 * \brief Handles errors.
	 */
	std::unique_ptr<FlacErrorHandler> error_handler_;
};

/** @} */

} // namespace flac
} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

#endif

