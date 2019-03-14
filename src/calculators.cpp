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

#ifndef __LIBARCSDEC_IDENTIFIER_HPP__
#include <arcs/identifier.hpp>
#endif
#ifndef __LIBARCSDEC_CALCULATE_HPP__
#include <arcs/calculate.hpp>
#endif
#ifndef __LIBARCSDEC_LOGGING_HPP__
#include <arcs/logging.hpp>
#endif
#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"
#endif
#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include "metaparser.hpp"
#endif


namespace arcs
{

namespace details
{

/**
 * Extract the filenames from a \ref TOC to an iterable container
 *
 * \param[in] toc The TOC to get the offsets from
 *
 * \return The offsets of this TOC as an iterable container
 */
std::vector<std::string> get_filenames(const TOC &toc);


std::vector<std::string> get_filenames(const TOC &toc)
{
	std::size_t size = toc.track_count();

	std::vector<std::string> files;
	files.reserve(size);

	for (unsigned int i = 1; i <= size; ++i)
	{
		files.emplace_back(toc.filename(i));
	}

	return files;
}


/**
 * Returns the audiofile layout of a \ref TOC.
 *
 * The first value of the returned pair is TRUE iff \c toc references a single
 * audio file, otherwise FALSE.
 *
 * The second value of the returned pair is TRUE iff \c toc references a
 * pairwise distinct list of audio files, otherwise FALSE.
 *
 * <tt><TRUE, TRUE></tt> : only one file
 *
 * <tt><FALSE, TRUE></tt> : multiple files, one per track
 *
 * <tt><FALSE, FALSE></tt> : multiple files, but some files contain more than
 *							one track
 *
 * <tt><TRUE, FALSE></tt> : impossible
 *
 * \param[in] toc The TOC to analyze
 *
 * \return TRUE iff \c toc references a single audio file.
 */
std::pair<bool,bool> audiofile_layout(const TOC &toc);


std::pair<bool,bool> audiofile_layout(const TOC &toc)
{
	std::vector<std::string> files  = get_filenames(toc);
	bool is_single_file             = true;
	bool is_pairwise_distinct_files = true;

	// Validate TOC:
	// check whether the metafile references references either:
	// - exactly one audiofile (== all filenames equal)
	// - or pairwise different audiofiles, one per track

	std::unordered_set<std::string> fileset; // keep set of distinct file names
	for (uint16_t i = 2; i <= files.size(); ++i)
	{
		// Next file reference points to same file as current?
		is_single_file = is_single_file and (files[i] == files[i-1]);

		// Insert current file reference to set
		fileset.insert(files[i-1]);

		// Next file reference is not yet inserted?
		is_pairwise_distinct_files = is_pairwise_distinct_files
			and (fileset.end() == fileset.find(files[i]));
	}

	return std::make_pair(is_single_file, is_pairwise_distinct_files);
}

} // namespace details


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
 * \internal \defgroup calcImpl Implementation details of the Calc API
 *
 * \ingroup calc
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
/// \internal \addtogroup calcImpl
/// @{


/**
 * Private implementation of an ARCSCalculator.
 */
class ARCSCalculator::Impl final
{

public:

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
	std::unique_ptr<ChecksumSet> calculate(const std::string &audiofilename,
		const bool &skip_front, const bool &skip_back);


protected:

	/**
	 * Worker method: calculating the ARCS of a single audiofile.
	 *
	 * \param[in] audiofilename Name of the audiofile
	 *
	 * \return The AccurateRip checksum of this track
	 */
	virtual std::unique_ptr<ChecksumSet> calculate_track(
		const std::string &audiofilename,
		const bool &skip_front, const bool &skip_back);
};


/// @}
/// \endcond IMPL_ONLY


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
		AudioReaderCreator creator;

		reader = creator.create_audio_reader(audiofilename);

