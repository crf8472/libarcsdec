/**
 * \file
 *
 * \brief Implementing high-level API for calculating ARCSs of files.
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

#ifndef __LIBARCSDEC_DESCRIPTORS_HPP__
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


namespace arcsdec
{

inline namespace v_1_0_0
{

using arcstk::TOC;
using arcstk::ARId;
using arcstk::Calculation;
using arcstk::Checksums;
using arcstk::ChecksumSet;
using arcstk::make_arid;
using arcstk::make_context;
using arcstk::make_empty_arid;



/**
 * \internal \defgroup calculatorsImpl Implementation
 *
 * \ingroup calculators
 *
 * \brief Implementation details of \ref calculators
 *
 * Contains the private implementations of ARCSCalculator and ARIdCalculator.
 * @{
 */


/**
 * \brief Private implementation of a TOCParser.
 */
class TOCParser::Impl final
{
public:

	Impl();

	std::unique_ptr<TOC> parse(const std::string &metafilename) const;

	void set_selection(const FileReaderSelection *selection);

	const FileReaderSelection& selection() const;

private:

	const FileReaderSelection *selection_;
};


/**
 * \brief Private implementation of an ARIdCalculator.
 */
class ARIdCalculator::Impl final
{
public:

	Impl();

	std::unique_ptr<ARId> calculate(const std::string &metafilename) const;

	std::unique_ptr<ARId> calculate(const std::string &audiofilename,
			const std::string &metafilename) const;

	void set_toc_selection(const FileReaderSelection *selection);

	const FileReaderSelection& toc_selection() const;

	void set_audio_selection(const FileReaderSelection *selection);

	const FileReaderSelection& audio_selection() const;

private:

	/**
	 * \brief Worker: calculate ID from TOC while taking leadout from audio
	 * file.
	 *
	 * \param[in] toc           TOC of the image
	 * \param[in] audiofilename Name of the image audiofile
	 *
	 * \return The AccurateRip id for this medium
	 */
	std::unique_ptr<ARId> calculate(const TOC &toc,
			const std::string &audiofilename) const;

	const FileReaderSelection* toc_selection_;

	const FileReaderSelection* audio_selection_;
};


/**
 * \brief Private implementation of an ARCSCalculator.
 */
class ARCSCalculator::Impl final
{
public:

	Impl(const arcstk::checksum::type type);

	Impl() : Impl(arcstk::checksum::type::ARCS2) { /* empty */ };

	std::pair<Checksums, ARId> calculate(
		const std::string &audiofilename, const TOC &toc);

	Checksums calculate(const std::vector<std::string> &audiofilenames,
		const bool &first_track_with_skip,
		const bool &last_track_with_skip);

	ChecksumSet calculate(const std::string &audiofilename,
		const bool &skip_front, const bool &skip_back);

	void set_selection(const FileReaderSelection *selection);

	const FileReaderSelection& selection() const;

	void set_type(const arcstk::checksum::type type);

	arcstk::checksum::type type() const;

private:

	/**
	 * \brief Worker: process a file and calculate the results.
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
		const int32_t buffer_size, const bool use_cbuffer) const;

	/**
	 * \brief Worker: check samples_todo() and warn if < 0 and error if > 0
	 *
	 * \param[in] calc Calculation to check
	 */
	void log_completeness_check(const Calculation &calc) const;

	/**
	 * \brief Worker method: calculating the ARCS of a single audiofile.
	 *
	 * \param[in] audiofilename Name of the audiofile
	 *
	 * \return The AccurateRip checksum of this track
	 */
	ChecksumSet calculate_track(const std::string &audiofilename,
		const bool &skip_front, const bool &skip_back);

	/**
	 * \brief Internal AudioReaderSelection.
	 */
	const FileReaderSelection *selection_;

	/**
	 * \brief Internal checksum type.
	 */
	arcstk::checksum::type type_;
};


/** @} */


// TOCParser::Impl


TOCParser::Impl::Impl()
	: selection_ { nullptr }
{
	// empty
}


std::unique_ptr<TOC> TOCParser::Impl::parse(const std::string &metafilename)
	const
{
	if (metafilename.empty())
	{
		ARCS_LOG_ERROR <<
			"TOC info was requested but metadata filename was empty";

		throw FileReadException(
				"Requested metadata file parser for empty filename.");
	}

	CreateMetadataParser get_parser;

	return get_parser(selection(), metafilename)->parse(metafilename);
}


