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

#include <algorithm>   // for find_if
#include <type_traits>

/**
 * \file
 *
 * Tests for all API classes exported by selection.hpp
 */


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


TEST_CASE ( "DefaultSelector", "[defaultselector]")
{
	using arcsdec::DefaultSelector;

/*
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
*/

	// TODO Test for move constructor
}


TEST_CASE ( "FileReaderRegistry", "[filereaderregistry]")
{
	using arcsdec::FileReaderRegistry;
	using arcsdec::DescriptorWavPCM;
	using DescriptorTestType = arcsdec::RegisterDescriptor<DescriptorWavPCM>;


	SECTION ( "Copy constructor and assignment operator are available" )
	{
		// not in base class
		CHECK ( std::is_copy_constructible<FileReaderRegistry>::value );
		CHECK ( std::is_copy_assignable<FileReaderRegistry>::value );

		// available in subclass
		CHECK ( std::is_copy_constructible<DescriptorTestType>::value );
		CHECK ( std::is_copy_assignable<DescriptorTestType>::value );
	}

	SECTION ( "Move constructor and move assignment operator are present" )
	{
		// in base class
		CHECK ( std::is_nothrow_move_constructible<FileReaderRegistry>::value );
		CHECK ( std::is_nothrow_move_assignable<FileReaderRegistry>::value );

		// available in subclass
		CHECK ( std::is_nothrow_move_constructible<DescriptorTestType>::value );
		CHECK ( std::is_nothrow_move_assignable<DescriptorTestType>::value );
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

	SECTION ( "Registered descriptors are present" )
	{
		// at least 2 readers:
		// Maybe not each available reader was compiled, but we will always have
		// the genuine wav reader + libcue-based cuesheet parser
		CHECK ( 2 <= arcsdec::FileReaderRegistry::readers()->size() );

		// in case each parser + each reader is compiled
		CHECK ( 7 >= arcsdec::FileReaderRegistry::readers()->size() );

		CHECK ( nullptr != arcsdec::FileReaderRegistry::reader("flac") );
		CHECK ( nullptr != arcsdec::FileReaderRegistry::reader("ffmpeg") );
		CHECK ( nullptr != arcsdec::FileReaderRegistry::reader("libsndfile") );
		CHECK ( nullptr != arcsdec::FileReaderRegistry::reader("wavpcm") );
	}
}


// TODO
//TEST_CASE ( "RegisterFormat", "[registerformat]")
//{
//}


TEST_CASE ( "RegisterDescriptor", "[registerdescriptor]")
{
	using arcsdec::RegisterDescriptor;
	using arcsdec::DescriptorWavPCM;
	using arcsdec::DescriptorCue;

	// at least 9 stock formats
	REQUIRE ( 9 <= arcsdec::FileReaderRegistry::formats()->size() );
	REQUIRE ( 0  < arcsdec::FileReaderRegistry::formats()->size() );

/*
	SECTION ( "RegisterAudioDescriptor<> is final" )
	{
		using WavPCMRegisterType = RegisterDescriptor<DescriptorWavPCM>;

		CHECK ( std::is_final<WavPCMRegisterType>::value );
		CHECK ( 7 >= arcsdec::FileReaderRegistry::readers()->size() );
	}

	SECTION ( "RegisterMetadataDescriptor<> is final" )
	{
		using CueRegisterType = RegisterDescriptor<DescriptorCue>;

		CHECK ( std::is_final<CueRegisterType>::value );
		CHECK ( 7 >= arcsdec::FileReaderRegistry::readers()->size() );
	}
*/
}


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


	SECTION ( "Default settings select wavpcm for RIFFWAVE/PCM16LE" )
	{
		auto reader = default_selection->get(Format::WAV, Codec::PCM_S16LE,
				*default_readers );

		CHECK ( "wavpcm" == reader->id() );
	}

	SECTION ( "Default settings select wavpcm for RIFFWAVE/UNKNOWN" )
	{
		auto reader = default_selection->get(Format::WAV, Codec::UNKNOWN,
				*default_readers );

		CHECK ( "wavpcm" == reader->id() );
	}


	SECTION ( "Default settings select flac for FLAC/FLAC" )
	{
		auto reader = default_selection->get(Format::FLAC, Codec::FLAC,
				*default_readers );

		CHECK ( "flac" == reader->id() );
	}

	SECTION ( "Default settings select flac for FLAC/UNKNOWN" )
	{
		auto reader = default_selection->get(Format::FLAC, Codec::UNKNOWN,
				*default_readers );

		CHECK ( "flac" == reader->id() );
	}


	SECTION ( "Default settings select wavpack for WV/Wavpack" )
	{
		auto reader = default_selection->get(Format::WV, Codec::WAVPACK,
				*default_readers );

		CHECK ( "wavpack" == reader->id() );
	}

	SECTION ( "Default settings select wavpack for WV/UNKNOWN" )
	{
		auto reader = default_selection->get(Format::WV, Codec::UNKNOWN,
				*default_readers );

		CHECK ( "wavpack" == reader->id() );
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


	SECTION ( "Default settings select libsndfile for AIFF/PCM_S16LE" )
	{
		auto reader = default_selection->get(Format::AIFF, Codec::PCM_S16LE,
				*default_readers );

		CHECK ( "libsndfile" == reader->id() );
	}

	SECTION ( "Default settings select libsndfile for AIFF/UNKNOWN" )
	{
		auto reader = default_selection->get(Format::AIFF, Codec::UNKNOWN,
				*default_readers );

		CHECK ( "libsndfile" == reader->id() );
	}
}

