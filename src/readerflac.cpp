/**
 * \file readerflac.cpp Implements audio reader for FLAC audio files
 *
 */


#ifndef __LIBARCSDEC_READERFLAC_HPP__
#include "readerflac.hpp"
#endif

#include "FLAC++/decoder.h"
#include "FLAC++/metadata.h"

#include <cstdint>
#include <locale>       // for locale
#include <memory>
#include <string>

#ifndef __LIBARCS_CALCULATE_HPP__
#include <arcs/calculate.hpp>
#endif
#ifndef __LIBARCS_SAMPLES_HPP__
#include <arcs/samples.hpp>
#endif
#ifndef __LIBARCS_LOGGING_HPP__
#include <arcs/logging.hpp>
#endif

#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"
#endif
#ifndef __LIBARCSDEC_AUDIOBUFFER_HPP__
#include "audiobuffer.hpp"
#endif


namespace arcs
{


namespace
{


/**
 * \cond IMPL_ONLY
 *
 * \internal \defgroup readerflacImpl Implementation details for reading Flac files
 *
 * \ingroup readerflac
 *
 * The Flac AudioReader will only read files in flac file format. Flac/Ogg is
 * currently not supported. Validation requires CDDA conform samples. Embedded
 * CUEsheets are ignored.
 *
 * @{
 */


/**
 * Provides an implementation of the FLAC__metadata_callback handler that
 * validates the audio data for conforming to CDDA.
 *
 * Each call of the callback can call one of the handler methods.
 *
 * It is not the responsibility of the FlacMetadataHandler to inform the
 * internal \ref Calculation about the total number of samples or bytes. This is done
 * within the implementation of metadata_callback().
 */
class FlacMetadataHandler : public ReaderValidatingHandler
{

public:

	/**
	 * Default constructor
	 */
	FlacMetadataHandler();

	/**
	 * Virtual default destructor
	 */
	~FlacMetadataHandler() noexcept override;

	/**
	 * To be called manually by the AudioReaderImpl to trigger validation.
	 * Validates it for CDDA compliance.
	 *
	 * \param[in] streaminfo The Streaminfo object from FLAC++
	 *
	 * \return TRUE if metadata indicates CDDA conformity, otherwise FALSE
	 */
	bool streaminfo(const FLAC::Metadata::StreamInfo &streaminfo);

	/**
	 * Handler method: to be called by FLAC's metadata_callback() to handle
	 * the streaminfo metatdata block. Validates it for CDDA compliance.
	 *
	 * \param[in] metadata The StreamMetadata from FLAC
	 *
	 * \return TRUE if metadata indicates CDDA conformity, otherwise FALSE
	 */
	bool streaminfo(::FLAC__StreamMetadata *metadata);

	/**
	 * Handler method: to be called by FLAC's metadata_callback() to handle
	 * the cuesheet metatdata block.
	 *
	 * \todo Unimplemented. This could extract the offsets directly
	 * from the audio file if they are available.
	 *
	 * \param[in] metadata The StreamMetadata from FLAC
	 */
	void cuesheet(const ::FLAC__StreamMetadata *metadata);


private:

	// make class non-copyable (1/2)
	FlacMetadataHandler(const FlacMetadataHandler &) = delete;

	// make class non-copyable (2/2)
	FlacMetadataHandler& operator = (const FlacMetadataHandler &) = delete;
};


FlacMetadataHandler::FlacMetadataHandler() = default;


FlacMetadataHandler::~FlacMetadataHandler() noexcept = default;


bool FlacMetadataHandler::streaminfo(
		const FLAC::Metadata::StreamInfo &streaminfo)
{
	ARCS_LOG_DEBUG << "Found FLAC streaminfo metadata block";

	if (not this->assert_true("Test: Valid streaminfo?",
		streaminfo.is_valid(),
		"FLAC Streaminfo is not valid"))
	{
		return false;
	}

	// Validate sampling rate, channels and bps for CDDA compliance

	CDDAValidator validate;

	if (not this->assert_true("Test (CDDA): Bits per sample",
		validate.bits_per_sample(streaminfo.get_bits_per_sample()),
		"Number of bits per sample does not conform to CDDA"))
	{
		return false;
	}

	if (not this->assert_true("Test (CDDA): Channels",
		validate.num_channels(streaminfo.get_channels()),
		"Number of channels does not conform to CDDA"))
	{
		return false;
	}

	if (not this->assert_true("Test (CDDA): Samples per second",
		validate.samples_per_second(streaminfo.get_sample_rate()),
		"Number of samples per second does not conform to CDDA"))
	{
		return false;
	}

	return true;
}


bool FlacMetadataHandler::streaminfo(::FLAC__StreamMetadata *metadata)
{
	const FLAC::Metadata::StreamInfo streaminfo(metadata, false);
	return this->streaminfo(streaminfo);
}


void FlacMetadataHandler::cuesheet(const ::FLAC__StreamMetadata *)
{
	ARCS_LOG_DEBUG << "Found FLAC cuesheet metadata block (ignored)";

	// Implement cuesheet
}


/**
 * File reader implementation for files in CDDA/Flac format, i.e. containing
 * 44.100 Hz/16 bit Stereo PCM data samples.
 *
 * This class provides the PCM sample data as a succession of blocks of 32 bit
 * PCM samples to its \ref Calculation. The first block starts with the very
 * first PCM sample in the file. The streaminfo metadata block is validated to
 * conform to CDDA.
 */
class FlacAudioReaderImpl : public AudioReaderImpl, FLAC::Decoder::File {

public:

	/**
	 * Default constructor
	 */
	FlacAudioReaderImpl();

	// make class non-copyable (1/2)
	FlacAudioReaderImpl(const FlacAudioReaderImpl &) = delete;

	// TODO Move constructor

	/**
	 * Default destructor
	 */
	~FlacAudioReaderImpl() noexcept override;

	/**
	 * Implement FLAC's write_callback(): pass frames to internal handler
	 *
	 * \param[in] frame  The frame describing object as defined by FLAC
	 * \param[in] buffer The sample buffer
	 */
	::FLAC__StreamDecoderWriteStatus
		write_callback(	const ::FLAC__Frame	*frame,
						const FLAC__int32   * const buffer[]) override;

	/**
	 * Implement FLAC's metadata_callback(): pass metadata by type to internal
	 * handler. Passes the the total number of samples in the stream to the
	 * sample_count() method since this number is required by process_file() in
	 * any configuration setup.
	 *
	 * \param[in] metadata The StreamMetadata from FLAC
	 */
	void metadata_callback(const ::FLAC__StreamMetadata *metadata) override;

	/**
	 * Implement FLAC's error_callback(): just log the decoder's error status
	 *
	 * \param[in] status The StreamDecoderErrorStatus
	 */
	void error_callback(::FLAC__StreamDecoderErrorStatus status) override;

	/**
	 * Register a FLACMetadataHandler to this instance
	 *
	 * \param[in] hndlr Set the FLACMetadataHandler of this instance
	 */
	void register_validate_handler(std::unique_ptr<FlacMetadataHandler> hndlr);

	/**
	 * Access the FLACMetadataHandler of this instance
	 *
	 * \return The FLACMetadataHandler of this instance
	 */
	const FlacMetadataHandler& validate_handler();

	// make class non-copyable (2/2)
	FlacAudioReaderImpl& operator = (const FlacAudioReaderImpl &) = delete;

	// TODO Move assignment


private:

	std::unique_ptr<AudioSize> do_acquire_size(const std::string &filename)
		override;

	void do_process_file(const std::string &filename) override;

	/**
	 * Internal SampleSequence instance
	 */
	SampleSequence<FLAC__int32, true> smplseq_;

