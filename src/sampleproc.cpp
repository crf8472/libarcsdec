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

using arcstk::SampleInputIterator;


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

	// TODO Move this to calc processor, not every processor needs this
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
	: processor_{}
{
	// empty
}


void SampleProviderBase::do_signal_startinput()
{
	use_processor()->start_input();
}


void SampleProviderBase::do_signal_appendsamples(
		SampleInputIterator begin, SampleInputIterator end)
{
	use_processor()->append_samples(begin, end);
}


void SampleProviderBase::do_signal_updateaudiosize(const AudioSize &size)
{
	use_processor()->update_audiosize(size);
}


void SampleProviderBase::do_signal_endinput()
{
	use_processor()->end_input();
}


void SampleProviderBase::do_attach_processor(SampleProcessor &processor)
{
	this->attach_processor_impl(processor);
}


void SampleProviderBase::attach_processor_impl(SampleProcessor &processor)
{
	processor_ = &processor;
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

