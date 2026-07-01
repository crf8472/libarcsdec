## CMake build script for configuring and building feature tests
## vim:fdm=marker

add_test_suite (libarcsdec_x_parserlibcue
	LABEL       "parserlibcue"
	DEPS_FROM libarcsdec_parserlibcue
	TIMEOUT 10
)

## Example (commented out): for use on CLI
#add_custom_target (libarcsdec_test_parserlibcue
#    COMMAND ${CMAKE_CTEST_COMMAND} -L "parserlibcue" --verbose
#)

