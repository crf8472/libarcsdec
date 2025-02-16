#ifndef __LIBARCSDEC_CALCULATORS_HPP__
#error "Do not include calculators_details.hpp, include calculators.hpp instead"
#endif
#ifndef __LIBARCSDEC_CALCULATORS_DETAILS_HPP__
#define __LIBARCSDEC_CALCULATORS_DETAILS_HPP__

/**
 * \internal
 *
 * \file
 *
 * \brief Implementation details of calculators.hpp.
 */

#ifndef __LIBARCSDEC_SAMPLEPROC_HPP__
#include "sampleproc.hpp"       // for SampleProcessor
#endif

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp> // for Checksums, ChecksumtypeSet, Points
#endif
#ifndef __LIBARCSTK_METADATA_HPP__
#include <arcstk/metadata.hpp>  // for ToC
#endif

#include <cstdint>  // for uint32_t, int32_t
#include <memory>   // for unique_ptr


namespace arcsdec
{
inline namespace v_1_0_0
{

class AudioReader;

namespace details
{

using arcstk::Algorithm;
using arcstk::AudioSize;
using arcstk::Calculation;
using arcstk::ChecksumSet;
using arcstk::Checksums;
using arcstk::ChecksumtypeSet;
using arcstk::Context;
using arcstk::Points;
using arcstk::SampleInputIterator;
using arcstk::Settings;
using arcstk::ToC;
using arcstk::ToCData;


/**
 * \brief A duplicate-free aggregate of Algorithm instances without particular
 * order.
 */
using Algorithms = std::unordered_set<std::unique_ptr<Algorithm>>;

/**
 * \brief Acquire the algorithms for calculating a set of types.
 *
 * \param[in] types Set of types
 *
 * \return Duplicate-free set of Algorithm instances
 */
Algorithms get_algorithms(const ChecksumtypeSet& types);

/**
 * \brief Wrapper for get_algorithms that throws on an empty set of algorithms.
 *
 * \param[in] types Set of types
 *
 * \return Duplicate-free set of Algorithm instances
 *
 * \throws If the resulting set of Algorithm instances would be empty
 */
Algorithms get_algorithms_or_throw(const ChecksumtypeSet& types);

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

/**
 * \brief Combine all results of the specified Calculation instances in a
 * single, duplicate-free object.
 * .
 * \param[in] calculations Calculations to aggregate the results from
 *
 * \return Aggregated results from all input Calculation instances
 */
Checksums merge_results(const std::vector<Calculation>& calculations);

/**
 * \brief Derive an audiofile from a ToC.
 *
 * Iff the ToC contains exactly one audiofilename, this name is the result. Iff
 * the ToC contains multiple filenames that are identical, this name is the
 * result. Iff the ToC contains either no filename or multiple different
 * filenames, the function throws.
 *
 * \param[in] toc The ToC to inspect for audiofile name
 */
std::string get_audiofilename(const ToC& toc);

/**
 * \brief Worker: process an audio file via specified SampleProcessor.
 *
 * The \c buffer_size is specified as number of 32 bit PCM samples. It is
 * applied to the created \link AudioReader AudioReaders.
 *
 * \param[in] audiofilename  Name of the audiofile
 * \param[in] reader         Audio reader
 * \param[in] buffer_size    Read buffer size in number of samples
 * \param[in] processor      The SampleProcessor to use
 */
void process_audio_file(const std::string& audiofilename,
		std::unique_ptr<AudioReader> reader, const int64_t buffer_size,
		SampleProcessor& processor);


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
	CalculationProcessor(Calculation& calculation);

	/**
	 * \brief Default destructor.
	 */
	~CalculationProcessor() noexcept final;

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

} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

#endif

