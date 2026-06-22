#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for descriptor.hpp.
 */

#ifndef LIBARCSDEC_DESCRIPTOR_HPP_
#include "descriptor.hpp"               // TO BE TESTED
#endif


TEST_CASE ( "read_bytes", "[read_bytes]" )
{
	using arcsdec::read::FileReadException;

	SECTION ( "Reading existing bytes from valid file works" )
	{
		auto bytes = arcsdec::read::details::read_bytes("data/test01.wav", 0, 44);
		// ARCS1: E35EF68A, ARCS2: E3631C44

		CHECK ( bytes.size() == 44 );

		CHECK ( bytes [0] == 'R' );
		CHECK ( bytes [1] == 'I' );
		CHECK ( bytes [2] == 'F' );
		CHECK ( bytes [3] == 'F' );

		CHECK ( bytes [8] == 'W' );
		CHECK ( bytes [9] == 'A' );
		CHECK ( bytes[10] == 'V' );
		CHECK ( bytes[11] == 'E' );

		CHECK ( bytes[12] == 'f' );
		CHECK ( bytes[13] == 'm' );
		CHECK ( bytes[14] == 't' );
		CHECK ( bytes[15] == ' ' );

		CHECK ( bytes[36] == 'd' );
		CHECK ( bytes[37] == 'a' );
		CHECK ( bytes[38] == 't' );
		CHECK ( bytes[39] == 'a' );
	}

	SECTION ( "Opening non-existing file causes exception" )
	{
		try
		{
			arcsdec::read::details::read_bytes("does_not_exist.wav", 0, 12);
			FAIL ( "Expected FileReadException was not thrown" );
		} catch (const FileReadException &e)
		{
			CHECK ( e.byte_pos() == 0 );
		}
	}

	SECTION ( "Trying to read beyond EOF causes exception" )
	{
		try
		{
			arcsdec::read::details::read_bytes("data/test01.wav", 0, 4146);
			FAIL ( "Expected FileReadException was not thrown" );
		} catch (const FileReadException &e)
		{
			CHECK ( e.byte_pos() == 4145 );
		}
	}
}
