## CMake build script for configuring and building feature tests
## vim:fdm=marker

add_test_suite (libarcsdec_parsercue
	LABEL "parsercue"
	TIMEOUT 10 )

## FIXME Should not know the target name
target_include_directories (libarcsdec_parsercue_tests
	PRIVATE ${LIBARCSDEC_BINARY_DIR}/src/parsercue/cuesheet )


## for use on CLI
add_custom_target (libarcsdec_test_parsercue
    COMMAND ${CMAKE_CTEST_COMMAND} -L "parsercue" --verbose
)

