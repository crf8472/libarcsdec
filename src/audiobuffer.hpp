#ifndef __LIBARCSDEC_AUDIOBUFFER_HPP__
#define __LIBARCSDEC_AUDIOBUFFER_HPP__

/**
 * \file audiobuffer.hpp Toolkit for buffering audio samples
 */

#include <chrono>
#include <functional>
#include <fstream>
#include <memory>

#ifndef __LIBARCS_SAMPLES_HPP__
#include <arcs/samples.hpp>   // for SampleSequence
#endif
#ifndef __LIBARCSDEC_SAMPLEPROC_HPP__
#include "sampleproc.hpp"
#endif


// forward declarations
namespace arcs
{
inline namespace v_1_0_0
{

// from arcs/calculate.hpp
class AudioSize;
class Calculation;
class PCMForwardIterator;

} // namespace v_1_0_0
} // namespace arcs


namespace arcs
{

/**
 * \internal \defgroup audiobuffer Level 0 API: Basic Audio Sample buffering
 *
 * \brief Tool classes for buffering audio samples.
 *
 * Decouple the amount of bytes read at once and the amount of samples
 * processed in one step by buffering the audio samples.
 *
 * Note that the block size of an AudioReaderImpl set by
 * set_samples_per_block() does not define how many samples are to be read in a
 * single operation. Instead, this definition controls how many samples are to
 * be pushed to calculation at once. The block size is therefore not about
 * reading bytes at once but about how many bytes approximately are represented
 * in the memory at once. In fact, it may be more if the buffer is flushed in
 * the middle of a sequence and a fragment of the sequence is postponed to the
 * next buffer.
 *
 * In fact, the number of decoded samples per sequence is the value that
 * identifies how many bytes are read at once. This is not configurable in the
 * current version, but specific for each AudioReaderImpl and may or may not
 * depend on the current block size. Note that decoded samples is not to be
 * identified with "32 bit PCM samples" but with whatever the decoder returns.
 * Usually, these are signed integers of 32 bit representing a single 16 bit
 * sample that may have positive or negative values.
 *
 * The splitting of sequences can be entirely avoided for combinations of
 * sequence size and block size where the block size is a multiple of a
 * sequence size so the sequence are aligned to the blocks.
 *
 * The basic buffering interface is BlockCreator.
 *
 * The most facilitated subclass of BlockCreator is BlockAccumulator that
 * implements the accumulation of sequences of decoded samples into blocks of
 * predefined size. BlockAccumulator targets situations in which reading an
 * entire block of samples in one I/O operation is not intended.
 * BlockAccumulator buffers up sequences of samples until the defined buffer
 * size is reached.
 *
 * Classes SampleBuffer and PCMBlockReader are built on this API.
 *
 * @{
 */


/**
 * Base class of a block creating policy.
 *
 * Readers that pull their blocks from the filesystem can control the
 * size of the blocks directly. This does not restrict them to read the entire
 * block at once. On the other hand, readers that are pushed their
 * blocks to via some callback have to accumulate samples until a block is
 * complete.
 *
 * The BlockCreator is the abstraction of these different policies. It
 * encapsulates a sample buffer which a size can be adjusted to and a consumer
 * that the blocks are passed to once they are complete. Subclasses can control
 * what happens when a block is complete by overriding method block_complete().
 * Althoug BlockCreator is not abstract and could be instantiated, it is
 * intended as a mere blueprint for subclasses.
 */
class BlockCreator
{

public:

	/**
	 * Constructs a BlockCreator with buffer of size BLOCKSIZE::DEFAULT
	 */
	BlockCreator();

	/**
	 * Constructs a BlockCreator with buffer of size samples_per_block.
	 *
	 * \param[in] samples_per_block Number of 32 bit PCM samples in one block
	 */
	explicit BlockCreator(const uint32_t &samples_per_block);

	// make class non-copyable (1/2)
	BlockCreator(const BlockCreator &) = delete;

	// TODO Move constructor

	/**
	 * Virtual default destructor
	 */
	virtual ~BlockCreator() noexcept;

	/**
	 * Set the maximal number of samples a block can contain.
	 *
	 * \param[in] samples_per_block
	 *     The number of 32 bit PCM samples in one block
	 */
	void set_samples_per_block(const uint32_t &samples_per_block);

	/**
	 * Return the maximal number of samples a block can contain.
	 *
	 * \return The number of 32 bit PCM samples that the block can store
	 */
	uint32_t samples_per_block() const;

	// make class non-copyable (2/2)
	BlockCreator& operator = (const BlockCreator &) = delete;

	// TODO Move assignment


protected:

	/**
	 * Returns the minimum block size of this instance
	 *
	 * \return Minimum number of samples per block
	 */
	virtual uint32_t min_samples_per_block() const;

	/**
	 * Returns the maximum block size of this instance
	 *
	 * \return Maximum number of samples per block
	 */
	virtual uint32_t max_samples_per_block() const;

	/**
	 * Clip the parameter to be between the values of min_samples_per_block()
	 * and max_samples_per_block()
	 *
	 * \param[in] samples_per_block Requested number of samples per block
	 *
	 * \return Valid number of samples per block
	 */
	uint32_t clip_samples_per_block(const uint32_t &samples_per_block) const;


private:

