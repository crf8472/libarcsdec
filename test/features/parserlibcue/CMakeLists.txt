## CMake build script for configuring and building feature tests
## vim:fdm=marker

add_test_suite (libarcsdec_parserlibcue
	LABEL "parserlibcue"
	TIMEOUT 10 )

## for use on CLI
add_custom_target (libarcsdec_test_parserlibcue
    COMMAND ${CMAKE_CTEST_COMMAND} -L "parserlibcue" --verbose
)

