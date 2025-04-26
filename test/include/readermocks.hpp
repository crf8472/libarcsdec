#ifndef __LIBARCSDEC_READERMOCKS_HPP__
#define __LIBARCSDEC_READERMOCKS_HPP__

#ifndef __LIBARCSDEC_SAMPLEPROC_HPP__
#include "sampleproc.hpp"
#endif


/**
 * \brief Mock for a SampleProcessor.
 *
 * All functions are implemented empty.
 */
class Mock_SampleProcessor: public arcsdec::SampleProcessor
{
	void do_start_input() final
	{
		// empty
	}

	void do_append_samples(arcstk::SampleInputIterator /*begin*/,
			arcstk::SampleInputIterator /*end*/) final
	{
		// empty
	}

	void do_update_audiosize(const arcstk::AudioSize &/*size*/) final
	{
		// empty
	}

	void do_end_input() final
	{
		// empty
	}
};

#endif

