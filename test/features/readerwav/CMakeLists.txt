## CMake build script for adding a test suite
## vim:fdm=marker

add_test_suite (libarcsdec_readerwav
	LABEL "readerwav"
	DEPS_FROM libarcsdec_readerwav
	TIMEOUT 10 )

