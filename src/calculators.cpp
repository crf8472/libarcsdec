/**
 * \file calculators.cpp Implementing high-level API for calculating ARCSs of files.
 */

#ifndef __LIBARCSDEC_CALCULATORS_HPP__
#include "calculators.hpp"
#endif

#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#ifndef __LIBARCS_IDENTIFIER_HPP__
#include <arcs/identifier.hpp>
#endif
#ifndef __LIBARCS_CALCULATE_HPP__
#include <arcs/calculate.hpp>
#endif
#ifndef __LIBARCS_LOGGING_HPP__
#include <arcs/logging.hpp>
#endif

#ifndef __LIBARCSDEC_FILEFORMATS_HPP__
#include "fileformats.hpp"
#endif
#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"
#endif
#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include "metaparser.hpp"
#endif


namespace arcs
{


// TOCParser


TOCParser::~TOCParser() noexcept = default;


TOC TOCParser::parse(const std::string &metafilename)
{
	if (metafilename.empty())
	{
		ARCS_LOG_ERROR <<
			"TOC info was requested but metadata filename was empty";

		throw FileReadException(
				"Requested metadata file parser for empty filename.");
	}

	MetadataParserCreator creator;
	std::unique_ptr<MetadataParser> parser =
		creator.create_metadata_parser(metafilename);

	ARCS_LOG_INFO << "Start to parse metadata input";

	std::unique_ptr<TOC> toc { parser->parse(metafilename) };

	if (!toc)
	{
		ARCS_LOG_ERROR << "Parser did not provide a TOC";

		// TODO Throw Exception? If parse() did not throw, toc won't be null
	}

	return *toc;
}



/**
 * \cond IMPL_ONLY
 *
 * \internal \defgroup calculatorsImpl Implementation details of the Calc API
 *
 * \ingroup calculators
 *
 * \brief Implementation details of the \ref calc
 *
 * Contains the private implementations of ARCSCalculator and ARIdCalculator.
 * @{
 */


/**
 * Private implementation of an ARIdCalculator
 */
class ARIdCalculator::Impl final
{

public:

	/**
	 * Calculate ARId using the specified metadata file
	 *
	 * \param[in] metafilename Name of the metadata file
	 *
	 * \return The AccurateRip id for this medium
	 */
	std::unique_ptr<ARId> calculate(const std::string &metafilename);

	/**
	 * Calculate ARId using the specified metadata file and the specified
	 * audio file
	 *
	 * \param[in] audiofilename Name of the audiofile
	 * \param[in] metafilename  Name of the metadata file
	 *
	 * \return The AccurateRip id for this medium
	 */
	std::unique_ptr<ARId> calculate(const std::string &audiofilename,
			const std::string &metafilename);
};


/// @}
/// \endcond IMPL_ONLY


std::unique_ptr<ARId> ARIdCalculator::Impl::calculate(
		const std::string &metafilename)
{
	std::unique_ptr<TOC> toc;
	{
		TOCParser parser;
		toc = std::make_unique<TOC>(parser.parse(metafilename));
	}

	if (not toc->complete())
	{
		ARCS_LOG_ERROR <<
			"Incomplete TOC and no audio file provided. Bail out.";

		return make_empty_arid();
	}

	return make_arid(*toc);
}


std::unique_ptr<ARId> ARIdCalculator::Impl::calculate(
		const std::string &audiofilename, const std::string &metafilename)
{
	std::unique_ptr<TOC> toc;
	{
		TOCParser parser;
		toc = std::make_unique<TOC>(parser.parse(metafilename));
	}

	// If TOC is incomplete, analyze audio data to derive leadout

	uint32_t leadout = toc->leadout();

	if (not toc->complete())
	{
		// A complete multitrack configuration of the \ref Calculation requires
		// two informations:
		// 1.) the LBA offset of each track_count
		//	=> which are known at this point by inspecting the TOC
		// 2.) at least one of the following three:
		//	a) the LBA track index of the leadout frame
		//	b) the total number of 16bit samples in <audiofilename>
		//	c) the total number bytes in <audiofilename> representing samples
		//	=> which may or may not be represented in the TOC

		// A TOC is the result of parsing a TOC providing file. Not all TOC
		// providing file formats contain sufficient information to calculate
		// the leadout (certain CUESheets for example do not). Therefore, TOCs
		// are accepted to be incomplete up to this point.

		// However, this means we have to inspect the audio file to get the
		// missing information and pass it to make_arid().

		// The total PCM byte count is exclusively known to the AudioReader in
		// the process of reading the audio file. (We cannot deduce it from the
		// mere file size.) We get the information by acquiring a CalcContext
		// from the audio file, although we do now intend to read the audio
		// samples.

		// The builder has to check its input values either way when it is
		// requested to start processing.

		AudioReaderCreator creator;

		std::unique_ptr<AudioReader> reader =
			creator.create_audio_reader(audiofilename);

		std::unique_ptr<AudioSize> audiosize =
			reader->acquire_size(audiofilename);

		leadout = audiosize->leadout_frame();
	}

	return make_arid(*toc, leadout);
}


/// \cond IMPL_ONLY
/// \internal \addtogroup calculatorsImpl
/// @{


/**
 * Private implementation of an ARCSCalculator.
 */
class ARCSCalculator::Impl final
{

public:

