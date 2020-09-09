#ifndef __LIBARCSDEC_SAMPLEPROC_HPP__
#define __LIBARCSDEC_SAMPLEPROC_HPP__

/**
 * \file
 *
 * \brief Interface for processing samples
 */

#include <functional>

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
using arcstk::Calculation;

/**
 * \brief Symbolic constants for certain block sizes
 */
struct BLOCKSIZE_t
{
	/**
	 * \brief Maximum buffer size in number of PCM 32 bit samples.
	 *
	 * Currently, this is 256 MiB.
	 */
	const int32_t MAX     = 67108864; // == 256 * 1024^2 / 4

	/**
	 * \brief Default buffer size in number of PCM 32 bit samples.
	 *
	 * Currently, this is 64 MiB.
	 */
	const int32_t DEFAULT = 16777216; // == 64 * 1024^2 / 4

	/**
	 * \brief Minimum buffer size in number of PCM 32 bit samples.
	 *
	 * Currently, this is 256 KiB.
	 *
	 * This is the maximal size of a fLaC frame. This setting entails that at
	 * least one fLaC frame of maximal size is guaranteed to fit in a block of
	 * minimal size.
	 */
	const int32_t MIN     = 65536; // == 256 * 1024 / 4
};


/**
 * \brief Global symbolic block sizes
 */
extern const BLOCKSIZE_t BLOCKSIZE;


/**
 * \brief Interface for processing samples as provided by an AudioReaderImpl.
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

	// TODO clone()

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
 * \brief Adapter to wrap a Calculation in an unbuffered SampleProcessor.
 */
class CalculationProcessor final : virtual public SampleProcessor
{
public:

	/**
	 * \brief Converting constructor for Calculation instances.
	 *
	 * \param[in] calculation The Calculation to use
	 */
	CalculationProcessor(Calculation &calculation);

	/**
	 * \brief Virtual default destructor.
	 */
	~CalculationProcessor() noexcept override;

	CalculationProcessor(const CalculationProcessor &rhs) noexcept = delete;
	CalculationProcessor& operator = (const CalculationProcessor &rhs) noexcept
		= delete;

private:

	void do_start_input() override;

	void do_append_samples(SampleInputIterator begin, SampleInputIterator end)
		override;

	void do_update_audiosize(const AudioSize &size) override;

	void do_end_input() override;

	/**
	 * \brief Internal pointer to the calculation to wrap.
	 */
	Calculation *calculation_;
};


/**
 * \brief Inteface for providers of sample sequences.
 */
class ISampleProvider
{
public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~ISampleProvider() noexcept;

	/**
	 * \brief Register a function callback called as \c start_input().
	 *
	 * \param[in] f The function to register
	 */
	virtual void register_startinput(std::function<void()> f)
	= 0;

	/**
	 * \brief Register a function callback called as \c append_samples().
	 *
	 * \param[in] f The function to register
	 */
	virtual void register_appendsamples(
			std::function<void(SampleInputIterator, SampleInputIterator)> f)
	= 0;

	/**
	 * \brief Register a function callback called as \c update_audiosize().
	 *
	 * \param[in] f The function to register
	 */
	virtual void register_updateaudiosize(
			std::function<void(const AudioSize &)> f)
	= 0;

	/**
	 * \brief Register a function callback called as \c end_input().
	 *
	 * \param[in] f The function to register
	 */
	virtual void register_endinput(std::function<void()> f)
	= 0;

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
	virtual void attach_processor(SampleProcessor &processor)
	= 0;

	/**
	 * \brief Return the registered SampleProcessor.
	 *
	 * \return The SampleProcessor the reader uses
	 */
	virtual const SampleProcessor* processor() const
	= 0;

private:

	/**
	 * \brief Call the registered \c start_input() callback.
	 */
	virtual void call_startinput()
	= 0;

	/**
	 * \brief Call the registered \c append_samples() callback.
	 *
	 * \param[in] begin Iterator pointing to the begin of the sequence
	 * \param[in] end   Iterator pointing to the end of the sequence
	 */
	virtual void call_appendsamples(
			SampleInputIterator begin, SampleInputIterator end)
	= 0;

	/**
	 * \brief Call the registered \c update_audiosize() callback.
	 *
	 * The actual method call is just passed to the registered SampleProcessor.
	 *
	 * \param[in] size AudioSize to report
	 */
	virtual void call_updateaudiosize(const AudioSize &size)
	= 0;

	/**
	 * \brief Call the registered \c end_input() callback.
	 *
	 * \param[in] last_sample_index The 0-based index of the last sample.
	 */
	virtual void call_endinput()
	= 0;
};


/**
 * \brief A provider of sample sequences.
 *
 * It can have functions registered for starting and ending input,
 * appending samples and updating buffer size.
 */
class SampleProvider : public ISampleProvider
{
public:

	/**
	 * \brief Constructor
	 */
	SampleProvider();

	SampleProvider(const SampleProvider &rhs) = delete;
	SampleProvider& operator = (const SampleProvider &rhs) = delete;

	SampleProvider(SampleProvider &&) = default;
	SampleProvider& operator = (SampleProvider &&) = default;

	void register_startinput(std::function<void()> f) final;

	void register_appendsamples(
			std::function<void(SampleInputIterator, SampleInputIterator)> f)
		final;

	void register_updateaudiosize(std::function<void(const AudioSize &size)>)
		final;

	void register_endinput(std::function<void()> f) final;

	void attach_processor(SampleProcessor &processor) final;

	const SampleProcessor* processor() const final;

protected:

	/**
	 * \brief Default destructor.
	 */
	~SampleProvider() noexcept override;

	void call_startinput() override;

	void call_appendsamples(SampleInputIterator begin, SampleInputIterator end)
		override;

	void call_updateaudiosize(const AudioSize &size) override;

	void call_endinput() override;

	SampleProcessor* use_processor();

private:

	/**
	 * \brief Hook to be called before leaving register_processor().
	 */
	virtual void hook_post_attachprocessor();

	/**
	 * \brief Callback pointer for indicating the start of the sample input.
	 */
	std::function<void()> start_input_;

	/**
	 * \brief Callback pointer for appending samples sequences to processing.
	 */
	std::function<void(SampleInputIterator begin, SampleInputIterator end)>
		append_samples_;

	/**
	 * \brief Callback pointer for updateing the AudioSize.
	 */
	std::function<void(const AudioSize &size)> update_audiosize_;

	/**
	 * \brief Callback pointer for indicating the end of the sample input.
	 */
	std::function<void()> end_input_;

	/**
	 * \brief Internal pointer to the SampleProcessor.
	 */
	SampleProcessor* processor_;
};

/// @}

} // namespace v_1_0_0
} // namespace arcsdec

#endif

