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

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>// for ARId, TOC, make_arid
#endif
#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp> // for Checksums, SampleInputIterator, ...
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

using arcstk::TOC;
using arcstk::ARId;
using arcstk::AudioSize;
using arcstk::Calculation;
using arcstk::Checksums;
using arcstk::ChecksumSet;
using arcstk::SampleInputIterator;
using arcstk::make_arid;
using arcstk::make_context;


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
 * \brief Provide a SampleProcessor that updates a Calculation.
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
		, total_samples_   { 0 }
	{
		/* empty */
	}

	/**
	 * \brief Default destructor.
	 */
	~CalculationProcessor() noexcept final = default;

	CalculationProcessor(const CalculationProcessor &rhs) noexcept = delete;
	CalculationProcessor& operator = (const CalculationProcessor &rhs) noexcept
		= delete;

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
	Calculation *calculation_;

	/**
	 * \brief Sequence counter.
	 *
	 * Counts the calls of SampleProcessor::append_samples.
	 */
	int64_t total_sequences_;

	/**
	 * \brief PCM 32 Bit Sample counter.
	 *
	 * Counts the total number of processed PCM 32 bit samples.
	 */
	int64_t total_samples_;
};


void CalculationProcessor::do_start_input()
{
	ARCS_LOG(DEBUG2) << "CalculationProcessor received: START INPUT";
}


void CalculationProcessor::do_append_samples(SampleInputIterator begin,
		SampleInputIterator end)
{
	ARCS_LOG(DEBUG2) << "CalculationProcessor received: APPEND SAMPLES";

	++total_sequences_;
	total_samples_ += std::distance(begin, end);

	calculation_->update(begin, end);
}


