## CMake build script for configuring and building feature tests
## vim:fdm=marker

add_test_suite (libarcsdec_parsertoc
	LABEL "parsertoc"
	DEPS_FROM libarcsdec_parsertoc
	TIMEOUT 10 )

## FIXME Should not know the target name
target_include_directories (libarcsdec_parsertoc_tests
	# find cdrtoc_lexer_defs.hpp:
	PRIVATE ${LIBARCSDEC_SOURCE_DIR}/features/parsertoc/src/cdrtoc
	# find cdrtoc.tab.hpp:
	PRIVATE ${LIBARCSDEC_BINARY_DIR}/src/features/parsertoc/src/cdrtoc
)


## for use on CLI
add_custom_target (libarcsdec_test_parsertoc
    COMMAND ${CMAKE_CTEST_COMMAND} -L "parsertoc" --verbose
)

