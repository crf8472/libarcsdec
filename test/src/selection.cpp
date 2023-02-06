#include "catch2/catch_test_macros.hpp"

#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"
#endif

#ifndef __LIBARCSDEC_DESCRIPTOR_HPP__
#include "descriptor.hpp"
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
 * Tests for all API classes exported by selection.hpp
 */

TEST_CASE ( "DescriptorSet", "[descriptorset]" )
{
	using arcsdec::DescriptorSet;
	using arcsdec::DescriptorWavPCM;

	DescriptorSet s;


	SECTION ( "Initialization is ok" )
	{
		CHECK ( s.size() == 0 );
		CHECK ( s.empty() );
		CHECK ( s.begin() == s.end() );
	}

	SECTION ( "Adding is ok on initial state" )
	{
		s.insert(std::make_unique<DescriptorWavPCM>());

		CHECK ( s.size() == 1 );
		CHECK ( not s.empty() );
		CHECK ( s.begin() != s.end() );
		CHECK ( "RIFF/WAV(PCM)" == s.begin()->second->name() );
		CHECK ( s.get("wavpcm") != nullptr );
	}
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
	using arcsdec::DescriptorWavPCM;
	using DescriptorTestType = arcsdec::RegisterDescriptor<DescriptorWavPCM>;


	SECTION ( "Copy constructor and assignment operator are declared protected" )
	{
		// not in base class
		CHECK ( not std::is_copy_constructible<FileReaderRegistry>::value );
		CHECK ( not std::is_copy_assignable<FileReaderRegistry>::value );

		// available in subclass
		CHECK ( std::is_copy_constructible<DescriptorTestType>::value );
		CHECK ( std::is_copy_assignable<DescriptorTestType>::value );
	}

	SECTION ( "Move constructor and assignment operator are declared protected" )
	{
		// not in base class
		CHECK ( not std::is_nothrow_move_constructible<FileReaderRegistry>::value );
		CHECK ( not std::is_nothrow_move_assignable<FileReaderRegistry>::value );

		// available in subclass
		CHECK ( std::is_nothrow_move_constructible<DescriptorTestType>::value );
		CHECK ( std::is_nothrow_move_assignable<DescriptorTestType>::value );
	}
}


TEST_CASE ( "RegisterDescriptor", "[registerdescriptor]")
{
	using arcsdec::RegisterDescriptor;
	using arcsdec::DescriptorWavPCM;
	using arcsdec::DescriptorCue;

	REQUIRE ( 7 >= arcsdec::FileReaderRegistry::descriptors()->size() );
	REQUIRE ( 0  < arcsdec::FileReaderRegistry::descriptors()->size() );


	SECTION ( "RegisterAudioDescriptor<> is final" )
	{
		using WavPCMRegisterType = RegisterDescriptor<DescriptorWavPCM>;

		CHECK ( std::is_final<WavPCMRegisterType>::value );
		CHECK ( 7 >= arcsdec::FileReaderRegistry::descriptors()->size() );
	}

	SECTION ( "RegisterMetadataDescriptor<> is final" )
	{
		using CueRegisterType = RegisterDescriptor<DescriptorCue>;

		CHECK ( std::is_final<CueRegisterType>::value );
		CHECK ( 7 >= arcsdec::FileReaderRegistry::descriptors()->size() );
	}
}

