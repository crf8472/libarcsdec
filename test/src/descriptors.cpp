#include "catch2/catch_test_macros.hpp"

#include <algorithm>
#include <regex>
#include <iostream>
#include <type_traits>

#ifndef __LIBARCSDEC_DESCRIPTORS_HPP__
#include "descriptors.hpp"
#endif
#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"
#endif
#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include "metaparser.hpp"
#endif
#ifndef __LIBARCSDEC_PARSERCUE_HPP__
#include "parsercue.hpp"
#endif
#ifndef __LIBARCSDEC_READERWAV_HPP__
#include "readerwav.hpp"
#endif
#ifndef __LIBARCSDEC_READERFLAC_HPP__
#include "readerflac.hpp"
#endif
#ifndef __LIBARCSDEC_VERSION_HPP__
#include "version.hpp"
#endif


/**
 * \file
 *
 * Tests for all API classes exported by descriptors.hpp
 */


// TODO is_audio_format


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


TEST_CASE ( "FileTest", "[filetest]" )
{
	using arcsdec::FileTest;
	using arcsdec::FileTestName;
	using arcsdec::FileTestBytes;


	SECTION ( "Copy constructor and assignment operator are not declared" )
	{
		CHECK ( not std::is_copy_constructible<FileTest>::value );
		CHECK ( not std::is_copy_assignable<FileTest>::value );
	}


	SECTION ( "Move constructor and assignment operator are not declared" )
	{
		CHECK ( not std::is_nothrow_move_constructible<FileTest>::value );
		CHECK ( not std::is_nothrow_move_assignable<FileTest>::value );
	}


	SECTION ( "Equality comparison is correct" )
	{
		std::unique_ptr<FileTest> t01 = std::make_unique<FileTestName>();
		std::unique_ptr<FileTest> t02 = std::make_unique<FileTestBytes>(0, 7);

		CHECK ( *t01 != *t02 );

		std::unique_ptr<FileTest> t03 = std::make_unique<FileTestBytes>(0, 7);

		CHECK ( *t02 == *t03 );

		std::unique_ptr<FileTest> t04 = std::make_unique<FileTestBytes>(0, 12);

		CHECK ( *t02 != *t04 );
		CHECK ( *t03 != *t04 );
	}
}


TEST_CASE ( "FileTestBytes", "[filetestbytes]" )
{
	using arcsdec::FileTest;
	using arcsdec::FileTestBytes;


	SECTION ( "Is final")
	{
		CHECK ( std::is_final<FileTestBytes>::value );
	}

	SECTION ( "Copy constructor and assignment operator are declared" )
	{
		CHECK ( std::is_copy_constructible<FileTestBytes>::value );
		CHECK ( std::is_copy_assignable<FileTestBytes>::value );
	}

	SECTION ( "Move constructor and assignment operator are declared" )
	{
		CHECK ( std::is_nothrow_move_constructible<FileTestBytes>::value );
		CHECK ( std::is_nothrow_move_assignable<FileTestBytes>::value );
	}

	SECTION ( "Equality comparison is correct" )
	{
		FileTestBytes t01 = FileTestBytes( 0, 18);
		FileTestBytes t02 = FileTestBytes(10,  7);

		CHECK ( t01 != t02 );

		FileTestBytes t03 = FileTestBytes(10,  7);

		CHECK ( t02 == t03 );

		FileTestBytes t04 = FileTestBytes(10,  9);

		CHECK ( t02 != t04 );
		CHECK ( t03 != t04 );
	}

	SECTION ( "Swapping works correctly with std::swap" )
	{
		FileTestBytes t01(0, 12);
		FileTestBytes t02(4, 33);

		std::swap( t01,  t02);

		CHECK ( t01.offset() ==  4 );
		CHECK ( t01.length() == 33 );
		CHECK ( t02.offset() ==  0 );
		CHECK ( t02.length() == 12 );
	}
}


TEST_CASE ( "FileReaderSelector", "[filereaderselector]")
{
	using arcsdec::FileReaderSelector;

	SECTION ( "Copy constructor and assignment operator are not declared" )
	{
		CHECK ( not std::is_copy_constructible<FileReaderSelector>::value );
		CHECK ( not std::is_copy_assignable<FileReaderSelector>::value );
	}

	SECTION ( "Move constructor and assignment operator are not declared" )
	{
		CHECK ( not std::is_nothrow_move_constructible<FileReaderSelector>::value );
		CHECK ( not std::is_nothrow_move_assignable<FileReaderSelector>::value );
	}
}


TEST_CASE ( "DefaultSelector", "[defaultselector]")
{
	using arcsdec::DefaultSelector;

	SECTION ( "Copy constructor and assignment operator are declared" )
	{
		CHECK ( std::is_copy_constructible<DefaultSelector>::value );
		CHECK ( std::is_copy_assignable<DefaultSelector>::value );
	}

	SECTION ( "Move constructor and assignment operator are declared" )
	{
		CHECK ( std::is_nothrow_move_constructible<DefaultSelector>::value );
		CHECK ( std::is_nothrow_move_assignable<DefaultSelector>::value );
	}


	// TODO Test for move constructor
}