void CalculationProcessor::do_update_audiosize(const AudioSize &size)
{
	ARCS_LOG(DEBUG2) << "CalculationProcessor received: UPDATE AUDIOSIZE";

	calculation_->update_audiosize(size);
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
	return total_samples_;
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


// TOCParser


std::unique_ptr<TOC> TOCParser::parse(const std::string &metafilename) const
{
	if (metafilename.empty())
	{
		ARCS_LOG_ERROR <<
			"TOC info was requested but metadata filename was empty";

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
 * \brief Worker: process an audio file and calculate the results.
 *
 * The \c buffer_size is specified as number of 32 bit PCM samples. It is
 * applied to the created \link AudioReader AudioReaders.
 *
 * \param[in] audiofilename  Name of the audiofile
 * \param[in] reader         Audio reader
 * \param[in] calc           The Calculation to use
 * \param[in] buffer_size    Buffer size in number of samples
 */
void process_audio_file(const std::string& audiofilename,
		std::unique_ptr<AudioReader> reader, Calculation& calc,
		const std::size_t buffer_size);

void process_audio_file(const std::string& audiofilename,
		std::unique_ptr<AudioReader> reader, Calculation& calc,
		const std::size_t buffer_size)
{
	// Configure AudioReader and process file

	if (BLOCKSIZE::MIN <= buffer_size and buffer_size <= BLOCKSIZE::MAX)
	{
		ARCS_LOG(DEBUG1) << "Sample read chunk size: "
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

	CalculationProcessor calculator { calc };
	reader->set_processor(calculator);

	reader->process_file(audiofilename);
}


// ARCSCalculator


ARCSCalculator::ARCSCalculator(const arcstk::checksum::type type)
	: type_ { type }
{
	/* empty */
}


ARCSCalculator::ARCSCalculator()
	: ARCSCalculator(arcstk::checksum::type::ARCS2)
{
	/* empty */
}


std::pair<Checksums, ARId> ARCSCalculator::calculate(
		const std::string &audiofilename,
		const TOC &toc)
{
	ARCS_LOG_DEBUG << "Calculate by TOC and single audiofilename: "
		<< audiofilename;

	// Configure Calculation

	auto calc { std::make_unique<Calculation>(type(),
			make_context(toc, audiofilename)) };

	if (!calc)
	{
		throw std::logic_error("Could not instantiate Calculation object");
	}

	auto reader { create(audiofilename) };

	process_audio_file(audiofilename, std::move(reader), *calc,
			BLOCKSIZE::DEFAULT);

	log_completeness_check(*calc);

	return std::make_pair(calc->result(), calc->context().id());
}


Checksums ARCSCalculator::calculate(
	const std::vector<std::string> &audiofilenames,
	const bool &first_track_with_skip,
	const bool &last_track_with_skip)
{
	ARCS_LOG_DEBUG << "Calculate by audiofilenames, front_skip, back_skip";

	if (audiofilenames.empty())
	{
		return Checksums(0);
	}

	Checksums checksums { audiofilenames.size() };

	bool single_file { audiofilenames.size() == 1 };

	// Calculate first track

	ChecksumSet track {

		// Apply back skipping request on first file only if it's also the last

		this->calculate_track(audiofilenames[0], first_track_with_skip,
			(single_file ? last_track_with_skip : false))
	};

	checksums.append(track);

	// Avoid calculating a single track track twice

	if (single_file)
	{
		return checksums;
	}

	// Calculate second to second last track

	for (uint16_t i = 1; i < audiofilenames.size() - 1; ++i)
	{
		track = this->calculate_track(audiofilenames[i], false, false);

		checksums.append(track);
	}

	// Calculate last track

	track = this->calculate_track(audiofilenames.back(), false,
			last_track_with_skip);

	checksums.append(track);

	return checksums;
}


ChecksumSet ARCSCalculator::calculate(
	const std::string &audiofilename,
	const bool &skip_front,
	const bool &skip_back)
{
	return this->calculate_track(audiofilename, skip_front, skip_back);
}


void ARCSCalculator::set_type(const arcstk::checksum::type type)
{
	type_ = type;
}


arcstk::checksum::type ARCSCalculator::type() const
{
	return type_;
}


ChecksumSet ARCSCalculator::calculate_track(
	const std::string &audiofilename,
	const bool &skip_front, const bool &skip_back)
{
	ARCS_LOG_DEBUG << "Calculate track from file: " << audiofilename;

	// Configure Calculation

	auto calc { std::make_unique<Calculation>(type(),
		make_context(skip_front, skip_back, audiofilename)) };

	if (!calc)
	{
		throw std::logic_error("Could not instantiate Calculation object");
	}

	auto reader { create(audiofilename) };

	process_audio_file(audiofilename, std::move(reader), *calc,
			BLOCKSIZE::DEFAULT);

	log_completeness_check(*calc);

	// Sanity-check result

	const auto track_checksums { calc->result() };

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
		return make_arid(toc);
	}

	ARCS_LOG_INFO <<
		"Incomplete TOC and no audio file provided."
		" Try to find audio file references in TOC.";

	// Check whether TOC references exactly one audio file.
	// (Other cases are currently unsupported.)

	const auto audiofilenames { arcstk::toc::get_filenames(toc) };

	if (audiofilenames.empty())
	{
		throw std::runtime_error("Incomplete TOC, no audio file provided "
				"and TOC does not seem to reference any audio file.");
	}

	const std::unordered_set<std::string> name_set(
			audiofilenames.begin(), audiofilenames.end());

	if (name_set.size() != 1)
	{
		throw std::runtime_error("Incomplete TOC, no audio file provided "
				"and TOC does not reference exactly one audio file.");
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
		return make_arid(toc);
	}

	// If TOC is incomplete, analyze audio file passed

	return this->calculate(*toc, audiofilename);
}


std::unique_ptr<ARId> ARIdCalculator::calculate(const TOC &toc,
		const std::string &audiofilename) const
{
	// A complete multitrack configuration of the Calculation requires
	// two informations:
	// 1.) the LBA offset of each track
	//	=> which are known at this point by inspecting the TOC
	// 2.) at least one of the following four:
	//	a) the LBA track offset of the leadout frame
	//	b) the total number of 16bit samples in <audiofilename>
	//	c) the total number of bytes in <audiofilename> representing samples
	//	d) the length of the last track
	//	=> which may or may not be represented in the TOC

	// A TOC is the result of parsing a TOC providing file. Not all TOC
	// providing file formats contain sufficient information to calculate
	// the leadout (simple CueSheets for example do not). Therefore, TOCs
	// are accepted to be incomplete up to this point.

	// However, this means we additionally have to inspect the audio file to get
	// the missing information.

	// The total PCM byte count is exclusively known to the AudioReader in
	// the process of reading the audio file. (We cannot deduce it from the
	// mere file size.) We get the information by acquiring a CalcContext
	// from the audio file, although we do not intend to actually read the audio
	// samples.

	// The builder has to check its input values either way when it is
	// requested to start processing.

	return make_arid(toc, audio_.size(audiofilename)->leadout_frame());
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

