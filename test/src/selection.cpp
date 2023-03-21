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

	SECTION ( "Mandatory descriptors are registered" )
	{
		// At least the 2 non-optional descriptors:
		// Maybe not each available reader was compiled, but we will always have
		// the genuine wav reader + libcue-based cuesheet parser
		CHECK ( 2 <= arcsdec::FileReaderRegistry::readers()->size() );
		// Specific tests are in parsercue.cpp and readerwav.cpp
	}
}


//TEST_CASE ("FormatCue", "[parsercue]" )
//{
//	auto f = arcsdec::FormatCue{};
//
//	SECTION ("Returns own name correctly")
//	{
//		CHECK ( "cue" == f.name() );
//	}
//
//	SECTION ("Matches accepted bytes correctly")
//	{
//		CHECK ( f.bytes({}, 0) );
//		CHECK ( f.bytes({3, 2, 1}, 2) );
//		CHECK ( f.bytes({0x65, 0x32, 0x88}, 1) );
//		// TODO Check for always true
//	}
//
//	SECTION ("Matches accepted filenames correctly")
//	{
//		CHECK ( f.filename("foo.cue") );
//		CHECK ( f.filename("bar.CUE") );
//		CHECK ( f.filename("bar.CUe") );
//
//		CHECK ( !f.filename("bar.rcue") );
//		CHECK ( !f.filename("bar.PCUe") );
//
//		CHECK ( !f.filename("bar.cuef") );
//		CHECK ( !f.filename("bar.CUEl") );
//	}
//}


//TEST_CASE ("FormatToc", "[parsertoc]" )
//{
//	auto f = arcsdec::FormatToc{};
//
//	SECTION ("Matches accepted bytes correctly")
//	{
//		CHECK ( f.bytes({}, 0) );
//		CHECK ( f.bytes({3, 2, 1}, 2) );
//		CHECK ( f.bytes({0x65, 0x32, 0x88}, 1) );
//		// TODO Check for always true
//	}
//
//	SECTION ("Matches accepted filenames correctly")
//	{
//		CHECK ( f.filename("foo.toc") );
//		CHECK ( f.filename("bar.TOC") );
//		CHECK ( f.filename("bar.TOc") );
//
//		CHECK ( !f.filename("bar.rtoc") );
//		CHECK ( !f.filename("bar.PTOc") );
//
//		CHECK ( !f.filename("bar.tocf") );
//		CHECK ( !f.filename("bar.TOCl") );
//	}
//}


