/**
 * \file
 *
 * \brief Implementing high-level API for calculating ARCSs of files.
 */

#ifndef __LIBARCSDEC_CALCULATORS_HPP__
#include "calculators.hpp"
#endif
#ifndef __LIBARCSDEC_CALCULATORS_DETAILS_HPP__
#include "calculators_details.hpp"
#endif

#include <cstdint>       // for uint16_t, int64_t
#include <iterator>      // for distance
#include <memory>        // for unique_ptr, make_unique
#include <stdexcept>     // for logic_error, runtime_error
#include <string>        // for string, to_string
#include <unordered_set> // for unordered_set
#include <utility>       // for pair, move, make_pair
#include <vector>        // for vector
#include <iostream>

#ifndef __LIBARCSTK_ALGORITHMS_HPP__
#include <arcstk/algorithms.hpp>// for AccurateRipV1V2...
#endif
#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>// for ARId, make_arid
#endif
#ifndef __LIBARCSTK_METADATA_HPP__
#include <arcstk/metadata.hpp>    // for ToC
#endif
#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp> // for Checksums, SampleInputIterator, Points...
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>   // for ARCS_LOG, _ERROR, _WARNING, _INFO, _DEBUG
#endif

#ifndef __LIBARCSDEC_DESCRIPTOR_HPP__
#include "descriptor.hpp"
#endif
#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"        // for FormatList,FileReaders,FileReaderSelector
#endif
#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"      // for AudioReader
#endif
#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include "metaparser.hpp"       // for MetadataParser
#endif
#ifndef __LIBARCSDEC_SAMPLEPROC_HPP__
#include "sampleproc.hpp"       // for SampleProcessor, BLOCKSIZE
#endif


