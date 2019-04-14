#include "catch2/catch.hpp"


#ifndef __LIBARCSDEC_AUDIOBUFFER_HPP__
#include "audiobuffer.hpp"
#endif

#ifndef __LIBARCS_IDENTIFIER_HPP__
#include <arcs/identifier.hpp> // for CDDA
#endif
#ifndef __LIBARCS_calculate_HPP__
#include <arcs/calculate.hpp> // for PCMForwardIterator implementation
#endif
#ifndef __LIBARCS_IDENTIFIER_HPP__
#include <arcs/samples.hpp>
#endif

/**
 * \file audiobuffer.cpp Tests for all API classes exported by audiobuffer.hpp
 */

TEST_CASE ( "BlockAccumulator", "[audiobuffer] [blockaccumulator]" )
{
	arcs::BlockAccumulator accumulator; // Capacity: BLOCKSIZE::DEFAULT

	REQUIRE ( accumulator.samples_appended()   == 0 );


	SECTION ( "BlockAccumulator append works correct" )
	{
		// Create actual samples
		std::vector<int32_t> samples;
		samples.resize(4096); // smaller than BLOCKSIZE::DEFAULT

		// Configure sequence "adapter" for actual samples
		arcs::SampleSequence<int32_t, false> sequence; // left 0, right 1
		sequence.reset(samples.data(),
				samples.size() / arcs::CDDA.NUMBER_OF_CHANNELS);

		// We can append those sequences without registering a processor
		// because the won't trigger a flush()

		accumulator.append_to_block(sequence.begin(), sequence.end());

		REQUIRE ( accumulator.samples_appended()   == 1024 );

		accumulator.append_to_block(sequence.begin(), sequence.end());

		REQUIRE ( accumulator.samples_appended()   == 2048 );

		//accumulator.flush();
	}
}


TEST_CASE ( "SampleBuffer", "[audiobuffer] [samplebuffer]" )
{
	arcs::SampleBuffer buffer;

	REQUIRE ( buffer.samples_processed() == 0 );
}