	/**
	 * Empty constructor
	 */
	Impl();

	/**
	 * Calculate ARCS values for the given audio file, using the metadata from
	 * the given TOC.
	 *
	 * The TOC is supposed to contain the offsets of all tracks represented
	 * in the audio file. The result will contain ARCS v1 and v2 for all tracks
	 * specified in the TOC.
	 *
	 * \param[in] audiofilename Name of the audiofile
	 * \param[in] toc           Metadata for the audiofile
	 *
	 * \return AccurateRip checksums of all tracks specified in the TOC
	 */
	std::pair<Checksums, ARId> calculate(
		const std::string &audiofilename, const TOC &toc);

	/**
	 * Calculate <tt>ChecksumSet</tt>s for the given audio files.
	 *
	 * The ARCSs in the result will have the same order as the input files,
	 * so for any index i: 0 <= i < audiofilenames.size(), result[i] will be the
	 * result for audiofilenames[i]. The result will have the same size as
	 * audiofilenames.
	 *
	 * \param[in] audiofilenames       Names of the audiofiles
	 * \param[in] first_file_with_skip Process first file as first track
	 * \param[in] last_file_with_skip  Process last file as last track
	 *
	 * \return AccurateRip checksums of the input files
	 */
	Checksums calculate(const std::vector<std::string> &audiofilenames,
		const bool &first_track_with_skip,
		const bool &last_track_with_skip);

	/**
	 * Calculate a \ref arcs::ChecksumSet for the given audio file
	 *
	 * The flags skip_front and skip_back control whether the track is processed
	 * as first or last track of an album. If skip_front is set to TRUE, the
	 * first 2939 samples are skipped in the calculation. If skip_back
	 * is set to TRUE, the last 5 frames of are skipped in the calculation.
	 *
	 * \param[in] audiofilename  Name  of the audiofile
	 * \param[in] skip_front     Skip front samples of first track
	 * \param[in] skip_back      Skip back samples of last track
	 *
	 * \return The AccurateRip checksum of this track
	 */
	ChecksumSet calculate(const std::string &audiofilename,
		const bool &skip_front, const bool &skip_back);

	/**
	 * Set the AudioReaderCreator for this instance.
	 *
	 * \param[in] creator The AudioReaderCreator to use
	 */
	void set_audioreader_creator(std::unique_ptr<AudioReaderCreator> creator);

	/**
	 * Get the AudioReaderCreator used by this instance.
	 *
	 * \return The AudioReaderCreator used by this instance
	 */
	const AudioReaderCreator& audioreader_creator() const;


private:

	/**
	 * Worker method: calculating the ARCS of a single audiofile.
	 *
	 * \param[in] audiofilename Name of the audiofile
	 *
	 * \return The AccurateRip checksum of this track
	 */
	ChecksumSet calculate_track(const std::string &audiofilename,
		const bool &skip_front, const bool &skip_back);

