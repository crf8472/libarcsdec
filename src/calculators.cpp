/**
 * \internal
 *
 * \file
 *
 * \brief Implements symbols from calculators.hpp.
 */

#ifndef LIBARCSDEC_CALCULATORS_HPP_
#include "calculators.hpp"
#endif
#ifndef LIBARCSDEC_CALCULATORS_DETAILS_HPP_
#include "calculators_details.hpp"
#endif

#include <cstdint>       // for uint16_t, int64_t
#include <memory>        // for unique_ptr, make_unique
#include <string>        // for string, to_string
#include <utility>       // for pair, move, make_pair
#include <vector>        // for vector

#ifndef LIBARCSTK_CALCULATE_HPP_
#include <arcstk/calculate.hpp> // for Checksums, Points...
#endif
#ifndef LIBARCSTK_IDENTIFIER_HPP_
#include <arcstk/identifier.hpp>// for ARId, make_arid
#endif
#ifndef LIBARCSTK_LOGGING_HPP_
#include <arcstk/logging.hpp>   // for ARCS_LOG, _ERROR, _WARNING, _INFO, _DEBUG
#endif
#ifndef LIBARCSTK_METADATA_HPP_
#include <arcstk/metadata.hpp>  // for ToC
#endif

#ifndef LIBARCSDEC_AUDIOREADER_HPP_
#include "audioreader.hpp"      // for AudioReader
#endif
#ifndef LIBARCSDEC_DESCRIPTOR_HPP_
#include "descriptor.hpp"
#endif
#ifndef LIBARCSDEC_SELECTION_HPP_
#include "selection.hpp"        // for FormatList,FileReaders,FileReaderSelector
#endif
#ifndef LIBARCSDEC_METAPARSER_HPP_
#include "metaparser.hpp"       // for MetadataParser
#endif
#ifndef LIBARCSDEC_SAMPLEPROC_HPP_
#include "sampleproc.hpp"       // for SampleProcessor, BLOCKSIZE
#endif


