/**
 * \file
 *
 * \brief Implements Libsndfile-based generic audio reader.
 */

#ifndef __LIBARCSDEC_READERSNDFILE_HPP__
#include "readersndfile.hpp"
#endif
#ifndef __LIBARCSDEC_READERSNDFILE_DETAILS_HPP__
#include "readersndfile_details.hpp" // for LibsndfileAudioReaderImpl
#endif

#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"  // for AudioReaderImpl, InvalidAudioException
#endif
#ifndef __LIBARCSDEC_LIBINSPECT_HPP__
#include "libinspect.hpp"   // for first_libname_match
#endif
#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"    // for RegisterDescriptor
#endif

#ifndef __LIBARCSTK_METADATA_HPP__
#include <arcstk/metadata.hpp>   // for AudioSize, CDDA
#endif
#ifndef __LIBARCSTK_SAMPLES_HPP__
#include <arcstk/samples.hpp>    // for SampleSequence
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>    // for ARCS_LOG, _ERROR, _INFO, _DEBUG
#endif

#ifndef SNDFILE_HH
#include <sndfile.hh>  // for SndfileHandle, SFM_READ, SF_FORMAT_PCM_16
#endif

#include <cstdint>  // for int16_t, unit32_t, uint64_t
#include <memory>   // for unique_ptr
#include <set>      // for set
#include <sstream>  // for ostringstream
#include <string>   // for string, to_string
#include <utility>  // for make_unique, move
#include <vector>   // for vector


namespace arcsdec
{

inline namespace v_1_0_0
{

using arcstk::AudioSize;
using arcstk::CDDA;
using arcstk::SampleSequence;


namespace details
{
namespace sndfile
{


// LibsndfileAudioReaderImpl


LibsndfileAudioReaderImpl::~LibsndfileAudioReaderImpl() noexcept = default;


std::unique_ptr<AudioSize> LibsndfileAudioReaderImpl::do_acquire_size(
	const std::string& filename)
{
	using arcstk::AudioSize;
	using arcstk::UNIT;

	auto audiofile = SndfileHandle { filename };

	// FIXME works only for WAV??
	/* libsndfile's frames == libarcstk's samples */
	return std::make_unique<AudioSize>(audiofile.frames(), UNIT::SAMPLES );
}


void LibsndfileAudioReaderImpl::do_process_file(const std::string& filename)
{
	using arcstk::AudioSize;
	using arcstk::UNIT;

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

	this->signal_updateaudiosize(audiosize);

	// Prepare Read buffer (16 bit samples)

	const std::size_t buffer_len =
		this->samples_per_read() * CDDA::NUMBER_OF_CHANNELS;

	auto buffer   = std::vector<int16_t>(buffer_len);
	auto sequence = SampleSequence<int16_t, false>{};

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

		sequence.wrap_int_buffer(&buffer[0], buffer.size());

		ARCS_LOG(DEBUG1) << "  Size: "
				<< (buffer.size() * sizeof(buffer[0])) << " bytes";
		ARCS_LOG(DEBUG1) << "        "
				<< (buffer.size() / CDDA::NUMBER_OF_CHANNELS)
				<< " Stereo PCM samples (32 bit)";

		this->signal_appendsamples(sequence.begin(), sequence.end());

		sample_count += sequence.size();
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


// Add this descriptor to the audio descriptor registry

namespace {

const auto d = RegisterDescriptor<DescriptorSndfile>{};

} // namespace

} // namespace v_1_0_0
} // namespace arcsdec