TEST_CASE ( "FileReaderSelection", "[filereaderselection]" )
{
	using arcsdec::FileReaderSelection;
	using arcsdec::FileReaderDescriptor;

	FileReaderSelection selection;

	REQUIRE ( selection.total_tests() == 0 );
	REQUIRE ( selection.no_tests() );


	SECTION ( "Is final")
	{
		CHECK ( std::is_final<FileReaderSelection>::value );
	}

	SECTION ( "Copy constructor and assignment operator are not declared" )
	{
		CHECK ( not std::is_copy_constructible<FileReaderSelection>::value );
		CHECK ( not std::is_copy_assignable<FileReaderSelection>::value );
	}

	SECTION ( "Move constructor and assignment operator are declared" )
	{
		CHECK ( std::is_nothrow_move_constructible<FileReaderSelection>::value );
		CHECK ( std::is_nothrow_move_assignable<FileReaderSelection>::value );
	}

	SECTION ( "Adding tests works correctly" )
	{
		selection.register_test(std::make_unique<arcsdec::FileTestBytes>(0, 7));

		CHECK ( selection.total_tests() == 1 );
		CHECK ( not selection.no_tests() );

		selection.register_test(std::make_unique<arcsdec::FileTestName>());

		CHECK ( selection.total_tests() == 2 );
		CHECK ( not selection.no_tests() );
	}

	SECTION ( "Removing tests works correctly" )
	{
		selection.register_test(std::make_unique<arcsdec::FileTestBytes>(0, 6));
		selection.register_test(std::make_unique<arcsdec::FileTestName>());

		CHECK ( selection.total_tests() == 2 );
		CHECK ( not selection.no_tests() );

		const std::unique_ptr<arcsdec::FileTest> & name_test=
			std::make_unique<arcsdec::FileTestName>();

		auto d = selection.unregister_test(name_test);

		CHECK ( selection.total_tests() == 1 );
		CHECK ( not selection.no_tests() );
	}
}


TEST_CASE ( "FileReaderRegistry", "[filereaderregistry]")
{
	using arcsdec::FileReaderRegistry;
	using arcsdec::RegisterDescriptor;
	using arcsdec::DescriptorWavPCM;

	using AudioDescriptorTestType = RegisterDescriptor<DescriptorWavPCM>;
	using MetadataDescriptorTestType =
		RegisterDescriptor<DescriptorWavPCM>;


	SECTION ( "Copy constructor and assignment operator are declared protected" )
	{
		// not in base class
		CHECK ( not std::is_copy_constructible<FileReaderRegistry>::value );
		CHECK ( not std::is_copy_assignable<FileReaderRegistry>::value );

		// available in subclass
		CHECK ( std::is_copy_constructible<AudioDescriptorTestType>::value );
		CHECK ( std::is_copy_assignable<AudioDescriptorTestType>::value );
		CHECK ( std::is_copy_constructible<MetadataDescriptorTestType>::value );
		CHECK ( std::is_copy_assignable<MetadataDescriptorTestType>::value );
	}

	SECTION ( "Move constructor and assignment operator are declared protected" )
	{
		// not in base class
		CHECK ( not std::is_nothrow_move_constructible<FileReaderRegistry>::value );
		CHECK ( not std::is_nothrow_move_assignable<FileReaderRegistry>::value );

		// available in subclass
		CHECK ( std::is_nothrow_move_constructible<AudioDescriptorTestType>::value );
		CHECK ( std::is_nothrow_move_assignable<AudioDescriptorTestType>::value );
		CHECK ( std::is_nothrow_move_constructible<MetadataDescriptorTestType>::value );
		CHECK ( std::is_nothrow_move_assignable<MetadataDescriptorTestType>::value );
	}
}


TEST_CASE ( "CreateAudioReader Functor", "")
{
	using arcsdec::CreateAudioReader;
	using arcsdec::FileReaderRegistry;

	const CreateAudioReader create;

	SECTION ( "Create reader for RIFFWAV/PCM correctly" )
	{
		auto reader = create(*FileReaderRegistry::default_audio_selection(),
			*FileReaderRegistry::descriptors(), "test01.wav");

		CHECK ( reader != nullptr );
	}
}


TEST_CASE ( "CreateMetadataParser Functor", "")
{
	using arcsdec::CreateMetadataParser;
	using arcsdec::FileReaderRegistry;

	const CreateMetadataParser create;

	SECTION ( "Create reader for CueSheet correctly" )
	{
		auto reader = create(*FileReaderRegistry::default_toc_selection(),
			*FileReaderRegistry::descriptors(), "test01.cue");

		CHECK ( reader != nullptr );
	}
}


TEST_CASE ( "Register Descriptor Functors",
		"[registeraudiodescriptor, registermetadatadescriptor]")
{
	using arcsdec::RegisterDescriptor;
	using arcsdec::DescriptorWavPCM;
	using arcsdec::DescriptorCue;

	REQUIRE ( 7 == arcsdec::FileReaderRegistry::descriptors()->size() );

	SECTION ( "RegisterAudioDescriptor<> is final" )
	{
		using WavPCMRegisterType = RegisterDescriptor<DescriptorWavPCM>;

		CHECK ( std::is_final<WavPCMRegisterType>::value );
		CHECK ( 7 == arcsdec::FileReaderRegistry::descriptors()->size() );
	}

	SECTION ( "RegisterMetadataDescriptor<> is final" )
	{
		using CueRegisterType = RegisterDescriptor<DescriptorCue>;

		CHECK ( std::is_final<CueRegisterType>::value );
		CHECK ( 7 == arcsdec::FileReaderRegistry::descriptors()->size() );
	}
}

