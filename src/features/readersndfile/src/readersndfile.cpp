/**
 * \file
 *
 * \brief Implements Libsndfile-based generic audio reader.
 */

#ifndef LIBARCSDEC_READERSNDFILE_HPP_
#include "readersndfile.hpp"
#endif
#ifndef LIBARCSDEC_READERSNDFILE_DETAILS_HPP_
#include "readersndfile_details.hpp" // for LibsndfileAudioReaderImpl
#endif

#include <cstdint>  // for int16_t, unit32_t, uint64_t
#include <limits>   // for numeric_limits
#include <memory>   // for unique_ptr
#include <set>      // for set
#include <sstream>  // for ostringstream
#include <string>   // for string, to_string
#include <utility>  // for make_unique, move
#include <vector>   // for vector

#ifndef SNDFILE_HH
#include <sndfile.hh>  // for SndfileHandle, SFM_READ, SF_FORMAT_PCM_16
#endif

#ifndef LIBARCSTK_METADATA_HPP_
#include <arcstk/metadata.hpp>   // for AudioSize, CDDA
#endif
#ifndef LIBARCSTK_SAMPLES_HPP_
#include <arcstk/samples.hpp>    // for SampleSequence
#endif
#ifndef LIBARCSTK_LOGGING_HPP_
#include <arcstk/logging.hpp>    // for ARCS_LOG, _ERROR, _INFO, _DEBUG
#endif

#ifndef LIBARCSDEC_AUDIOREADER_HPP_
#include "audioreader.hpp"  // for AudioReaderImpl, InvalidAudioException
#endif
#ifndef LIBARCSDEC_LIBINSPECT_HPP_
#include "libinspect.hpp"   // for first_libname_match
#endif
#ifndef LIBARCSDEC_SELECTION_HPP_
#include "selection.hpp"    // for RegisterDescriptor
#endif


