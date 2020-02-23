#ifndef __LIBARCSDEC_SAMPLEPROC_HPP__
#define __LIBARCSDEC_SAMPLEPROC_HPP__

/**
 * \file sampleproc.hpp Interface for processing samples
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
 * \internal \defgroup sampleproc Interface for processing samples
 *
 * \brief Interface for processing samples
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
	const uint32_t MAX     = 67108864; // == 256 * 1024^2 / 4

	/**
	 * \brief Default buffer size in number of PCM 32 bit samples.
	 *
	 * Currently, this is 64 MiB.
	 */
	const uint32_t DEFAULT = 16777216; // == 64 * 1024^2 / 4

	/**
	 * \brief Minimum buffer size in number of PCM 32 bit samples.
	 *
	 * Currently, this is 256 KiB.
	 */
	const uint32_t MIN     = 65536; // == 256 * 1024 / 4
	// The size of a maximal fLaC block. This entails that at least one fLaC
	// frame is guaranteed to fit in a block of minimal size.
};


/**
 * \brief Global symbolic block sizes
 */
extern const BLOCKSIZE_t BLOCKSIZE;


/**
 * Interface for processing samples provided by an AudioReaderImpl
 */
class SampleProcessor
{

public:

	/**
	 * \brief Virtual default constructor.
	 */
	virtual ~SampleProcessor() noexcept;

	/**
	 * \brief Callback for sample sequences.
	 *
	 * \param[in] begin Begin of the sample sequence
	 * \param[in] end   End of the sample sequence
	 */
	void append_samples(SampleInputIterator begin, SampleInputIterator end);

	/**
	 * \brief Callback for AudioSize.
	 *
	 * \param[in] size AudioSize reported
	 */
	void update_audiosize(const AudioSize &size);

	/**
	 * \brief Callback for end of input
	 *
	 * \param[in] last_sample_index 0-based index of the last sample.
	 */
	void end_input(const uint32_t last_sample_index);

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
	 * \brief Implements SampleProcessor::append_samples(SampleInputIterator begin, SampleInputIterator end)
	 */
	virtual void do_append_samples(SampleInputIterator begin,
			SampleInputIterator end)
	= 0;

	/**
	 * \brief Implements SampleProcessor::update_audiosize(const AudioSize &size)
	 */
	virtual void do_update_audiosize(const AudioSize &size)
	= 0;

	/**
	 * \brief Implements SampleProcessor::end_input(const uint32_t last_sample_index)
	 */
	virtual void do_end_input(const uint32_t last_sample_index)
	= 0;

	/**
	 * \brief Sequence counter.
	 */
	int64_t total_sequences_ = 0;

	/**
	 * \brief PCM 32 Bit Sample counter.
	 */
	int64_t total_samples_ = 0;
};


/**
 * \brief Adapter to wrap a Calculation in an unbuffered SampleProcessor.
 */
class SampleProcessorAdapter : virtual public SampleProcessor
{

public:

	/**
	 * \brief Converting constructor for Calculation instances.
	 *
	 * \param[in] calculation The Calculation to use
	 */
	SampleProcessorAdapter(Calculation &calculation);

	SampleProcessorAdapter(const SampleProcessorAdapter &rhs) = delete;

	/**
	 * \brief Virtual default destructor.
	 */
	~SampleProcessorAdapter() noexcept override;

	SampleProcessorAdapter& operator = (const SampleProcessorAdapter &rhs)
		= delete;


private:

	void do_append_samples(SampleInputIterator begin, SampleInputIterator end)
		override;

	void do_update_audiosize(const AudioSize &size) override;

	void do_end_input(const uint32_t last_sample_index) override;

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
	 * \brief Register a function callback called as \c append_samples().
	 *
	 * \param[in] f
	 */
	virtual void register_appendsamples(
			std::function<void(SampleInputIterator, SampleInputIterator)> f)
	= 0;

	/**
	 * \brief Register a function callback called as \c update_audiosize().
	 *
	 * \param[in] f
	 */
	virtual void register_updatesize(std::function<void(const AudioSize &)> f)
	= 0;

	/**
	 * \brief Register a function callback called as \c end_input().
	 *
	 * \param[in] f The function to register
	 */
	virtual void register_endinput(std::function<void(const uint32_t)> f)
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
	virtual void register_processor(SampleProcessor &processor)
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
	 * \brief Call the registered \c append_samples() callback.
	 *
	 * \param[in] begin Iterator pointing to the begin of the sequence
	 * \param[in] end   Iterator pointing to the end of the sequence
	 */
	virtual void process_samples(
			SampleInputIterator begin, SampleInputIterator end)
	= 0;

	/**
	 * \brief Call the registered \c update_audiosize() callback.
	 *
	 * The actual method call is just passed to the registered SampleProcessor.
	 *
	 * \param[in] size AudioSize to report
	 */
	virtual void process_audiosize(const AudioSize &size)
	= 0;

	/**
	 * \brief Call the registered \c end_input() callback.
	 *
	 * \param[in] last_sample_index The 0-based index of the last sample.
	 */
	virtual void process_endinput(const uint32_t last_sample_index)
	= 0;
};


/**
 * \brief A provider of sample sequences.
 */
class SampleProvider : public ISampleProvider
{

public:

	/**
	 * \brief Constructor
	 */
	SampleProvider();

	SampleProvider(const SampleProvider &rhs) = delete;

	void register_appendsamples(
			std::function<void(SampleInputIterator, SampleInputIterator)> f)
		final;

	void register_updatesize(std::function<void(const AudioSize &size)>) final;

	void register_endinput(std::function<void(const uint32_t)> f) final;

	void register_processor(SampleProcessor &processor) final;

	const SampleProcessor* processor() const final;

	SampleProvider& operator = (const SampleProvider &rhs) = delete;


protected:

	/**
	 * \brief Default destructor.
	 */
	~SampleProvider() noexcept override;

	void process_samples(SampleInputIterator begin, SampleInputIterator end)
		override;

	void process_audiosize(const AudioSize &size) override;

	void process_endinput(const uint32_t last_sample_index) override;

	SampleProcessor* use_processor();


private:

	/**
	 * \brief Hook to be called before leaving register_processor().
	 */
	virtual void hook_post_register_processor();

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
	std::function<void(const uint32_t last_sample_index)> end_input_;

	/**
	 * \brief Internal pointer to the SampleProcessor.
	 */
	SampleProcessor* processor_;
};


/// @}

} // namespace v_1_0_0

} // namespace arcsdec

#endif

