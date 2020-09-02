/**
 * \file
 *
 * \brief Implements audio reader for FLAC audio files.
 */

#ifndef __LIBARCSDEC_READERFLAC_HPP__
#include "readerflac.hpp"
#endif
//#ifndef __LIBARCSDEC_READERFLAC_DETAILS_HPP__
//#include "readerflac_details.hpp"
//#endif

#include <FLAC++/decoder.h>
#include <FLAC++/metadata.h>

#include <cstdint>
#include <limits>      // for numeric_limits
#include <memory>
#include <sstream>     // for ostringstream
#include <string>

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>
#endif
#ifndef __LIBARCSTK_SAMPLES_HPP__
#include <arcstk/samples.hpp>
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"
#endif


namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{
namespace flac
{

using arcstk::SampleInputIterator;
using arcstk::AudioSize;
using arcstk::CDDA;
using arcstk::InvalidAudioException;
using arcstk::SampleSequence;


/**
 * \internal \defgroup readerflacImpl Implementation
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
 * \brief Provides an implementation of the FLAC__metadata_callback handler that
 * validates the audio data for conforming to CDDA.
 *
 * Each call of the callback can call one of the handler methods.
 *
 * It is not the responsibility of the FlacMetadataHandler to inform the
 * internal \ref Calculation about the total number of samples or bytes. This is done
 * within the implementation of metadata_callback().
 */
class FlacMetadataHandler final : public ReaderValidatingHandler
{
public:

	FlacMetadataHandler() = default;

	// class is non-copyable
	FlacMetadataHandler(const FlacMetadataHandler &) = delete;
	FlacMetadataHandler& operator = (const FlacMetadataHandler &) = delete;

	// class is movable
	FlacMetadataHandler(FlacMetadataHandler &&) noexcept = default;
	FlacMetadataHandler& operator = (FlacMetadataHandler &&) noexcept = default;

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
	bool streaminfo(const FLAC::Metadata::StreamInfo &streaminfo);
};


// FlacMetadataHandler


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

	static CDDAValidator validate;

	if (streaminfo.get_bits_per_sample() > std::numeric_limits<int>::max())
	{
		ARCS_LOG_ERROR << "Number of bits per sample exceeds size of int";
		return false;
	} // Check this explicitly because we will cast to int

	if (not this->assert_true("Test (CDDA): Bits per sample",
		validate.bits_per_sample(
			static_cast<int>(streaminfo.get_bits_per_sample())),
		"Number of bits per sample does not conform to CDDA"))
	{
		return false;
	}

	if (streaminfo.get_channels() > std::numeric_limits<int>::max())
	{
		ARCS_LOG_ERROR << "Number of channels exceeds size of int";
		return false;
	} // Check this explicitly because we will cast to int

	if (not this->assert_true("Test (CDDA): Channels",
		validate.num_channels(static_cast<int>(streaminfo.get_channels())),
		"Number of channels does not conform to CDDA"))
	{
		return false;
	}

	if (streaminfo.get_sample_rate() > std::numeric_limits<int>::max())
	{
		ARCS_LOG_ERROR << "Sample rate exceeds size of int";
		return false;
	} // Check this explicitly because we will cast to int

	if (not this->assert_true("Test (CDDA): Samples per second",
		validate.samples_per_second(
			static_cast<int>(streaminfo.get_sample_rate())),
		"Number of samples per second does not conform to CDDA"))
	{
		return false;
	}

	return true;
}


/**
 * \brief File reader implementation for files in CDDA/Flac format, i.e.
 * containing 44.100 Hz/16 bit Stereo PCM data samples.
 *
 * This class provides the PCM sample data as a succession of blocks of 32 bit
 * PCM samples to its \ref Calculation. The first block starts with the very
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
	FlacAudioReaderImpl(const FlacAudioReaderImpl &) = delete;
	FlacAudioReaderImpl& operator = (const FlacAudioReaderImpl &) = delete;

	// class is movable
	FlacAudioReaderImpl(FlacAudioReaderImpl &&rhs) noexcept = default;
	FlacAudioReaderImpl& operator = (FlacAudioReaderImpl &&rhs) noexcept
		= default;

	/**
	 * \brief Pass frames to internal handler.
	 *
	 * \param[in] frame  The frame describing object as defined by FLAC
	 * \param[in] buffer The sample buffer
	 *
	 * \return Decoder status info
	 */
	::FLAC__StreamDecoderWriteStatus write_callback(
			const ::FLAC__Frame *frame,
			const ::FLAC__int32 *const buffer[]) override;

	/**
	 * \brief Pass metadata by type to internal handler.
	 *
	 * Passes the the total number of samples in the stream to the
	 * sample_count() method since this number is required by process_file() in
	 * any configuration setup.
	 *
	 * \param[in] metadata The StreamMetadata from FLAC
	 */
	void metadata_callback(const ::FLAC__StreamMetadata *metadata) override;

	/**
	 * \brief Log the decoder's error status.
	 *
	 * \param[in] status The StreamDecoderErrorStatus
	 */
	void error_callback(::FLAC__StreamDecoderErrorStatus status) override;

	/**
	 * \brief Register a FLACMetadataHandler to this instance.
	 *
	 * \param[in] hndlr Set the FLACMetadataHandler of this instance
	 */
	void register_validate_handler(std::unique_ptr<FlacMetadataHandler> hndlr);

private:

	std::unique_ptr<AudioSize> do_acquire_size(const std::string &filename)
		override;

	void do_process_file(const std::string &filename) override;

	std::unique_ptr<FileReaderDescriptor> do_descriptor() const override;

	/**
	 * \brief Internal SampleSequence instance.
	 */
	SampleSequence<::FLAC__int32, true> smplseq_;

	/**
	 * \brief Handles each metadata block.
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


::FLAC__StreamDecoderWriteStatus FlacAudioReaderImpl::write_callback(
		const ::FLAC__Frame *frame,
		const ::FLAC__int32 *const buffer[])
{
	smplseq_.wrap(buffer[0], buffer[1], frame->header.blocksize);
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

			size.set_total_samples(metadata->data.stream_info.total_samples);
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

			ARCS_LOG_DEBUG << "Found FLAC cuesheet metadata block (ignored)";
			// TODO Implement cuesheet handler
			break;

		default:
			break;
	}
}


void FlacAudioReaderImpl::error_callback(
		::FLAC__StreamDecoderErrorStatus status)
{
	std::ostringstream ss;

	switch (status)
	{
		case ::FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER:
			ss << "BAD_HEADER";
			break;

		case ::FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC:
			ss << "LOST_SYNC";
			break;

		case ::FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH:
			ss << "FRAME_CRC_MISMATCH";
			break;

		case ::FLAC__STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM:
			ss << "UNPARSEABLE_STREAM";
			break;

		default:
			ss << "ERROR UNKNOWN";
			break;
	}

	throw FileReadException(ss.str());
}


std::unique_ptr<AudioSize> FlacAudioReaderImpl::do_acquire_size(
	const std::string &filename)
{
	::FLAC::Metadata::StreamInfo streaminfo;
	::FLAC::Metadata::get_streaminfo(filename.c_str(), streaminfo);

	if (!metadata_handler_->streaminfo(streaminfo))
	{
		ARCS_LOG_ERROR << metadata_handler_->last_error();
		return nullptr;
	}

	std::unique_ptr<AudioSize> audiosize = std::make_unique<AudioSize>();

	const auto total_samples = streaminfo.get_total_samples();
	static const auto MAX_AUDIOSIZE =
		std::numeric_limits<decltype(audiosize->total_samples())>::max();
	if (total_samples > MAX_AUDIOSIZE)
	{
		ARCS_LOG_ERROR << "Total samples in FLAC stream " << total_samples
			<< " exceed the maximum value of " << 0xFFFFFFFF;
	}
	audiosize->set_total_samples(total_samples);

	return audiosize;
}


void FlacAudioReaderImpl::do_process_file(const std::string &filename)
{
	// Process decoded samples

	this->set_md5_checking(false); // We check for checksums for ourselves

	::FLAC__StreamDecoderInitStatus init_status = this->init(filename);
	if (init_status != ::FLAC__STREAM_DECODER_INIT_STATUS_OK)
	{
		ARCS_LOG_ERROR << "Initializing decoder failed.";
		ARCS_LOG_ERROR << "FLAC__StreamDecoderInitStatus: "
				<< std::string(
					::FLAC__StreamDecoderInitStatusString[init_status]);
		return;
	}

	// Get channel order to decide whether order must be swapped.
	// FLAC says: "Where defined, the channel order follows SMPTE/ITU-R
	// recommendations." and only defines left/right orderings.

	const auto channel_assignment = this->get_channel_assignment();
	switch (channel_assignment)
	{
		case ::FLAC__CHANNEL_ASSIGNMENT_INDEPENDENT:
			ARCS_LOG_INFO << "Channel assignment: left/right";
			break;

		case ::FLAC__CHANNEL_ASSIGNMENT_LEFT_SIDE:
			ARCS_LOG_INFO << "Channel assignment: left/side stereo";
			break;

		case ::FLAC__CHANNEL_ASSIGNMENT_RIGHT_SIDE:
			ARCS_LOG_INFO << "Channel assignment: right/side stereo";
			break;

		case ::FLAC__CHANNEL_ASSIGNMENT_MID_SIDE:
			ARCS_LOG_INFO << "Channel assignment: mid/side stereo";
			break;

		default:
			ARCS_LOG_WARNING << "Could not determine channel assignment";
	}
	// end channel order stuff

	const bool success = this->process_until_end_of_stream();

	if (not success)
	{
		ARCS_LOG_ERROR << "Decoding failed";
		ARCS_LOG_ERROR << "Last decoder state: "
				<< std::string(this->get_state().as_cstring());
	}

	this->finish();
	ARCS_LOG_INFO << "Audio file closed";
}


std::unique_ptr<FileReaderDescriptor> FlacAudioReaderImpl::do_descriptor()
	const
{
	return std::make_unique<DescriptorFlac>();
}


void FlacAudioReaderImpl::register_validate_handler(
		std::unique_ptr<FlacMetadataHandler> hndlr)
{
	metadata_handler_ = std::move(hndlr);
}

/// @}

} // namespace details
} // namespace flac


// DescriptorFlac


DescriptorFlac::~DescriptorFlac() noexcept = default;


std::string DescriptorFlac::do_name() const
{
	return "Flac";
}


LibInfo DescriptorFlac::do_libraries() const
{
	using details::find_lib;
	using details::libarcsdec_libs;

	return { { "libFLAC++", find_lib(libarcsdec_libs(), "libFLAC++") },
			 { "libFLAC"  , find_lib(libarcsdec_libs(), "libFLAC")   }
	};
}


bool DescriptorFlac::do_accepts_bytes(const std::vector<unsigned char> &bytes,
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


std::unique_ptr<FileReader> DescriptorFlac::do_create_reader() const
{
	using details::flac::FlacAudioReaderImpl;
	using details::flac::FlacMetadataHandler;

	auto impl = std::make_unique<FlacAudioReaderImpl>();
	impl->register_validate_handler(std::make_unique<FlacMetadataHandler>());
	return std::make_unique<AudioReader>(std::move(impl));
}


bool DescriptorFlac::do_accepts(Codec codec) const
{
	return codec == Codec::FLAC;
}


std::set<Codec> DescriptorFlac::do_codecs() const
{
	return { Codec::FLAC };
}


bool DescriptorFlac::do_accepts(Format format) const
{
	return format == Format::FLAC; // TODO OGG ?
}


std::set<Format> DescriptorFlac::do_formats() const
{
	return { Format::FLAC }; // TODO OGG ?
}


std::unique_ptr<FileReaderDescriptor> DescriptorFlac::do_clone() const
{
	return std::make_unique<DescriptorFlac>();
}


// Add this descriptor to the audio descriptor registry

namespace {

const auto d = RegisterAudioDescriptor<DescriptorFlac>();

} // namespace

} // namespace v_1_0_0

} // namespace arcsdec

