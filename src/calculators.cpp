/**
 * \file
 *
 * \brief Implementing high-level API for calculating ARCSs of files.
 */

#ifndef __LIBARCSDEC_CALCULATORS_HPP__
#include "calculators.hpp"
#endif

#include <cstddef>       // for size_t
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
using arcstk::Points;
using arcstk::SampleInputIterator;
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
	LogProcessor(const LogProcessor &rhs) noexcept = delete;
	LogProcessor& operator = (const LogProcessor &rhs) noexcept = delete;

private:

	void do_start_input() final;

	void do_append_samples(SampleInputIterator, SampleInputIterator) final;

	void do_update_audiosize(const AudioSize &) final;

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


void LogProcessor::do_update_audiosize(const AudioSize &)
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
	CalculationProcessor(Calculation &calculation)
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

	void do_update_audiosize(const AudioSize &size) final;

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


void CalculationProcessor::do_update_audiosize(const AudioSize &size)
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

	void do_update_audiosize(const AudioSize &size) final;

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


void MultiCalculationProcessor::do_update_audiosize(const AudioSize &size)
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


std::unique_ptr<ToC> ToCParser::parse(const std::string &metafilename) const
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


/**
 * \brief Worker: check samples_todo() and warn if < 0 and error if > 0
 *
 * \param[in] calc Calculation to check
 */
void log_completeness_check(const Calculation &calc);

void log_completeness_check(const Calculation &calc)
{
	if (not calc.complete())
	{
		ARCS_LOG_ERROR << "Calculation not complete after last input sample: "
			<< "Expected total samples: " << calc.samples_expected()
			<< " "
			<< "Processed total samples: " << calc.samples_processed();
	}

	if (calc.samples_todo() < 0)
	{
		ARCS_LOG_WARNING << "More samples than expected. "
			<< "Expected: " << calc.samples_expected()
			<< " "
			<< "Processed: " << calc.samples_processed();
	}
}


/**
 * \brief Worker: process an audio file via specified SampleProcessor.
 *
 * The \c buffer_size is specified as number of 32 bit PCM samples. It is
 * applied to the created \link AudioReader AudioReaders.
 *
 * \param[in] audiofilename  Name of the audiofile
 * \param[in] reader         Audio reader
 * \param[in] processor      The SampleProcessor to use
 * \param[in] buffer_size    Buffer size in number of samples
 */
void process_audio_file(const std::string& audiofilename,
		std::unique_ptr<AudioReader> reader, SampleProcessor& processor,
		const int64_t buffer_size);

void process_audio_file(const std::string& audiofilename,
		std::unique_ptr<AudioReader> reader, SampleProcessor& processor,
		const int64_t buffer_size)
{
	// Configure AudioReader and process file

	if (BLOCKSIZE::MIN <= buffer_size and buffer_size <= BLOCKSIZE::MAX)
	{
		ARCS_LOG(DEBUG1) << "Chunk size for reading samples: "
			<< std::to_string(buffer_size) << " bytes";

		reader->set_samples_per_read(buffer_size);

	} else
	{
		// Buffer size is illegal.
		// Do nothing, AudioReaderImpl uses its default.

		ARCS_LOG_WARNING << "Specified buffer size of " << buffer_size
			<< " bytes is not within the legal range of "
			<< BLOCKSIZE::MIN << " - " << BLOCKSIZE::MAX
			<< " samples. Fall back to AudioReader's implementation default: "
			<< reader->samples_per_read()
			<< " bytes";
	}

	reader->set_processor(processor);
	reader->process_file(audiofilename);
}


/**
 * \brief A duplicate-free aggregate of Algorithm instances without particular
 * order.
 */
using Algorithms = std::unordered_set<std::unique_ptr<Algorithm>>;


/**
 * \brief A duplicate-free aggregate of checksum::type values without particular
 * order.
 */
using Types      = std::unordered_set<arcstk::checksum::type>;
// TODO duplicate of ChecksumtypeSet

