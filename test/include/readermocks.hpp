#ifndef LIBARCSDEC_READERMOCKS_HPP_
#define LIBARCSDEC_READERMOCKS_HPP_

#ifndef LIBARCSDEC_SAMPLEPROC_HPP_
#include "sampleproc.hpp"
#endif


/**
 * \brief Mock for a SampleProcessor.
 *
 * All functions are implemented empty.
 */
class Mock_SampleProcessor final : public arcsdec::read::AudioEventHandler
{
	void do_start_input() final
	{
		// empty
	}

	void do_audiosize(const arcstk::AudioSize &/*size*/) final
	{
		// empty
	}

	void do_end_input() final
	{
		// empty
	}
};

#endif

