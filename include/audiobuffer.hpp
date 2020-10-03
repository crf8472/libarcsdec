#ifndef __LIBARCSDEC_AUDIOBUFFER_HPP__
#define __LIBARCSDEC_AUDIOBUFFER_HPP__

/**
 * \file
 *
 * \brief Toolkit for buffering audio samples
 */

#include <functional>
#include <vector>

#ifndef __LIBARCSDEC_SAMPLEPROC_HPP__
#include "sampleproc.hpp"
#endif

namespace arcsdec
{
inline namespace v_1_0_0
{

/**
 * \internal
 * \defgroup audiobuffer Basic audio sample buffer
 *
 * \brief Tool classes for buffering audio samples.
 *
 * Decouple the amount of bytes read at once and the amount of samples
 * processed in one step by buffering the audio samples in an intermediate
 * step.
 *
 * The basic buffering interface is BlockCreator. A BlockCreater can be
 * parametized with a number of sample that specifies the usual size of the
 * blocks it is expected to create. The last block, however, may be smaller
 * than this size.
 *
 * PCMBlockReader is build on this interface.
 *
 * \note Motivation: The readers based on libFLAC++ and ffmpeg do not seem to
 * provide any capability of specifying how many samples to read at once. So
 * each frame is passed to arcstk::Calculation in its own call of \c update().
 * This seems inflexible and may lead to huge logfiles. It may be a requirement
 * to control the amount of samples that used for a single \c update(). Most
 * readers can do this by just specifying the amount of samples or bytes to read
 * in one read operation. For the readers that do not offer this option,
 * buffering is a solution.
 *
 * \warning This interface is just a stub at the moment. The ffmpeg-based reader
 * could just use an AVAudioFifo but that would either be a solution for only
 * one reader and make the implementation dependent on the availability of
 * ffmpeg (3rd party dependency). The FLAC reader should not depend on anything
 * except libFLAC and libFLAC++.
 *
 * @{
 */


/**
 * \brief Base class of a sample block creator.
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
	 * \brief Constructs a BlockCreator with buffer of size BLOCKSIZE::DEFAULT
	 */
	BlockCreator();

	/**
	 * \brief Constructs a BlockCreator with buffer of size samples_per_block.
	 *
	 * \param[in] samples_per_block Number of 32 bit PCM samples in one block
	 */
	explicit BlockCreator(const int32_t samples_per_block);

	// make class non-copyable (1/2)
	BlockCreator(const BlockCreator &) = delete;

	/**
	 * \brief Virtual default destructor
	 */
	virtual ~BlockCreator() noexcept;

	/**
	 * \brief Set the maximal number of samples a block can contain.
	 *
	 * \param[in] samples_per_block
	 *     The number of 32 bit PCM samples in one block
	 */
	void set_samples_per_block(const int32_t samples_per_block);

	/**
	 * \brief Return the maximal number of samples a block can contain.
	 *
	 * \return The number of 32 bit PCM samples that the block can store
	 */
	int32_t samples_per_block() const;

	// make class non-copyable (2/2)
	BlockCreator& operator = (const BlockCreator &) = delete;

protected:

	/**
	 * \brief Returns the minimum block size of this instance
	 *
	 * \return Minimum number of samples per block
	 */
	virtual int32_t min_samples_per_block() const;

	/**
	 * \brief Returns the maximum block size of this instance
	 *
	 * \return Maximum number of samples per block
	 */
	virtual int32_t max_samples_per_block() const;

	/**
	 * \brief Clip the parameter to be between the values of
	 * min_samples_per_block() and max_samples_per_block().
	 *
	 * \param[in] samples_per_block Requested number of samples per block
	 *
	 * \return Valid number of samples per block
	 */
	int32_t clip_samples_per_block(const int32_t samples_per_block) const;

private:

	/**
	 * \brief Number of 32bit PCM samples per block
	 */
	int32_t samples_per_block_;
};

/// @}

} // namespace v_1_0_0

} // namespace arcsdec

#endif

