/**
 * \file audiobuffer.cpp Implements toolkit classes for buffering audio samples
 *
 */


#ifndef __LIBARCSDEC_AUDIOBUFFER_HPP__
#include "audiobuffer.hpp"
#endif

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#ifndef __LIBARCS_IDENTIFIER_HPP__
#include <arcs/identifier.hpp> // for CDDA
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
	// Inspired by FLAC:
	// Biggest value for the number of PCM samples in a FLAC frame. This
	// entails that at least one FLAC frame is guaranteed to fit in a block of
	// minimal size.

	return 65536;
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
	, samples_processed_(0)
	, sequences_processed_(0)
	, blocks_processed_(0)
{
	// empty
}


BlockAccumulator::BlockAccumulator(const uint32_t &samples_per_block)
	: BlockCreator(samples_per_block)
	, consume_()
	, samples_(samples_per_block)
	, samples_processed_(0)
	, sequences_processed_(0)
	, blocks_processed_(0)
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


void BlockAccumulator::append(PCMForwardIterator begin, PCMForwardIterator end)
{
	this->do_append(begin, end);
}


void BlockAccumulator::register_block_consumer(const std::function<void(
			PCMForwardIterator begin, PCMForwardIterator end
		)> &consume)
{
	consume_ = consume;
}


uint64_t BlockAccumulator::bytes_processed() const
{
	return samples_processed_ * sizeof(uint32_t);

	// Type uint32_t represents a single sample.
	// Using CDDA.BYTES_PER_SAMPLE would ignore the actual data type.
}


uint64_t BlockAccumulator::samples_processed() const
{
	return samples_processed_;
}


uint64_t BlockAccumulator::sequences_processed() const
{
	return sequences_processed_;
}


uint64_t BlockAccumulator::blocks_processed() const
{
	return blocks_processed_;
}


void BlockAccumulator::do_init()
{
	samples_processed_   = 0;
	sequences_processed_ = 0;
	blocks_processed_    = 0;

	this->init_buffer();
}


void BlockAccumulator::do_flush()
{
	// Statistics

	++blocks_processed_;

	// Logging

	ARCS_LOG_DEBUG << "READ BLOCK " << blocks_processed_;

	if (samples_.size() > 0) // samples_.front() must be accessible
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


void BlockAccumulator::do_append(PCMForwardIterator begin,
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
					(samples_processed_ % this->samples_per_block());

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

		samples_processed_ += samples_buffered; // statistics

		// Buffer full? => Flush it and put remaining samples in the sequence
		// in a new buffer. Do while there remain samples to process.

		if (buffer_avail == 0)
		{
			this->flush();
		}
	}

	++sequences_processed_;
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
	, call_update_audiosize_()
{
	this->init();
}


SampleBuffer::SampleBuffer(const uint32_t samples_per_block)
	: BlockAccumulator(samples_per_block)
	, call_update_audiosize_()
{
	this->init();
}


SampleBuffer::~SampleBuffer() noexcept = default;


void SampleBuffer::reset()
{
	this->init();
}


void SampleBuffer::notify_total_samples(const uint32_t sample_count)
{
	ARCS_LOG_DEBUG << "Total samples updated to: " << sample_count;

	AudioSize size;
	size.set_sample_count(sample_count);
	call_update_audiosize_(size); // call registered method
}


void SampleBuffer::register_processor(Calculation &calc)
{
	// Set Calculation instance as consumer for blocks
	this->register_block_consumer(
		std::bind(&Calculation::update, &calc,
			std::placeholders::_1, std::placeholders::_2));

	// Inform Calculation instance when total number of samples is set/updated
	call_update_audiosize_ =
		std::bind(&Calculation::update_audiosize, &calc, std::placeholders::_1);
}


} // namespace arcs

