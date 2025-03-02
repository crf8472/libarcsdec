/**
 * \file
 *
 * \brief Implements audio reader for FLAC audio files.
 */

#include <stdexcept>
#ifndef __LIBARCSDEC_READERFLAC_HPP__
#include "readerflac.hpp"
#endif
#ifndef __LIBARCSDEC_READERFLAC_DETAILS_HPP__
#include "readerflac_details.hpp"
#endif

#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"      // for AudioReaderImpl, InvalidAudioException
#endif
#ifndef __LIBARCSDEC_LIBINSPECT_HPP__
#include "libinspect.hpp"       // for libinfo_entry_filepath
#endif
#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"        // for RegisterDescriptor
#endif

#ifndef __LIBARCSTK_METADATA_HPP__
#include <arcstk/metadata.hpp>  // for AudioSize
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>   // for ARCS_LOG_ERROR, _WARNING, _INFO, _DEBUG
#endif

//#include <FLAC/ordinals.h>     // for FLAC__int64_t, FLAC__int32_t

#include <FLAC++/decoder.h>		// for FLAC::Decoder::File,
								// FLAC__StreamDecoderWriteStatus,
								// FLAC__StreamDecoderErrorStatus
#include <FLAC++/metadata.h>	// for FLAC::Metadata::StreamInfo,
								// FLAC__StreamMetadata
								// for FLAC__Frame

#include <limits>      // for numeric_limits
#include <memory>      // for unique_ptr
#include <set>         // for set
#include <sstream>     // for ostringstream
#include <string>      // for string
#include <utility>     // for make_unique, move


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


FlacMetadataHandler::~FlacMetadataHandler() noexcept = default;


void FlacMetadataHandler::validate(const FLAC::Metadata::StreamInfo& streaminfo)
{
	do_validate(streaminfo);
}


void FlacMetadataHandler::cuesheet(const FLAC::Metadata::CueSheet& cuesheet)
{
	do_cuesheet(cuesheet);
}


// FlacErrorHandler


FlacErrorHandler::~FlacErrorHandler() noexcept = default;


void FlacErrorHandler::error(::FLAC__StreamDecoderErrorStatus status)
{
	do_error(status);
}


// FlacDefaultMetadataHandler


FlacDefaultMetadataHandler::FlacDefaultMetadataHandler()
	= default;


FlacDefaultMetadataHandler::FlacDefaultMetadataHandler(
		FlacDefaultMetadataHandler&&) noexcept
= default;


FlacDefaultMetadataHandler&
		FlacDefaultMetadataHandler::operator = (FlacDefaultMetadataHandler&&)
		noexcept
= default;


void FlacDefaultMetadataHandler::do_validate(
		const FLAC::Metadata::StreamInfo& metadata)
{
	if (!validate_streaminfo(metadata))
	{
		ARCS_LOG_ERROR << "Validation of Flac file failed. Error is:"
			<< last_error();

		throw InvalidAudioException(last_error());
	}
}


void FlacDefaultMetadataHandler::do_cuesheet(
		const FLAC::Metadata::CueSheet& cuesheet)
{
	ARCS_LOG_INFO << "Ignore CueSheet found in FLAC file";

	// TODO Implement
}


