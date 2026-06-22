#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for descriptor.hpp.
 */

#ifndef LIBARCSDEC_DESCRIPTOR_HPP_
#include "descriptor.hpp"               // TO BE TESTED
#endif

#include <climits>                      // for ? TODO
#include <type_traits>                  // for ? TODO


TEST_CASE ( "ci_string", "[ci_string]" )
{
// TODO
  //using arcsdec::read::details::ci_string;
}


TEST_CASE ( "ByteSeq", "[byteseq]")
{
	using arcsdec::read::ByteSeq;
	using arcsdec::read::Bytes;

	SECTION ("Constants are correct")
	{
		CHECK ( ByteSeq::max_byte_value >= UCHAR_MAX );
		CHECK ( Bytes::any > ByteSeq::max_byte_value );
	}

	SECTION ("Empty ByteSeq")
	{
		ByteSeq b { };

		CHECK ( b.size() == 0 );
		CHECK ( !b.is_wildcard(0) );
	}

	SECTION ("ByteSeq is constructed correctly")
	{
		//ByteSeq b1 { 0x16, 0x22,         0x4B, 0xFF };
		ByteSeq b2 { 0x16, Bytes::any, 0x4B, 0xFF };

		CHECK ( b2.size() == 4 );

		CHECK ( !b2.is_wildcard(0) );
		CHECK (  b2.is_wildcard(1) );
		CHECK ( !b2.is_wildcard(2) );
		CHECK ( !b2.is_wildcard(3) );
	}
}


TEST_CASE ( "Bytes special members and operators", "[bytes]" )
{
	using arcsdec::read::Bytes;
	using arcsdec::read::ByteSequence;

	auto bytes  { Bytes(0, { 0x01, 0x02, 0x06, 0x07, 0x4C, 0xF0 }) };


	SECTION ("Class Bytes is copy-constructible")
	{
		CHECK ( std::is_copy_constructible<Bytes>::value );
	}

	SECTION ("Class Bytes is copy-assignable")
	{
		CHECK ( std::is_copy_assignable<Bytes>::value );
	}

	SECTION ("Class Bytes is move-constructible")
	{
		CHECK ( std::is_move_constructible<Bytes>::value );
	}

	SECTION ("Class Bytes is move-assignable")
	{
		CHECK ( std::is_move_assignable<Bytes>::value );
	}

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

	SECTION ("Swap works correctly")
	{
		// non-equal to 'bytes'
		auto bytes2 { Bytes(5, { 0x05, 0x09, 0x01, 0x00, 0x42, 0x08 }) };
		bytes.swap(bytes2);

		CHECK ( bytes.sequence()  == ByteSequence{ 0x05, 0x09, 0x01, 0x00, 0x42, 0x08 } );
		CHECK ( bytes2.sequence() == ByteSequence{ 0x01, 0x02, 0x06, 0x07, 0x4C, 0xF0 } );

		CHECK ( bytes.offset()  == 5 );
		CHECK ( bytes2.offset() == 0 );
	}
}


TEST_CASE ( "Bytes::match()", "[bytes]" )
{
	using arcsdec::read::Bytes;
	using arcsdec::read::ByteSequence;

	auto bytes  { Bytes(0, { 0x01, 0x02, 0x06, 0x07, 0x4C, 0xF0 }) };

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


TEST_CASE ( "FileReader special members", "[filereader]")
{
	using arcsdec::read::FileReader;

	SECTION ( "FileReader is not copy-constructible" )
	{
		CHECK ( not std::is_copy_constructible<FileReader>::value );
	}

	SECTION ( "FileReader is copy-assignable" )
	{
		CHECK ( std::is_copy_assignable<FileReader>::value );
	}

	SECTION ( "FileReader is not move-constructible" )
	{
		CHECK ( not std::is_move_constructible<FileReader>::value );
	}

	SECTION ( "FileReader is move-assignable" )
	{
		CHECK ( std::is_move_assignable<FileReader>::value );
	}

	// TODO hash, swap, ==
}


TEST_CASE ( "FileReaderDescriptor special members", "[filereaderdescriptor]")
{
}


TEST_CASE ( "libinfo_entry", "[libinfo]" )
{
	using arcsdec::read::LibInfoEntry;
	using arcsdec::read::LibInfo;
	using arcsdec::read::libinfo_entry_filepath;
}


// TODO
//TEST_CASE ( "ci_match_suffix", "[ci_match_suffix]" )
//{
//}


// TODO
//TEST_CASE ( "get_suffix", "[get_suffix]" )
//{
//}

