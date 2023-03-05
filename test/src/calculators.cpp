#include "catch2/catch_test_macros.hpp"

#ifndef __LIBARCSDEC_CALCULATORS_HPP__
#include "calculators.hpp"
#endif

/**
 * \file
 *
 * Tests for all API classes exported by calculators.hpp
 */


//TEST_CASE ( "CreateAudioReader Functor", "")
//{
//	using arcsdec::CreateAudioReader;
//	using arcsdec::FileReaderRegistry;
//
//	const CreateAudioReader create;
//
//	SECTION ( "Create reader for RIFFWAV/PCM correctly" )
//	{
//		auto reader = create(*FileReaderRegistry::default_audio_selection(),
//			*FileReaderRegistry::descriptors(), "test01.wav");
//
//		CHECK ( reader != nullptr );
//	}
//}
//
//
//TEST_CASE ( "CreateMetadataParser Functor", "")
//{
//	using arcsdec::CreateMetadataParser;
//	using arcsdec::FileReaderRegistry;
//
//	const CreateMetadataParser create;
//
//	SECTION ( "Create reader for CueSheet correctly" )
//	{
//		auto reader = create(*FileReaderRegistry::default_toc_selection(),
//			*FileReaderRegistry::descriptors(), "test01.cue");
//
//		CHECK ( reader != nullptr );
//	}
//}


TEST_CASE ( "TOCParser", "[calculators]" )
{
	using arcsdec::TOCParser;

	TOCParser p;

	SECTION ("Initial DescriptorSet is present and complete")
	{
		CHECK ( 7 >= p.filereaders().size() );
		CHECK ( not p.filereaders().empty() );
	}

	// TODO parse()
}


TEST_CASE ( "ARIdCalculator", "[calculators]" )
{
	using arcsdec::ARIdCalculator;

	ARIdCalculator c;

	SECTION ("Initial DescriptorSet is present and complete")
	{
		CHECK ( 7 >= c.filereaders().size() );
		CHECK ( not c.filereaders().empty() );
	}

	// TODO calculate()
}


TEST_CASE ( "ARCSCalculator", "[calculators]" )
{
	using arcsdec::ARCSCalculator;

	ARCSCalculator c;

	SECTION ("Initial DescriptorSet is present and complete")
	{
		CHECK ( 7 >= c.filereaders().size() );
		CHECK ( not c.filereaders().empty() );
	}

	// TODO calculate()
}

