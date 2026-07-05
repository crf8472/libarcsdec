#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for parsertoc_details.hpp.
 */

#ifndef LIBARCSDEC_PARSERTOC_HPP_
#define LIBARCSDEC_PARSERTOC_HPP_      // allow parsertoc_details.hpp
#endif
#ifndef LIBARCSDEC_PARSERTOC_DETAILS_HPP_
#include "parsertoc_details.hpp"        // TO BE TESTED
#endif


TEST_CASE ("TocParserImpl", "[parsertoc]" )
{
	using arcsdec::read::details::cdrtoc::TocParserImpl;
	//using arcsdec::read::DescriptorToc;

	auto d = TocParserImpl{}.descriptor();

	// SECTION ("Parser implementation returns correct descriptor type")
	// {
	// 	CHECK ( d );
	// 	auto p = d.get();
	//
	// 	CHECK ( dynamic_cast<const DescriptorToc*>(p) != nullptr );
	// }
}

