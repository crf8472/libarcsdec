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
 * Provides an implementation of the FLAC__write_callback handler that
 * accumulates samples to blocks and send each block to processing.
 *
 * Each call of the write_callback can call the frame() method and after
 * processing of the file is complete, the \ref Calculation instance pointed to
 * holds the results. FLACFrameHandler is a mere wrapper that hides the FLAC API
 * to BlockAccumulator.
 */
class FlacFrameHandler
{

public:

	/**
	 * Constructor that sets the \ref Calculation of this handler
	 *
	 * \param[in] calc The \ref Calculation instance of this handler
	 */
	explicit FlacFrameHandler(std::unique_ptr<Calculation> calc);

	/**
	 * Default destructor
	 */
	virtual ~FlacFrameHandler() noexcept;

	/**
	 * Handler method: flac frame occurred.
	 *
	 * \param[in] frame  The frame describing object as defined by FLAC
	 * \param[in] buffer The sample buffer
	 */
	void frame(	const ::FLAC__Frame *       frame,
				const   FLAC__int32 * const buffer[] );

	/**
	 * Number of frames processed since init() was called the last time.
	 *
	 * \return Return the number of frames processed
	 */
	uint64_t frames_processed() const;

	/**
	 * Set the \ref Calculation instance for this handler
	 *
	 * \param[in] calc The \ref Calculation instance for this handler
	 */
	void set_calc(std::unique_ptr<Calculation> calc);

	/**
	 * Get the \ref Calculation instance of this handler
	 *
	 * \return The \ref Calculation of this handler
	 */
	const Calculation& calc();

	/**
	 * Set the number of samples to be read in one block
	 *
	 * \param[in] samples_per_block The number of samples to be processed in one
	 * block
	 */
	void set_samples_per_block(const uint32_t &samples_per_block);

	/**
	 * Return the number of samples to be read in one block
	 *
	 * \return The number of samples to be processed in one block
	 */
	uint32_t samples_per_block() const;

	/**
	 * Returns the calculation result
	 *
	 * \return The Profile of the file
	 */
	Checksums get_result();

	/**
	 * Notify this instance about how many samples in total are to be
	 * processed
	 *
	 * \param[in] sample_count The total number of 32 bit PCM samples in the
	 * file
	 */
	void notify_sample_count(const uint64_t &sample_count);

	/**
	 * Initialize handler. To be called by the client of this handler before
	 * using this handler.
	 */
	void init();

	/**
	 * Finish handler. To be called by the client of this handler after
	 * the last use of this handler.
	 */
	void finish();

	/**
	 * Number of samples processed since init() was called the last time.
	 *
	 * \return Return the number of samples processed
	 */
	uint64_t samples_processed() const;

	/**
	 * Number of bytes processed since init() was called the last time.
	 *
	 * \return Return the number of bytes processed
	 */
	uint64_t bytes_processed() const;

	/**
	 * Number of blocks processed since init() was called the last time.
	 *
	 * \return Return the number of blocks processed
	 */
	uint64_t blocks_processed() const;


private:

	/**
	 * Internal pointer to the \ref Calculation instance.
	 */
	std::unique_ptr<Calculation> calc_;

	/**
	 * Internal BlockAccumulator instance
	 */
	SampleBuffer buffer_;

	/**
	 * Internal SampleSequence instance
	 */
	SampleSequence<FLAC__int32, true> smplseq_;


	// make class non-copyable (1/2)
	FlacFrameHandler(const FlacFrameHandler &) = delete;

	// make class non-copyable (2/2)
	FlacFrameHandler& operator = (const FlacFrameHandler &) = delete;
};


/// @}
/// \endcond IMPL_ONLY


FlacFrameHandler::FlacFrameHandler(std::unique_ptr<Calculation> calc)
	: calc_(std::move(calc))
	, buffer_()
	, smplseq_()
{
	// This line is doubled up in set_calc()
	buffer_.register_processor(*calc_);
}


FlacFrameHandler::~FlacFrameHandler() noexcept = default;


void FlacFrameHandler::frame(
	const ::FLAC__Frame *       frame,
	const FLAC__int32	* const buffer[])
{
	smplseq_.reset(buffer[0], buffer[1], frame->header.blocksize);
	buffer_.append(smplseq_.begin(), smplseq_.end());
}


uint64_t FlacFrameHandler::frames_processed() const
{
	return buffer_.sequences_processed();
}


