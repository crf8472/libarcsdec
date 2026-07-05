## CMake build script for configuring and building feature tests
## vim:fdm=marker

add_test_suite (libarcsdec_readerffmpeg
	LABEL "readerffmpeg"
	DEPS_FROM libarcsdec_readerffmpeg
	TIMEOUT 10 )

