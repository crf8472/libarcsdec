#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for parsertoc_details.hpp.
 */

#ifndef __LIBARCSDEC_PARSERTOC_HPP__
#define __LIBARCSDEC_PARSERTOC_HPP__ // allow parsertoc_details.hpp
#endif
#ifndef __LIBARCSDEC_PARSERTOC_DETAILS_HPP__
#include "parsertoc_details.hpp"        // TO BE TESTED
#endif


TEST_CASE ("TocParserImpl", "[parsertoc]" )
{
	using arcsdec::details::cdrdao::TocParserImpl;
	//using arcsdec::DescriptorToc;

	auto d = TocParserImpl{}.descriptor();

	// SECTION ("Parser implementation returns correct descriptor type")
	// {
	// 	CHECK ( d );
	// 	auto p = d.get();
	//
	// 	CHECK ( dynamic_cast<const DescriptorToc*>(p) != nullptr );
	// }
}