void FlacFrameHandler::set_calc(std::unique_ptr<Calculation> calc)
{
	calc_ = std::move(calc);
	buffer_.register_processor(*calc_);
}


const Calculation& FlacFrameHandler::calc()
{
	return *calc_;
}


void FlacFrameHandler::set_samples_per_block(
		const uint32_t &samples_per_block)
{
	buffer_.set_samples_per_block(samples_per_block);
}


uint32_t FlacFrameHandler::samples_per_block() const
{
	return buffer_.samples_per_block();
}


Checksums FlacFrameHandler::get_result()
{
	return calc_->result();
}


void FlacFrameHandler::notify_sample_count(const uint64_t &sample_count)
{
	buffer_.notify_total_samples(sample_count);
}


void FlacFrameHandler::init()
{
	buffer_.reset();
}


void FlacFrameHandler::finish()
{
	buffer_.flush();
}


uint64_t FlacFrameHandler::samples_processed() const
{
	return buffer_.samples_processed();
}


uint64_t FlacFrameHandler::bytes_processed() const
{
	return buffer_.bytes_processed();
}


uint64_t FlacFrameHandler::blocks_processed() const
{
	return buffer_.blocks_processed();
}


/// \cond IMPL_ONLY
/// \internal \addtogroup readerflacImpl
/// @{

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

/// @}
/// \endcond IMPL_ONLY


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


/// \cond IMPL_ONLY
/// \internal \addtogroup readerflacImpl
/// @{

/**
 * File reader implementation for files in CDDA/Flac format, i.e. containing
 * 44.100 Hz/16 bit Stereo PCM data samples.
 *
 * This class provides the PCM sample data as a succession of blocks
 * of 32 bit PCM samples to its \ref Calculation. The first block starts with the very
 * first PCM sample in the file. The streaminfo metadata block is validated to
 * conform to CDDA.
 */
class FlacAudioReaderImpl : public AudioReaderImpl, FLAC::Decoder::File
{

public:

	/**
	 * Default constructor
	 */
	FlacAudioReaderImpl();

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

	/**
	 * Register a FlacFrameHandler to this instance
	 *
	 * \param[in] hndlr Set the FlacFrameHandler of this instance
	 */
	void register_proc_handler(std::unique_ptr<FlacFrameHandler> hndlr);

	/**
	 * Access the FlacFrameHandler of this instance
	 *
	 * \return The FlacFrameHandler of this instance
	 */
	const FlacFrameHandler& proc_handler();


private:

	/**
	 * Callback method: inform instance about the total number of samples
	 * found in the stream.
	 *
	 * \param[in] sample_count Number of total 32 bit PCM samples in the file
	 */
	virtual void notify_sample_count(const uint64_t &sample_count);

	/**
	 * Read the FLAC file and optionally validate it. This method provides
	 * the implementation of FlacAudioReader::process_file().
	 *
	 * \param[in] filename The audiofile to process
	 * \param[in,out] total_pcm_bytes  The total number of sample bytes in the
	 * file
	 *
	 * \return TRUE if file was processed without errors
	 */
	bool process_file(const std::string &filename,
			uint32_t& total_pcm_bytes);

	std::unique_ptr<AudioSize> do_acquire_size(const std::string &filename)
		override;

	Checksums do_process_file(const std::string &filename) override;

	void do_set_samples_per_block(const uint32_t &samples_per_block) override;

	uint32_t do_get_samples_per_block() const override;

	void do_set_calc(std::unique_ptr<Calculation> calc) override;

	const Calculation& do_get_calc() const override;

	/**
	 * Handles each frame
	 */
	std::unique_ptr<FlacFrameHandler> frame_handler_;

	/**
	 * Handles each metadata block
	 */
	std::unique_ptr<FlacMetadataHandler> metadata_handler_;


	// make class non-copyable (1/2)
	FlacAudioReaderImpl(const FlacAudioReaderImpl &) = delete;

	// make class non-copyable (2/2)
	FlacAudioReaderImpl& operator = (const FlacAudioReaderImpl &) = delete;
};

/// @}
/// \endcond IMPL_ONLY


FlacAudioReaderImpl::FlacAudioReaderImpl()
	: frame_handler_()
	, metadata_handler_()
{
	// empty
}


FlacAudioReaderImpl::~FlacAudioReaderImpl() noexcept = default;


::FLAC__StreamDecoderWriteStatus FlacAudioReaderImpl::write_callback(
		const ::FLAC__Frame	*frame,
		const FLAC__int32   * const buffer[])
{
	frame_handler_->frame(frame, buffer);

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}


