#ifndef __LIBARCSDEC_AUDIOBUFFER_HPP__
#include "audiobuffer.hpp"
#endif

/**
 * \file
 *
 * \brief Implements toolkit classes for buffering audio samples
 */

#include <algorithm>
#include <cstdint>
#include <functional>
#include <limits>

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif


namespace arcsdec
{
inline namespace v_1_0_0
{


// BlockCreator


BlockCreator::BlockCreator()
	: samples_per_block_(BLOCKSIZE.DEFAULT)
{
	// empty
}


BlockCreator::BlockCreator(const int32_t samples_per_block)
	: samples_per_block_(samples_per_block)
{
	// empty
}


BlockCreator::~BlockCreator() noexcept = default;


void BlockCreator::set_samples_per_block(const int32_t samples_per_block)
{
	samples_per_block_ = samples_per_block;
}


int32_t BlockCreator::samples_per_block() const
{
	return samples_per_block_;
}


int32_t BlockCreator::min_samples_per_block() const
{
	return BLOCKSIZE.MIN;
}


int32_t BlockCreator::max_samples_per_block() const
{
	return std::numeric_limits<decltype(samples_per_block_)>::max();
}


int32_t BlockCreator::clip_samples_per_block(
		const int32_t samples_per_block) const
{
	return samples_per_block >= min_samples_per_block()
		? (samples_per_block <= max_samples_per_block()
			? samples_per_block
			: max_samples_per_block())
		: min_samples_per_block();
}

} // namespace v_1_0_0
} // namespace arcsdec