void TOCParser::Impl::set_selection(const FileReaderSelection *selection)
{
	selection_ = selection;
}


const FileReaderSelection& TOCParser::Impl::selection() const
{
	return *selection_;
}


// ARIdCalculator::Impl


ARIdCalculator::Impl::Impl()
	: toc_selection_   { nullptr }
	, audio_selection_ { nullptr }
{
	// empty
}


std::unique_ptr<ARId> ARIdCalculator::Impl::calculate(
		const std::string &metafilename) const
{
	TOCParser parser;
	const auto toc { parser.parse(metafilename) };

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

	auto pos = metafilename.find_last_of("/\\"); // XXX Is this really portable?

	if (pos != std::string::npos)
	{
		// If pos+1 would be illegal, Parser would already have Thrown
		audiofile = metafilename.substr(0, pos + 1).append(audiofile);
	}

	// Single Audio File Guaranteed
	return this->calculate(*toc, audiofile);
}


std::unique_ptr<ARId> ARIdCalculator::Impl::calculate(
		const std::string &audiofilename, const std::string &metafilename) const
{
	if (audiofilename.empty())
	{
		return this->calculate(metafilename);
	}

	CreateMetadataParser get_parser;

	const auto toc = get_parser(toc_selection(), metafilename)->parse(
			metafilename);

	if (toc->complete())
	{
		return make_arid(toc);
	}

	// If TOC is incomplete, analyze audio file passed

	return this->calculate(*toc, audiofilename);
}


std::unique_ptr<ARId> ARIdCalculator::Impl::calculate(const TOC &toc,
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
	// the leadout (simple CUESheets for example do not). Therefore, TOCs
	// are accepted to be incomplete up to this point.

	// However, this means we additionally have to inspect the audio file to get
	// the missing information.

	// The total PCM byte count is exclusively known to the AudioReader in
	// the process of reading the audio file. (We cannot deduce it from the
	// mere file size.) We get the information by acquiring a CalcContext
	// from the audio file, although we do now intend to actually read the audio
	// samples.

	// The builder has to check its input values either way when it is
	// requested to start processing.

	CreateAudioReader get_audioreader;

	const auto audiosize =
		get_audioreader(audio_selection(), audiofilename)->acquire_size(
				audiofilename);

	return make_arid(toc, audiosize->leadout_frame());
}


void ARIdCalculator::Impl::set_toc_selection(
		const FileReaderSelection *selection)
{
	toc_selection_ = selection;
}


const FileReaderSelection& ARIdCalculator::Impl::toc_selection() const
{
	return *toc_selection_;
}


void ARIdCalculator::Impl::set_audio_selection(
		const FileReaderSelection *selection)
{
	audio_selection_ = selection;
}


const FileReaderSelection& ARIdCalculator::Impl::audio_selection() const
{
	return *audio_selection_;
}


// ARCSCalculator::Impl


ARCSCalculator::Impl::Impl(const arcstk::checksum::type type)
	: selection_ {}
	, type_ { type }
{
	// empty
}


