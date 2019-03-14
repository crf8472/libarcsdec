/**
 * \file audiobuffer.hpp Toolkit for buffering audio samples
 *
 */


#ifndef __LIBARCSDEC_AUDIOBUFFER_HPP__
#define __LIBARCSDEC_AUDIOBUFFER_HPP__

#include <chrono>
#include <functional>
#include <fstream>
#include <memory>

#include <arcs/calculate.hpp> // for PCMForwardIterator
#include <arcs/samples.hpp>   // for SampleSequence


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
 * Classes SampleBuffer and PCMBlockReader in \ref audioreader are built on this
 * API.
 *
 * @{
 */


/**
 * Number of 32 bit samples for some block sizes
 */
enum SAMPLES : uint32_t
{
	FOR_256MB = 67108864,

	FOR_128MB = 33554432,

	FOR_64MB  = 16777216,

	FOR_32MB  = 8388608
};


/**
 * Symbolic constants for default and maximum block size
 */
enum BLOCKSIZE : uint32_t
{
	DEFAULT = SAMPLES::FOR_64MB,

	MAX     = SAMPLES::FOR_256MB
};


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
	 * Constructs a BlockCreator with buffer of size SAMPLES.PER_BLOCK_DEFAULT.
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
class BlockAccumulator : public BlockCreator
{

public:

	/**
	 * Default constructor.
	 *
	 * Constructs a BlockAccumulator with buffer of size
	 * SAMPLES.PER_BLOCK_DEFAULT.
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
	 * Call this method before passing the first sample sequence
	 */
	void init();

	/**
	 * Call this method after having passed the last sample sequence
	 */
	void flush();

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
	void append(PCMForwardIterator begin, PCMForwardIterator end);

	/**
	 * Registers a consuming method for sample sequences.
	 *
	 * \param[in] func The functor to be registered as sample consumer.
	 */
	void register_block_consumer(const std::function<void(
			PCMForwardIterator begin, PCMForwardIterator end)> &func);

	/**
	 * Returns the number of bytes processed
	 *
	 * \return Number of bytes processed since init() was called
	 */
	uint64_t bytes_processed() const;

	/**
	 * Returns the number of samples processed
	 *
	 * \return Number of samples processed since init() was called
	 */
	uint64_t samples_processed() const;

	/**
	 * Returns the number of sequences processed
	 *
	 * \return Number of sequences processed since init() was called
	 */
	uint64_t sequences_processed() const;

	/**
	 * Returns the number of blocks processed
	 *
	 * \return Number of blocks processed since init() was called
	 */
	uint64_t blocks_processed() const;

	// make class non-copyable (2/2)
	BlockAccumulator& operator = (const BlockAccumulator &) = delete;

	// TODO Move assignment


protected:

	/**
	 * Virtual implementation of init()
	 */
	virtual void do_init();

	/**
	 * Virtual implementation of flush()
	 */
	virtual void do_flush();

	/**
	 * Virtual implementation of append()
	 *
	 * \param[in] begin Begin of the sample sequence
	 * \param[in] end   End of the sample sequence
	 */
	virtual void do_append(PCMForwardIterator begin, PCMForwardIterator end);

	/**
	 * Reinitialize internal buffer to configured block size.
	 */
	virtual void init_buffer();

	/**
	 * Reinitialize internal buffer to specified size as number of 32 bit PCM
	 * samples.
	 *
	 * \param[in] total_samples Reinitialize buffer for new block
	 */
	void init_buffer(const uint32_t &total_samples);


private:

	/**
	 * Internal sample buffer
	 */
	std::vector<uint32_t> samples_;

	/**
	 * Registered callback method to consume a block.
	 *
	 * Called by block_complete().
	 */
	std::function<void(PCMForwardIterator begin, PCMForwardIterator end)>
		consume_;

	/**
	 * Number of samples processed
	 */
	uint64_t samples_processed_;

	/**
	 * Number of frames processed
	 */
	uint64_t sequences_processed_;

	/**
	 * Number of blocks processed
	 */
	uint64_t blocks_processed_;
};

/// @}

} // namespace arcs

#endif

