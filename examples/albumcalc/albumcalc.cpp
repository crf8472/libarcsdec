//
// Example for calculating AccurateRip checksums from each track of an album,
// represented by a Cuesheet and a single losslessly encoded audio file.
//

#ifndef __LIBARCSDEC_CALCULATORS_HPP__ // libarcsdec: ToCParser, ARCSCalculator
#include <arcsdec/calculators.hpp>
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
		std::cout << "Usage: albumcalc <cuesheet> <audiofile>" << '\n';
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
	auto tocptr { parser.parse(metafilename) };

	// Read the audio file and calculate the checksums.
	// Note that technical details of the audio input are "abstracted away" by
	// libarcsdec. ARCSCalculator takes some audio and gives you the ARCSs.
	arcsdec::ARCSCalculator calculator;
	const auto [ checksums, toc ] =
		calculator.calculate(audiofilename, *tocptr);

	// The result is a tuple containing the checksums as well as the completed
	// ToC. While we had to test tocptr for completeness before using it, the
	// ToC object returned by the calculator is guaranteed to be suitable to use
	// it to get the AccurateRip id.
	const auto arid { arcstk::make_arid(toc) };

	// We print both to the command line. Of course you can use the URL to
	// request the reference values and then verify them with one of
	// libarcstk's Matchers or just parse them to plaintext.

	// Print the ARId.
	std::cout << "AccurateRip URL: " << arid->url() << '\n';

	// Print the actual checksums.
	std::cout << "Track  ARCSv1    ARCSv2" << '\n';
	int trk_no = 1;

	using arcstk::checksum::type;

	for (const auto& track_values : checksums)
	{
		auto arcs1 = track_values.get(type::ARCS1);
		auto arcs2 = track_values.get(type::ARCS2);

		std::cout << std::dec << " " << std::setw(2) << std::setfill(' ')
			<< trk_no << "   " << std::hex << std::uppercase
			<< std::setw(8) << std::setfill('0') << arcs1.value()
			<< "  "
			<< std::setw(8) << std::setfill('0') << arcs2.value()
			<< '\n';

		++trk_no;
	}
}