std::pair<Checksums, ARId> ARCSCalculator::Impl::calculate(
		const std::string &audiofilename,
		const TOC &toc)
{
	ARCS_LOG_DEBUG << "Calculate by TOC and single audiofilename: "
		<< audiofilename;

	auto calc = std::make_unique<Calculation>(type(),
			make_context(toc, audiofilename));

	if (!calc)
	{
		// TODO Throw something
	}

	this->process_file(audiofilename, *calc, BLOCKSIZE.DEFAULT, false);

	this->log_completeness_check(*calc);

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


ChecksumSet ARCSCalculator::Impl::calculate(
	const std::string &audiofilename,
	const bool &skip_front,
	const bool &skip_back)
{
	return this->calculate_track(audiofilename, skip_front, skip_back);
}


void ARCSCalculator::Impl::process_file(const std::string &audiofilename,
		Calculation& calc, const int32_t buffer_size, const bool use_cbuffer)
		const
{
	CreateAudioReader audioreader;
	auto reader = audioreader(selection(), audiofilename);

	// Configure AudioReader and process file

	const bool buffer_size_is_legal =
			(BLOCKSIZE.MIN <= buffer_size and buffer_size <= BLOCKSIZE.MAX);

	SampleProcessorAdapter proc { calc };

	if (use_cbuffer and !reader->configurable_read_buffer()
			and buffer_size_is_legal)
	{
		ARCS_LOG_DEBUG << "Buffering for sample conversion requested";

		// Conversion/Buffering was explicitly requested

		SampleBuffer buffer(buffer_size);
		buffer.register_processor(proc);

		ARCS_LOG(DEBUG1) << "Configure sample buffer of size: "
			<< std::to_string(buffer_size) << " bytes";

		reader->set_processor(buffer);
	} else
	{
		// Conversion/Buffering was not requested

		if (reader->configurable_read_buffer())
		{
			ARCS_LOG_DEBUG << "AudioReader provides configurable read buffer";

			if (buffer_size_is_legal)
			{
				ARCS_LOG(DEBUG1) << "Configure sample read chunk size: "
					<< std::to_string(buffer_size) << " bytes";

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
			ARCS_LOG_INFO << "AudioReader has no configuration option for read "
				<< "buffer size and no conversion buffer was requested."
				<< " Use implementation's default since no conversion buffer "
				<< "was requested.";
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

	auto calc = std::make_unique<Calculation>(type(),
		make_context(skip_front, skip_back, audiofilename));

	if (!calc)
	{
		// TODO Throw something
	}

	this->process_file(audiofilename, *calc, BLOCKSIZE.DEFAULT, false);

	// Sanity-check result

	this->log_completeness_check(*calc);

	const auto track_checksums { calc->result() };

	if (track_checksums.size() == 0)
	{
		ARCS_LOG_ERROR << "Calculation lead to no result, return null";

		return ChecksumSet { 0 };
	}

	return track_checksums[0];
}


void ARCSCalculator::Impl::set_selection(const FileReaderSelection *selection)
{
	selection_ = std::move(selection);
}


const FileReaderSelection& ARCSCalculator::Impl::selection() const
{
	return *selection_;
}


void ARCSCalculator::Impl::set_type(const arcstk::checksum::type type)
{
	type_ = type;
}


arcstk::checksum::type ARCSCalculator::Impl::type() const
{
	return type_;
}


void ARCSCalculator::Impl::log_completeness_check(const Calculation &calc) const
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


// TOCParser


TOCParser::TOCParser()
	: impl_ { std::make_unique<TOCParser::Impl>() }
{
	impl_->set_selection(FileReaderRegistry::toc_selection());
}


TOCParser::~TOCParser() noexcept = default;


std::unique_ptr<TOC> TOCParser::parse(const std::string &metafilename) const
{
	return impl_->parse(metafilename);
}


void TOCParser::set_selection(const FileReaderSelection *selection)
{
	this->impl_->set_selection(selection);
}


const FileReaderSelection& TOCParser::selection() const
{
	return impl_->selection();
}


// ARIdCalculator


ARIdCalculator::ARIdCalculator()
	: impl_(std::make_unique<ARIdCalculator::Impl>())
{
	impl_->set_toc_selection(FileReaderRegistry::toc_selection());
	impl_->set_audio_selection(FileReaderRegistry::audio_selection());
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


void ARIdCalculator::set_toc_selection(
		const FileReaderSelection *selection)
{
	impl_->set_toc_selection(selection);
}


const FileReaderSelection& ARIdCalculator::toc_selection() const
{
	return impl_->toc_selection();
}


void ARIdCalculator::set_audio_selection(
		const FileReaderSelection *selection)
{
	impl_->set_audio_selection(selection);
}


const FileReaderSelection& ARIdCalculator::audio_selection() const
{
	return impl_->audio_selection();
}


// ARCSCalculator


ARCSCalculator::ARCSCalculator()
	: impl_(std::make_unique<ARCSCalculator::Impl>())
{
	impl_->set_selection(FileReaderRegistry::audio_selection());
}


ARCSCalculator::ARCSCalculator(const arcstk::checksum::type type)
	: impl_(std::make_unique<ARCSCalculator::Impl>(type))
{
	impl_->set_selection(FileReaderRegistry::audio_selection());
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


void ARCSCalculator::set_selection(const FileReaderSelection *selection)
{
	this->impl_->set_selection(std::move(selection));
}


const FileReaderSelection& ARCSCalculator::selection() const
{
	return impl_->selection();
}


void ARCSCalculator::set_type(const arcstk::checksum::type type)
{
	impl_->set_type(type);
}


arcstk::checksum::type ARCSCalculator::type() const
{
	return impl_->type();
}

} // namespace v_1_0_0

} // namespace arcsdec

