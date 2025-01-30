/**
 * \file
 *
 * \brief Implements audio reader for FLAC audio files.
 */

#ifndef __LIBARCSDEC_READERFLAC_HPP__
#include "readerflac.hpp"
#endif
#ifndef __LIBARCSDEC_READERFLAC_DETAILS_HPP__
#include "readerflac_details.hpp"
#endif

#include <FLAC++/decoder.h>
#include <FLAC++/metadata.h>

#include <cstdint>     // for uint64_t
#include <limits>      // for numeric_limits
#include <memory>      // for unique_ptr
#include <set>         // for set
#include <sstream>     // for ostringstream
#include <string>      // for string
#include <utility>     // for make_unique, move

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp> // for AudioSize
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>   // for ARCS_LOG_ERROR, _WARNING, _INFO, _DEBUG
#endif

#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"      // for AudioReaderImpl, InvalidAudioException
#endif
#ifndef __LIBARCSDEC_LIBINSPECT_HPP__
#include "libinspect.hpp"       // for first_libname_match
#endif
#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"        // for RegisterDescriptor
#endif


namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{
namespace flac
{

using arcstk::AudioSize;


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

	if (streaminfo.get_bits_per_sample() > std::numeric_limits<int>::max())
	{
		ARCS_LOG_ERROR << "Number of bits per sample exceeds size of int";
		return false;
	} // Check this explicitly because we will cast to int

	if (not this->assert_true("Test (CDDA): Bits per sample",
		CDDAValidator::bits_per_sample(
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
		CDDAValidator::num_channels(
			static_cast<int>(streaminfo.get_channels())),
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
		CDDAValidator::samples_per_second(
			static_cast<int>(streaminfo.get_sample_rate())),
		"Number of samples per second does not conform to CDDA"))
	{
		return false;
	}

	return true;
}


AudioValidator::codec_set_type FlacMetadataHandler::do_codecs() const
{
	return { Codec::FLAC };
}


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
	smplseq_.wrap_int_buffer(buffer[0], buffer[1], frame->header.blocksize);
	this->signal_appendsamples(smplseq_.begin(), smplseq_.end());

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

			size.set_samples(metadata->data.stream_info.total_samples);
			this->signal_updateaudiosize(size);

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
		std::numeric_limits<decltype(audiosize->samples())>::max();
	if (total_samples > MAX_AUDIOSIZE)
	{
		ARCS_LOG_ERROR << "Total samples in FLAC stream " << total_samples
			<< " exceed the maximum value of " << 0xFFFFFFFF;
	}
	audiosize->set_samples(total_samples);

	return audiosize;
}


void FlacAudioReaderImpl::do_process_file(const std::string &filename)
{
	this->signal_startinput();

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

	this->signal_endinput();

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


} // namespace details
} // namespace flac


// DescriptorFlac


DescriptorFlac::~DescriptorFlac() noexcept = default;


std::string DescriptorFlac::do_id() const
{
	return "flac";
}


std::string DescriptorFlac::do_name() const
{
	return "Flac";
}


std::set<Format> DescriptorFlac::define_formats() const
{
	return { Format::FLAC }; // TODO OGG ?
}


std::set<Codec> DescriptorFlac::define_codecs() const
{
	return { Codec::FLAC };
}


LibInfo DescriptorFlac::do_libraries() const
{
	return { libinfo_entry_filepath("libFLAC++"),
			 libinfo_entry_filepath("libFLAC") };
}


std::unique_ptr<FileReader> DescriptorFlac::do_create_reader() const
{
	using details::flac::FlacAudioReaderImpl;
	using details::flac::FlacMetadataHandler;

	auto impl = std::make_unique<FlacAudioReaderImpl>();
	impl->register_validate_handler(std::make_unique<FlacMetadataHandler>());
	return std::make_unique<AudioReader>(std::move(impl));
}


std::unique_ptr<FileReaderDescriptor> DescriptorFlac::do_clone() const
{
	return std::make_unique<DescriptorFlac>();
}


// Add this descriptor to the audio descriptor registry

namespace {

const auto d = RegisterDescriptor<DescriptorFlac>();

} // namespace

} // namespace v_1_0_0
} // namespace arcsdec

