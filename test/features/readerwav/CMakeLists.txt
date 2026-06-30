## CMake build script for adding a test suite
## vim:fdm=marker

add_test_suite (libarcsdec_readerwav
	LABEL "readerwav"
	LINK_TARGET libarcsdec_readerwav
	TIMEOUT 10 )

