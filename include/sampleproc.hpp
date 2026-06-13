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
namespace read
{

/**
 * \brief Event handler for audio read events.
 */
class AudioEventHandler // TODO Should be MetadataHandler
{
public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~AudioEventHandler() noexcept = default;

	/**
	 * \brief Call on signal start_input.
	 */
	void start_input()
	{
		return do_start_input();
	}

	/**
	 * \brief Call on signal size.
	 *
	 * \param[in] size Updated audio size
	 */
	void audiosize(const arcstk::AudioSize& size)
	{
		return do_audiosize(size);
	}

	/**
	 * \brief Call on signal end_input.
	 */
	void end_input()
	{
		return do_end_input();
	}

private:

	virtual void do_start_input()
	= 0;

	virtual void do_audiosize(const arcstk::AudioSize& size)
	= 0;

	virtual void do_end_input()
	= 0;
};

} // namespace read

namespace calc
{

using arcstk::InterleavedSamples;
using arcstk::PlanarSamples;

/**
 * \brief Receiver of samples.
 *
 * CRTP for receiving samples.
 */
template <typename T>
class SampleReceiver
{
public:

	/**
	 * \brief Callback for sample sequences.
	 *
	 * \param[in] samples Samples sequence
	 */
	template <typename I>
	void receive_samples(const arcstk::PlanarSamples<I>& samples)
	{
		static_cast<T*>(this)->do_receive_samples(samples);
	}

	/**
	 * \brief Callback for sample sequences.
	 *
	 * \param[in] samples Samples sequence
	 */
	template <typename I>
	void receive_samples(const arcstk::InterleavedSamples<I>& samples)
	{
		static_cast<T*>(this)->do_receive_samples(samples);
	}

	/**
	 * \brief Implements \ref append_samples().
	 *
	 * \param[in] start Start of the sample sequence
	 * \param[in] stop  End of the sample sequence
	 */
	template <typename B, typename E>
	void receive_samples(B start, E stop)
	{
		static_cast<T*>(this)->do_receive_samples(start, stop);
	}

	//SampleReceiver() = default;

	virtual ~SampleReceiver() noexcept = default;
};

using arcstk::AudioSize;
using arcstk::CalculationSet;
using arcstk::Checksums;
using arcstk::ChecksumtypeSet;
using arcstk::Points;
using arcstk::Settings;

/**
 * \brief SampleProcessor that updates a Calculation.
 */
class CalculationProcessor final : public SampleReceiver<CalculationProcessor>
								 , public read::AudioEventHandler
{
public:

	CalculationProcessor()
	{
		// empty
	};

	/**
	 * \brief Constructor.
	 *
	 * \param[in] types    Requested checksum types to calculate
	 * \param[in] settings Calculation settings
	 * \param[in] offsets  Track offsets (in LBA frames)
	 * \param[in] leadout  Leadout frame
	 */
	CalculationProcessor(const ChecksumtypeSet& types, const Settings& settings,
		const Points& offsets, const AudioSize& leadout)
		: types_    { types    }
		, settings_ { settings }
		, offsets_  { offsets  }
		, leadout_  { leadout  }
		, calculationset_ { nullptr }
	{
		// empty
	}

	/**
	 * \brief Default destructor.
	 */
	~CalculationProcessor() noexcept final = default;

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
	 * \brief Callback for sample sequences.
	 *
	 * \param[in] samples Samples sequence
	 */
	template <typename I>
	void do_receive_samples(const arcstk::PlanarSamples<I>& samples)
	{
		ARCS_LOG(DEBUG2) << "CalculationProcessor received: RECEIVE SAMPLES";

		using std::cbegin;
		using std::cend;
		do_receive_samples(cbegin(samples), cend(samples));
	}

	/**
	 * \brief Callback for sample sequences.
	 *
	 * \param[in] samples Samples sequence
	 */
	template <typename I>
	void do_receive_samples(const arcstk::InterleavedSamples<I>& samples)
	{
		ARCS_LOG(DEBUG2) << "CalculationProcessor received: RECEIVE SAMPLES";

		using std::cbegin;
		using std::cend;
		do_receive_samples(cbegin(samples), cend(samples));
	}

	/**
	 * \brief Implements \ref append_samples().
	 *
	 * \param[in] start Start of the sample sequence
	 * \param[in] stop  End of the sample sequence
	 */
	template <typename B, typename E>
	void do_receive_samples(B start, E stop)
	{
		ARCS_LOG(DEBUG2) << "CalculationProcessor received: RECEIVE SAMPLES";

		if (!calculationset_)
		{
			calculationset_ =
				arcstk::make_calculationset<B, E>(types_, settings_);
			calculationset_->init(offsets_, leadout_);
		}

		using updateable_type = arcstk::UpdateableCalculationSet<B, E>;

		if (auto calc = dynamic_cast<updateable_type*>(calculationset_.get()))
		{
			calc->update(start, stop);
		} else
		{
			// TODO Error!
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
			return {/* empty */};
		}

		return calculationset_->result();
	}

private:

	// SampleProcessor

	void do_start_input() final
	{
		// TODO Log sth
	}

	void do_audiosize(const AudioSize& size) final
	{
		leadout_ = size;
	}

	void do_end_input() final
	{
		// TODO Log sth
	}

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

/**
 * \brief Typedef for a SampleProcessor.
 */
using SampleProcessor = SampleReceiver<CalculationProcessor>;

} // namespace calc

                                                  /** \cond NAMESPACE_v_1_0_0 */
} // namespace v_1_0_0
                                                                 /** \endcond */
} // namespace arcsdec

#endif

