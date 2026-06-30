## CMake build script for adding a test suite
## vim:fdm=marker

add_test_suite (libarcsdec_readerwvpk
	LABEL "readerwvpk"
	LINK_TARGET libarcsdec_readerwvpk
	TIMEOUT 10 )

## FIXME redundant
find_package (libwavpack 5.0.0 REQUIRED )

target_include_directories (libarcsdec_readerwvpk_tests
	PRIVATE ${libwavpack_INCLUDE_DIRS} )

target_link_libraries      (libarcsdec_readerwvpk_tests
	PRIVATE ${libwavpack_LIBRARIES} )