namespace arcsdec
{

inline namespace v_1_0_0
{
namespace read
{

using arcstk::AudioSize;
using arcstk::CDDA;
using arcstk::InterleavedSamples;


namespace details
{
namespace sndfile
{


// LibsndfileAudioReaderImpl


LibsndfileAudioReaderImpl::~LibsndfileAudioReaderImpl() noexcept = default;


AudioSize LibsndfileAudioReaderImpl::do_acquire_size(
		const std::string& filename)
{
	using arcstk::AudioSize;
	using arcstk::UNIT;

	auto audiofile = SndfileHandle { filename };

	// FIXME works only for WAV??
	/* libsndfile's frames == libarcstk's samples */
	if (const auto total_values = audiofile.frames();
			total_values <= std::numeric_limits<int32_t>::max())
	{
		return { static_cast<int32_t>(total_values), UNIT::SAMPLES };
	} else
	{
		// FIXME total_values is too big for AudioSize
		throw std::runtime_error("Could not acquire size, too many samples");
	}

	return AudioSize{}; // unreachable
}


void LibsndfileAudioReaderImpl::do_process_file(const std::string& filename)
{
	using arcstk::AudioSize;
	using arcstk::UNIT;

	using std::cbegin;
	using std::cend;

	auto* handler = this->handler();

	if (handler)
	{
		handler->start_input();
	}

	auto audiofile = SndfileHandle { filename, SFM_READ };

	// TODO Check whether this was successful

	// Perform validation
	// TODO Use validator

	if (audiofile.samplerate() - CDDA::SAMPLES_PER_SECOND != 0)
	{
		ARCS_LOG_DEBUG << "Samplerate: " << audiofile.samplerate();
		return;
	}

	if (audiofile.channels() - CDDA::NUMBER_OF_CHANNELS != 0)
	{
		ARCS_LOG_DEBUG << "Channels: " << audiofile.channels();
		return;
	}

	if (!(audiofile.format() | SF_FORMAT_PCM_16))
	{
		using std::to_string;
		ARCS_LOG_DEBUG << "Format: " << to_string(audiofile.format());
		return;
	}

	ARCS_LOG_INFO << "Validation completed: file seems to be CDDA";


	// Update Calculation with sample count

	const auto total_samples = audiofile.frames();
	const auto audiosize     = to_audiosize(total_samples, UNIT::SAMPLES);

	if (handler)
	{
		handler->audiosize(audiosize);
	}

	// Prepare Read buffer (16 bit samples)

	const std::size_t buffer_len =
		this->samples_per_read() * CDDA::NUMBER_OF_CHANNELS;

	auto buffer   = std::vector<int16_t>(buffer_len);
	using sequence_type = arcstk::InterleavedSamples<int16_t>;
	// SampleSequence<int16_t, false>{};

	// Checking

	auto ints_in_block = int { 0 };

	// Logging

	auto sample_count     = uint64_t { 0 };
	auto blocks_processed = uint32_t { 0 };

	// Read blocks

	while ((ints_in_block = audiofile.read (&buffer[0], buffer_len)))
	{
		++blocks_processed;

		ARCS_LOG_DEBUG << "READ BLOCK " << blocks_processed;

		// Expected amount was read?

		if (static_cast<unsigned int>(ints_in_block) != buffer_len)
		{
			// This is allowed only for the last block

			const auto expected_total { audiosize.samples() - sample_count };

			if (expected_total != ints_in_block / CDDA::NUMBER_OF_CHANNELS)
			{
				auto ss = std::ostringstream{};
				ss << "  Block contains "
					<< ints_in_block
					<< " integers, expected were "
					<< buffer_len
					<< ". Resize buffer";

				ARCS_LOG_ERROR << ss.str();

				throw FileReadException(ss.str());
			}

			// Resize to actual amount of integers

			ARCS_LOG_INFO << "  Last block contains "
					<< ints_in_block
					<< " integers instead of "
					<< buffer_len
					<< ". Resize buffer";

			buffer.resize(ints_in_block);
		}

		// FIXME respect channel ordering
		auto sequence = sequence_type { &buffer[0], buffer.size(), false };

		ARCS_LOG(DEBUG1) << "  Size: "
				<< (buffer.size() * sizeof(buffer[0])) << " bytes";
		ARCS_LOG(DEBUG1) << "        "
				<< (buffer.size() / CDDA::NUMBER_OF_CHANNELS)
				<< " Stereo PCM samples (32 bit)";

		if (auto* proc = sample_processor(); proc)
		{
			proc->receive_samples(sequence);
		} else
		{
			// TODO Error?
		}

		sample_count += sequence.size();
	}

	if (handler)
	{
		handler->end_input();
	}
}


std::unique_ptr<FileReaderDescriptor> LibsndfileAudioReaderImpl::do_descriptor()
	const
{
	return std::make_unique<DescriptorSndfile>();
}


} // namespace sndfile
} // namespace details


// DescriptorSndfile


DescriptorSndfile::~DescriptorSndfile() noexcept = default;


std::string DescriptorSndfile::do_id() const
{
	return "libsndfile";
}


std::string DescriptorSndfile::do_name() const
{
	return "Libsndfile";
}


std::set<Format> DescriptorSndfile::define_formats() const
{
	return {
		Format::WAV,
		Format::FLAC,
		Format::AIFF
		//Format::OGG // FIXME Accept OGG once it works!
		//Format::CAF // FIXME Accept CAF once it works!
	};
}


std::set<Codec> DescriptorSndfile::define_codecs() const
{
	return {
		Codec::PCM_S16BE,
		Codec::PCM_S16BE_PLANAR,
		Codec::PCM_S16LE,
		Codec::PCM_S16LE_PLANAR,
		Codec::PCM_S32BE,
		Codec::PCM_S32BE_PLANAR,
		Codec::PCM_S32LE,
		Codec::PCM_S32LE_PLANAR,
		Codec::FLAC,
		Codec::ALAC
	};
}


LibInfo DescriptorSndfile::do_libraries() const
{
	return { libinfo_entry_filepath("libsndfile") };
}


std::unique_ptr<FileReader> DescriptorSndfile::do_create_reader() const
{
	using details::sndfile::LibsndfileAudioReaderImpl;

	auto impl = std::make_unique<LibsndfileAudioReaderImpl>();

	return std::make_unique<AudioReader>(std::move(impl));
}


std::unique_ptr<FileReaderDescriptor> DescriptorSndfile::do_clone() const
{
	return std::make_unique<DescriptorSndfile>();
}

} // namespace read


// Add this descriptor to the audio descriptor registry

namespace {

using select::RegisterDescriptor;
using read::DescriptorSndfile;

const auto d = RegisterDescriptor<DescriptorSndfile>{};

} // namespace

} // namespace v_1_0_0
} // namespace arcsdec

