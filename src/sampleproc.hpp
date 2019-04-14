#ifndef __LIBARCSDEC_SAMPLEPROC_HPP__
#define __LIBARCSDEC_SAMPLEPROC_HPP__

/**
 * \file sampleproc.hpp Interface for processing samples
 */

#include <functional>

#ifndef __LIBARCS_CALCULATE_HPP__
#include <arcs/calculate.hpp>
#endif


namespace arcs
{

/**
 * \internal \defgroup sampleproc Interface for processing samples
 *
 * \brief Interface for processing samples
 *
 * @{
 */


/**
 * Interface for processing samples provided by an AudioReaderImpl
 */
class SampleProcessor
{

public:

	/**
	 * Virtual default constructor
	 */
	virtual ~SampleProcessor() noexcept;

	/**
	 * \brief Callback for sample sequences.
	 *
	 * \param[in] begin Begin of the sample sequence
	 * \param[in] end   End of the sample sequence
	 */
	void append_samples(PCMForwardIterator begin, PCMForwardIterator end);

	/**
	 * \brief Callback for AudioSize.
	 *
	 * \param[in] size AudioSize reported
	 */
	void update_audiosize(const AudioSize &size);

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
	 * Implements SampleProcessor::samples_callback(PCMForwardIterator begin, PCMForwardIterator end)
	 */
	virtual void do_append_samples(PCMForwardIterator begin,
			PCMForwardIterator end)
	= 0;

	/**
	 * Implements SampleProcessor::audiosize_callback(const AudioSize &size)
	 */
	virtual void do_update_audiosize(const AudioSize &size)
	= 0;

	/**
	 * Sequence counter
	 */
	int64_t total_sequences_ = 0;

	/**
	 * PCM 32 Bit Sample counter
	 */
	int64_t total_samples_ = 0;
};


/**
 * Unbuffered wrapper for a Calculation.
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
	 * Virtual default destructor
	 */
	~SampleProcessorAdapter() noexcept override;

	SampleProcessorAdapter& operator = (const SampleProcessorAdapter &rhs)
		= delete;


private:

	/**
	 * Implements SampleProcessor::append_samples
	 */
	void do_append_samples(PCMForwardIterator begin, PCMForwardIterator end)
		override;

	/**
	 * Implements SampleProcessor::update_audiosize
	 */
	void do_update_audiosize(const AudioSize &size) override;

	/**
	 * Internal pointer to the calculation to wrap
	 */
	Calculation *calculation_;
};


/**
 * \brief Inteface for providers of sample sequences.
 */
class ISampleProvider
{

public:

	virtual ~ISampleProvider() noexcept;

	/**
	 *
	 * \param[in] processor The SampleProcessor to use
	 */
	virtual void register_processor(SampleProcessor &processor)
	= 0;

	virtual void register_appendsamples(
			std::function<void(PCMForwardIterator, PCMForwardIterator)> f)
	= 0;

	virtual void register_updatesize(std::function<void(const AudioSize &)> f)
	= 0;

	/**
	 *
	 * \return The SampleProcessor the reader uses
	 */
	virtual const SampleProcessor& processor() const
	= 0;


protected:

	/**
	 * Call append_samples on the registered SampleProcessor.
	 *
	 * The actual method call is just passed to the registered SampleProcessor.
	 *
	 * \param[in] begin Iterator pointing to the begin of the sequence
	 * \param[in] end   Iterator pointing to the end of the sequence
	 */
	virtual void process_samples(
			PCMForwardIterator begin, PCMForwardIterator end)
	= 0;


	/**
	 * Call update_audiosize on the registered SampleProcessor.
	 *
	 * The actual method call is just passed to the registered SampleProcessor.
	 *
	 * \param[in] size AudioSize to report
	 */
	virtual void process_audiosize(const AudioSize &size)
	= 0;
};


/**
 * \brief A provider of sample sequences.
 */
class SampleProvider : public ISampleProvider
{

public:

	/**
	 * Constructor
	 */
	SampleProvider();

	SampleProvider(const SampleProvider &rhs) = delete;

	/**
	 *
	 * \param[in] processor The SampleProcessor to use
	 */
	void register_processor(SampleProcessor &processor) override;

	void register_appendsamples(
			std::function<void(PCMForwardIterator, PCMForwardIterator)> f)
		final;

	void register_updatesize(std::function<void(const AudioSize &size)>) final;

	/**
	 *
	 * \return The SampleProcessor the reader uses
	 */
	const SampleProcessor& processor() const final;

	SampleProvider& operator = (const SampleProvider &rhs) = delete;


protected:

	/**
	 * Default destructor.
	 */
	~SampleProvider() noexcept;

	void process_samples(PCMForwardIterator begin, PCMForwardIterator end)
		override;

	void process_audiosize(const AudioSize &size) override;


private:

	/**
	 * Callback pointer for appending samples sequences to processing
	 */
	std::function<void(PCMForwardIterator begin, PCMForwardIterator end)>
		append_samples_;

	/**
	 * Callback pointer for updateing the AudioSize
	 */
	std::function<void(const AudioSize &size)> update_audiosize_;

	/**
	 * Internal pointer to the SampleProcessor.
	 */
	SampleProcessor* processor_;
};


/// @}

} // namespace arcs

#endif

