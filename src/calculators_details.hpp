#ifndef LIBARCSDEC_CALCULATORS_HPP_
#error "Do not include calculators_details.hpp, include calculators.hpp instead"
#endif
#ifndef LIBARCSDEC_CALCULATORS_DETAILS_HPP_
#define LIBARCSDEC_CALCULATORS_DETAILS_HPP_

/**
 * \internal
 *
 * \file
 *
 * \brief Implementation details of calculators.hpp.
 */

#include <string>   // for string

#ifndef LIBARCSTK_METADATA_HPP_
#include "metadata.hpp"           // for AudioSize
#endif

namespace arcsdec
{
                                                  /** \cond NAMESPACE_v_1_0_0 */
inline namespace v_1_0_0
{
                                                                 /** \endcond */
namespace read
{

// forward delcaration
class AudioReader;

namespace details
{

using arcstk::AudioSize;

/**
 * \brief Ensure a non-zero leadout.
 *
 * If it is non-zero, use the leadout passed, otherwise call acquire_size()
 * on the \c reader for the \c audiofilename passed and return the result.
 */
AudioSize ensure_leadout(const AudioSize& leadout,
		const AudioReader& reader, const std::string& audiofilename);

} // namespace details
} // namespace read

                                                  /** \cond NAMESPACE_v_1_0_0 */
} // namespace v_1_0_0
                                                                 /** \endcond */
} // namespace arcsdec

#endif

