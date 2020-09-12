/**
 * \file
 *
 * \brief Implements interface for processing samples
 */


#ifndef __LIBARCSDEC_SAMPLEPROC_HPP__
#include "sampleproc.hpp"
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
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
	ARCS_LOG(DEBUG1) << "START INPUT";

	this->do_start_input();
}


void SampleProcessor::append_samples(SampleInputIterator begin,
		SampleInputIterator end)
{
	ARCS_LOG(DEBUG1) << "APPEND SAMPLES";

	this->do_append_samples(begin, end);

	++total_sequences_;
	total_samples_ += std::distance(begin, end);
}


void SampleProcessor::update_audiosize(const AudioSize &size)
{
	ARCS_LOG(DEBUG1) << "UPDATE AUDIOSIZE";

	ARCS_LOG_INFO << "Update total number of samples to: "
		<< size.total_samples();

	this->do_update_audiosize(size);
}


void SampleProcessor::end_input()
{
	ARCS_LOG(DEBUG1) << "END INPUT";

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


// SampleProvider


SampleProvider::SampleProvider() = default;


SampleProvider::~SampleProvider() noexcept = default;


void SampleProvider::attach_processor(SampleProcessor &processor)
{
	this->do_attach_processor(processor);
}


const SampleProcessor* SampleProvider::processor() const
{
	return this->do_processor();
}


void SampleProvider::signal_startinput()
{
	this->do_signal_startinput();
}


void SampleProvider::signal_appendsamples(SampleInputIterator begin,
			SampleInputIterator end)
{
	this->do_signal_appendsamples(begin, end);
}


void SampleProvider::signal_updateaudiosize(const AudioSize &size)
{
	this->do_signal_updateaudiosize(size);
}


void SampleProvider::signal_endinput()
{
	this->do_signal_endinput();
}


// SampleProviderBase


SampleProviderBase::SampleProviderBase()
	: start_input_{}
	, append_samples_{}
	, update_audiosize_{}
	, end_input_{}
	, processor_{}
{
	// empty
}


void SampleProviderBase::register_startinput(std::function<void()> func)
{
	this->start_input_ = func;
}


void SampleProviderBase::register_appendsamples(
		std::function<void(SampleInputIterator begin,
			SampleInputIterator end)> func)
{
	this->append_samples_ = func;
}


void SampleProviderBase::register_updateaudiosize(
		std::function<void(const AudioSize &size)> func)
{
	this->update_audiosize_ = func;
}


void SampleProviderBase::register_endinput(std::function<void()> func)
{
	this->end_input_ = func;
}


void SampleProviderBase::do_signal_startinput()
{
	this->start_input_();
}


void SampleProviderBase::do_signal_appendsamples(
		SampleInputIterator begin, SampleInputIterator end)
{
	this->append_samples_(begin, end);
}


void SampleProviderBase::do_signal_updateaudiosize(const AudioSize &size)
{
	this->update_audiosize_(size);
}


void SampleProviderBase::do_signal_endinput()
{
	this->end_input_();
}


void SampleProviderBase::do_attach_processor(SampleProcessor &processor)
{
	this->attach_processor_impl(processor);
}


void SampleProviderBase::attach_processor_impl(SampleProcessor &processor)
{
	processor_ = &processor;

	this->register_startinput(
			std::bind(&SampleProcessor::start_input, &processor));

	this->register_appendsamples(
			std::bind(&SampleProcessor::append_samples, &processor,
				std::placeholders::_1, std::placeholders::_2));

	this->register_updateaudiosize(
			std::bind(&SampleProcessor::update_audiosize, &processor,
				std::placeholders::_1));

	this->register_endinput(
			std::bind(&SampleProcessor::end_input, &processor));
}


const SampleProcessor* SampleProviderBase::do_processor() const
{
	return this->processor_;
}


SampleProcessor* SampleProviderBase::use_processor()
{
	return this->processor_;
}

} // namespace v_1_0_0
} // namespace arcsdec

