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

#ifndef LIBARCSTK_METADATA_HPP_
#include "metadata.hpp"           // for AudioSize
#endif

#ifndef LIBARCSDEC_AUDIOREADER_HPP_
#include "audioreader.hpp"        // for AudioEventHandler
#endif
#ifndef LIBARCSDEC_SAMPLEPROC_HPP_
#include "sampleproc.hpp"         // for CalculationProcessor
#endif

namespace arcsdec
{
                                                  /** \cond NAMESPACE_v_1_0_0 */
inline namespace v_1_0_0
{
                                                                 /** \endcond */
namespace calc::details
{

using arcstk::AudioSize;

using read::AudioEventHandler;

/**
 * \brief AudioEventHandler that acts as an adaptor to an CalculationProcessor.
 */
class CalculationHandler final : public AudioEventHandler
{
	CalculationProcessor* processor_ {};

	void do_start_input() final
	{
		// empty
	}

	void do_audiosize(const arcstk::AudioSize& size) final
	{
		if (!processor_)
		{
			ARCS_LOG_ERROR << "Missed updated leadout: no CalculationProcessor";
			return;
		}

		processor_->set_leadout(size);
	}

	void do_end_input() final
	{
		// empty
	}

public:

	explicit CalculationHandler(CalculationProcessor* processor)
		: processor_ { processor }
	{
		// empty
	}

	CalculationHandler(const CalculationHandler&) = default;
	CalculationHandler& operator = (const CalculationHandler&) = default;

	CalculationHandler(CalculationHandler&&) noexcept = default;
	CalculationHandler& operator = (CalculationHandler&&) noexcept = default;

	~CalculationHandler() noexcept final = default;
};

} // namespace calc::details
                                                  /** \cond NAMESPACE_v_1_0_0 */
} // namespace v_1_0_0
                                                                 /** \endcond */
} // namespace arcsdec

#endif

