#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for readerffmpeg.hpp.
 */

#ifndef __LIBARCSDEC_READERFFMPEG_HPP__
#include "readerffmpeg.hpp"             // TO BE TESTED
#endif

#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"                // for FileReaderSelection
#endif


TEST_CASE ("FileReaderSelection", "[filereaderselection]")
{
	using arcsdec::FileReaderSelection;
	using arcsdec::FileReaderRegistry;
	using arcsdec::Format;
	using arcsdec::Codec;

	const auto default_selection {
		FileReaderRegistry::default_audio_selection() };

	REQUIRE ( default_selection );

	const auto default_readers { FileReaderRegistry::readers() };

	REQUIRE ( default_readers );


	SECTION ( "Descriptor is registered" )
	{
		CHECK ( nullptr != arcsdec::FileReaderRegistry::reader("ffmpeg") );
	}


	SECTION ( "Default settings select ffmpeg for OGG/FLAC" )
	{
		auto reader = default_selection->get(Format::OGG, Codec::FLAC,
				*default_readers );

		CHECK ( "ffmpeg" == reader->id() );
	}

	SECTION ( "Default settings select ffmpeg for OGG/UNKNOWN" )
	{
		auto reader = default_selection->get(Format::OGG, Codec::UNKNOWN,
				*default_readers );

		CHECK ( "ffmpeg" == reader->id() );
	}


	SECTION ( "Default settings select ffmpeg for CAF/ALAC" )
	{
		auto reader = default_selection->get(Format::CAF, Codec::ALAC,
				*default_readers );

		CHECK ( "ffmpeg" == reader->id() );
	}

	SECTION ( "Default settings select ffmpeg for CAF/UNKNOWN" )
	{
		auto reader = default_selection->get(Format::CAF, Codec::UNKNOWN,
				*default_readers );

		CHECK ( "ffmpeg" == reader->id() );
	}


	SECTION ( "Default settings select ffmpeg for M4A/ALAC" )
	{
		auto reader = default_selection->get(Format::M4A, Codec::ALAC,
				*default_readers );

		CHECK ( "ffmpeg" == reader->id() );
	}

	SECTION ( "Default settings select ffmpeg for M4A/UNKNOWN" )
	{
		auto reader = default_selection->get(Format::M4A, Codec::UNKNOWN,
				*default_readers );

		CHECK ( "ffmpeg" == reader->id() );
	}


	SECTION ( "Default settings select ffmpeg for APE/MONKEY" )
	{
		auto reader = default_selection->get(Format::APE, Codec::MONKEY,
				*default_readers );

		CHECK ( "ffmpeg" == reader->id() );
	}

	SECTION ( "Default settings select ffmpeg for APE/UNKNOWN" )
	{
		auto reader = default_selection->get(Format::APE, Codec::UNKNOWN,
				*default_readers );

		CHECK ( "ffmpeg" == reader->id() );
	}
}

