#ifndef __LIBARCSDEC_AUDIOBUFFER_HPP__
#include "audiobuffer.hpp"
#endif

/**
 * \file audiobuffer.cpp Implements toolkit classes for buffering audio samples
 */

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#ifndef __LIBARCS_CALCULATE_HPP__
#include <arcs/calculate.hpp> // for AudioSize, Calculation, PCMForwardIterator
#endif
#ifndef __LIBARCS_LOGGING_HPP__
#include <arcs/logging.hpp>
#endif


namespace arcs
{


// BlockCreator


BlockCreator::BlockCreator()
	: samples_per_block_(BLOCKSIZE::DEFAULT)
{
	// empty
}


BlockCreator::BlockCreator(const uint32_t &samples_per_block)
	: samples_per_block_(samples_per_block)
{
	// empty
}


BlockCreator::~BlockCreator() noexcept = default;


void BlockCreator::set_samples_per_block(const uint32_t &samples_per_block)
{
	samples_per_block_ = samples_per_block;
}


uint32_t BlockCreator::samples_per_block() const
{
	return samples_per_block_;
}


uint32_t BlockCreator::min_samples_per_block() const
{
	return BLOCKSIZE::MIN;
}


uint32_t BlockCreator::max_samples_per_block() const
{
	return 0xFFFFFFFF; // maximal value for uint32_t
}


uint32_t BlockCreator::clip_samples_per_block(
		const uint32_t &samples_per_block) const
{
	return samples_per_block >= min_samples_per_block()
		? (samples_per_block <= max_samples_per_block()
			? samples_per_block
			: max_samples_per_block())
		: min_samples_per_block();
}


// BlockAccumulator


BlockAccumulator::BlockAccumulator()
	: consume_()
	, samples_(BLOCKSIZE::DEFAULT)
	, samples_appended_(0)
{
	// empty
}


BlockAccumulator::BlockAccumulator(const uint32_t &samples_per_block)
	: BlockCreator(samples_per_block)
	, consume_()
	, samples_(samples_per_block)
	, samples_appended_(0)
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
			PCMForwardIterator begin, PCMForwardIterator end
		)> &consume)
{
	consume_ = consume;
}


uint64_t BlockAccumulator::samples_appended() const
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
		ARCS_LOG(LOG_DEBUG1) << "  Size: "
			<< (samples_.size() * sizeof(samples_.front())) << " bytes / "
			<< samples_.size() << " Stereo PCM samples (32 bit)";
	} else
	{
		ARCS_LOG(LOG_DEBUG1) << "  Size: 0";
	}

	// Mark the block as processed

	consume_(samples_.begin(), samples_.end());
}


void BlockAccumulator::append_to_block(PCMForwardIterator begin,
		PCMForwardIterator end)
{
	uint32_t seq_size         = std::distance(begin, end);
	// TODO Verify size within legal bounds
	uint32_t buffer_avail     = 0;
	uint32_t samples_todo     = seq_size;
	uint32_t samples_buffered = 0;

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


void BlockAccumulator::init_buffer(const uint32_t &buffer_size)
{
	ARCS_LOG(LOG_DEBUG1) << "Init buffer for " << buffer_size << " samples";

	// Remove trailing zeros
	samples_.resize(buffer_size);
}


// SampleBuffer


SampleBuffer::SampleBuffer()
	: BlockAccumulator(BLOCKSIZE::DEFAULT)
{
	this->init();
}


SampleBuffer::SampleBuffer(const uint32_t samples_per_block)
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


//void SampleBuffer::notify_total_samples(const uint32_t sample_count)
//{
//	ARCS_LOG_DEBUG << "Total samples updated to: " << sample_count;
//
//	AudioSize size;
//	size.set_sample_count(sample_count);
//
//	this->process_audiosize(size);
//}


//void SampleBuffer::register_processor(Calculation &calc)
//{
//	this->register_appendsamples(
//			std::bind(&Calculation::update, &calc,
//				std::placeholders::_1, std::placeholders::_2));
//
//	this->register_updatesize(
//			std::bind(&Calculation::update_audiosize, &calc,
//				std::placeholders::_1));
//
//	// Attach Calculation to the inherited BlockAccumulator
//	this->register_block_consumer(
//		std::bind(&Calculation::update, &calc,
//			std::placeholders::_1, std::placeholders::_2));
//}


void SampleBuffer::do_append_samples(PCMForwardIterator begin,
		PCMForwardIterator end)
{
	this->append_to_block(begin, end);
	// append_to_block calls process_samples() once the buffer is full
}


void SampleBuffer::do_update_audiosize(const AudioSize &size)
{
	// do nothing, just pass on to registered processor
	this->process_audiosize(size);
}


void SampleBuffer::do_end_input(const uint32_t last_sample_index)
{
	this->flush();

	// pass on to registered processor
	this->process_endinput(last_sample_index);
}


void SampleBuffer::hook_post_register_processor()
{
	// Attach SampleProcessor to the inherited BlockAccumulator
	this->register_block_consumer(
		std::bind(&SampleProcessor::append_samples, this->use_processor(),
			std::placeholders::_1, std::placeholders::_2));
}


} // namespace arcs

