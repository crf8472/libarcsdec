/**
 * \file
 *
 * \brief Implements audio reader for FLAC audio files.
 */

#ifndef LIBARCSDEC_READERFLAC_HPP_
#include "readerflac.hpp"
#endif
#ifndef LIBARCSDEC_READERFLAC_DETAILS_HPP_
#include "readerflac_details.hpp"
#endif

#include <limits>      // for numeric_limits
#include <memory>      // for unique_ptr
#include <set>         // for set
#include <sstream>     // for ostringstream
#include <string>      // for string
#include <utility>     // for make_unique, move

#include <FLAC++/decoder.h>		// for FLAC::Decoder::File,
								// FLAC__StreamDecoderWriteStatus,
								// FLAC__StreamDecoderErrorStatus
#include <FLAC++/metadata.h>	// for FLAC::Metadata::StreamInfo,
								// FLAC__StreamMetadata
								// for FLAC__Frame

#ifndef LIBARCSTK_METADATA_HPP_
#include <arcstk/metadata.hpp>  // for AudioSize, UNIT
#endif
#ifndef LIBARCSTK_LOGGING_HPP_
#include <arcstk/logging.hpp>   // for ARCS_LOG_ERROR,...
#endif

#ifndef LIBARCSDEC_AUDIOREADER_HPP_
#include "audioreader.hpp"      // for AudioReaderImpl, InvalidAudioException
#endif
#ifndef LIBARCSDEC_LIBINSPECT_HPP_
#include "libinspect.hpp"       // for libinfo_entry_filepath
#endif
#ifndef LIBARCSDEC_SELECTION_HPP_
#include "selection.hpp"        // for RegisterDescriptor
#endif


