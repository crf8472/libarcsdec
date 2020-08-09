#include "catch2/catch.hpp"


#ifndef __LIBARCSDEC_AUDIOBUFFER_HPP__
#include "audiobuffer.hpp"
#endif

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp> // for CDDA
#endif
#ifndef __LIBARCSTK_calculate_HPP__
#include <arcstk/calculate.hpp> // for SampleInputIterator implementation
#endif
#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/samples.hpp>
#endif

/**
 * \file
 *
 * Tests for all API classes exported by audiobuffer.hpp
 */

TEST_CASE ( "BlockAccumulator", "[audiobuffer] [blockaccumulator]" )
{
	using arcstk::CDDA;
	using arcstk::SampleSequence;

	arcsdec::BlockAccumulator accumulator; // Capacity: BLOCKSIZE.DEFAULT

	CHECK ( accumulator.samples_appended() == 0 );


	SECTION ( "BlockAccumulator append works correct" )
	{
		// Create actual samples
		std::vector<int32_t> samples;
		samples.resize(4096); // smaller than BLOCKSIZE.DEFAULT

		// Configure sequence "adapter" for actual samples
		SampleSequence<int32_t, false> sequence; // left 0, right 1
		sequence.wrap(samples.data(),
			samples.size() / static_cast<unsigned int>(CDDA.NUMBER_OF_CHANNELS));

		// We can append those sequences without registering a processor
		// because the won't trigger a flush()

		accumulator.append_to_block(sequence.begin(), sequence.end());

		CHECK ( accumulator.samples_appended()   == 1024 );

		accumulator.append_to_block(sequence.begin(), sequence.end());

		CHECK ( accumulator.samples_appended()   == 2048 );

		//accumulator.flush();
	}
}


TEST_CASE ( "SampleBuffer", "[audiobuffer] [samplebuffer]" )
{
	arcsdec::SampleBuffer buffer;

	CHECK ( buffer.samples_processed() == 0 );
}

