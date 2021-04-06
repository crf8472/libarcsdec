#ifndef __LIBARCSDEC_SAMPLEPROC_HPP__
#define __LIBARCSDEC_SAMPLEPROC_HPP__

/**
 * \file
 *
 * \brief Interface for processing samples
 */

#include <functional> // FIXME Seems unnecessary

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>
#endif

namespace arcsdec
{
inline namespace v_1_0_0
{

/**
 * \defgroup sampleproc API for processing samples
 *
 * \brief API for processing samples.
 *
 * @{
 */

using arcstk::SampleInputIterator;
using arcstk::AudioSize;


/**
 * \brief Symbolic constants for certain block sizes (in PCM 32 bit samples).
 */
struct BLOCKSIZE
{
	/**
	 * \brief Maximum buffer size in number of PCM 32 bit samples.
	 *
	 * Currently, this is 256 MiB.
	 */
	constexpr static unsigned MAX     = 67108864; // == 256 * 1024^2 / 4

	/**
	 * \brief Default buffer size in number of PCM 32 bit samples.
	 *
	 * Currently, this is 64 MiB.
	 */
	constexpr static unsigned DEFAULT = 16777216; // == 64 * 1024^2 / 4

	/**
	 * \brief Minimum buffer size in number of PCM 32 bit samples.
	 *
	 * Currently, this is 256 KiB.
	 *
	 * This is the maximal size of a fLaC frame. This setting entails that at
	 * least one fLaC frame of maximal size is guaranteed to fit in a block of
	 * minimal size.
	 */
	constexpr static unsigned MIN     = 65536; // == 256 * 1024 / 4
};


/**
 * \brief Interface for processing samples as provided by a SampleProvider.
 */
class SampleProcessor
{
public:

	/**
	 * \brief Constructor.
	 */
	SampleProcessor();

	/**
	 * \brief Virtual default constructor.
	 */
	virtual ~SampleProcessor() noexcept;

	/**
	 * \brief Callback for start of input.
	 */
	void start_input();

	/**
	 * \brief Callback for sample sequences.
	 *
	 * \param[in] begin Begin of the sample sequence
	 * \param[in] end   End of the sample sequence
	 */
	void append_samples(SampleInputIterator begin, SampleInputIterator end);

	/**
	 * \brief Callback for updating the AudioSize.
	 *
	 * \param[in] size New AudioSize
	 */
	void update_audiosize(const AudioSize &size);

	/**
	 * \brief Callback for end of input
	 */
	void end_input();

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

protected:

	SampleProcessor(const SampleProcessor &) = default;
	SampleProcessor& operator = (const SampleProcessor &) = default;

	SampleProcessor(SampleProcessor &&) noexcept = default;
	SampleProcessor& operator = (SampleProcessor &&) noexcept = default;

private:

	/**
	 * \brief Implements SampleProcessor::start_input.
	 */
	virtual void do_start_input()
	= 0;

	/**
	 * \brief Implements SampleProcessor::append_samples.
	 */
	virtual void do_append_samples(SampleInputIterator begin,
			SampleInputIterator end)
	= 0;

	/**
	 * \brief Implements SampleProcessor::update_audiosize.
	 */
	virtual void do_update_audiosize(const AudioSize &size)
	= 0;

	/**
	 * \brief Implements SampleProcessor::end_input.
	 */
	virtual void do_end_input()
	= 0;

	/**
	 * \brief Sequence counter.
	 *
	 * Counts the calls of SampleProcessor::append_samples.
	 */
	int64_t total_sequences_;

	/**
	 * \brief PCM 32 Bit Sample counter.
	 *
	 * Counts the total number of processed PCM 32 bit samples.
	 */
	int64_t total_samples_;
};


/**
 * \brief Inteface for providers of sample sequences.
 *
 * A SampleProvider is a source for sample sequences and updated AudioSize
 * values. It can signal different events while processing the audio input.
 * A SampleProcessor can be attached to it as an addressee of those events.
 *
 * \see SampleProviderBase
 */
class SampleProvider
{
public:

	/**
	 * \brief Constructor.
	 */
	SampleProvider();

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~SampleProvider() noexcept;

	/**
	 * \brief Attach a SampleProcessor.
	 *
	 * \param[in] processor The SampleProcessor to use
	 */
	void attach_processor(SampleProcessor &processor);

	/**
	 * \brief Return the registered SampleProcessor.
	 *
	 * \return The SampleProcessor the reader uses
	 */
	const SampleProcessor* processor() const;

	/**
	 * \brief Signal the processor that input starts.
	 */
	void signal_startinput();

	/**
	 * \brief Signal the processor to append the following range of samples.
	 *
	 * \param[in] begin Start of the sample sequence
	 * \param[in] end   End of the sample sequence
	 */
	void signal_appendsamples(SampleInputIterator begin,
			SampleInputIterator end);

	/**
	 * \brief Signal the processor to update the audio size.
	 *
	 * \param[in] size The updated value to signal
	 */
	void signal_updateaudiosize(const AudioSize &size);

	/**
	 * \brief Signal the processor that input ends.
	 */
	void signal_endinput();

protected:

	SampleProvider(const SampleProvider &) = default;
	SampleProvider& operator = (const SampleProvider &) = default;

	SampleProvider(SampleProvider &&) noexcept = default;
	SampleProvider& operator = (SampleProvider &&) noexcept = default;

private:

	/**
	 * \brief Register a SampleProcessor.
	 *
	 * This will register all callback methods of the \c processor. Already
	 * registered callbacks will be overwritten by this method.
	 *
	 * The method is a mere convenience for completely registering a single
	 * SampleProcessor instance.
	 *
	 * \param[in] processor The SampleProcessor to use
	 */
	virtual void do_attach_processor(SampleProcessor &processor)
	= 0;

	/**
	 * \brief Return the registered SampleProcessor.
	 *
	 * \return The SampleProcessor the reader uses
	 */
	virtual const SampleProcessor* do_processor() const
	= 0;

	/**
	 * \brief Implements signal_startinput().
	 */
	virtual void do_signal_startinput()
	= 0;

	/**
	 * \brief Implements signal_appendsamples().
	 *
	 * \param[in] begin Start of the sample sequence
	 * \param[in] end   End of the sample sequence
	 */
	virtual void do_signal_appendsamples(SampleInputIterator begin,
			SampleInputIterator end)
	= 0;

	/**
	 * \brief Signal the processor to update the audio size.
	 *
	 * \param[in] size The updated value to signal
	 */
	virtual void do_signal_updateaudiosize(const AudioSize &size)
	= 0;

	/**
	 * \brief Implements signal_endinput().
	 */
	virtual void do_signal_endinput()
	= 0;
};


/**
 * \brief Base class for SampleProvider implementations.
 *
 * Implements SampleProvider.
 */
class SampleProviderBase : public SampleProvider
{
public:

	/**
	 * \brief Constructor
	 */
	SampleProviderBase();

protected:

	~SampleProviderBase() noexcept = default;

	SampleProviderBase(const SampleProviderBase &rhs) = default;
	SampleProviderBase& operator = (const SampleProviderBase &rhs) = default;

	SampleProviderBase(SampleProviderBase &&) noexcept = default;
	SampleProviderBase& operator = (SampleProviderBase &&) noexcept = default;

	/**
	 * \brief Default implementation of attach_processor().
	 */
	void attach_processor_impl(SampleProcessor &processor);

	/**
	 * \brief Use the internal SampleProcessor.
	 */
	SampleProcessor* use_processor();

private:

	void do_signal_startinput() override;

	void do_signal_appendsamples(SampleInputIterator begin,
			SampleInputIterator end) override;

	void do_signal_updateaudiosize(const AudioSize &size) override;

	void do_signal_endinput() override;

	void do_attach_processor(SampleProcessor &processor) override;

	const SampleProcessor* do_processor() const override;

	/**
	 * \brief Internal pointer to the SampleProcessor.
	 */
	SampleProcessor* processor_;
};

/// @}

} // namespace v_1_0_0
} // namespace arcsdec

#endif

