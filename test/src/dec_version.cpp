#include "catch2/catch_test_macros.hpp"


#ifndef __LIBARCSDEC_VERSION_HPP__
#include "version.hpp"
#endif

/**
 * \file Fixtures for functions in version.cpp
 */


TEST_CASE ( "constants", "[version]" )
{
	CHECK ( arcsdec::v_1_0_0::LIBARCSDEC_NAME == "libarcsdec" );

	CHECK ( arcsdec::v_1_0_0::LIBARCSDEC_VERSION_MAJOR  == 0 );
	CHECK ( arcsdec::v_1_0_0::LIBARCSDEC_VERSION_MINOR  == 1 );
	CHECK ( arcsdec::v_1_0_0::LIBARCSDEC_VERSION_PATCH  == 1 );
	CHECK ( arcsdec::v_1_0_0::LIBARCSDEC_VERSION_SUFFIX == "alpha.1" );
	CHECK ( arcsdec::v_1_0_0::LIBARCSDEC_VERSION        == "0.1.1-alpha.1" );
}


TEST_CASE ( "functions", "[version]" )
{
	using arcsdec::v_1_0_0::api_version_is_at_least;

	SECTION ("api_version_is_at_least")
	{
		CHECK (  api_version_is_at_least(0, 0, 0) );
		CHECK (  api_version_is_at_least(0, 0, 1) );
		CHECK (  api_version_is_at_least(0, 0, 2) );
		CHECK (  api_version_is_at_least(0, 1, 0) );
		CHECK (  api_version_is_at_least(0, 1, 1) ); // <= this version
		CHECK ( !api_version_is_at_least(0, 2, 0) );
		CHECK ( !api_version_is_at_least(0, 2, 1) );
		CHECK ( !api_version_is_at_least(1, 0, 0) );
		CHECK ( !api_version_is_at_least(1, 1, 0) );
		CHECK ( !api_version_is_at_least(1, 1, 1) );
		CHECK ( !api_version_is_at_least(2, 1, 1) );
	}
}