		if (!reader)
		{
			ARCS_LOG_ERROR << "No AudioReader available. Bail out.";

			return std::make_pair(Checksums(0), ARId(0,0,0,0));
		}
	}


	// Configure AudioReader and process file

	reader->set_calc(std::move(calc));

	Checksums checksums = reader->process_file(audiofilename);

	auto id = reader->calc().context().id();

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

	std::unique_ptr<ChecksumSet> arcs {

		// Apply back skipping request on first file only if it's also the last

		this->calculate_track(audiofilenames[0], first_track_with_skip,
			(single_file ? last_track_with_skip : false))
	};

	if (arcs)
	{
		arcss[0] = *arcs;
	} else
	{
		ARCS_LOG_WARNING << "Got nullptr for first track";
	}

	// Avoid calculating a single track track twice

	if (single_file)
	{
		return arcss;
	}

	// Calculate second to second last track

	for (uint16_t i = 1; i < audiofilenames.size() - 1; ++i)
	{
		arcs = this->calculate_track(audiofilenames[i], false, false);

		if (arcs)
		{
			arcss[i] = *arcs;
		} else
		{
			ARCS_LOG_INFO << "Got nullptr for track "
				<< std::to_string(i+1) << ", file: " << audiofilenames[i];
			return arcss;
		}
	}

	// Calculate last track

	arcs = this->calculate_track(audiofilenames.back(), false,
			last_track_with_skip);

	if (arcs)
	{
		arcss[arcss.size() - 1] = *arcs;
	} else
	{
		ARCS_LOG_INFO << "Got nullptr for last track";
		return arcss;
	}

	return arcss;
}


std::unique_ptr<ChecksumSet> ARCSCalculator::Impl::calculate(
	const std::string &audiofilename,
	const bool &skip_front,
	const bool &skip_back)
{
	return this->calculate_track(audiofilename, skip_front, skip_back);
}


