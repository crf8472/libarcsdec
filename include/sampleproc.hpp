#ifndef LIBARCSDEC_SAMPLEPROC_HPP_
#define LIBARCSDEC_SAMPLEPROC_HPP_

/**
 * \file
 *
 * \brief Interface for processing samples
 */

#ifndef LIBARCSTK_CALCULATE_HPP_
#include <arcstk/calculate.hpp>
#endif


namespace arcsdec
{
                                                  /** \cond NAMESPACE_v_1_0_0 */
inline namespace v_1_0_0
{
                                                                 /** \endcond */
namespace calc
{

using arcstk::AudioSize;
using arcstk::CalculationSet;
using arcstk::Checksums;
using arcstk::ChecksumtypeSet;
using arcstk::InterleavedSamples;
using arcstk::PlanarSamples;
using arcstk::Points;
using arcstk::Settings;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
// -Wnon-virtual-dtor is deactivated: warns about protected non-virtual dtors in
// the base CRTP. This is a false positive since the instances are never
// destroyed by a pointer to SampleReceiver<T> and a CRTP does not have virtual
// members.

/**
 * \brief SampleProcessor that updates a Calculation.
 */
class CalculationProcessor final
{
public:

	/**
	 * \brief Constructor.
	 */
	CalculationProcessor() = default;

	/**
	 * \brief Constructor.
	 *
	 * \param[in] types    Requested checksum types to calculate
	 * \param[in] settings Calculation settings
	 * \param[in] offsets  Track offsets (in LBA frames)
	 * \param[in] leadout  Leadout frame
	 */
	CalculationProcessor(ChecksumtypeSet types, Settings settings,
		Points offsets, const AudioSize& leadout)
		: types_    { std::move(types)   }
		, settings_ { settings }
		, offsets_  { std::move(offsets) }
		, leadout_  { leadout  }
		, calculationset_ { nullptr }
	{
		// empty
	}

	/**
	 * \brief Default destructor.
	 */
	~CalculationProcessor() noexcept = default;

	// not copy-constructible, not copy-assignable

	CalculationProcessor(const CalculationProcessor& rhs) noexcept
		= delete;

	CalculationProcessor& operator = (const CalculationProcessor& rhs) noexcept
		= delete;

	// move-constructible + -assignable

	CalculationProcessor(CalculationProcessor&& rhs) noexcept
		= default;

	CalculationProcessor& operator = (CalculationProcessor&& rhs) noexcept
		= default;

	/**
	 * \brief Total number of tracks to process.
	 *
	 * \return Total tracks
	 */
	std::size_t total_tracks() const
	{
		return offsets_.size();
	}

	/**
	 * \brief Offsets.
	 *
	 * \return Offsets.
	 */
	Points offsets() const
	{
		return offsets_;
	}

	void set_offsets(const Points& offsets)
	{
		offsets_ = offsets;
	}

	/**
	 * \brief Leadout frame.
	 *
	 * \return Leadout.
	 */
	AudioSize leadout() const
	{
		return leadout_;
	}

	void set_leadout(const AudioSize& leadout)
	{
		ARCS_LOG_DEBUG << "Updated leadout: " << leadout;

		leadout_ = leadout;
	}

	/**
	 * \brief Callback for sample sequences.
	 *
	 * \param[in] samples Samples sequence
	 */
	template <typename I>
	void receive_samples(const arcstk::PlanarSamples<I>& samples)
	{
		ARCS_LOG(DEBUG2) << "CalculationProcessor received: RECEIVE SAMPLES";

		using std::cbegin;
		using std::cend;
		receive_samples(cbegin(samples), cend(samples));
	}

	/**
	 * \brief Callback for sample sequences.
	 *
	 * \param[in] samples Samples sequence
	 */
	template <typename I>
	void receive_samples(const arcstk::InterleavedSamples<I>& samples)
	{
		ARCS_LOG(DEBUG2) << "CalculationProcessor received: RECEIVE SAMPLES";

		using std::cbegin;
		using std::cend;
		receive_samples(cbegin(samples), cend(samples));
	}

	/**
	 * \brief Receive samples from sample provider.
	 *
	 * \tparam B Iterator type of begin iterator
	 * \tparam E Iterator type of end iterator
	 *
	 * \param[in] start Start of the sample sequence
	 * \param[in] stop  End of the sample sequence
	 */
	template <typename B, typename E>
	void receive_samples(B start, E stop)
	{
		ARCS_LOG(DEBUG2) << "CalculationProcessor received: RECEIVE SAMPLES";

		if (!calculationset_)
		{
			ARCS_LOG(DEBUG3) << "Create and initialize calculation objects";

			auto cset = arcstk::make_calculationset<B, E>(types_, settings_);
			// TODO Checks?
			cset->init(offsets_, leadout_);

			calculationset_ = std::move(cset);
		} else
		{
			ARCS_LOG(DEBUG3) << "Reuse calculationset";
		}

		using updateable_type = arcstk::UpdateableCalculationSet<B, E>;
		auto* calc = dynamic_cast<updateable_type*>(calculationset_.get());

		if (calc)
		{
			ARCS_LOG(DEBUG3) << "Pass samples to calculation object";

			calc->update(start, stop);
		} else
		{
			ARCS_LOG(DEBUG3) << "No calculation object, discard samples";
		}
	}

	/**
	 * \brief Return result.
	 *
	 * \return Combined calculation results.
	 */
	Checksums result() const
	{
		if (!calculationset_)
		{
			ARCS_LOG(DEBUG3) << "No calculation object, no Checksums";

			return {/* empty */};
		}

		return calculationset_->result();
	}

private:

	// For lazy initialization we have to cache all the stuff

	/**
	 * \brief Requested checksum types.
	 */
	ChecksumtypeSet types_ {}; // default

	/**
	 * \brief Settings for all Calculation instances.
	 */
	Settings settings_     {}; // default

	/**
	 * \brief Track offsets.
	 */
	Points offsets_        {}; // empty

	/**
	 * \brief Leadout.
	 */
	AudioSize leadout_     {}; // empty

	/**
	 * \brief Internal CalculationSet.
	 */
	std::unique_ptr<CalculationSet> calculationset_ {};
};

#pragma GCC diagnostic pop

/**
 * \brief Typedef for a SampleProcessor.
 */
using SampleProcessor = CalculationProcessor;

} // namespace calc

                                                  /** \cond NAMESPACE_v_1_0_0 */
} // namespace v_1_0_0
                                                                 /** \endcond */
} // namespace arcsdec

#endif

