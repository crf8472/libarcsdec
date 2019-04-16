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

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>
#endif
#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __LIBARCSDEC_FILEFORMATS_HPP__
#include "descriptors.hpp"
#endif
#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"
#endif
#ifndef __LIBARCSDEC_AUDIOBUFFER_HPP__
#include "audiobuffer.hpp"
#endif
#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include "metaparser.hpp"
#endif


namespace arcs
{

inline namespace v_1_0_0
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

	MetadataParserSelection selection;
	std::unique_ptr<MetadataParser> parser = selection.for_file(metafilename);

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

		AudioReaderSelection selection;
		std::unique_ptr<AudioReader> reader = selection.for_file(audiofilename);
		auto audiosize = reader->acquire_size(audiofilename);

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
	 * Set the AudioReaderSelection for this instance.
	 *
	 * \param[in] selection The AudioReaderSelection to use
	 */
	void set_selection(std::unique_ptr<AudioReaderSelection> selection);

	/**
	 * Get the AudioReaderSelection used by this instance.
	 *
	 * \return The AudioReaderSelection used by this instance
	 */
	const AudioReaderSelection& selection() const;


protected:

	/**
	 * Worker: process a file and calculate the results.
	 *
	 * The \c buffer_size is specified as number of 32 bit PCM samples. It is
	 * applied to the created AudioReader's read buffer iff it has a
	 * configurable_read_buffer(). In this case, parameter \c use_cbuffer will
	 * stay irrelevant.
	 *
	 * If the AudioReader has no configurable_read_buffer(), the behaviour
	 * depends on the parameter \c use_cbuffer. If it is FALSE, this renders
	 * \c buffer_size without effect. If it is TRUE, a buffering in the
	 * specified \c buffer-size will be enforced. Thus, parameter \c use_cbuffer
	 * controls whether buffering is enforced.
	 *
	 * \param[in] audiofilename  Name  of the audiofile
	 * \param[in] calc           The Calculation to use
	 * \param[in] buffer_size    Buffer size in number of samples
	 * \param[in] use_cbuffer    Enforce a converting buffer
	 */
	void process_file(const std::string &audiofilename, Calculation& calc,
		const uint32_t buffer_size, const bool use_cbuffer) const;


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
	 * Internal AudioReaderSelection
	 */
	std::unique_ptr<AudioReaderSelection> selection_;
};


/// @}
/// \endcond IMPL_ONLY


ARCSCalculator::Impl::Impl()
	: selection_(std::make_unique<AudioReaderSelection>())
{
	// empty
}


std::pair<Checksums, ARId> ARCSCalculator::Impl::calculate(
		const std::string &audiofilename,
		const TOC &toc)
{
	ARCS_LOG_DEBUG << "Calculate by TOC and single audiofilename: "
		<< audiofilename;

	auto calc = std::make_unique<Calculation>(make_context(audiofilename, toc));

	this->process_file(audiofilename, *calc, BLOCKSIZE.DEFAULT, false);


	// Sanity-check result

	if (not calc->complete())
	{
		ARCS_LOG_ERROR << "Calculation is not complete after last sample";
	}

	return std::make_pair(calc->result(), calc->context().id());
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


void ARCSCalculator::Impl::process_file(const std::string &audiofilename,
		Calculation& calc, const uint32_t buffer_size, const bool use_cbuffer)
		const
{
	std::unique_ptr<AudioReader> reader = selection().for_file(audiofilename);

	if (!reader)
	{
		ARCS_LOG_ERROR << "No AudioReader available. Bail out.";
		throw std::logic_error("No AudioReader available. Bail out.");
	}

	const bool buffer_size_is_legal =
			(BLOCKSIZE.MIN <= buffer_size and buffer_size <= BLOCKSIZE.MAX);


	// Configure AudioReader and process file

	SampleProcessorAdapter proc { calc };

	if (use_cbuffer and !reader->configurable_read_buffer()
			and buffer_size_is_legal)
	{
		// Conversion/Buffering was explicitly requested

		SampleBuffer buffer(buffer_size);
		buffer.register_processor(proc);

		reader->set_processor(buffer);
	} else
	{
		// Conversion/Buffering was not requested

		if (reader->configurable_read_buffer())
		{
			if (buffer_size_is_legal)
			{
				reader->set_samples_per_read(buffer_size);

			} else
			{
				ARCS_LOG_WARNING << "Specified buffer size of " << buffer_size
					<< ", but this is not within the legal range of "
					<< BLOCKSIZE.MIN << " - " << BLOCKSIZE.MAX
					<< ". Use implementations default instead.";

				// Do nothing, AudioReaderImpl uses its default
			}
		} else
		{
			ARCS_LOG_INFO << "AudioReader has no configuration option "
				<< "for read buffer size. Use implementation's default since "
				<< "no CBuffer was requested.";
		}

		reader->set_processor(proc);
	}

	reader->process_file(audiofilename);
}


ChecksumSet ARCSCalculator::Impl::calculate_track(
	const std::string &audiofilename,
	const bool &skip_front, const bool &skip_back)
{
	ARCS_LOG_DEBUG << "Calculate track from file: " << audiofilename;

	// Configure Calculation

	auto calc = std::make_unique<Calculation>(
		make_context(audiofilename, skip_front, skip_back));

	this->process_file(audiofilename, *calc, BLOCKSIZE.DEFAULT, false);


	// Sanity-check result

	if (not calc->complete())
	{
		ARCS_LOG_ERROR << "Calculation is not complete after last sample";
	}

	auto track_checksums { calc->result() };

	if (track_checksums.size() == 0)
	{
		ARCS_LOG_ERROR << "Calculation lead to no result, return null";

		return ChecksumSet { 0 };
	}

	return track_checksums[0];
}


void ARCSCalculator::Impl::set_selection(
		std::unique_ptr<AudioReaderSelection> selection)
{
	selection_ = std::move(selection);
}


const AudioReaderSelection& ARCSCalculator::Impl::selection() const
{
	return *selection_;
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


void ARCSCalculator::set_selection(
		std::unique_ptr<AudioReaderSelection> selection)
{
	this->impl_->set_selection(std::move(selection));
}


const AudioReaderSelection& ARCSCalculator::selection() const
{
	return impl_->selection();
}

} // namespace v_1_0_0

} // namespace arcs

