/**
 * \file sampleproc.cpp Implements interface for processing samples
 */


#ifndef __LIBARCSDEC_SAMPLEPROC_HPP__
#include "sampleproc.hpp"
#endif



namespace arcsdec
{

inline namespace v_1_0_0
{

const BLOCKSIZE_t BLOCKSIZE;


// SampleProcessor


SampleProcessor::~SampleProcessor() noexcept = default;


void SampleProcessor::append_samples(PCMForwardIterator begin,
		PCMForwardIterator end)
{
	this->do_append_samples(begin, end);

	++total_sequences_;
	total_samples_ += std::distance(begin, end);
}


void SampleProcessor::update_audiosize(const AudioSize &size)
{
	this->do_update_audiosize(size);
}


void SampleProcessor::end_input(const uint32_t last_sample_index)
{
	this->do_end_input(last_sample_index);
}


int64_t SampleProcessor::sequences_processed() const
{
	return total_sequences_;
}


int64_t SampleProcessor::samples_processed() const
{
	return total_samples_;
}


// SampleProcessorAdapter


SampleProcessorAdapter::SampleProcessorAdapter(Calculation &calculation)
	: calculation_(&calculation)
{
	// empty
}


SampleProcessorAdapter::~SampleProcessorAdapter() noexcept = default;


void SampleProcessorAdapter::do_append_samples(
		PCMForwardIterator begin, PCMForwardIterator end)
{
	calculation_->update(begin, end);
}


void SampleProcessorAdapter::do_update_audiosize(const AudioSize &size)
{
	calculation_->update_audiosize(size);
}


void SampleProcessorAdapter::do_end_input(const uint32_t /* last_sample_index*/)
{
	// empty
}


// ISampleProvider


ISampleProvider::~ISampleProvider() noexcept = default;


// SampleProvider


SampleProvider::SampleProvider()
	: append_samples_()
	, update_audiosize_()
	, end_input_()
	, processor_()
{
	// empty
}


SampleProvider::~SampleProvider() noexcept = default;


void SampleProvider::register_appendsamples(
		std::function<void(PCMForwardIterator begin,
			PCMForwardIterator end)> func)
{
	this->append_samples_ = func;
}


void SampleProvider::register_updatesize(
		std::function<void(const AudioSize &size)> func)
{
	this->update_audiosize_ = func;
}


void SampleProvider::register_endinput(
		std::function<void(const uint32_t last_sample_index)> func)
{
	this->end_input_ = func;
}


void SampleProvider::process_samples(
		PCMForwardIterator begin, PCMForwardIterator end)
{
	this->append_samples_(begin, end);
}


void SampleProvider::process_audiosize(const AudioSize &size)
{
	this->update_audiosize_(size);
}


void SampleProvider::process_endinput(const uint32_t last_sample_index)
{
	this->end_input_(last_sample_index);
}


void SampleProvider::register_processor(SampleProcessor &processor)
{
	processor_ = &processor;

	this->register_appendsamples(std::bind(&SampleProcessor::append_samples,
			&processor,
			std::placeholders::_1, std::placeholders::_2));

	this->register_updatesize(std::bind(&SampleProcessor::update_audiosize,
			&processor,
			std::placeholders::_1));

	this->register_endinput(std::bind(&SampleProcessor::end_input,
			&processor,
			std::placeholders::_1));

	this->hook_post_register_processor();
}


const SampleProcessor* SampleProvider::processor() const
{
	return this->processor_;
}


SampleProcessor* SampleProvider::use_processor()
{
	return this->processor_;
}


void SampleProvider::hook_post_register_processor()
{
	// empty
}

} // namespace v_1_0_0

} // namespace arcsdec

