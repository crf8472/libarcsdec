## CMake build script for configuring and building feature tests
## vim:fdm=marker

add_test_suite (libarcsdec_parserlibcue
	LABEL "parserlibcue"
	TIMEOUT 10 )

## for use on CLI
add_custom_target (libarcsdec_test_parserlibcue
    COMMAND ${CMAKE_CTEST_COMMAND} -L "parserlibcue" --verbose
)

## FIXME this is redundant and already done in parserlibcue
find_package (libcue 2.0.0 REQUIRED )

target_include_directories (libarcsdec_parserlibcue_tests
	PRIVATE ${libcue_INCLUDE_DIRS} )

target_link_libraries      (libarcsdec_parserlibcue_tests
	PRIVATE ${libcue_LIBRARIES} )