//TEST_CASE ("FormatWavPCM", "[readerwav]" )
//{
//	using arcsdec::details::wave::RIFFWAV_PCM_CDDA_t;
//	auto d = arcsdec::FormatWavPCM {};
//
//	SECTION ("Matches accepted bytes correctly")
//	{
//		RIFFWAV_PCM_CDDA_t w;
//
//		CHECK ( not d.bytes( {}, 0 ));
//		CHECK ( not d.bytes( {}, 12 ));
//		CHECK ( not d.bytes( {}, 45 ));
//		CHECK ( not d.bytes( {}, 145 ));
//
//		// wav-header (0-11)
//		CHECK (     d.bytes( {'R', 'I', 'F', 'F'}, 0) );
//		CHECK ( not d.bytes( {'R', 'I', 'F', 'F'}, 3) );
//		CHECK (     d.bytes( {'I', 'F', 'F'}, 1) );
//		CHECK ( not d.bytes( {'I', 'F', 'F'}, 2) );
//		CHECK (     d.bytes( {'W', 'A', 'V', 'E'}, 8) );
//		CHECK ( not d.bytes( {'W', 'A', 'V', 'E'}, 9) );
//
//		// 'fmt ' (12-33)
//		CHECK (     d.bytes( {'f', 'm', 't', ' '}, 12) );
//		CHECK ( not d.bytes( {'f', 'm', 't', '_'}, 12) );
//		// size == 16, wFormatTag == 1, Channels == 2, dwSamplesPerSec = 44.100
//		CHECK ( d.bytes( { 16, 0, 0, 0, 1, 0, 2, 0, 68, 172, 0, 0 },
//				16) );
//		CHECK ( not d.bytes( { 16, 1, 0, 0, 1, 1, 2, 1, 68, 173, 0, 0 },
//				16) );
//		CHECK ( d.bytes( { 68, 172, 0, 0}, 24));
//		// dwAvgBytesPerSec == 176400, wBlockAlign  == 4
//		CHECK ( d.bytes( { 16, 177, 2, 0, 4, 0 }, 28));
//		CHECK ( not d.bytes( { 16, 177, 2, 1, 5, 0 }, 28));
//		// wBitsPerSample == 16
//		CHECK ( d.bytes( { 16, 0 }, 34));
//		CHECK ( not d.bytes( { 16, 1 }, 34));
//		CHECK ( not d.bytes( { 17, 0 }, 34));
//
//		CHECK ( not d.bytes( { 0, 0, 0, 16, 0, 1, 0, 2, 0, 0 }, 15) );
//		CHECK ( not d.bytes( { 0, 0, 0, 16, 0, 1, 1, 2, 0, 0 }, 16) );
//		CHECK ( not d.bytes( { 16, 176, 2, 0, 4, 0 }, 28));
//		CHECK ( not d.bytes( { 16, 176, 2, 0, 5, 0 }, 28));
//
//		// Accepts any declared file size?
//
//		CHECK (     d.bytes( {' ', ' ', ' ', ' ' }, 4) );
//		CHECK (     d.bytes( {' ', ' ', ' ', ' ', 'W' }, 4) );
//		CHECK ( not d.bytes( {' ', ' ', ' ', ' ', 'T' }, 4) );
//		CHECK (     d.bytes( {'I', 'F', 'F', ' ', ' ', ' ', ' ', 'W' },
//					1) );
//		CHECK (     d.bytes( {'I', 'F', 'F', '1', '2', '3', '4', 'W' },
//					1) );
//		CHECK ( not d.bytes( {'I', 'F', 'F', ' ', ' ', ' ', ' ', 'X' },
//					1) );
//
//		// Accepts any declared data chunk size?
//
//		CHECK (     d.bytes( {' ', ' ', ' ', ' ' }, 40) );
//		CHECK (     d.bytes( {' ', ' ', ' ', ' ', '%' }, 40) );
//		CHECK (     d.bytes( {'a', 't', 'a', ' ', ' ', ' ', ' ', 'W' },
//					37) );
//		CHECK (     d.bytes( {'a', 't', 'a', '1', '2', '3', '4', 'T' },
//					37) );
//		CHECK ( not d.bytes( {'a', 't', 'i', ' ', ' ', ' ', ' ', 'X' },
//					37) );
//		CHECK (     d.bytes( {'a', 't', 'a', '1', '2', '3', '4' },
//					37) );
//	}
//
//	SECTION ("Matches accepted filenames correctly")
//	{
//		CHECK ( d.filename("foo.wav") );
//		CHECK ( d.filename("bar.WAV") );
//		CHECK ( d.filename("foo.wave") );
//		CHECK ( d.filename("bar.WAVE") );
//		CHECK ( d.filename("foo.wAvE") );
//		CHECK ( d.filename("bar.Wave") );
//
//		CHECK ( not d.filename("bar.WAVX") );
//		CHECK ( not d.filename("bar.wavx") );
//		CHECK ( not d.filename("bar.waving") );
//		CHECK ( not d.filename("bar.warg") );
//		CHECK ( not d.filename("bar.walar") );
//		CHECK ( not d.filename("bar.WALINOR") );
//		CHECK ( not d.filename("bar.PWAV") );
//		CHECK ( not d.filename("bar.pwav") );
//		CHECK ( not d.filename("bar.CWAVE") );
//		CHECK ( not d.filename("bar.cwave") );
//	}
//}


//TEST_CASE ("FormatFlac", "[readerflac]" )
//{
//	auto f = arcsdec::FormatFlac{};
//
//	SECTION ("Matches accepted bytes correctly")
//	{
//		CHECK ( f.bytes({ 0x66, 0x4C, 0x61, 0x43 }, 0) );
//	}
//
//	SECTION ("Matches names correctly")
//	{
//		CHECK ( f.filename("foo.flac") );
//		CHECK ( f.filename("bar.FLAC") );
//		CHECK ( f.filename("bar.FlAc") );
//
//		CHECK ( !f.filename("bar.rflac") );
//		CHECK ( !f.filename("bar.PFLac") );
//
//		CHECK ( !f.filename("bar.flacr") );
//		CHECK ( !f.filename("bar.FLACD") );
//	}
//}


//TEST_CASE ("FormatWavpack", "[readerwvpk]" )
//{
//	auto f = arcsdec::FormatWavpack {};
//
//	SECTION ("Matches accepted bytes correctly")
//	{
//		CHECK ( f.bytes({ 0x77, 0x76, 0x70, 0x6B }, 0) );
//	}
//
//	SECTION ("Matches accepted filenames correctly")
//	{
//		CHECK ( f.filename("foo.wv") );
//		CHECK ( f.filename("bar.WV") );
//
//		CHECK ( !f.filename("bar.WAV") );
//		CHECK ( !f.filename("bar.wav") );
//
//		CHECK ( !f.filename("bar.rwv") );
//		CHECK ( !f.filename("bar.RWV") );
//
//		CHECK ( !f.filename("bar.wvx") );
//		CHECK ( !f.filename("bar.WVX") );
//	}
//}