/**
 * \brief Acquire the algorithms for calculating a set of types.
 *
 * \param[in] types Set of types
 *
 * \return Duplicate-free set of Algorithm instances
 */
Algorithms get_algorithms(const Types& types);

Algorithms get_algorithms(const Types& types)
{
	using arcstk::checksum::type;

	Algorithms a;

	// TODO Manual logic does not scale. OK only for the current 3 algorithms.

	if (types.empty()/* default */ || types.size() > 1/* all known types*/)
	{
		a.insert(std::make_unique<arcstk::accuraterip::V1and2>());
	} else
	{
		if (type::ARCS1 == *types.begin())
		{
			a.insert(std::make_unique<arcstk::accuraterip::V1>());
		} else
		{
			a.insert(std::make_unique<arcstk::accuraterip::V2>());
		}
	}

	return a;
}


/**
 * \brief Wrapper for get_algorithms that throws on an empty set of algorithms.
 *
 * \param[in] types Set of types
 *
 * \return Duplicate-free set of Algorithm instances
 *
 * \throws If the resulting set of Algorithm instances would be empty
 */
Algorithms get_algorithms_or_throw(const Types& types);

Algorithms get_algorithms_or_throw(const Types& types)
{
	auto algorithms = get_algorithms(types);

	if (algorithms.empty())
	{
		throw std::runtime_error(
				"Could not find algorithms for requested types");
		// TODO Print types
	}

	return algorithms;
}


/**
 * \brief Bulk-Initialize calculations for.settings, algorithms and data.
 *
 * \param[in] settings   Settings for each Calculation
 * \param[in] algorithms Algorithms to initialize Calculations for
 * \param[in] size       Sample amount to process
 * \param[in] points     Offset points (counted as samples)
 *
 * \return Initialized Calculation instances
 */
std::vector<Calculation> init_calculations(const arcstk::Settings& settings,
		const Algorithms& algorithms, const AudioSize& size,
		const Points& points);

std::vector<Calculation> init_calculations(const arcstk::Settings& settings,
		const Algorithms& algorithms, const AudioSize& size,
		const Points& points)
{
	auto calculations = std::vector<Calculation>();
	calculations.reserve(algorithms.size());

	for (const auto& algorithm : algorithms)
	{
		// We cannot move an object out of a set, so we have to clone
		calculations.emplace_back(settings, algorithm->clone(), size, points);
	}

	return calculations;
}


/**
 * \brief Convenience wrapper for init_calculations() to use with ToC info.
 *
 * \param[in] types   Set of requested types
 * \param[in] size    Size of the audio input
 * \param[in] points  Track offset samples
 *
 * \return Duplicate-free set of Algorithm instances
 *
 * \throws If the resulting set of Algorithm instances would be empty
 */
std::vector<Calculation> init_calculations(const Types& types,
		const AudioSize& size, const Points& points);

std::vector<Calculation> init_calculations(const Types& types,
		const AudioSize& size, const Points& points)
{
	return init_calculations({ arcstk::Context::ALBUM },
			get_algorithms_or_throw(types), size, points);
}


// TODO Remove that
std::vector<int32_t> offsets_as_samples(const ToC& toc);

std::vector<int32_t> offsets_as_samples(const ToC& toc)
{
	// Transform offsets to sample points
	//auto points { arcstk::toc::get_offsets(toc) };
	auto offsets { toc.offsets() };
	auto points = std::vector<int32_t>{};
	using std::cbegin;
	using std::cend;
	using std::begin;
	std::transform(cbegin(offsets), cend(offsets), begin(points),
			[](const AudioSize& a)
			{
				return a.frames();
			}
	);
	// TODO duplicate of arcstk::details::get_offset_sample_indices
	// TODO use arcstk::details::frames2samples

	return points;
}


/**
 * \brief Combine all results of the specified Calculation instances in a
 * single, duplicate-free object.
 * .
 * \param[in] calculations Calculations to aggregate the results from
 *
 * \return Aggregated results from all input Calculation instances
 */
