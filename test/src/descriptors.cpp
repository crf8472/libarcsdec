#include "catch2/catch.hpp"

#include <algorithm>
#include <regex>
#include <iostream>

#ifndef __LIBARCSDEC_DESCRIPTORS_HPP__
#include "descriptors.hpp"
#endif
#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"
#endif
#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include "metaparser.hpp"
#endif
#ifndef __LIBARCSDEC_VERSION_HPP__
#include "version.hpp"
#endif

/**
 * \file descriptors.cpp Tests for all API classes exported by descriptors.hpp
 */

TEST_CASE ( "Load runtime dependencies", "" )
{
	const auto& pattern = arcsdec::details::libname_pattern(
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
		auto patt1 = arcsdec::details::libname_pattern("libFLAC++");

		CHECK ( std::regex_match("/usr/lib/libFLAC++.so.6", patt1) );


		auto libname = std::string { "libFLAC++" };
		arcsdec::details::escape(libname, '+', "\\");
		auto patt2 = std::regex(".*\\b" + libname + "\\.so(\\.[0-9]+)*$",
			std::regex::icase);

		CHECK ( std::regex_match("/usr/lib/libFLAC++.so.6", patt2) );


		auto patt3 = std::regex(".*\\blibFLAC\\+\\+\\.so(\\.[0-9]+)*$",
			std::regex::icase);

		CHECK ( std::regex_match("/usr/lib/libFLAC++.so.6", patt3) );
	}


	SECTION ("Escaped libname is found in list")
	{
		const auto& list = arcsdec::details::libarcsdec_libs();

		CHECK ( !list.empty() );

		auto so_name = arcsdec::details::find_lib(list, "libFLAC++");

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

	SECTION ("libarcsdec runtime dependency list")
	{
		const auto& list = arcsdec::details::libarcsdec_libs();

		CHECK ( not list.empty() );

		for (const auto& entry : { "libcue", "libFLAC++", "libwavpack",
				"libavcodec", "libavformat", "libavutil" } )
		{
			auto found = std::find_if(list.begin(), list.end(),
				[entry](const std::string &name){ return name.find(entry); });

			CHECK ( found != list.end() );
		}
	}
}


TEST_CASE ( "List audio descriptors", "[audioreaderselection]" )
{
	SECTION ( "" )
	{
	}
}


TEST_CASE ( "List metadata descriptors", "[metadataparserselection]" )
{
	SECTION ( "" )
	{
	}
}

