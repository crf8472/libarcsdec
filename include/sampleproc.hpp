#ifndef __LIBARCSDEC_SAMPLEPROC_HPP__
#define __LIBARCSDEC_SAMPLEPROC_HPP__

/**
 * \file
 *
 * \brief Interface for processing samples
 */

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>
#endif

namespace arcsdec
{
inline namespace v_1_0_0
{

/**
 * \defgroup sampleproc Process samples
 *
 * \brief API for AudioReaders to process samples.
 *
 * @{
 */

using arcstk::SampleInputIterator;
using arcstk::AudioSize;


/**
 * \brief Symbolic constants for certain block sizes (in PCM 32 bit samples).
 */
struct BLOCKSIZE final
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
	void update_audiosize(const AudioSize& size);

	/**
	 * \brief Callback for end of input
	 */
	void end_input();

protected:

	SampleProcessor(const SampleProcessor&) = default;
	SampleProcessor& operator = (const SampleProcessor&) = default;

	SampleProcessor(SampleProcessor&&) noexcept = default;
	SampleProcessor& operator = (SampleProcessor&&) noexcept = default;

private:

	/**
	 * \brief Implements \ref start_input().
	 */
	virtual void do_start_input()
	= 0;

	/**
	 * \brief Implements \ref append_samples().
	 */
	virtual void do_append_samples(SampleInputIterator begin,
			SampleInputIterator end)
	= 0;

	/**
	 * \brief Implements \ref update_audiosize().
	 */
	virtual void do_update_audiosize(const AudioSize& size)
	= 0;

	/**
	 * \brief Implements \ref end_input().
	 */
	virtual void do_end_input()
	= 0;
};


/**
 * \brief Inteface for providers of sample sequences.
 *
 * A SampleProvider is a source for sample sequences and updated AudioSize
 * values. It can signal different events while processing the audio input.
 * A SampleProcessor can be attached to it as an addressee of those events.
 *
 * \see AudioReaderImpl
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
	void signal_updateaudiosize(const AudioSize& size);

	/**
	 * \brief Signal the processor that input ends.
	 */
	void signal_endinput();

	/**
	 * \brief Attach a SampleProcessor.
	 *
	 * \param[in] processor The SampleProcessor to use
	 */
	void attach_processor(SampleProcessor& processor);

	/**
	 * \brief Return the registered SampleProcessor.
	 *
	 * \return The SampleProcessor the reader uses
	 */
	const SampleProcessor* processor() const;

protected:

	SampleProvider(const SampleProvider&) = default;
	SampleProvider& operator = (const SampleProvider&) = default;

	SampleProvider(SampleProvider&&) noexcept = default;
	SampleProvider& operator = (SampleProvider&&) noexcept = default;

private:

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
	virtual void do_signal_updateaudiosize(const AudioSize& size)
	= 0;

	/**
	 * \brief Implements signal_endinput().
	 */
	virtual void do_signal_endinput()
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
	virtual void do_attach_processor(SampleProcessor& processor)
	= 0;

	/**
	 * \brief Return the registered SampleProcessor.
	 *
	 * \return The SampleProcessor the reader uses
	 */
	virtual const SampleProcessor* do_processor() const
	= 0;
};

/// @}

} // namespace v_1_0_0
} // namespace arcsdec

#endif

