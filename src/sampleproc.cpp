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


SampleProcessor::SampleProcessor()
	: total_sequences_ { 0 }
	, total_samples_   { 0 }
{
	// empty
}


SampleProcessor::~SampleProcessor() noexcept = default;


void SampleProcessor::start_input()
{
	this->do_start_input();
}


void SampleProcessor::append_samples(SampleInputIterator begin,
		SampleInputIterator end)
{
	this->do_append_samples(begin, end);

	++total_sequences_;
	total_samples_ += std::distance(begin, end);
}


void SampleProcessor::update_audiosize(const AudioSize &size)
{
	this->do_update_audiosize(size);
}


void SampleProcessor::end_input()
{
	this->do_end_input();
}


int64_t SampleProcessor::sequences_processed() const
{
	return total_sequences_;
}


int64_t SampleProcessor::samples_processed() const
{
	return total_samples_;
}


// CalculationProcessor


CalculationProcessor::CalculationProcessor(Calculation &calculation)
	: calculation_(&calculation)
{
	// empty
}


CalculationProcessor::~CalculationProcessor() noexcept = default;


void CalculationProcessor::do_start_input()
{
	// empty
}


void CalculationProcessor::do_append_samples(
		SampleInputIterator begin, SampleInputIterator end)
{
	calculation_->update(begin, end);
}


void CalculationProcessor::do_update_audiosize(const AudioSize &size)
{
	calculation_->update_audiosize(size);
}


void CalculationProcessor::do_end_input()
{
	// empty
}


// ISampleProvider


ISampleProvider::~ISampleProvider() noexcept = default;


// SampleProvider


SampleProvider::SampleProvider()
	: start_input_()
	, append_samples_()
	, update_audiosize_()
	, end_input_()
	, processor_()
{
	// empty
}


SampleProvider::~SampleProvider() noexcept = default;


void SampleProvider::register_startinput(std::function<void()> func)
{
	this->start_input_ = func;
}


void SampleProvider::register_appendsamples(
		std::function<void(SampleInputIterator begin,
			SampleInputIterator end)> func)
{
	this->append_samples_ = func;
}


void SampleProvider::register_updateaudiosize(
		std::function<void(const AudioSize &size)> func)
{
	this->update_audiosize_ = func;
}


void SampleProvider::register_endinput(std::function<void()> func)
{
	this->end_input_ = func;
}


void SampleProvider::call_startinput()
{
	this->start_input_();
}


void SampleProvider::call_appendsamples(
		SampleInputIterator begin, SampleInputIterator end)
{
	this->append_samples_(begin, end);
}


void SampleProvider::call_updateaudiosize(const AudioSize &size)
{
	this->update_audiosize_(size);
}


void SampleProvider::call_endinput()
{
	this->end_input_();
}


void SampleProvider::attach_processor(SampleProcessor &processor)
{
	processor_ = &processor;

	this->register_startinput(std::bind(&SampleProcessor::start_input,
			&processor));

	this->register_appendsamples(std::bind(&SampleProcessor::append_samples,
			&processor,
			std::placeholders::_1, std::placeholders::_2));

	this->register_updateaudiosize(std::bind(&SampleProcessor::update_audiosize,
			&processor,
			std::placeholders::_1));

	this->register_endinput(std::bind(&SampleProcessor::end_input,
			&processor));

	this->hook_post_attachprocessor();
}


const SampleProcessor* SampleProvider::processor() const
{
	return this->processor_;
}


SampleProcessor* SampleProvider::use_processor()
{
	return this->processor_;
}


void SampleProvider::hook_post_attachprocessor()
{
	// empty
}

} // namespace v_1_0_0

} // namespace arcsdec