namespace arcsdec
{
inline namespace v_1_0_0
{
namespace read
{
namespace details::flac
{

using arcstk::AudioSize;


// FlacMetadataHandler


void FlacMetadataHandler::update(const FLAC::Metadata::StreamInfo& streaminfo)
{
	do_update(streaminfo);
}


void FlacMetadataHandler::validate(const FLAC::Metadata::StreamInfo& streaminfo)
{
	do_validate(streaminfo);
}


void FlacMetadataHandler::cuesheet(const FLAC::Metadata::CueSheet& cuesheet)
{
	do_cuesheet(cuesheet);
}


void FlacMetadataHandler::register_handler(AudioEventHandler* handler)
{
	handler_ = handler;
}


AudioEventHandler* FlacMetadataHandler::handler()
{
	return handler_;
}


// FlacErrorHandler


void FlacErrorHandler::error(::FLAC__StreamDecoderErrorStatus status)
{
	do_error(status);
}


// FlacValidator


AudioValidator::codec_set_type FlacValidator::do_codecs() const
{
	return { Codec::FLAC };
}


bool FlacValidator::validate(const FLAC::Metadata::StreamInfo& streaminfo)
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


// FlacDefaultMetadataHandler


void FlacDefaultMetadataHandler::do_update(
		const FLAC::Metadata::StreamInfo& metadata)
{
	if (auto* handler = this->handler())
	{
		const auto total_samples =
					::FLAC::Metadata::StreamInfo{*metadata}.get_total_samples();

		handler->audiosize({ cast_to_int32(total_samples), UNIT::SAMPLES });
	}

	this->validate(*metadata);
	// Note: Streaminfo could already have been validated explicitly
}


void FlacDefaultMetadataHandler::do_validate(
		const FLAC::Metadata::StreamInfo& metadata)
{
	if (!validator_.validate(metadata))
	{
		const auto error = validator_.last_error();
		ARCS_LOG_ERROR << "Validation of Flac file failed. Error is:" << error;
		throw InvalidAudioException(error);
	}
}


void FlacDefaultMetadataHandler::do_cuesheet(
		const FLAC::Metadata::CueSheet& /*cuesheet*/)
{
	ARCS_LOG_INFO << "Ignore CueSheet found in FLAC file";

	// TODO Implement
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


// FlacAudioFile


::FLAC__StreamDecoderWriteStatus FlacAudioFile::write_callback(
		const ::FLAC__Frame* frame,
		const ::FLAC__int32* const buffer[]) // NOLINT(*-avoid-c-arrays)
{
	const arcstk::PlanarSamples<::FLAC__int32> sequence {
		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		buffer[0], buffer[1],
		frame->header.blocksize,
		false /* channels are never swapped in fLaC */
	};

	// fLaC says: "Where defined, the channel order follows SMPTE/ITU-R
	// recommendations." and only defines left/right orderings.
	// We only respect left/right ordering here.

	if (processor_)
	{
		processor_->receive_samples(sequence);
	} else
	{
		ARCS_LOG_ERROR << "No processor available, samples will be dropped";
		// TODO throw
	}

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}


void FlacAudioFile::metadata_callback(
		const ::FLAC__StreamMetadata* metadata)
{
	switch (metadata->type)
	{
		case FLAC__METADATA_TYPE_STREAMINFO:

			metadata_handler_->update(*metadata);
			break;

		case FLAC__METADATA_TYPE_CUESHEET:

			metadata_handler_->cuesheet(*metadata);
			break;

		default:
			break;
	}
}


void FlacAudioFile::error_callback(
		::FLAC__StreamDecoderErrorStatus status)
{
	error_handler_->error(status);
}


void FlacAudioFile::register_sample_processor(
		calc::CalculationProcessor* processor)
{
	processor_ = processor;
}


void FlacAudioFile::register_metadata_handler(FlacMetadataHandler* handler)
{
	metadata_handler_ = handler;
}


void FlacAudioFile::register_error_handler(FlacErrorHandler* handler)
{
	error_handler_ = handler;
}


void FlacAudioFile::process(const std::string& filename)
{
	set_md5_checking(false); // TODO part of validation?

	// Initialize

	const auto init_status = this->init(filename);

	if (init_status != ::FLAC__STREAM_DECODER_INIT_STATUS_OK)
	{
		ARCS_LOG_ERROR << "Initializing decoder failed."
			<< " FLAC__StreamDecoderInitStatus: "
			<< std::string{
					::FLAC__StreamDecoderInitStatusString[init_status] };
		// TODO finish() required?
		return;
	}

	ARCS_LOG(DEBUG3) << "Initialized decoder successfully";

	// Process decoded samples

	const bool success = this->process_until_end_of_stream();

	if (!success)
	{
		ARCS_LOG_ERROR << "Decoding failed."
			<< " Last decoder state: "
			<< std::string { this->get_state().as_cstring() };
	}

	this->finish();
}


// FlacAudioReaderImpl


void FlacAudioReaderImpl::register_metadata_handler(
		std::unique_ptr<FlacMetadataHandler> handler)
{
	if (handler)
	{
		metadata_handler_ = std::move(handler);
		metadata_handler_->register_handler(this->handler());
	}
}


void FlacAudioReaderImpl::register_error_handler(
		std::unique_ptr<FlacErrorHandler> hndlr)
{
	error_handler_ = std::move(hndlr);
}


AudioSize FlacAudioReaderImpl::do_acquire_size(const std::string& filename)
{
	auto streaminfo = ::FLAC::Metadata::StreamInfo{};

	::FLAC::Metadata::get_streaminfo(filename.c_str(), streaminfo);

	// Commented out, acquire_size() does not perform validation
	//metadata_handler_->validate(streaminfo);

	return { cast_to_int32(streaminfo.get_total_samples()), UNIT::SAMPLES };
}


void FlacAudioReaderImpl::do_process_file(const std::string& filename)
{
	FlacAudioFile file {};

	file.register_sample_processor(this->sample_processor());
	file.register_metadata_handler(metadata_handler_.get());
	file.register_error_handler(error_handler_.get());

	auto* handler = this->handler();

	if (handler)
	{
		handler->start_input();
	}

	file.process(filename);

	if (handler)
	{
		handler->end_input();
	}

	ARCS_LOG_INFO << "Audio file closed";
}


std::unique_ptr<FileReaderDescriptor> FlacAudioReaderImpl::do_descriptor()
	const
{
	return std::make_unique<DescriptorFlac>();
}

} // namespace details::flac


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

} // namespace read


// Add this descriptor to the audio descriptor registry

namespace {

using select::RegisterDescriptor;
using read::DescriptorFlac;

const auto d = RegisterDescriptor<DescriptorFlac>{};

} // namespace

} // namespace v_1_0_0
} // namespace arcsdec

