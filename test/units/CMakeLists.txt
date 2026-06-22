## CMake build script for configuring and building unit tests
##
## vim:fdm=marker

cmake_minimum_required (VERSION 3.10 )

add_test_suite (libarcsdec_unit
	LABEL "unit"
	TIMEOUT 10 )

## for use on CLI
add_custom_target (libarcsdec_unit_all
    COMMAND ${CMAKE_CTEST_COMMAND} -L "units" --verbose
)

