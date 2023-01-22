#ifndef __LIBARCSDEC_READERMOCKS_HPP__
#define __LIBARCSDEC_READERMOCKS_HPP__

#ifndef __LIBARCSDEC_SAMPLEPROC_HPP__
#include "sampleproc.hpp"
#endif


class SampleProcessorMock : public arcsdec::SampleProcessor
{
	void do_start_input() final
	{
	}

	void do_append_samples(arcstk::SampleInputIterator /*begin*/,
			arcstk::SampleInputIterator /*end*/) final
	{
	}

	void do_update_audiosize(const arcstk::AudioSize &/*size*/) final
	{
	}

	void do_end_input() final
	{
	}
};

#endif

