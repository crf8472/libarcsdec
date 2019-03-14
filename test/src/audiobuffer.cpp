#include "catch2/catch.hpp"

//#ifndef __LIBARCSDEC_SAMPLESEQUENCE_HPP__
//#include "samplesequence.hpp"
//#endif
#ifndef __LIBARCSDEC_AUDIOBUFFER_HPP__
#include "audiobuffer.hpp"
#endif

#ifndef __LIBARCS_IDENTIFIER_HPP__
#include <arcs/identifier.hpp>
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

	REQUIRE ( accumulator.samples_processed()   == 0 );
	REQUIRE ( accumulator.bytes_processed()     == 0 );
	REQUIRE ( accumulator.sequences_processed() == 0 );
	REQUIRE ( accumulator.blocks_processed()    == 0 );


	SECTION ( "BlockAccumulator append works correct" )
	{
		// Create actual samples (i.e. WVPK)
		std::vector<int32_t> samples;
		samples.resize(4096); // smaller than BLOCKSIZE::DEFAULT

		// Configure sequence "adapter" for actual samples
		arcs::SampleSequence<int32_t, false> sequence; // left 0, right 1
		sequence.reset(samples.data(),
				samples.size() / arcs::CDDA.NUMBER_OF_CHANNELS);

		accumulator.append(sequence.begin(), sequence.end());

		REQUIRE ( accumulator.samples_processed()   == 1024 );
		REQUIRE ( accumulator.sequences_processed() ==    1 );
		REQUIRE ( accumulator.blocks_processed()    ==    0 );

		accumulator.append(sequence.begin(), sequence.end());

		REQUIRE ( accumulator.samples_processed()   == 2048 );
		REQUIRE ( accumulator.sequences_processed() ==    2 );
		REQUIRE ( accumulator.blocks_processed()    ==    0 );

		//accumulator.flush();
	}
}

