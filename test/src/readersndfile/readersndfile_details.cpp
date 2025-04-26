#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for readersndfile_details.hpp.
 */

#ifndef __LIBARCSDEC_READERSNDFILE_HPP__
#define __LIBARCSDEC_READERSNDFILE_HPP__ // allow readersndfile_details.hpp
#endif
#ifndef __LIBARCSDEC_READERSNDFILE_DETAILS_HPP__
#include "readersndfile_details.hpp"    // TO BE TESTED
#endif


TEST_CASE ( "LibsndfileAudioReaderImpl", "[libsndfileaudioreaderimpl]" )
{
	auto i = arcsdec::details::sndfile::LibsndfileAudioReaderImpl{};
	// TODO

	SECTION ("Descriptor is correct")
	{
		CHECK ( i.descriptor()->id() == "libsndfile" );
	}
}

