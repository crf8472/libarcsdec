#ifndef __LIBARCSDEC_AUDIOBUFFER_HPP__
#include "audiobuffer.hpp"
#endif

/**
 * \file
 *
 * \brief Implements toolkit classes for buffering audio samples
 */

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp> // for AudioSize, Calculation, SampleInputIterator
#endif
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


// BlockAccumulator


BlockAccumulator::BlockAccumulator()
	: consume_ { }
	, samples_(static_cast<decltype(samples_)::size_type>(BLOCKSIZE.DEFAULT))
	, samples_appended_ { 0 }
{
	// empty
}


BlockAccumulator::BlockAccumulator(const int32_t samples_per_block)
	: BlockCreator(samples_per_block)
	, consume_ { }
	, samples_(static_cast<decltype(samples_)::size_type>(samples_per_block))
	, samples_appended_ { 0 }
{
	// empty
}


BlockAccumulator::~BlockAccumulator() noexcept = default;


void BlockAccumulator::init()
{
	this->do_init();
}


void BlockAccumulator::flush()
{
	this->do_flush();
}


void BlockAccumulator::register_block_consumer(const std::function<void(
			SampleInputIterator begin, SampleInputIterator end
		)> &consume)
{
	consume_ = consume;
}


int32_t BlockAccumulator::samples_appended() const
{
	return samples_appended_;
}


void BlockAccumulator::do_init()
{
	samples_appended_   = 0;

	this->init_buffer();
}


void BlockAccumulator::do_flush()
{
	ARCS_LOG_DEBUG << "BLOCK COMPLETED";

	if (samples_.size() > 0)
	{
		ARCS_LOG(DEBUG1) << "  Size: "
			<< (samples_.size() * sizeof(samples_.front())) << " bytes / "
			<< samples_.size() << " Stereo PCM samples (32 bit)";
	} else
	{
		ARCS_LOG(DEBUG1) << "  Size: 0";
	}

	// Mark the block as processed

	consume_(samples_.begin(), samples_.end());
}


void BlockAccumulator::append_to_block(SampleInputIterator begin,
		SampleInputIterator end)
{
	int32_t seq_size         = std::distance(begin, end);
	// TODO Verify size within legal bounds
	int32_t buffer_avail     = 0;
	int32_t samples_todo     = seq_size;
	int32_t samples_buffered = 0;

	auto smpl { samples_.begin() };
	auto last { samples_.end()   };

	while (samples_todo)
	{
		// Some free space available in current block? Get the number of
		// available free positions.

		buffer_avail = this->samples_per_block() -
					(this->samples_appended() % this->samples_per_block());

		// Set pointers to start and end of the buffer slice to be filled
		// with samples from the sequence.

		// smpl: First position of the remaining buffer capacity
		smpl = samples_.end() - buffer_avail;

		// Take whatever is smaller: the remaining buffer capacity or the
		// amount of unprocessed samples in the sequence.

		last = smpl + std::min(buffer_avail, samples_todo);

		// Fill the remaining buffer capacity with samples from the current
		// sequence. If the sequence is smaller than the available capacity, the
		// while loop will stop since there are no more samples left to process.

		samples_buffered = 0;
		auto input { begin + (seq_size - samples_todo) };
		for (; smpl != last; ++smpl, ++input, ++samples_buffered)
		{
			*smpl = *input;
		}
		samples_todo -= samples_buffered;
		buffer_avail -= samples_buffered;

		samples_appended_ += samples_buffered; // statistics

		// Buffer full? => Flush it and put remaining samples in the sequence
		// in a new buffer. Do while there remain samples to process.

		if (buffer_avail == 0)
		{
			this->flush();
		}
	}
}


void BlockAccumulator::init_buffer()
{
	this->init_buffer(this->samples_per_block());
}


void BlockAccumulator::init_buffer(const int32_t buffer_size)
{
	ARCS_LOG(DEBUG1) << "Init buffer for " << buffer_size << " samples";

	// Remove trailing zeros
	samples_.resize(static_cast<decltype(samples_)::size_type>(buffer_size));
}


// SampleBuffer


SampleBuffer::SampleBuffer()
	: BlockAccumulator(BLOCKSIZE.DEFAULT)
{
	this->init();
}


SampleBuffer::SampleBuffer(const int32_t samples_per_block)
	: BlockAccumulator(samples_per_block)
{
	this->init();
}


SampleBuffer::~SampleBuffer() noexcept = default;


void SampleBuffer::reset()
{
	this->init();
}


void SampleBuffer::flush()
{
	BlockAccumulator::flush();
}


void SampleBuffer::do_start_input()
{
	this->call_startinput();
}


void SampleBuffer::do_append_samples(SampleInputIterator begin,
		SampleInputIterator end)
{
	this->append_to_block(begin, end);
	// append_to_block does call_appendsamples() when flushing the buffer
}


void SampleBuffer::do_update_audiosize(const AudioSize &size)
{
	// do nothing, just pass on to registered processor
	this->call_updateaudiosize(size);
}


void SampleBuffer::do_end_input()
{
	this->flush();

	// pass on to registered processor
	this->call_endinput();
}


void SampleBuffer::hook_post_attachprocessor()
{
	// Attach SampleProcessor to the inherited BlockAccumulator
	this->register_block_consumer(
		std::bind(&SampleProcessor::append_samples, this->use_processor(),
			std::placeholders::_1, std::placeholders::_2));
}

} // namespace v_1_0_0

} // namespace arcsdec

