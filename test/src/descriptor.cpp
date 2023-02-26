#include "catch2/catch_test_macros.hpp"

#include <type_traits>

#ifndef __LIBARCSDEC_DESCRIPTOR_HPP__
#include "descriptor.hpp"
#endif


/**
 * \file
 *
 * Tests for classes in descriptor.cpp
 */


TEST_CASE ( "ci_string", "[ci_string]" )
{
	using arcsdec::details::ci_string;
}


TEST_CASE ( "Bytes", "[bytes]" )
{
	using arcsdec::Bytes;
	using arcsdec::ByteSequence;

	auto bytes  { Bytes(0, { 0x01, 0x02, 0x06, 0x07, 0x4C, 0xF0 }) };
	//auto bytes1 { Bytes(2, { 0x41, 0xC2, 0xA9, 0x08, 0xBF, 0xC4 }) };

	SECTION ( "Instantiation is done correctly" )
	{
		CHECK ( 0 == bytes.offset() );
		CHECK ( ByteSequence{ 0x01, 0x02, 0x06, 0x07, 0x4C, 0xF0 }
				== bytes.sequence() );
	}

	SECTION ( "Equality operator works correctly" )
	{
		// equal to 'bytes'
		auto bytes2 { Bytes(0, { 0x01, 0x02, 0x06, 0x07, 0x4C, 0xF0 }) };

		CHECK ( bytes  == bytes2 );
		CHECK ( bytes  == Bytes(0, { 0x01, 0x02, 0x06, 0x07, 0x4C, 0xF0 }) );

		// different from 'bytes'
		auto other_bytes1 { Bytes(0, { 0x02, 0x00, 0x06, 0x0F, 0x7C, 0xD1 }) };
		auto other_bytes2 { Bytes(0, { 0x01, 0x02, 0x06, 0x07, 0x4C, 0xF1 }) };
		auto other_bytes3 { Bytes(0, { 0x00, 0x02, 0x06, 0x07, 0x4C, 0xF0 }) };
		auto other_bytes4 { Bytes(1, { 0x01, 0x02, 0x06, 0x07, 0x4C, 0xF0 }) };

		CHECK ( bytes  != other_bytes1 );
		CHECK ( bytes2 != other_bytes1 );
		CHECK ( bytes  != other_bytes2 );
		CHECK ( bytes2 != other_bytes2 );
		CHECK ( bytes  != other_bytes3 );
		CHECK ( bytes2 != other_bytes3 );
		CHECK ( bytes  != other_bytes4 );
		CHECK ( bytes2 != other_bytes4 );
	}

	SECTION ( "match() matches matching sequences with different offsets" )
	{
		// longer input
		CHECK ( bytes.match({ 0x01, 0x02, 0x06, 0x07, 0x4C, 0xF0, 0xC1 }, 0) );

		// input of equal length
		CHECK ( bytes.match({ 0x01, 0x02, 0x06, 0x07, 0x4C, 0xF0 }, 0) );

		// shorter input with non-zero offset
		CHECK ( bytes.match({ 0x02, 0x06, 0x07, 0x4C, 0xF0 }, 1) );
		CHECK ( bytes.match({ 0x06, 0x07, 0x4C, 0xF0 }, 2) );
		CHECK ( bytes.match({ 0x07, 0x4C, 0xF0 }, 3) );
		CHECK ( bytes.match({ 0x4C, 0xF0 }, 4) );
		CHECK ( bytes.match({ 0xF0 }, 5) );
		CHECK ( bytes.match({ }, 6) );

		// shorter input with zero offset
		CHECK ( bytes.match({ 0x01, 0x02, 0x06, 0x07, 0x4C }, 0) );
		CHECK ( bytes.match({ 0x01, 0x02, 0x06, 0x07 }, 0) );
		CHECK ( bytes.match({ 0x01, 0x02, 0x06 }, 0) );
		CHECK ( bytes.match({ 0x01, 0x02 }, 0) );
		CHECK ( bytes.match({ 0x01 }, 0) );
	}

	SECTION ( "match() matches empty sequences of every offset" )
	{
		CHECK ( bytes.match({}, 0) );
		CHECK ( bytes.match({}, 1) );
		CHECK ( bytes.match({}, 2) );
		CHECK ( bytes.match({}, 12) );
		CHECK ( bytes.match({}, 23) );
		CHECK ( bytes.match({}, 49) );
		CHECK ( bytes.match({}, 127) );
	}

	SECTION ( "match() does not match equal sequence with different offset" )
	{
		// wrong offset before sequence end
		CHECK ( !bytes.match({ 0x01, 0x02, 0x06, 0x07, 0x4C, 0xF0 }, 1) );
		CHECK ( !bytes.match({ 0x01, 0x02, 0x06, 0x07, 0x4C, 0xF0 }, 2) );
		CHECK ( !bytes.match({ 0x01, 0x02, 0x06, 0x07, 0x4C, 0xF0 }, 3) );
		CHECK ( !bytes.match({ 0x01, 0x02, 0x06, 0x07, 0x4C, 0xF0 }, 4) );
		CHECK ( !bytes.match({ 0x01, 0x02, 0x06, 0x07, 0x4C, 0xF0 }, 5) );

		// offset behind sequence end
		CHECK ( !bytes.match({ 0x01, 0x02, 0x06, 0x07, 0x4C, 0xF0 }, 6) );
		CHECK ( !bytes.match({ 0x01, 0x02, 0x06, 0x07, 0x4C, 0xF0 }, 7) );
	}

	SECTION ( "match() does not match non-matching sequences" )
	{
		// last byte is different
		CHECK ( !bytes.match({ 0x01, 0x02, 0x06, 0x07, 0x4C, 0xF1 }, 0) );

		// first byte is different
		CHECK ( !bytes.match({ 0x09, 0x02, 0x06, 0x07, 0x4C, 0xF0 }, 0) );
	}

	SECTION ( "match() handles wildcards correctly" )
	{
		auto bytes2 { Bytes(0, { 0x01, Bytes::any, 0x06, 0x07, 0x4C, 0xF0 }) };

		CHECK ( bytes2.match({ 0x01, 0x02, 0x06, 0x07, 0x4C, 0xF0 }, 0) );
		CHECK ( bytes2.match({ 0x01, 0x6D, 0x06, 0x07, 0x4C, 0xF0 }, 0) );
		CHECK ( bytes2.match({ 0x01, 0x1F, 0x06, 0x07, 0x4C, 0xF0 }, 0) );
	}

	SECTION ( "match() works on M4A format" )
	{
		auto m4a { Bytes(4, {0x66, 0x74, 0x79, 0x70, 0x4D, 0x34, 0x41}) };

		CHECK ( m4a.match({0x00, 0x00, 0x00, 0x00, /*non-matching bytes ahead*/
					0x66, 0x74, 0x79, 0x70, 0x4D, 0x34, 0x41}, 0) );
	}
}


