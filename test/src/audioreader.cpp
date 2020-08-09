#include "catch2/catch.hpp"

#include "limits.h"

#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"
#endif

/**
 * \file
 *
 * Tests for all API classes exported by audioreader.hpp
 */


TEST_CASE ( "ByteConverter", "[byteconverter]" )
{
	arcsdec::ByteConverter cnv;
	const auto MAXCHAR = std::numeric_limits<char>::max();

	CHECK ( MAXCHAR == 0x7Fu ); // Report if char maximum is other than 127


	SECTION ( "Convert char to uint8_t", "[byteconverter]" )
	{
		CHECK ( cnv.byte_to_uint8(MAXCHAR) == 0x7F );
	}

	SECTION ( "Convert 2 LE chars to int16_t", "[byteconverter]" )
	{
		CHECK ( cnv.le_bytes_to_int16(0x12, 0x34) == 0x3412 );
		CHECK ( cnv.le_bytes_to_int16(0x69, 0x7F) == 0x7F69 );
	}

	SECTION ( "Convert 2 LE chars to uint16_t", "[byteconverter]" )
	{
		CHECK ( cnv.le_bytes_to_uint16(0x69, 0x7F) == 0x7F69 );
	}

	SECTION ( "Convert 4 LE chars to int32_t", "[byteconverter]" )
	{
		CHECK ( cnv.le_bytes_to_int32(0x7F, 0x6E, 0x5D, 0x4C) == 0x4C5D6E7F );
	}

	SECTION ( "Convert 4 LE chars to uint32_t", "[byteconverter]" )
	{
		CHECK ( cnv.le_bytes_to_uint32(0x7F, 0x6E, 0x5D, 0x4C) == 0x4C5D6E7Fu );
	}

	SECTION ( "Convert 4 BE chars to int32_t", "[byteconverter]" )
	{
		CHECK ( cnv.be_bytes_to_int32(0x7F, 0x6E, 0x5D, 0x4C) == 0x7F6E5D4C );
	}

	SECTION ( "Convert 4 BE chars to uint32_t", "[byteconverter]" )
	{
		CHECK ( cnv.be_bytes_to_uint32(0x7F, 0x6E, 0x5D, 0x4C) == 0x7F6E5D4Cu );
	}
}