namespace arcsdec
{
inline namespace v_1_0_0
{

using arcstk::ToC;
using arcstk::Algorithm;
using arcstk::ARId;
using arcstk::AudioSize;
using arcstk::Calculation;
using arcstk::Checksums;
using arcstk::ChecksumSet;
using arcstk::Context;
using arcstk::Points;
using arcstk::SampleInputIterator;
using arcstk::Settings;
using arcstk::make_arid;


/**
 * \brief SampleProcessor that logs the received signals.
 */
/*
class LogProcessor final : public SampleProcessor
{
public:

	LogProcessor() = default;
	~LogProcessor() noexcept final = default;
	LogProcessor(const LogProcessor& rhs) noexcept = delete;
	LogProcessor& operator = (const LogProcessor& rhs) noexcept = delete;

private:

	void do_start_input() final;

	void do_append_samples(SampleInputIterator, SampleInputIterator) final;

	void do_update_audiosize(const AudioSize& ) final;

	void do_end_input() final;
};


void LogProcessor::do_start_input()
{
	ARCS_LOG(DEBUG2) << "LogProcessor received: START INPUT";
}


void LogProcessor::do_append_samples(SampleInputIterator, SampleInputIterator)
{
	ARCS_LOG(DEBUG2) << "LogProcessor received: APPEND SAMPLES";
}


void LogProcessor::do_update_audiosize(const AudioSize& )
{
	ARCS_LOG(DEBUG2) << "LogProcessor received: UPDATE AUDIOSIZE";
}


void LogProcessor::do_end_input()
{
	ARCS_LOG(DEBUG2) << "LogProcessor received: END INPUT";
}
*/


/**
 * \brief SampleProcessor that updates a Calculation.
 */
class CalculationProcessor final : public SampleProcessor
{
public:

	/**
	 * \brief Converting constructor for Calculation instances.
	 *
	 * \param[in] calculation The Calculation to use
	 */
	CalculationProcessor(Calculation& calculation)
		: calculation_ (&calculation)
		, total_sequences_ { 0 }
	{
		/* empty */
	}

	/**
	 * \brief Default destructor.
	 */
	~CalculationProcessor() noexcept final = default;

	// not copy-constructible, not copy-assignable
	explicit CalculationProcessor(const CalculationProcessor& rhs) noexcept
		= delete;
	CalculationProcessor& operator = (const CalculationProcessor& rhs) noexcept
		= delete;

	explicit CalculationProcessor(CalculationProcessor&& rhs) noexcept;
	CalculationProcessor& operator = (CalculationProcessor&& rhs) noexcept;

	/**
	 * \brief Number of sample sequence that this instance has processed.
	 *
	 * This value is identical to how often append_samples() was called.
	 *
	 * \return Number of sequences processed
	 */
	int64_t sequences_processed() const;

	/**
	 * \brief Number of PCM 32 bit samples processed.
	 *
	 * \return Number of samples processed
	 */
	int64_t samples_processed() const;

private:

	void do_start_input() final;

	void do_append_samples(SampleInputIterator begin, SampleInputIterator end)
		final;

	void do_update_audiosize(const AudioSize& size) final;

	void do_end_input() final;

	/**
	 * \brief Internal pointer to the calculation to wrap.
	 */
	Calculation* calculation_;

	/**
	 * \brief Sequence counter.
	 *
	 * Counts the calls of SampleProcessor::append_samples.
	 */
	int64_t total_sequences_;
};


CalculationProcessor::CalculationProcessor(CalculationProcessor&& rhs) noexcept
	: calculation_     { std::move(rhs.calculation_)     }
	, total_sequences_ { std::move(rhs.total_sequences_) }
{
	// empty
}


CalculationProcessor& CalculationProcessor::operator = (
		CalculationProcessor&& rhs) noexcept
{
	calculation_     = std::move(rhs.calculation_);
	total_sequences_ = std::move(rhs.total_sequences_);
	return *this;
}


void CalculationProcessor::do_start_input()
{
	ARCS_LOG(DEBUG2) << "CalculationProcessor received: START INPUT";
}


void CalculationProcessor::do_append_samples(SampleInputIterator begin,
		SampleInputIterator end)
{
	ARCS_LOG(DEBUG2) << "CalculationProcessor received: APPEND SAMPLES";

	++total_sequences_;
	// Commented out: arcstk::Calculation does this
	//total_samples_ += std::distance(begin, end);

	calculation_->update(begin, end);
}


void CalculationProcessor::do_update_audiosize(const AudioSize& size)
{
	ARCS_LOG(DEBUG2) << "CalculationProcessor received: UPDATE AUDIOSIZE";

	calculation_->update(size);
}


void CalculationProcessor::do_end_input()
{
	ARCS_LOG(DEBUG2) << "CalculationProcessor received: END INPUT";
}


int64_t CalculationProcessor::sequences_processed() const
{
	return total_sequences_;
}


int64_t CalculationProcessor::samples_processed() const
{
	return calculation_->samples_processed();
}


/**
 * \brief SampleProcessor that updates multiple Calculation instances.
 */
class MultiCalculationProcessor final : public SampleProcessor
{
public:

	MultiCalculationProcessor();

	void add(Calculation& c);

private:

	void do_start_input() final;

	void do_append_samples(SampleInputIterator begin, SampleInputIterator end)
		final;

	void do_update_audiosize(const AudioSize& size) final;

	void do_end_input() final;

	/**
	 * \brief Internal pointer to the processors to wrap.
	 */
	std::vector<CalculationProcessor> processors_;
};


MultiCalculationProcessor::MultiCalculationProcessor()
	: processors_ { /* default */ }
{
	// empty
}


void MultiCalculationProcessor::add(Calculation& c)
{
	processors_.emplace_back(c);
}


void MultiCalculationProcessor::do_start_input()
{
	ARCS_LOG(DEBUG2) << "MultiCalculationProcessor received: START INPUT";

	using std::begin;
	using std::end;
	std::for_each(begin(processors_), end(processors_),
			[](SampleProcessor& p)
			{
				p.start_input();
			});
}


void MultiCalculationProcessor::do_append_samples(SampleInputIterator start,
		SampleInputIterator stop)
{
	ARCS_LOG(DEBUG2) << "MultiCalculationProcessor received: APPEND SAMPLES";

	using std::begin;
	using std::end;
	std::for_each(begin(processors_), end(processors_),
			[&start,&stop](SampleProcessor& p)
			{
				p.append_samples(start, stop);
			});
}


void MultiCalculationProcessor::do_update_audiosize(const AudioSize& size)
{
	ARCS_LOG(DEBUG2) << "MultiCalculationProcessor received: UPDATE AUDIOSIZE";

	using std::begin;
	using std::end;
	std::for_each(begin(processors_), end(processors_),
			[&size](SampleProcessor& p)
			{
				p.update_audiosize(size);
			});
}


void MultiCalculationProcessor::do_end_input()
{
	ARCS_LOG(DEBUG2) << "MultiCalculationProcessor received: END INPUT";

	using std::begin;
	using std::end;
	std::for_each(begin(processors_), end(processors_),
			[](SampleProcessor& p)
			{
				p.end_input();
			});
}


// ReaderAndFormatHolder


ReaderAndFormatHolder::ReaderAndFormatHolder()
	: formats_      { FileReaderRegistry::formats() }
	, descriptors_  { FileReaderRegistry::readers() }
{
	/* empty */
}


ReaderAndFormatHolder::~ReaderAndFormatHolder() noexcept = default;


void ReaderAndFormatHolder::set_formats(const FormatList *formats)
{
	formats_ = formats;
}


const FormatList* ReaderAndFormatHolder::formats() const
{
	return formats_;
}


void ReaderAndFormatHolder::set_readers(const FileReaders *descriptors)
{
	descriptors_ = descriptors;
}


const FileReaders* ReaderAndFormatHolder::readers() const
{
	return descriptors_;
}


// default_selection()


template <>
const FileReaderSelection* default_selection<AudioReader>()
{
	return FileReaderRegistry::default_audio_selection();
}


template <>
const FileReaderSelection* default_selection<MetadataParser>()
{
	return FileReaderRegistry::default_toc_selection();
}


// ToCParser


std::unique_ptr<ToC> ToCParser::parse(const std::string& metafilename) const
{
	if (metafilename.empty())
	{
		ARCS_LOG_ERROR <<
			"ToC info was requested but metadata filename was empty";

		throw FileReadException(
				"Requested metadata file parser for empty filename.");
	}

	return create(metafilename)->parse(metafilename);
}


// calculate_details.hpp


// get_algorithms


Algorithms get_algorithms(const ChecksumtypeSet& types)
{
	using Type = arcstk::checksum::type;
	namespace AccurateRip = arcstk::accuraterip;

	Algorithms algorithms;

	// TODO Improve selection logic.

	// Manual logic does not scale. OK for now since there are only
	// 3 algorithms in libarcstk 0.3. libarcstk should have an API function that
	// gives a preferred algorithm for a checksum type. If we add a type like
	// checksum::type::ARCS1and2 we could do this as a bijection like
	// get_algorithm_for_type() without having to consider the entire set of
	// requested types like it is done here.

	if (types.empty()/* default */ || types.size() > 1/* all known types*/)
	{
		algorithms.insert(std::make_unique<AccurateRip::V1and2>());
	} else
	{
		// exactly one type

		using std::cbegin;

		if (Type::ARCS1 == *cbegin(types))
		{
			algorithms.insert(std::make_unique<AccurateRip::V1>());
		} else
		{
			algorithms.insert(std::make_unique<AccurateRip::V2>());
		}
	}

	return algorithms;
}


// get_algorithms_or_throw


Algorithms get_algorithms_or_throw(const ChecksumtypeSet& types)
{
	auto algorithms { get_algorithms(types) };

	if (algorithms.empty())
	{
		throw std::runtime_error(
				"Could not find algorithms for requested types");
		// TODO Print types
	}

	return algorithms;
}


// init_calculations


std::vector<Calculation> init_calculations(const arcstk::Settings& settings,
		const Algorithms& algorithms, const AudioSize& size,
		const Points& points)
{
	auto calculations { std::vector<Calculation>() };
	calculations.reserve(algorithms.size());

	for (const auto& algorithm : algorithms)
	{
		// We cannot move an object out of a set, so we have to clone
		calculations.emplace_back(settings, algorithm->clone(), size, points);
	}

	return calculations;
}


// merge_results


Checksums merge_results(const std::vector<Calculation>& calculations)
{
	const auto size { calculations[0].result().size() };
	auto tracks { std::vector<ChecksumSet>(size, ChecksumSet{0}) };

	using std::begin;
	using std::cbegin;
	using std::cend;

	// Aggregate results in vector 'tracks'

	std::for_each(cbegin(calculations), cend(calculations),
		[&tracks](const Calculation& c)
		{
			auto checksums { c.result() };

			std::transform(cbegin(checksums), cend(checksums), begin(tracks),
				begin(tracks),
				[](const ChecksumSet& s, ChecksumSet& t) -> ChecksumSet
				{
					t.merge(s);
					t.set_length(s.length());
					// FIXME Supposing lengths all-equal is an error
					return t;
				}
			);
		});

	// Convert to Checksums

	auto result = Checksums{};
	std::for_each(begin(tracks), end(tracks),
		[&result](const ChecksumSet& s) { result.push_back(s); });

	return result;
}


// audiofile


std::string audiofilename(const ToC& toc)
{
	// Check whether ToC references exactly one audio file.
	// (Other cases are currently unsupported.)

	const auto audiofilenames { toc.filenames() };

	if (audiofilenames.empty())
	{
		throw std::runtime_error(
				"ToC does not seem to reference any audio file.");
	}

	using std::cbegin;
	using std::cend;

	const std::unordered_set<std::string> name_set(
			cbegin(audiofilenames), cend(audiofilenames));

	if (name_set.size() != 1)
	{
		throw std::runtime_error(
				"ToC does not reference exactly one audio file.");
	}

	return *cbegin(name_set);
}


// process_audio_file


void process_audio_file(const std::string& audiofilename,
		std::unique_ptr<AudioReader> reader, const int64_t buffer_size,
		SampleProcessor& processor)
{
	using std::to_string;

	// Configure AudioReader and process file

	if (BLOCKSIZE::MIN <= buffer_size and buffer_size <= BLOCKSIZE::MAX)
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

	reader->set_processor(processor);
	reader->process_file(audiofilename);
}


// calculate.hpp


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


std::pair<Checksums, ARId> ARCSCalculator::calculate(
		const std::string& audiofilename,
		const ToC& toc)
{
	ARCS_LOG_DEBUG << "Calculate by ToC and single audiofilename";

	using arcstk::make_arid;

	auto size { std::make_unique<AudioSize>() };
	*size = toc.leadout(); // maybe zero

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

	const auto track_checksums {
		calculate(audiofilename, Context::ALBUM, types(),
				size/*get leadout passed here*/, toc.offsets())
	};

	const auto id = !toc.complete() ? make_arid(toc, *size) : make_arid(toc);

	return std::make_pair(track_checksums, *id);
}


Checksums ARCSCalculator::calculate(
	const std::vector<std::string>& audiofilenames,
	const bool& first_file_is_first_track,
	const bool& last_file_is_last_track)
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
	const bool& is_first_track,
	const bool& is_last_track)
{
	ARCS_LOG_DEBUG <<
		"Calculate by single audiofilename and flags for 1st and last track";

	const auto context { to_context(is_first_track, is_last_track) };

	auto ignore_size { std::make_unique<AudioSize>() };

	const auto checksums {
		calculate(audiofilename, context,
				types(), ignore_size, {/*no offsets*/})
	};

	if (checksums.empty())
	{
		return ChecksumSet { 0 };
	}

	return checksums[0];
}


Checksums ARCSCalculator::calculate(const std::string& audiofilename,
		const Settings& settings, const ChecksumtypeSet& types,
		std::unique_ptr<AudioSize>& leadout, const Points& offsets)
{
	ARCS_LOG_DEBUG <<
		"Calculate by single audiofilename and complete input data";

	// Put it all together

	const auto algorithms { get_algorithms_or_throw(types) };

	auto reader { create(audiofilename) };

	if (leadout->zero())
	{
		leadout = reader->acquire_size(audiofilename); // output param
	}

	auto calculations {
		init_calculations(settings, algorithms, *leadout, offsets) };

	// Run

	{
		MultiCalculationProcessor processor{};

		for (auto& c : calculations)
		{
			processor.add(c);
		}

		process_audio_file(audiofilename, std::move(reader), read_buffer_size(),
				processor);
	}

	// Check results

	for (const auto& c : calculations)
	{
		if (not c.complete())
		{
			ARCS_LOG_ERROR << "Calculation not complete "
				"after last input sample: "
				<< "Expected total samples: " << c.samples_expected()
				<< " "
				<< "Processed total samples: " << c.samples_processed();
		}

		if (c.samples_todo() < 0)
		{
			ARCS_LOG_WARNING << "More samples than expected. "
				<< "Expected: " << c.samples_expected()
				<< " "
				<< "Processed: " << c.samples_processed();
		}
	}

	const auto checksums { merge_results(calculations) };

	if (checksums.size() == 0)
	{
		ARCS_LOG_ERROR << "Calculations lead to no result, return empty set";
	}

	return checksums;
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
	const bool& is_first_track,
	const bool& is_last_track) const
{
	auto context = Context { Context::TRACK };

	const auto flags = is_first_track + 2 * is_last_track;
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


// AudioInfo


std::unique_ptr<AudioSize> AudioInfo::size(const std::string& filename) const
{
	return create(filename)->acquire_size(filename);
}


// ARIdCalculator


ARIdCalculator::ARIdCalculator()
	: audio_ { /* default */ }
{
	/* empty */
}


std::unique_ptr<ARId> ARIdCalculator::calculate(
		const std::string& metafilename) const
{
	const auto toc { create(metafilename)->parse(metafilename) };

	if (toc->complete())
	{
		return make_arid(*toc);
	}

	// As in ARCSCalculator::calculate(), we have the offsets but not the
	// leadout. We thus have to inspect the actual audio input. Since we only
	// know the name of the toc file, the name of the audio file must be derived
	// from this information.

	ARCS_LOG_INFO << "Incomplete ToC and no audio file provided."
		" Try to find audio file references in ToC.";

	auto audiofile = audiofilename(*toc);

	ARCS_LOG_INFO << "Found audiofile: " << audiofile << ", try loading";

	// Use path from metafile (if any) as search path for the audio file

	// TODO use C++17 std::filesystem
	auto pos { metafilename.find_last_of("/\\") }; // XXX Really portable?

	if (pos != std::string::npos)
	{
		// If pos+1 would be illegal, Parser would already have Thrown
		auto path = metafilename.substr(0, pos + 1);

		audiofile = path.append(audiofile);

		ARCS_LOG_INFO << "Use path of metadata file: " << audiofile;
	}

	// Single Audio File Guaranteed
	return calculate(*toc, audiofile);
}


std::unique_ptr<ARId> ARIdCalculator::calculate(const std::string& metafilename,
		const std::string& audiofilename) const
{
	if (audiofilename.empty())
	{
		return calculate(metafilename);
	}

	const auto toc { create(metafilename)->parse(metafilename) };

	return calculate(*toc, audiofilename);
}


std::unique_ptr<ARId> ARIdCalculator::calculate(const ToC& toc,
		const std::string& audiofilename) const
{
	if (toc.complete())
	{
		return make_arid(toc);
	}

	return make_arid(toc, *audio()->size(audiofilename));
}


const AudioInfo* ARIdCalculator::audio() const
{
	return &audio_;
}


void ARIdCalculator::set_audio(const AudioInfo& audio)
{
	audio_ = audio;
}

} // namespace v_1_0_0
} // namespace arcsdec