std::unique_ptr<ChecksumSet> ARCSCalculator::Impl::calculate_track(
	const std::string &audiofilename,
	const bool &skip_front, const bool &skip_back)
{
	ARCS_LOG_DEBUG << "Calculate track from file: " << audiofilename;

	// Configure Calculation

	auto calc = std::make_unique<Calculation>(
		make_context(audiofilename, skip_front, skip_back));


	// Create AudioReader

	AudioReaderCreator creator;
	std::unique_ptr<AudioReader> reader =
		creator.create_audio_reader(audiofilename);

	if (!reader)
	{
		ARCS_LOG_ERROR << "No AudioReader available. Bail out.";

		return std::make_unique<ChecksumSet>(0);
	}


	// Configure AudioReader, process file, sanity-check result

	reader->set_calc(std::move(calc));

	auto track_checksums { reader->process_file(audiofilename) };

	if (track_checksums.size() == 0)
	{
		ARCS_LOG_ERROR << "Unexpected empty result for single track";

		return std::make_unique<ChecksumSet>(0);
	}


	// Convert checksums

	auto checksums { std::make_unique<ChecksumSet>(track_checksums[0]) };

	if (checksums->empty())
	{
		ARCS_LOG_ERROR << "Calculation lead to no result, return null";

		return std::make_unique<ChecksumSet>(0);
	}

	return checksums;
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


std::unique_ptr<ChecksumSet> ARCSCalculator::calculate(
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


/**
 * Private implementation of a ARCSMultifileAlbumCalculator
 */
class ARCSMultifileAlbumCalculator::Impl final
{

public:

	std::tuple<Checksums, ARId, std::unique_ptr<TOC>> calculate(
			const std::vector<std::string> &audiofilenames,
			const std::string &metafilename) const;

	std::tuple<Checksums, ARId, std::unique_ptr<TOC>> calculate(
			const std::string &metafilename,
			const std::string &searchpath) const;
};


std::tuple<Checksums, ARId, std::unique_ptr<TOC>>
	ARCSMultifileAlbumCalculator::Impl::calculate(
			const std::vector<std::string> &audiofilenames,
			const std::string &metafilename) const
{
	if (audiofilenames.empty())
	{
		return std::make_tuple(Checksums(0), *make_empty_arid(), nullptr);
	}

	TOCParser parser;
	TOC toc { parser.parse(metafilename) };

	ARCSCalculator calculator;

	if (1 == audiofilenames.size())
	{
		auto result { calculator.calculate(audiofilenames[0], toc) };

		return std::make_tuple(result.first, result.second,
			std::make_unique<TOC>(toc));
	}

	auto arcss { calculator.calculate(audiofilenames, true, true) };

	// Compute ARId (avoiding re-reading audiofiles for leadout)

	if (toc.track_count() != arcss.size())
	{
		ARCS_LOG_ERROR << "TOC specifies " << toc.track_count() << " tracks "
			<< " but " << arcss.size() << " input files were passed";
	}

	ARCS_LOG_DEBUG << "Compute result from audiofilenames and metafilename";

	return std::make_tuple(arcss, *make_arid(toc), std::make_unique<TOC>(toc));
}


std::tuple<Checksums, ARId, std::unique_ptr<TOC>>
	ARCSMultifileAlbumCalculator::Impl::calculate(
		const std::string &metafilename,
		const std::string &audiosearchpath) const
{
	if (metafilename.empty())
	{
		return std::make_tuple(Checksums(0), *make_empty_arid(), nullptr);
	}

	TOCParser parser;
	TOC toc { parser.parse(metafilename) };

	auto audiofile_layout = details::audiofile_layout(toc);

	if (not audiofile_layout.first and not audiofile_layout.second)
	{
		ARCS_LOG_ERROR <<
			"TOC references multiple audio files, but not pairwise distinct";

		return std::make_tuple(Checksums(0), *make_empty_arid(), nullptr);
	}

	// Calculate ARCSs

	ARCSCalculator calculator;

	if (audiofile_layout.first) // single audio file?
	{
		ARCS_LOG_DEBUG << "TOC references single audio file: "
			<< toc.filename(1);

		if (audiosearchpath.empty())
		{
			// Just take the audiofilename as-is from the TOC

			auto result { calculator.calculate(toc.filename(1), toc) };
			return std::make_tuple(result.first, result.second,
					std::make_unique<TOC>(toc));
		}

		std::string audiofile { toc.filename(1) };

		if (not audiosearchpath.empty())
		{
			// NOTE: there MUST be a platform-specific file separator at the
			// end of audiosearchpath or this will fail
			audiofile.insert(0, audiosearchpath);
		}

		auto result { calculator.calculate(audiofile, toc) };

		return std::make_tuple(result.first, result.second,
					std::make_unique<TOC>(toc));
	}

	ARCS_LOG_DEBUG << "TOC references multiple files";

	auto filenames { details::get_filenames(toc) };

	if (not audiosearchpath.empty())
	{
		for (auto& filename : filenames)
		{
			// NOTE: there MUST be a platform-specific file separator at the
			// end of audiosearchpath or this will fail
			filename.insert(0, audiosearchpath);
		}
	}

	auto arcss = calculator.calculate(filenames, true, true);

	ARCS_LOG_DEBUG << "Calculate result from metafilename (and searchpath)";

	return std::make_tuple(arcss, *make_arid(toc), std::make_unique<TOC>(toc));
}


// ARCSMultifileAlbumCalculator


ARCSMultifileAlbumCalculator::ARCSMultifileAlbumCalculator()
	: impl_(std::make_unique<ARCSMultifileAlbumCalculator::Impl>())
{
	// empty
}


ARCSMultifileAlbumCalculator::~ARCSMultifileAlbumCalculator() noexcept = default;


std::tuple<Checksums, ARId, std::unique_ptr<TOC>>
	ARCSMultifileAlbumCalculator::calculate(
		const std::vector<std::string> &audiofilenames,
		const std::string &metafilename) const
{
	return impl_->calculate(audiofilenames, metafilename);
}


std::tuple<Checksums, ARId, std::unique_ptr<TOC>>
	ARCSMultifileAlbumCalculator::calculate(
		const std::string &metafilename, const std::string &searchpath) const
{
	return impl_->calculate(metafilename, searchpath);
}

} // namespace arcs

