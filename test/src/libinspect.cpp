#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for libinspect.hpp.
 */

#ifndef __LIBARCSDEC_LIBINSPECT_HPP__
#include "libinspect.hpp"               // TO BE TESTED
#endif

#ifndef __LIBARCSDEC_VERSION_HPP__
#include "version.hpp"                  // for LIBARCSDEC_NAME
#endif

#include <regex>                        // for regex, regex_match


TEST_CASE ( "Names of libraries are matched correctly", "[libraries]" )
{
	const auto pattern = arcsdec::details::to_libname_pattern(
			arcsdec::LIBARCSDEC_NAME);


	SECTION ("libnames are correctly escaped")
	{
		auto libname = std::string { "libFLAC++" };
		arcsdec::details::escape(libname, '+', "\\");

		CHECK ( libname == "libFLAC\\+\\+" );

		CHECK ( ".*\\blibFLAC\\+\\+\\.so(\\.[0-9]+)*$"
				== ".*\\b" + libname + "\\.so(\\.[0-9]+)*$");
	}


	SECTION ("libnames match so names")
	{
		const auto patt1 = arcsdec::details::to_libname_pattern("libFLAC++");

		CHECK ( std::regex_match("/usr/lib/libFLAC++.so.6", patt1) );


		auto libname = std::string { "libFLAC++" };
		arcsdec::details::escape(libname, '+', "\\");
		const auto patt2 = std::regex(".*\\b" + libname + "\\.so(\\.[0-9]+)*$",
			std::regex::icase);

		CHECK ( std::regex_match("/usr/lib/libFLAC++.so.6", patt2) );


		const auto patt3 = std::regex(".*\\blibFLAC\\+\\+\\.so(\\.[0-9]+)*$",
			std::regex::icase);

		CHECK ( std::regex_match("/usr/lib/libFLAC++.so.6", patt3) );
	}


	SECTION ("Escaped libname is found in libarcsdec list")
	{
		const auto& list = arcsdec::details::libarcsdec_deps();

		CHECK ( !list.empty() );

		auto so_name = arcsdec::details::first_libname_match(list, "libFLAC++");

		CHECK ( !so_name.empty() );
	}


	SECTION ("libarcsdec shared object pattern is correct")
	{
		CHECK ( std::regex_match("libarcsdec.so", pattern) );
		CHECK ( std::regex_match("libarcsdec.so.9", pattern) );
		CHECK ( std::regex_match("libarcsdec.so.9.8", pattern) );
		CHECK ( std::regex_match("libarcsdec.so.90.8", pattern) );
		CHECK ( std::regex_match("libarcsdec.so.90.845.2", pattern) );

		CHECK ( std::regex_match("LIBARCSDEC.SO", pattern) );
		CHECK ( std::regex_match("LIBARCSDEC.SO.9", pattern) );
		CHECK ( std::regex_match("LIBARCSDEC.SO.9.8", pattern) );
		CHECK ( std::regex_match("LIBARCSDEC.SO.90.8", pattern) );
		CHECK ( std::regex_match("LIBARCSDEC.SO.90.845.2", pattern) );

		CHECK ( std::regex_match("/usr/lib/libarcsdec.so", pattern) );
		CHECK ( std::regex_match("/usr/lib/libarcsdec.so.9", pattern) );
		CHECK ( std::regex_match("/usr/lib/libarcsdec.so.9.8", pattern) );
		CHECK ( std::regex_match("/usr/lib/libarcsdec.so.90.8", pattern) );
		CHECK ( std::regex_match("/usr/lib/libarcsdec.so.90.845.2", pattern) );

		CHECK ( not std::regex_match("foobarlibarcsdec.so", pattern) );
		CHECK ( not std::regex_match("foobarlibarcsdec.so.1", pattern) );
		CHECK ( not std::regex_match("foobarlibarcsdec.so.1.2", pattern) );
		CHECK ( not std::regex_match("foobarlibarcsdec.so.12.23", pattern) );
		CHECK ( not std::regex_match("/usr/lib/foobarlibarcsdec.so", pattern) );
		CHECK ( not std::regex_match("/usr/lib/foobarlibarcsdec.so.1",
					pattern) );
		CHECK ( not std::regex_match("/usr/lib/foobarlibarcsdec.so.1.2",
					pattern) );
		CHECK ( not std::regex_match("/usr/lib/foobarlibarcsdec.so.12.23",
					pattern) );
	}

// Commented out since it would fail when changing configuration switches

/*
	SECTION ("libarcsdec's list of runtime dependencies is complete")
	{
		const auto& list = arcsdec::details::libarcsdec_deps();
		auto found = list.end();

		CHECK ( not list.empty() );

		using std::begin;
		using std::end;

		for (const auto& entry : {
				"libcue",
				"libcdio",
				"libsndfile",
				"libFLAC++",
				"libwavpack",
				"libavcodec",
				"libavformat",
				"libavutil"    })
		{
			found = std::find_if(begin(list), end(list),
				[entry](const std::string &name)
					{ return name.find(entry) != std::string::npos; });

			CHECK ( found != list.end() );
		}
	}
*/
}