bool FlacDefaultMetadataHandler::validate_streaminfo(
		const FLAC::Metadata::StreamInfo& streaminfo)
{
	ARCS_LOG_DEBUG << "Found FLAC streaminfo metadata block";

	if (!this->assert_true("Test: Valid streaminfo?", streaminfo.is_valid(),
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

	validate_bits_per_sample(
			static_cast<int>(streaminfo.get_bits_per_sample()));


	if (streaminfo.get_channels() > std::numeric_limits<int>::max())
	{
		ARCS_LOG_ERROR << "Number of channels exceeds size of int";
		return false;
	} // Check this explicitly because we will cast to int

	validate_num_channels(static_cast<int>(streaminfo.get_channels()));


	if (streaminfo.get_sample_rate() > std::numeric_limits<int>::max())
	{
		ARCS_LOG_ERROR << "Sample rate exceeds size of int";
		return false;
	} // Check this explicitly because we will cast to int

	validate_samples_per_second(static_cast<int>(streaminfo.get_sample_rate()));

	return true;
}


AudioValidator::codec_set_type FlacDefaultMetadataHandler::do_codecs() const
{
	return { Codec::FLAC };
}


// FlacDefaultErrorHandler


void FlacDefaultErrorHandler::do_error(::FLAC__StreamDecoderErrorStatus status)
{
	auto ss = std::ostringstream{};

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


// FlacAudioReaderImpl


FlacAudioReaderImpl::FlacAudioReaderImpl()
	: smplseq_          { /* empty */ }
	, metadata_handler_ { /* empty */ }
	, error_handler_    { /* empty */ }
{
	// empty
}


::FLAC__StreamDecoderWriteStatus FlacAudioReaderImpl::write_callback(
		const ::FLAC__Frame* frame,
		const ::FLAC__int32* const buffer[])
{
	smplseq_.wrap_int_buffer(buffer[0], buffer[1], frame->header.blocksize);

	using std::cbegin;
	using std::cend;

	this->signal_appendsamples(cbegin(smplseq_), cend(smplseq_));

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}


void FlacAudioReaderImpl::metadata_callback(
		const ::FLAC__StreamMetadata* metadata)
{
	switch (metadata->type)
	{
		case FLAC__METADATA_TYPE_STREAMINFO:

			this->signal_updateaudiosize(
					to_audiosize(metadata->data.stream_info.total_samples,
						UNIT::SAMPLES));

			metadata_handler_->validate(*metadata);
			// Note: Streaminfo could already have been validated explicitly

			break;

		case FLAC__METADATA_TYPE_CUESHEET:

			metadata_handler_->cuesheet(*metadata);

			break;

		default:
			break;
	}
}


void FlacAudioReaderImpl::error_callback(
		::FLAC__StreamDecoderErrorStatus status)
{
	error_handler_->error(status);
}


void FlacAudioReaderImpl::register_metadata_handler(
		std::unique_ptr<FlacMetadataHandler> hndlr)
{
	metadata_handler_ = std::move(hndlr);
}


void FlacAudioReaderImpl::register_error_handler(
		std::unique_ptr<FlacErrorHandler> hndlr)
{
	error_handler_ = std::move(hndlr);
}


std::unique_ptr<AudioSize> FlacAudioReaderImpl::do_acquire_size(
	const std::string& filename)
{
	auto streaminfo = ::FLAC::Metadata::StreamInfo{};

	::FLAC::Metadata::get_streaminfo(filename.c_str(), streaminfo);

	// Commented out, acquire_size() does not perform validation
	//metadata_handler_->validate(streaminfo);

	return std::make_unique<AudioSize>(to_audiosize(
				streaminfo.get_total_samples(), UNIT::SAMPLES));
}


void FlacAudioReaderImpl::do_process_file(const std::string& filename)
{
	set_md5_checking(false); // TODO part of validation?

	this->signal_startinput();

	// Process decoded samples

	const auto init_status = this->init(filename);

	if (init_status != ::FLAC__STREAM_DECODER_INIT_STATUS_OK)
	{
		ARCS_LOG_ERROR << "Initializing decoder failed.";
		ARCS_LOG_ERROR << "FLAC__StreamDecoderInitStatus: "
				<< std::string{
					::FLAC__StreamDecoderInitStatusString[init_status] };
		return;
	}

	ARCS_LOG(DEBUG3) << "Initialized decoder successfully";

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

	if (!success)
	{
		ARCS_LOG_ERROR << "Decoding failed";
		ARCS_LOG_ERROR << "Last decoder state: "
				<< std::string { this->get_state().as_cstring() };
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
	using details::flac::FlacDefaultMetadataHandler;
	using details::flac::FlacDefaultErrorHandler;

	auto impl = std::make_unique<FlacAudioReaderImpl>();
	impl->register_metadata_handler(
			std::make_unique<FlacDefaultMetadataHandler>());
	impl->register_error_handler(
			std::make_unique<FlacDefaultErrorHandler>());

	return std::make_unique<AudioReader>(std::move(impl));
}


std::unique_ptr<FileReaderDescriptor> DescriptorFlac::do_clone() const
{
	return std::make_unique<DescriptorFlac>();
}


// Add this descriptor to the audio descriptor registry

namespace {

const auto d = RegisterDescriptor<DescriptorFlac>{};

} // namespace

} // namespace v_1_0_0
} // namespace arcsdec