	/**
	 * Number of 32bit PCM samples per block
	 */
	uint32_t samples_per_block_;
};


/**
 * Accumulates SampleSequences to the configured block size and passes the
 * block to a consumer as soon as it is filled completely.
 *
 * In come sequences of non-zero size smaller than the block, out go blocks of
 * the defined size.
 *
 * Not every decoder is compatible to pull-reading a specified block size.
 * If it is required or advantageous to read the samples in smaller sequences,
 * the sequences have to be accumulated to a block. To make a decoder
 * compatible with this strategy, define an appropriate subclass of
 * SampleSequence.
 */
class BlockAccumulator : public virtual BlockCreator
{

public:

	/**
	 * Default constructor.
	 *
	 * Constructs a BlockAccumulator with buffer of size BLOCKSIZE::DEFAULT
	 */
	BlockAccumulator();

	/**
	 * Constructs a BlockAccumulator with buffer of size
	 * samples_per_block.
	 *
	 * \param[in] samples_per_block Number of 32 bit PCM samples in one block
	 */
	explicit BlockAccumulator(const uint32_t &samples_per_block);

	// make class non-copyable (1/2)
	BlockAccumulator(const BlockAccumulator &) = delete;

	// TODO Move constructor

	/**
	 * Virtual default destructor
	 */
	~BlockAccumulator() noexcept override;

	/**
	 * Registers a consuming method for sample sequences.
	 *
	 * \param[in] func The functor to be registered as sample consumer.
	 */
	void register_block_consumer(const std::function<void(
			PCMForwardIterator begin, PCMForwardIterator end)> &func);

	/**
	 * Call this method before passing the first sample sequence
	 */
	void init();

	/**
	 * Appends a sample sequence to the buffer.
	 *
	 * It is guaranteed that the entire sequence is appended. However,
	 * if the sequence is longer than the remaining buffer capacity, the buffer
	 * is immediately flushed when full, so appending may cause flushing. Hence
	 * it is not guaranteed, that the entire sequence is part of the same block.
	 *
	 * \param[in] begin Begin of the sample sequence
	 * \param[in] end   End of the sample sequence
	 */
	void append_to_block(PCMForwardIterator begin, PCMForwardIterator end);

	/**
	 * Call this method after having passed the last sample sequence
	 */
	void flush();

	/**
	 * Returns the number of samples processed
	 *
	 * \return Number of samples processed since init() was called
	 */
	uint64_t samples_appended() const;

	// make class non-copyable (2/2)
	BlockAccumulator& operator = (const BlockAccumulator &) = delete;

	// TODO Move assignment


protected:

	/**
	 * Reinitialize internal buffer to specified size as number of 32 bit PCM
	 * samples.
	 *
	 * \param[in] total_samples Reinitialize buffer for new block
	 */
	void init_buffer(const uint32_t &total_samples);


private:

	/**
	 * Implementation of init()
	 */
	virtual void do_init();

	/**
	 * Implementation of flush()
	 */
	virtual void do_flush();

	/**
	 * Reinitialize internal buffer to configured block size.
	 */
	virtual void init_buffer();

	/**
	 * Registered callback method to consume a block.
	 *
	 * Called by block_complete().
	 */
	std::function<void(PCMForwardIterator begin, PCMForwardIterator end)>
		consume_;

	/**
	 * Internal sample buffer
	 */
	std::vector<uint32_t> samples_;

	/**
	 * Number of samples processed
	 */
	uint64_t samples_appended_;
};


/**
 * A format and reader independent sample buffer.
 *
 * Enhances BlockAccumulator to a SampleProcessor that also transports the
 * AudioSize update and is also a SampleProvider - which means it can have a
 * further SampleProcessor registered. Provides a convenience method for
 * registering a Calculation as addressee of all updates.
 */
class SampleBuffer  : public  virtual SampleProvider
					, public  virtual SampleProcessor
					, private virtual BlockAccumulator
{

public:

	/**
	 * Default constructor
	 */
	SampleBuffer();

	/**
	 * Constructs a SampleBuffer with buffer of size samples_per_block.
	 *
	 * \param[in] samples_per_block Number of 32 bit PCM samples in one block
	 */
	explicit SampleBuffer(const uint32_t samples_per_block);

	/**
	 * Default destructor
	 */
	virtual ~SampleBuffer() noexcept;

	/**
	 * Reset the buffer to its initial state, thereby discarding its content.
	 * The current buffer capacity is preserved.
	 */
	void reset();

	/**
	 * Flush the buffer.
	 */
	void flush();

	/**
	 * Handler method that informs about the number of total samples.
	 *
	 * Constructs an AudioSize instance of the passed sample amount and calls
	 * the registered update_audiosize() callback method. This is mere
	 * convenience for the caller not to be forced to construct the AudioSize
	 * object.
	 *
	 * \param[in] idx Index of the last 32 bit PCM sample
	 */
	//void notify_total_samples(const uint32_t idx);

	/**
	 * Register a Calculation to update by this instance.
	 *
	 * \param[in] calc The Calculation updated by this instance
	 */
	//void register_processor(Calculation &calc);


private:

	void do_append_samples(PCMForwardIterator begin, PCMForwardIterator end)
		override;

	void do_update_audiosize(const AudioSize &size) override;

	void do_end_input(const uint32_t last_sample_index) override;

	void hook_post_register_processor() override;
};

/// @}

} // namespace arcs

#endif

