//
// Example for calculating AccurateRip checksums from each track of an album,
// represented by a Cuesheet and a single losslessly encoded audio file.
//

#ifndef __LIBARCSDEC_CALCULATORS_HPP__ // for ToCParser, ARCSCalculator
#include <arcsdec/calculators.hpp>
#endif

#ifndef __LIBARCSTK_IDENTIFIER_HPP__   // for ARId
#include <arcstk/identifier.hpp>
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__      // libarcstk: log what you do
#include <arcstk/logging.hpp>
#endif

#include <iomanip>   // for setw, setfill, hex
#include <iostream>  // for cerr, cout
#include <memory>    // for make_unique
#include <string>    // for string


// ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! !
// NOTE! THIS IS EXAMPLE CODE! IT IS INTENDED TO DEMONSTRATE HOW LIBARCSDEC
// COULD BE USED. IT IS NOT INTENDED TO BE USED IN REAL LIFE PRODUCTION. IT IS
// IN NO WAY TESTED FOR PRODUCTION. TAKE THIS AS A STARTING POINT TO YOUR OWN
// SOLUTION, NOT AS A TOOL.
// ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! !


int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::cout << "Usage: albumid <cuesheet> <audiofile>" << '\n';
		return EXIT_SUCCESS;
	}

	// Of course you would validate your input parameters in production code.
	const std::string metafilename  { argv[1] };
	const std::string audiofilename { argv[2] };

	// If you like, you can activate the internal logging of libarcstk to
	// see what's going on behind the scenes. We provide an appender for stdout
	// and set the loglevel to 'INFO', which means you should probably not see
	// anything unless you give libarcstk unexpected input.
	arcstk::Logging::instance().add_appender(
			std::make_unique<arcstk::Appender>("stdout", stdout));

	// Set this to DEBUG or DEBUG1 if you want to see what libarcsdec and
	// libarcstk are doing with your input.
	arcstk::Logging::instance().set_level(arcstk::LOGLEVEL::WARNING);
	arcstk::Logging::instance().set_timestamps(false); // We do not need them

	// Parse the metadata file.
	// Actually, this step is completely format independent and not restricted
	// to Cuesheets. We require a Cuesheet for this example since at the time of
	// writing, Cuesheet is the only actual input format implemented. :-)
	arcsdec::ToCParser parser;
	const auto tocptr { parser.parse(metafilename) };

	// Read the audio file and calculate the result.
	// Note that technical details of the audio input are "abstracted away" by
	// libarcsdec. ARCSCalculator takes some audio and gives you the ARCSs.
	arcsdec::ARIdCalculator calculator;
	const auto id { calculator.calculate(*tocptr, audiofilename) };

	// Print the ARId.
	using std::string;
	std::cout << "ID:          " << to_string(*id) << '\n';
	std::cout << "Filename:    " << id->filename() << '\n';
	std::cout << "Request-URL: " << id->url()      << '\n';

	return EXIT_SUCCESS;
}

