#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Testcases for libinspect.hpp with readerflac present.
 */

#ifndef LIBARCSDEC_LIBINSPECT_HPP_
#include "libinspect.hpp"               // TO BE TESTED
#endif

#include <regex>                        // for regex, regex_match


TEST_CASE ( "lib name of libFLAC++ is correct" )
{
	const auto flac_name = std::string { "libFLAC++" };

	auto libname_tmp = flac_name;
	arcsdec::read::details::escape(libname_tmp, '+', "\\");

	const auto libname = libname_tmp;

	REQUIRE ( libname == "libFLAC\\+\\+" );



	SECTION ("name of libFLAC++ is correctly escaped")
	{
		CHECK ( ".*\\blibFLAC\\+\\+\\.so(\\.[0-9]+)*$"
				== ".*\\b" + libname + "\\.so(\\.[0-9]+)*$");
	}


	SECTION ("name pattern of libFLAC++ matches typical so names")
	{
		const auto patt1 =
			arcsdec::read::details::to_libname_pattern("libFLAC++");

		CHECK ( std::regex_match("/usr/lib/libFLAC++.so.6", patt1) );

		const auto patt2 = std::regex(".*\\b" + libname + "\\.so(\\.[0-9]+)*$",
			std::regex::icase);

		CHECK ( std::regex_match("/usr/lib/libFLAC++.so.6", patt2) );

		const auto patt3 = std::regex(".*\\blibFLAC\\+\\+\\.so(\\.[0-9]+)*$",
			std::regex::icase);

		CHECK ( std::regex_match("/usr/lib/libFLAC++.so.6", patt3) );
	}
}

TEST_CASE ( "libinspect finds libFLAC++ dependency" )
{
	const auto& list = arcsdec::read::details::libarcsdec_deps();

	REQUIRE ( !list.empty() );

	SECTION ("Name of libFLAC++ is found in libarcsdec list")
	{
		// this checks escaped name
		auto so_name =
			arcsdec::read::details::first_libname_match(list, "libFLAC++");

		CHECK ( !so_name.empty() );
	}
}

