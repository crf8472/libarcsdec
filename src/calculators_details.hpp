#ifndef __LIBARCSDEC_CALCULATORS_DETAILS_HPP__
#define __LIBARCSDEC_CALCULATORS_DETAILS_HPP__
/**
 * \file
 *
 * \brief Implementation details of calculators.hpp
 */

#ifndef __LIBARCSTK_METADATA_HPP__
#include <arcstk/metadata.hpp>  // for ToC
#endif
#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp> // for Checksums, ChecksumtypeSet, Points
#endif

#include <cstdint>  // for uint32_t, int32_t
#include <memory>   // for unique_ptr


namespace arcsdec
{
inline namespace v_1_0_0
{

using arcstk::ToC;
using arcstk::Algorithm;
using arcstk::AudioSize;
using arcstk::Calculation;
using arcstk::Checksums;
using arcstk::ChecksumSet;
using arcstk::ChecksumtypeSet;
using arcstk::Context;
using arcstk::Points;
using arcstk::SampleInputIterator;
using arcstk::Settings;
using arcstk::ToCData;

class AudioReader;
class SampleProcessor;


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

} // namespace v_1_0_0
} // namespace arcsdec

#endif

