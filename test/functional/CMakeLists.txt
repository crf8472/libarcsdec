## CMake build script for configuring and building functional tests
##
## vim:fdm=marker

cmake_minimum_required (VERSION 3.10 )

add_test_suite (libarcsdec_functional
	LABEL "functional"
	TIMEOUT 10 )

## for use on CLI
add_custom_target (libarcsdec_functional_all
    COMMAND ${CMAKE_CTEST_COMMAND} -L "functionals" --verbose
)


