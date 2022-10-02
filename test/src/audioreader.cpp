#include "catch2/catch_test_macros.hpp"

#include "limits.h"

#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"
#endif

/**
 * \file
 *
 * Tests for all API classes exported by audioreader.hpp
 */


TEST_CASE ( "LittleEndianBytes", "[littleendianbytes]" )
{
	using LE = arcsdec::LittleEndianBytes;

	SECTION ( "Convert 2 LE chars to int16_t", "[littleendianbytes]" )
	{
		CHECK ( LE::to_int16(0x12, 0x34) == 0x3412 );
		CHECK ( LE::to_int16(0x69, 0x7F) == 0x7F69 );
	}

	SECTION ( "Convert 2 LE chars to uint16_t", "[littleendianbytes]" )
	{
		CHECK ( LE::to_uint16(0x00, 0x01) == 0x0100 );
		CHECK ( LE::to_uint16(0x12, 0x34) == 0x3412 );
		CHECK ( LE::to_uint16(0x69, 0x7F) == 0x7F69 );
		CHECK ( LE::to_uint16(0x7F, 0x7F) == 0x7F7F );
	}

	SECTION ( "Convert 4 LE chars to int32_t", "[littleendianbytes]" )
	{
		CHECK ( LE::to_int32(0x7F, 0x6E, 0x5D, 0x4C) == 0x4C5D6E7F );
	}

	SECTION ( "Convert 4 LE chars to uint32_t", "[littleendianbytes]" )
	{
		CHECK ( LE::to_uint32(0x7F, 0x6E, 0x5D, 0x4C) == 0x4C5D6E7Fu );
	}
}


TEST_CASE ( "BigEndianBytes", "[bigendianbytes]" )
{
	using BE = arcsdec::BigEndianBytes;

	SECTION ( "Convert 4 BE chars to int32_t", "[bigendianbytes]" )
	{
		CHECK ( BE::to_int32(0x7F, 0x6E, 0x5D, 0x4C) == 0x7F6E5D4C );
	}

	SECTION ( "Convert 4 BE chars to uint32_t", "[bigendianbytes]" )
	{
		CHECK ( BE::to_uint32(0x7F, 0x6E, 0x5D, 0x4C) == 0x7F6E5D4Cu );
	}
}