Checksums harvest_result(const std::vector<Calculation>& calculations);

Checksums harvest_result(const std::vector<Calculation>& calculations)
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


// ARCSCalculator


ARCSCalculator::ARCSCalculator(const ChecksumTypeset& typeset)
	: types_ { typeset }
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
		const std::string &audiofilename,
		const ToC &toc)
{
	using arcstk::make_arid;

	ARCS_LOG_DEBUG << "Calculate by ToC and single audiofilename: "
		<< audiofilename;

	// Acquire reader

	auto reader { create(audiofilename) };

	// Configure Calculation

	auto size = AudioSize{};
	auto id   = std::unique_ptr<ARId>{};

	// TODO Really??
	if (!toc.complete())
	{
		size = *reader->acquire_size(audiofilename);
		id   = make_arid(toc, size);
	} else
	{
		size = toc.leadout();
		id   = make_arid(toc);
	}

	auto calculations { init_calculations(types(), size, toc.offsets()) };

	if (calculations.empty())
	{
		throw std::logic_error("Could not instantiate Calculation objects");
	}

	MultiCalculationProcessor proc{};
	for (auto& c : calculations)
	{
		proc.add(c);
	}

	// Run

	process_audio_file(audiofilename, std::move(reader), proc,
			BLOCKSIZE::DEFAULT);

	for (auto& c : calculations)
	{
		log_completeness_check(c);
	}

	// Result handling

	return std::make_pair(harvest_result(calculations), *id);
}


Checksums ARCSCalculator::calculate(
	const std::vector<std::string> &audiofilenames,
	const bool &first_track_with_skip,
	const bool &last_track_with_skip)
{
	ARCS_LOG_DEBUG << "Calculate by audiofilenames + front_skip + back_skip";

	if (audiofilenames.empty())
	{
		return Checksums(0);
	}

	Checksums checksums { audiofilenames.size() };

	const bool single_file { audiofilenames.size() == 1 };

	// Calculate first track

	ChecksumSet track {

		// Apply back skipping request on first file only if it's also the last

		this->calculate_track(audiofilenames.front(), first_track_with_skip,
			(single_file ? last_track_with_skip : false))
	};

	checksums.push_back(track);

	// Avoid calculating a single track track twice

	if (single_file)
	{
		return checksums;
	}

	// Calculate second to second last track

	for (uint16_t i = 1; i < audiofilenames.size() - 1; ++i)
	{
		track = this->calculate_track(audiofilenames[i], false, false);

		checksums.push_back(track);
	}

	// Calculate last track

	track = this->calculate_track(audiofilenames.back(), false,
			last_track_with_skip);

	checksums.push_back(track);

	return checksums;
}


ChecksumSet ARCSCalculator::calculate(
	const std::string &audiofilename,
	const bool &skip_front,
	const bool &skip_back)
{
	ARCS_LOG_DEBUG <<
		"Calculate by single audiofilename + front_skip + back_skip";

	return this->calculate_track(audiofilename, skip_front, skip_back);
}


void ARCSCalculator::set_types(const ChecksumTypeset& typeset)
{
	types_ = typeset;
}


ChecksumTypeset ARCSCalculator::types() const
{
	return types_;
}


ChecksumSet ARCSCalculator::calculate_track(
	const std::string &audiofilename,
	const bool &skip_front, const bool &skip_back)
{
	ARCS_LOG_DEBUG << "Calculate track from file: " << audiofilename;

	// Acquire reader

	auto reader { create(audiofilename) };

	// Configure Calculation

	// FIXME Repair this
	using arcstk::Settings;
	using arcstk::Context;
	auto settings = Settings { Context::NONE };
	const auto flags = skip_front + 2 * skip_back;
	settings = flags == 3
			? Context::ALBUM
			: flags == 2
				? Context::LAST_TRACK
				: flags == 1
					? Context::FIRST_TRACK
					: Context::NONE;

	auto algorithms { get_algorithms_or_throw(types()) };

	auto calculations { init_calculations(settings, algorithms,
			*reader->acquire_size(audiofilename), {}) };

	if (calculations.empty())
	{
		throw std::logic_error("Could not instantiate Calculation objects");
	}

	MultiCalculationProcessor proc{};
	for (auto& c : calculations)
	{
		proc.add(c);
	}

	// Run

	process_audio_file(audiofilename, std::move(reader), proc,
			BLOCKSIZE::DEFAULT);

	for (auto& c : calculations)
	{
		log_completeness_check(c);
	}

	// Sanity-check result

	const auto track_checksums = harvest_result(calculations);

	if (track_checksums.size() == 0)
	{
		ARCS_LOG_ERROR << "Calculation lead to no result, return null";

		return ChecksumSet { 0 };
	}

	return track_checksums[0];
}


