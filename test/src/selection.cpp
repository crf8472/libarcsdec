#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for selection.hpp.
 */

#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"                // TO BE TESTED
#endif

#include <type_traits>                  // for is_copy_constructible,...


TEST_CASE ( "FileReaderSelector", "[filereaderselector]")
{
	using arcsdec::FileReaderSelector;


	SECTION ( "Copy constructor and assignment operator are not declared" )
	{
		CHECK ( not std::is_copy_constructible<FileReaderSelector>::value );
		CHECK ( std::is_copy_assignable<FileReaderSelector>::value );
	}

	SECTION ( "Move constructor and assignment operator are not declared" )
	{
		CHECK ( not std::is_nothrow_move_constructible<FileReaderSelector>::value );
		CHECK ( std::is_nothrow_move_assignable<FileReaderSelector>::value );
	}
}


TEST_CASE ( "FileReaderRegistry", "[filereaderregistry]")
{
	using arcsdec::FileReaderRegistry;
	//using arcsdec::DescriptorWavPCM;
	//using DescriptorTestType = arcsdec::RegisterDescriptor<DescriptorWavPCM>;


	SECTION ( "Copy constructor and assignment operator are available" )
	{
		// not in base class
		CHECK ( std::is_copy_constructible<FileReaderRegistry>::value );
		CHECK ( std::is_copy_assignable<FileReaderRegistry>::value );

		// available in subclass
		//CHECK ( std::is_copy_constructible<DescriptorTestType>::value );
		//CHECK ( std::is_copy_assignable<DescriptorTestType>::value );
	}

	SECTION ( "Move constructor and move assignment operator are present" )
	{
		// in base class
		CHECK ( std::is_nothrow_move_constructible<FileReaderRegistry>::value );
		CHECK ( std::is_nothrow_move_assignable<FileReaderRegistry>::value );

		// available in subclass
		//CHECK ( std::is_nothrow_move_constructible<DescriptorTestType>::value );
		//CHECK ( std::is_nothrow_move_assignable<DescriptorTestType>::value );
	}

	SECTION ( "Exactly the supported formats are present" )
	{
		using arcsdec::Format;

		CHECK ( FileReaderRegistry::has_format(Format::CUE) );
		CHECK ( FileReaderRegistry::has_format(Format::CDRDAO) );
		CHECK ( FileReaderRegistry::has_format(Format::WAV) );
		CHECK ( FileReaderRegistry::has_format(Format::FLAC) );
		CHECK ( FileReaderRegistry::has_format(Format::APE) );
		CHECK ( FileReaderRegistry::has_format(Format::CAF) );
		CHECK ( FileReaderRegistry::has_format(Format::M4A) );
		CHECK ( FileReaderRegistry::has_format(Format::OGG) );
		CHECK ( FileReaderRegistry::has_format(Format::WV) );
		CHECK ( FileReaderRegistry::has_format(Format::AIFF) );

		CHECK ( 10 == FileReaderRegistry::formats()->size() );
	}

	SECTION ( "Mandatory descriptors are registered" )
	{
		// At least the 2 non-optional descriptors:
		// Maybe not each available reader was compiled, but we will always have
		// the genuine wav reader + libcue-based cuesheet parser
		CHECK ( 2 <= arcsdec::FileReaderRegistry::readers()->size() );
		// Specific tests are in parserlibcue.cpp and readerwav.cpp
	}
}