	/**
	 * Internal AudioReaderCreator
	 */
	std::unique_ptr<AudioReaderCreator> creator_;
};


/// @}
/// \endcond IMPL_ONLY


ARCSCalculator::Impl::Impl()
	: creator_(std::make_unique<AudioReaderCreator>())
{
	// empty
}


std::pair<Checksums, ARId> ARCSCalculator::Impl::calculate(
		const std::string &audiofilename,
		const TOC &toc)
{
	ARCS_LOG_DEBUG << "Calculate by TOC and single audiofilename: "
		<< audiofilename;

	// Configure Calculation

	auto calc = std::make_unique<Calculation>(make_context(audiofilename, toc));


	// Create AudioReader

	std::unique_ptr<AudioReader> reader;

	{
		reader = audioreader_creator().create_audio_reader(audiofilename);

		if (!reader)
		{
			ARCS_LOG_ERROR << "No AudioReader available. Bail out.";

			return std::make_pair(Checksums(0), ARId(0,0,0,0));
		}
	}


	// Configure AudioReader and process file

	SampleProcessorAdapter proc { *calc };
	reader->register_processor(proc);
	reader->process_file(audiofilename);

	if (not calc->complete())
	{
		ARCS_LOG_ERROR << "Calculation is not complete after last sample";
	}

	auto checksums { calc->result() };

	if (checksums.size() == 0)
	{
		ARCS_LOG_ERROR << "No checksums";
	}

	auto id { calc->context().id() };

	if (id.empty())
	{
		ARCS_LOG_ERROR << "Empty ARId";
	}

	return std::make_pair(checksums, id);
}


Checksums ARCSCalculator::Impl::calculate(
	const std::vector<std::string> &audiofilenames,
	const bool &first_track_with_skip,
	const bool &last_track_with_skip)
{
	ARCS_LOG_DEBUG << "Calculate by audiofilenames, front_skip, back_skip";

	if (audiofilenames.empty())
	{
		return Checksums(0);
	}

	Checksums arcss { audiofilenames.size() };

	bool single_file { audiofilenames.size() == 1 };


	// Calculate first track

	ChecksumSet arcs {

		// Apply back skipping request on first file only if it's also the last

		this->calculate_track(audiofilenames[0], first_track_with_skip,
			(single_file ? last_track_with_skip : false))
	};

	arcss[0] = arcs;

	// Avoid calculating a single track track twice

	if (single_file)
	{
		return arcss;
	}

	// Calculate second to second last track

	for (uint16_t i = 1; i < audiofilenames.size() - 1; ++i)
	{
		arcs = this->calculate_track(audiofilenames[i], false, false);

		arcss[i] = arcs;
	}

	// Calculate last track

	arcs = this->calculate_track(audiofilenames.back(), false,
			last_track_with_skip);

	arcss[arcss.size() - 1] = arcs;

	return arcss;
}


ChecksumSet ARCSCalculator::Impl::calculate(
	const std::string &audiofilename,
	const bool &skip_front,
	const bool &skip_back)
{
	return this->calculate_track(audiofilename, skip_front, skip_back);
}


ChecksumSet ARCSCalculator::Impl::calculate_track(
	const std::string &audiofilename,
	const bool &skip_front, const bool &skip_back)
{
	ARCS_LOG_DEBUG << "Calculate track from file: " << audiofilename;

	// Configure Calculation

	auto calc = std::make_unique<Calculation>(
		make_context(audiofilename, skip_front, skip_back));


	// Create AudioReader

	std::unique_ptr<AudioReader> reader =
		audioreader_creator().create_audio_reader(audiofilename);

	if (!reader)
	{
		ARCS_LOG_ERROR << "No AudioReader available. Bail out.";

		return ChecksumSet { 0 };
	}


	// Configure AudioReader, process file, sanity-check result

	SampleProcessorAdapter proc { *calc };
	reader->register_processor(proc);
	reader->process_file(audiofilename);

	if (not calc->complete())
	{
		ARCS_LOG_ERROR << "Calculation is not complete after last sample";
	}

	auto track_checksums { calc->result() };

	if (track_checksums.size() == 0)
	{
		ARCS_LOG_ERROR << "Unexpected empty result for single track";

		return ChecksumSet { 0 };
	}


	// Convert checksums

	auto checksums { track_checksums[0] };

	if (checksums.empty())
	{
		ARCS_LOG_ERROR << "Calculation lead to no result, return null";

		return ChecksumSet { 0 };
	}

	return checksums;
}


void ARCSCalculator::Impl::set_audioreader_creator(
		std::unique_ptr<AudioReaderCreator> creator)
{
	creator_ = std::move(creator);
}


const AudioReaderCreator& ARCSCalculator::Impl::audioreader_creator() const
{
	return *creator_;
}


// ARIdCalculator


ARIdCalculator::ARIdCalculator()
	: impl_(std::make_unique<ARIdCalculator::Impl>())
{
	// empty
}


ARIdCalculator::~ARIdCalculator() noexcept = default;


std::unique_ptr<ARId> ARIdCalculator::calculate(const std::string &metafilename)
{
	return impl_->calculate(metafilename);
}


std::unique_ptr<ARId> ARIdCalculator::calculate(
		const std::string &audiofilename,
		const std::string &metafilename)
{
	return impl_->calculate(audiofilename, metafilename);
}


// ARCSCalculator


ARCSCalculator::ARCSCalculator()
	: impl_(std::make_unique<ARCSCalculator::Impl>())
{
	// empty
}


ARCSCalculator::~ARCSCalculator() noexcept = default;


std::pair<Checksums, ARId> ARCSCalculator::calculate(
		const std::string &audiofilename, const TOC &toc)
{
	return impl_->calculate(audiofilename, toc);
}


ChecksumSet ARCSCalculator::calculate(
	const std::string &audiofilename,
	const bool &skip_front,
	const bool &skip_back)
{
	return impl_->calculate(audiofilename, skip_front, skip_back);
}


Checksums ARCSCalculator::calculate(
		const std::vector<std::string> &audiofilenames,
		const bool &first_track_with_skip,
		const bool &last_track_with_skip)
{
	return impl_->calculate(audiofilenames,
			first_track_with_skip, last_track_with_skip);
}


void ARCSCalculator::set_audioreader_creator(
		std::unique_ptr<AudioReaderCreator> creator)
{
	this->impl_->set_audioreader_creator(std::move(creator));
}


const AudioReaderCreator& ARCSCalculator::audioreader_creator() const
{
	return impl_->audioreader_creator();
}

} // namespace arcs