void FlacAudioReaderImpl::metadata_callback(
		const ::FLAC__StreamMetadata *metadata)
{
	switch (metadata->type)
	{
		case FLAC__METADATA_TYPE_STREAMINFO:

			// Inform calculator instance about sample count

			this->notify_sample_count(metadata->data.stream_info.total_samples);

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


Checksums FlacAudioReaderImpl::do_process_file(const std::string &filename)
{
	if (not frame_handler_)
	{
		ARCS_LOG_ERROR << "No processing handler available in reader. Stop.";
		return Checksums(0);
	}

	// Process decoded samples

	frame_handler_->set_samples_per_block(this->samples_per_block());
	frame_handler_->init();

	this->set_md5_checking(false); // We check for checksums for ourselves

	FLAC__StreamDecoderInitStatus init_status = this->init(filename);
	if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK)
	{
		ARCS_LOG_ERROR << "Initializing decoder failed.";
		ARCS_LOG_ERROR << "FLAC__StreamDecoderInitStatus: "
				<< std::string(
					FLAC__StreamDecoderInitStatusString[init_status]);

		return Checksums(0);
	}

	// Get channel order to decide whether order must be swapped.
	// FLAC says: "Where defined, the channel order follows SMPTE/ITU-R
	// recommendations." and only defines left/right orderings.

	const auto channel_assignment = this->get_channel_assignment();
	switch (channel_assignment)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7: ARCS_LOG_INFO << "Channel assignment: left/right";
				break;
		case 8: ARCS_LOG_INFO << "Channel assignment: left/side stereo";
				break;
		case 9: ARCS_LOG_INFO << "Channel assignment: right/side stereo";
				break;
		case 10: ARCS_LOG_INFO << "Channel assignment: mid/side stereo";
				break;
	}
	// end channel order stuff

	bool success = this->process_until_end_of_stream();

	if (not success)
	{
		ARCS_LOG_ERROR << "Decoding failed";
		ARCS_LOG_ERROR << "Last decoder state: "
				<< std::string(this->get_state().as_cstring());

		// finish() resets decoder state, log state before

		this->finish();

		ARCS_LOG_INFO << "Audio file closed";

		return Checksums(0);
	}

	this->finish();
	ARCS_LOG_INFO << "Audio file closed";

	frame_handler_->finish();

	ARCS_LOG(LOG_DEBUG1) << "Read " << frame_handler_->blocks_processed()
			<< " blocks";
	ARCS_LOG(LOG_DEBUG1) << "Read samples: "
			<< frame_handler_->samples_processed();
	ARCS_LOG(LOG_DEBUG1) << "    in bytes: "
			<< frame_handler_->bytes_processed();

	if (!frame_handler_)
	{
		return Checksums(0);
	}

	return frame_handler_->get_result();
}


void FlacAudioReaderImpl::do_set_samples_per_block(
		const uint32_t &samples_per_block)
{
	frame_handler_->set_samples_per_block(samples_per_block);
}


uint32_t FlacAudioReaderImpl::do_get_samples_per_block() const
{
	return frame_handler_->samples_per_block();
}


void FlacAudioReaderImpl::do_set_calc(std::unique_ptr<Calculation> calc)
{
	frame_handler_->set_calc(std::move(calc));
}


const Calculation& FlacAudioReaderImpl::do_get_calc() const
{
	return frame_handler_->calc();
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


void FlacAudioReaderImpl::register_proc_handler(
		std::unique_ptr<FlacFrameHandler> hndlr)
{
	frame_handler_ = std::move(hndlr);
}


const FlacFrameHandler& FlacAudioReaderImpl::proc_handler()
{
	return *frame_handler_;
}


void FlacAudioReaderImpl::notify_sample_count(const uint64_t &sample_count)
{
	if (frame_handler_)
	{
		frame_handler_->notify_sample_count(sample_count);

		ARCS_LOG_INFO << "Total samples: " + std::to_string(sample_count);
		return;
	}

	ARCS_LOG_WARNING << "No calculator instance";
}


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

	std::unique_ptr<Calculation> calc = std::make_unique<Calculation>(
				make_context(std::string(), false, false));

	impl->register_proc_handler(
			std::make_unique<FlacFrameHandler>(std::move(calc)));

	return std::make_unique<AudioReader>(std::move(impl));
}


std::unique_ptr<FileFormat> FileFormatFlac::do_clone() const
{
	return std::make_unique<FileFormatFlac>();
}

} // namespace arcs

