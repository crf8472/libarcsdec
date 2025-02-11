/**
 * \file
 *
 * \brief Implements interface for processing samples
 */


#ifndef __LIBARCSDEC_SAMPLEPROC_HPP__
#include "sampleproc.hpp"
#endif


namespace arcsdec
{
inline namespace v_1_0_0
{

using arcstk::SampleInputIterator;


// SampleProcessor


SampleProcessor::SampleProcessor()
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
}


void SampleProcessor::update_audiosize(const AudioSize& size)
{
	this->do_update_audiosize(size);
}


void SampleProcessor::end_input()
{
	this->do_end_input();
}


// SampleProvider


SampleProvider::SampleProvider() = default;


SampleProvider::~SampleProvider() noexcept = default;


void SampleProvider::attach_processor(SampleProcessor& processor)
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


void SampleProvider::signal_updateaudiosize(const AudioSize& size)
{
	this->do_signal_updateaudiosize(size);
}


void SampleProvider::signal_endinput()
{
	this->do_signal_endinput();
}

} // namespace v_1_0_0
} // namespace arcsdec