	/**
	 * Handles each metadata block
	 */
	std::unique_ptr<FlacMetadataHandler> metadata_handler_;
};


// FlacAudioReaderImpl


FlacAudioReaderImpl::FlacAudioReaderImpl()
	: smplseq_()
	, metadata_handler_()
{
	// empty
}


FlacAudioReaderImpl::~FlacAudioReaderImpl() noexcept = default;


::FLAC__StreamDecoderWriteStatus FlacAudioReaderImpl::write_callback(
		const ::FLAC__Frame	*frame,
		const FLAC__int32   * const buffer[])
{
	smplseq_.reset(buffer[0], buffer[1], frame->header.blocksize);
	this->process_samples(smplseq_.begin(), smplseq_.end());

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}


void FlacAudioReaderImpl::metadata_callback(
		const ::FLAC__StreamMetadata *metadata)
{
	AudioSize size;

	switch (metadata->type)
	{
		case FLAC__METADATA_TYPE_STREAMINFO:

			// Inform calculator instance about sample count

			size.set_sample_count(metadata->data.stream_info.total_samples);
			this->process_audiosize(size);

			// Streaminfo could already have been validated explicitly

			if (not metadata_handler_->streaminfo(metadata))
			{
				ARCS_LOG_ERROR << "Validation of Flac file failed. Error is:"
						<< metadata_handler_->last_error();

				throw InvalidAudioException(metadata_handler_->last_error());
			}

			break;

		case FLAC__METADATA_TYPE_CUESHEET:

			metadata_handler_->cuesheet(metadata);
			break;

		default:
			break;
	}
}


void FlacAudioReaderImpl::error_callback(
		::FLAC__StreamDecoderErrorStatus status)
{
	std::stringstream ss;

	switch (status)
	{
		case FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER:
			ss << "BAD HEADER";
			break;

		case FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC:
			ss << "LOST SYNC";
			break;

		case FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH:
			ss << "FRAME CRC MISMATCH";
			break;

		case FLAC__STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM:
			ss << "UNPARSEABLE STREAM";
			break;

		default:
			ss << "ERROR UNKNOWN";
			break;
	}

	ARCS_LOG_ERROR << ss.str();

	throw FileReadException(ss.str());
}


std::unique_ptr<AudioSize> FlacAudioReaderImpl::do_acquire_size(
	const std::string &filename)
{
	FLAC::Metadata::StreamInfo streaminfo;
	FLAC::Metadata::get_streaminfo(filename.c_str(), streaminfo);

	if (not metadata_handler_->streaminfo(streaminfo))
	{
		ARCS_LOG_ERROR << metadata_handler_->last_error();
		return nullptr;
	}

	FLAC__uint64 total_samples = streaminfo.get_total_samples();

	if (total_samples > 0xFFFFFFFF) // FIXME Never true, doesn't make sense!
	{
		ARCS_LOG_ERROR << "Too many samples: " << total_samples
				<< ". Does not seem to be a CDDA image.";
	}

	std::unique_ptr<AudioSize> audiosize = std::make_unique<AudioSize>();

	audiosize->set_sample_count(static_cast<uint32_t>(total_samples));

	return audiosize;
}


void FlacAudioReaderImpl::do_process_file(const std::string &filename)
{
	// Process decoded samples

	this->set_md5_checking(false); // We check for checksums for ourselves

	FLAC__StreamDecoderInitStatus init_status = this->init(filename);
	if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK)
	{
		ARCS_LOG_ERROR << "Initializing decoder failed.";
		ARCS_LOG_ERROR << "FLAC__StreamDecoderInitStatus: "
				<< std::string(
					FLAC__StreamDecoderInitStatusString[init_status]);
		return;
	}

	// Get channel order to decide whether order must be swapped.
	// FLAC says: "Where defined, the channel order follows SMPTE/ITU-R
	// recommendations." and only defines left/right orderings.

	const auto channel_assignment = this->get_channel_assignment();
	switch (channel_assignment)
	{
		case FLAC__CHANNEL_ASSIGNMENT_INDEPENDENT:
			ARCS_LOG_INFO << "Channel assignment: left/right";
			break;

		case FLAC__CHANNEL_ASSIGNMENT_LEFT_SIDE:
			ARCS_LOG_INFO << "Channel assignment: left/side stereo";
			break;

		case FLAC__CHANNEL_ASSIGNMENT_RIGHT_SIDE:
			ARCS_LOG_INFO << "Channel assignment: right/side stereo";
			break;

		case FLAC__CHANNEL_ASSIGNMENT_MID_SIDE:
			ARCS_LOG_INFO << "Channel assignment: mid/side stereo";
			break;
	}
	// end channel order stuff

	bool success = this->process_until_end_of_stream();

	if (not success)
	{
		ARCS_LOG_ERROR << "Decoding failed";
		ARCS_LOG_ERROR << "Last decoder state: "
				<< std::string(this->get_state().as_cstring());
	}

	this->finish();
	ARCS_LOG_INFO << "Audio file closed";
}


void FlacAudioReaderImpl::register_validate_handler(
		std::unique_ptr<FlacMetadataHandler> hndlr)
{
	metadata_handler_ = std::move(hndlr);
}


const FlacMetadataHandler& FlacAudioReaderImpl::validate_handler()
{
	return *metadata_handler_;
}

/// @}

} // namespace


// FileFormatFlac


FileFormatFlac::~FileFormatFlac() noexcept = default;


std::string FileFormatFlac::do_name() const
{
	return "Flac";
}


bool FileFormatFlac::do_can_have_bytes(const std::vector<char> &bytes,
		const uint64_t &offset) const
{
	return  bytes.size() >= 4
		and offset       == 0
		and bytes[0]     == 0x66  // f
		and bytes[1]     == 0x4C  // L
		and bytes[2]     == 0x61  // a
		and bytes[3]     == 0x43  // C
		;
}


bool FileFormatFlac::do_can_have_suffix(const std::string &suffix) const
{
	std::locale locale;
	return std::tolower(suffix, locale) == "flac";
}


std::unique_ptr<FileReader> FileFormatFlac::do_create_reader() const
{
	auto impl = std::make_unique<FlacAudioReaderImpl>();

	impl->register_validate_handler(std::make_unique<FlacMetadataHandler>());

	return std::make_unique<AudioReader>(std::move(impl));
}


std::unique_ptr<FileFormat> FileFormatFlac::do_clone() const
{
	return std::make_unique<FileFormatFlac>();
}

} // namespace arcs