// TODO
//TEST_CASE ( "FormatDescriptor", "[formatdescriptor]" )
//{
//}


TEST_CASE ( "FileReader", "[filereader]")
{
	using arcsdec::FileReader;

	SECTION ( "Copy constructor and assignment operator are not declared" )
	{
		CHECK ( not std::is_copy_constructible<FileReader>::value );
		CHECK ( not std::is_copy_assignable<FileReader>::value );
	}


	SECTION ( "Move constructor and assignment operator are not accessible" )
	{
		CHECK ( not std::is_nothrow_move_constructible<FileReader>::value );
		CHECK ( not std::is_nothrow_move_assignable<FileReader>::value );
	}

	// TODO Test for move
}


TEST_CASE ( "libinfo_entry", "[libinfo]" )
{
	using arcsdec::LibInfoEntry;
	using arcsdec::LibInfo;
	using arcsdec::libinfo_entry;
}


TEST_CASE ( "read_bytes", "[read_bytes]" )
{
	using arcsdec::FileReadException;

	SECTION ( "Reading existing bytes from valid file works" )
	{
		auto bytes = arcsdec::details::read_bytes("test01.wav", 0, 44);
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
			arcsdec::details::read_bytes("does_not_exist.wav", 0, 12);
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
			arcsdec::details::read_bytes("test01.wav", 0, 4146);
			FAIL ( "Expected FileReadException was not thrown" );
		} catch (const FileReadException &e)
		{
			CHECK ( e.byte_pos() == 4145 );
		}
	}
}


// TODO
//TEST_CASE ( "get_suffix", "[get_suffix]" )
//{
//}