namespace arcsdec
{
inline namespace v_1_0_0
{

using arcstk::ARId;
using arcstk::AudioSize;
using arcstk::ChecksumSet;
using arcstk::Checksums;
using arcstk::Context;
using arcstk::Points;
using arcstk::Settings;
using arcstk::ToC;
using arcstk::make_arid;


// calculate_details.hpp

namespace read
{
namespace details
{

// ensure_leadout


AudioSize ensure_leadout(const AudioSize& leadout,
		const AudioReader& reader, const std::string& audiofilename)
{
	if (!leadout.zero())
	{
		return leadout;
	}

	ARCS_LOG_DEBUG <<
		"Empty leadout passed, acquire size from audio file";

	return reader.acquire_size(audiofilename);
}

} // namespace details
} // namespace read


// calculate.hpp

namespace calc
{

using arcsdec::read::BLOCKSIZE;

// ToCParser


ToC ToCParser::parse(const std::string& metafilename) const
{
	if (metafilename.empty())
	{
		ARCS_LOG_ERROR <<
			"ToC info was requested but metadata filename was empty";

		using arcsdec::read::FileReadException;
		throw FileReadException(
				"Requested metadata file parser for empty filename.");
	}

	return create(metafilename)->parse(metafilename);
}


// AudioInfo


AudioSize AudioInfo::size(const std::string& filename) const
{
	return create(filename)->acquire_size(filename);
}


// ARCSCalculator


ARCSCalculator::ARCSCalculator(const ChecksumtypeSet& typeset)
	: types_             { typeset }
	, read_buffer_size_  { BLOCKSIZE::DEFAULT }
{
	/* empty */
}


ARCSCalculator::ARCSCalculator()
	: ARCSCalculator(
			{ arcstk::checksum::type::ARCS1, arcstk::checksum::type::ARCS2 })
{
	/* empty */
}


std::pair<Checksums, ToC> ARCSCalculator::calculate(
		const std::string& audiofilename,
		const ToC& toc)
{
	ARCS_LOG_DEBUG << "Calculate by ToC and single audiofilename";

	// A Calculation requires two informations about the input audio data:
	//
	// 1.) the LBA offset of each track
	//	=>	which are either known at this point by inspecting the ToC or by
	//		knowing that the audiofile represents a single track.
	//
	// 2.) the size of the audio input, which requires to know at least one of
	// the following four:
	//	a) the LBA track offset of the leadout frame
	//	b) the total number of 16bit samples in <audiofilename>
	//	c) the total number of bytes in <audiofilename> representing samples
	//	d) the length of the last track
	//	=>	which may or may not be represented in the ToC

	// Not all ToC providing file formats contain sufficient information to
	// calculate the leadout - simple CueSheets for example do not. Therefore,
	// ToCs are accepted to be incomplete up to this point. They may contain
	// only the offstes but not the leadout.

	// However, this means we additionally have to inspect the audio file to get
	// the missing information.

	// The total PCM byte count is exclusively known to the AudioReader in
	// the process of reading the audio file. (We cannot deduce it from the
	// mere file size.) However, we do not intend to actually read the audio
	// samples. Hence, to know the leadout, we actually have to create an
	// AudioReader, open the file and get the information. Since this is an
	// expensive operation, we want to keep the reader.

	const auto [ track_checksums, leadout ] {
		calculate(audiofilename, Settings { Context::ALBUM }, types(),
				toc.leadout(), toc.offsets())
	};

	if (toc.leadout() == leadout)
	{
		return std::make_pair(track_checksums, toc);
	}

	auto updated_toc { toc };
	updated_toc.set_leadout(leadout);

	return std::make_pair(track_checksums, updated_toc);
}


Checksums ARCSCalculator::calculate(
	const std::vector<std::string>& audiofilenames,
	const bool first_file_is_first_track,
	const bool last_file_is_last_track)
{
	ARCS_LOG_DEBUG <<
		"Calculate by audiofilenames and flags for 1st and last track";

	if (audiofilenames.empty())
	{
		return Checksums(0);
	}

	const bool single_file { audiofilenames.size() == 1 };

	// Calculate first track

	ChecksumSet track {

		// Apply back skipping request on first file only if it's also the last

		calculate(audiofilenames.front(), first_file_is_first_track,
			(single_file ? last_file_is_last_track : false))
	};

	auto checksums = Checksums{};
	checksums.push_back(track);

	// Avoid calculating a single track track twice

	if (single_file)
	{
		return checksums;
	}

	// Calculate second to second last track

	for (uint16_t i = 1; i < audiofilenames.size() - 1; ++i)
	{
		track = calculate(audiofilenames[i], false, false);

		checksums.push_back(track);
	}

	// Calculate last track

	track = calculate(audiofilenames.back(), false, last_file_is_last_track);

	checksums.push_back(track);

	return checksums;
}


ChecksumSet ARCSCalculator::calculate(
	const std::string& audiofilename,
	const bool is_first_track,
	const bool is_last_track)
{
	ARCS_LOG_DEBUG <<
		"Calculate by single audiofilename and flags for 1st and last track";

	const auto ctx { to_context(is_first_track, is_last_track) };

	const auto [ checksums, leadout ] {
		calculate(audiofilename, Settings { ctx }, types(), {/*no size*/},
				{/*no offsets*/})
	};

	if (checksums.empty())
	{
		return ChecksumSet { {/*0*/} };
	}

	return checksums[0];
}


std::pair<Checksums, AudioSize> ARCSCalculator::calculate(
		const std::string& audiofilename,
		const Settings& settings, const ChecksumtypeSet& types,
		const AudioSize& leadout, const Points& offsets)
{
	ARCS_LOG_DEBUG <<
		"Calculate by single audiofilename and complete input data";

	// FIXME swap the following 2 lines and receive_samples<>() will break!
	auto processor = CalculationProcessor { types, settings, offsets, leadout };
	auto reader { create(audiofilename) };
	// It only works if 1. processor and 2. reader

	using std::to_string;

	// Configure number of samples per read

	const auto buffer_size { read_buffer_size() };

	if (BLOCKSIZE::MIN <= buffer_size && buffer_size <= BLOCKSIZE::MAX)
	{
		ARCS_LOG(DEBUG1) << "Chunk size for reading samples: "
			<< to_string(buffer_size) << " bytes";

		reader->set_samples_per_read(buffer_size);

	} else
	{
		// Buffer size is illegal.
		// Do nothing, AudioReaderImpl uses its default.

		ARCS_LOG_WARNING << "Specified buffer size of " << buffer_size
			<< " bytes is not within the legal range of "
			<< BLOCKSIZE::MIN << " - " << BLOCKSIZE::MAX
			<< " samples. Fall back to default: "
			<< reader->samples_per_read()
			<< " bytes";
	}

	// Perform

	reader->set_handler(&processor);
	reader->set_processor(&processor);
	reader->process_file(audiofilename);

	// Check results

	const auto checksums = processor.result();

	if (checksums.size() == 0)
	{
		ARCS_LOG_ERROR << "Calculations lead to no result, return empty set";
	}

	return { checksums, leadout }; // TODO use updated leadout
}


void ARCSCalculator::set_types(const ChecksumtypeSet& typeset)
{
	types_ = typeset;
}


ChecksumtypeSet ARCSCalculator::types() const
{
	return types_;
}


void ARCSCalculator::set_read_buffer_size(const int64_t total_samples)
{
	read_buffer_size_ = total_samples;
}


int64_t ARCSCalculator::read_buffer_size() const
{
	return read_buffer_size_;
}


Context ARCSCalculator::to_context(
	const bool is_first_track,
	const bool is_last_track) const
{
	auto context = Context { Context::TRACK };

	const auto flags { is_first_track + 2 * is_last_track };
	switch (flags)
	{
		case 3: context = Context::ALBUM;       break;
		case 2: context = Context::LAST_TRACK;  break;
		case 1: context = Context::FIRST_TRACK; break;
		case 0: context = Context::TRACK;       break;
		default: ;
	}

	return context;
}


// ARIdCalculator


ARIdCalculator::ARIdCalculator()
	: audio_ { /* default */ }
{
	/* empty */
}


ARId ARIdCalculator::calculate(const std::string& metafilename,
		const std::string& audiofilename) const
{
	const auto toc { create(metafilename)->parse(metafilename) };

	return calculate(toc, audiofilename);
}


ARId ARIdCalculator::calculate(const ToC& toc,
		const std::string& audiofilename) const
{
	if (toc.complete())
	{
		return make_arid(toc);
	}

	return make_arid(toc, audio()->size(audiofilename));
}


const AudioInfo* ARIdCalculator::audio() const
{
	return &audio_;
}


void ARIdCalculator::set_audio(const AudioInfo& audio)
{
	audio_ = audio;
}

} // namespace calc

} // namespace v_1_0_0
} // namespace arcsdec

