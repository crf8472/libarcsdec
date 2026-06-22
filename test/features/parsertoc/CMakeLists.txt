## CMake build script for configuring and building feature tests
## vim:fdm=marker

add_test_suite (libarcsdec_parsertoc
	LABEL "parsertoc"
	TIMEOUT 10 )

## FIXME Should not know the target name
target_include_directories (libarcsdec_parsertoc_tests
	PRIVATE ${LIBARCSDEC_BINARY_DIR}/src/parsertoc/cuesheet )


## for use on CLI
add_custom_target (libarcsdec_test_parsertoc
    COMMAND ${CMAKE_CTEST_COMMAND} -L "parsertoc" --verbose
)

