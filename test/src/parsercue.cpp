#include "catch2/catch_test_macros.hpp"

#include <fstream> // for ifstream

/**
 * \file
 *
 * Tests for classes in parsercue.cpp
 */

#ifndef __LIBARCSDEC_PARSERCUE_HPP__
#include "parsercue.hpp"
#endif
#ifndef __LIBARCSDEC_PARSERLIBCUE_DETAILS_HPP__
#include "parsercue_details.hpp"
#endif
#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"
#endif

// Implementation details:

#ifndef __LIBARCSDEC_CUESHEET_DRIVER_HPP__
#include "cuesheet/driver.hpp"
#endif
#ifndef __LIBARCSDEC_CUESHEET_TOCHANDLER_HPP__
#include "cuesheet/tochandler.hpp"
#endif


TEST_CASE ("DescriptorCuesheet", "[parsercue]" )
{
	using arcsdec::DescriptorCuesheet;
	using arcsdec::Format;
	using arcsdec::Codec;

	auto d = arcsdec::DescriptorCuesheet{};

	SECTION ("Returns own name correctly")
	{
		CHECK ( "CueSheet" == d.name() );
	}

	SECTION ("Returns linked libraries correctly")
	{
		const auto libs = d.libraries();

		CHECK ( libs.size() == 0 );
	}

	SECTION ("Does not match any codecs not accepted by this descriptor")
	{
		CHECK ( !d.accepts(Codec::UNKNOWN) );
		CHECK ( !d.accepts(Codec::PCM_S16BE) );
		CHECK ( !d.accepts(Codec::PCM_S16BE_PLANAR) );
		CHECK ( !d.accepts(Codec::PCM_S16LE) );
		CHECK ( !d.accepts(Codec::PCM_S16LE_PLANAR) );
		CHECK ( !d.accepts(Codec::PCM_S32BE) );
		CHECK ( !d.accepts(Codec::PCM_S32BE_PLANAR) );
		CHECK ( !d.accepts(Codec::PCM_S32LE) );
		CHECK ( !d.accepts(Codec::PCM_S32LE_PLANAR) );
		CHECK ( !d.accepts(Codec::FLAC) );
		CHECK ( !d.accepts(Codec::WAVPACK) );
		CHECK ( !d.accepts(Codec::MONKEY) );
		CHECK ( !d.accepts(Codec::ALAC) );
	}

	SECTION ("Returns accepted codecs correctly")
	{
		CHECK ( d.codecs() == std::set<Codec>{ } );
	}

	SECTION ("Returns no codecs that are not accepted")
	{
		CHECK ( d.codecs().size() == 0 );
	}

	SECTION ("Matches accepted formats correctly")
	{
		CHECK ( d.accepts(Format::CUE) );
	}

	SECTION ("Does not match any formats not accepted by this descriptor")
	{
		CHECK ( !d.accepts(Format::UNKNOWN) );
		CHECK ( !d.accepts(Format::CDRDAO)  );
		CHECK ( !d.accepts(Format::WAV)     );
		CHECK ( !d.accepts(Format::FLAC)    );
		CHECK ( !d.accepts(Format::APE)     );
		CHECK ( !d.accepts(Format::CAF)     );
		CHECK ( !d.accepts(Format::M4A)     );
		CHECK ( !d.accepts(Format::OGG)     );
		CHECK ( !d.accepts(Format::WV)      );
		CHECK ( !d.accepts(Format::AIFF)    );
	}

	SECTION ("Returns accepted formats correctly")
	{
		CHECK ( d.formats() == std::set<Format>{ Format::CUE } );
	}

	// TODO Does create_reader() really create a CueParserImpl?
}


TEST_CASE ("cuesheet", "[yycuesheet]" )
{
	using arcsdec::details::cuesheet::Driver;
	using arcsdec::details::cuesheet::TOCHandler;

	Driver driver;
	TOCHandler handler;

	driver.set_handler(handler);

	/* Activate debugging for flex-generated scanner */
	driver.set_lexer_debug_level(0);
	// Note: use --debug when running flex on cuesheet.l to have debug support

	/* Activate debugging for bison-generated parser */
	//driver.set_parser_debug_level(0);
	// Note: use --debug when running bison on cuesheet.y to have debug support

	SECTION ("Cuesheet without syntax errors and trailing newline is OK")
	{
		std::ifstream file;
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			file.open("cuesheet/ok01.cue", std::ifstream::in);
		}
		catch (const std::ifstream::failure& f)
		{
			throw std::runtime_error(
				std::string { "Failed to open file "
					"'cuesheet/ok01.cue', got: " }
				+ typeid(f).name()
				+ ", message: " + f.what());
		}

		driver.set_input(file);
		const int result { driver.parse() };

		CHECK ( result == 0 );
	}

	SECTION ("Cuesheet without syntax errors and no newline is OK")
	{
		std::ifstream file;
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			file.open("cuesheet/ok02.cue", std::ifstream::in);
		}
		catch (const std::ifstream::failure& f)
		{
			throw std::runtime_error(
				std::string { "Failed to open file "
					"'cuesheet/ok02.cue', got: " }
				+ typeid(f).name()
				+ ", message: " + f.what());
		}

		driver.set_input(file);
		const int result { driver.parse() };

		CHECK ( result == 0 );
	}

	SECTION ("Cuesheet without syntax errors and no newline is OK")
	{
		std::ifstream file;
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			file.open("cuesheet/ok03.cue", std::ifstream::in);
		}
		catch (const std::ifstream::failure& f)
		{
			throw std::runtime_error(
				std::string { "Failed to open file "
					"'cuesheet/ok03.cue', got: " }
				+ typeid(f).name()
				+ ", message: " + f.what());
		}

		driver.set_input(file);
		const int result { driver.parse() };

		CHECK ( result == 0 );

		CHECK ( handler.offsets().size() == 15 );
		CHECK ( handler.offsets() == std::vector<int32_t>{
					  33,
					5225,
					7390,
				   23380,
				   35608,
				   49820,
				   69508,
				   87733,
				  106333,
				  139495,
				  157863,
				  198495,
				  213368,
				  225320,
				  234103
				});
		CHECK ( handler.lengths().size() == 15 );
		CHECK ( handler.lengths() == std::vector<int32_t>{
					5192,
					2165,
				   15990,
				   12228,
				   14212,
				   19688,
				   18225,
				   18600,
				   33162,
				   18368,
				   40632,
				   14873,
				   11952,
					8783,
				      -1 //18935 // not from Cuesheet
				});
	}
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


	SECTION ( "Descriptor 'cuesheet' is registered" )
	{
		CHECK ( nullptr != arcsdec::FileReaderRegistry::reader("cuesheet") );
	}
}