// AudioInfo


std::unique_ptr<AudioSize> AudioInfo::size(const std::string &filename) const
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
		const std::string &metafilename) const
{
	const auto toc { create(metafilename)->parse(metafilename) };

	if (toc->complete())
	{
		return make_arid(*toc);
	}

	ARCS_LOG_INFO <<
		"Incomplete ToC and no audio file provided."
		" Try to find audio file references in ToC.";

	// Check whether ToC references exactly one audio file.
	// (Other cases are currently unsupported.)

	const auto audiofilenames { toc->filenames() };

	if (audiofilenames.empty())
	{
		throw std::runtime_error("Incomplete ToC, no audio file provided "
				"and ToC does not seem to reference any audio file.");
	}

	const std::unordered_set<std::string> name_set(
			audiofilenames.begin(), audiofilenames.end());

	if (name_set.size() != 1)
	{
		throw std::runtime_error("Incomplete ToC, no audio file provided "
				"and ToC does not reference exactly one audio file.");
	}

	auto audiofile { *name_set.begin() };
	ARCS_LOG_INFO << "Found audiofile: " << audiofile << ", try loading";

	// Use path from metafile (if any) as search path for the audio file

	auto pos { metafilename.find_last_of("/\\") }; // XXX Really portable?

	if (pos != std::string::npos)
	{
		// If pos+1 would be illegal, Parser would already have Thrown
		audiofile = metafilename.substr(0, pos + 1).append(audiofile);
	}

	// Single Audio File Guaranteed
	return this->calculate(*toc, audiofile);
}


std::unique_ptr<ARId> ARIdCalculator::calculate(const std::string &metafilename,
		const std::string &audiofilename) const
{
	if (audiofilename.empty())
	{
		return this->calculate(metafilename);
	}

	const auto toc { create(metafilename)->parse(metafilename) };

	if (toc->complete())
	{
		return make_arid(*toc);
	}

	// If ToC is incomplete, analyze audio file passed

	return this->calculate(*toc, audiofilename);
}


std::unique_ptr<ARId> ARIdCalculator::calculate(const ToC &toc,
		const std::string &audiofilename) const
{
	// A complete multitrack configuration of the Calculation requires
	// two informations:
	// 1.) the LBA offset of each track
	//	=> which are known at this point by inspecting the ToC
	// 2.) at least one of the following four:
	//	a) the LBA track offset of the leadout frame
	//	b) the total number of 16bit samples in <audiofilename>
	//	c) the total number of bytes in <audiofilename> representing samples
	//	d) the length of the last track
	//	=> which may or may not be represented in the ToC

	// A ToC is the result of parsing a ToC providing file. Not all ToC
	// providing file formats contain sufficient information to calculate
	// the leadout (simple CueSheets for example do not). Therefore, ToCs
	// are accepted to be incomplete up to this point.

	// However, this means we additionally have to inspect the audio file to get
	// the missing information.

	// The total PCM byte count is exclusively known to the AudioReader in
	// the process of reading the audio file. (We cannot deduce it from the
	// mere file size.) However, we do not intend to actually read the audio
	// samples.

	if (toc.complete())
	{
		return make_arid(toc);
	}

	return make_arid(toc, *audio_.size(audiofilename));
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

