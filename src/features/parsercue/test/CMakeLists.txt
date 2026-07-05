## CMake build script for configuring and building feature tests
## vim:fdm=marker

add_test_suite (libarcsdec_parsercue
	LABEL "parsercue"
	DEPS_FROM libarcsdec_parsercue
	TIMEOUT 10 )

## FIXME Should not know the target name
target_include_directories (libarcsdec_parsercue_tests
	# find cuesheet_lexer_defs.hpp:
	#PRIVATE ${LIBARCSDEC_SOURCE_DIR}/features/parsercue/src/cuesheet
	# find cuesheet.tab.hpp:
	PRIVATE ${LIBARCSDEC_BINARY_DIR}/src/features/parsercue/src/cuesheet
)


## for use on CLI
add_custom_target (libarcsdec_test_parsercue
    COMMAND ${CMAKE_CTEST_COMMAND} -L "parsercue" --verbose
)

