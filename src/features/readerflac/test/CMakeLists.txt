## CMake build script for adding a test suite
## vim:fdm=marker

add_test_suite (libarcsdec_readerflac
	LABEL "readerflac"
	DEPS_FROM libarcsdec_readerflac
	TIMEOUT 10 )

## FLAC dependency details belong here, but we lack some information

# ## FIXME Find a way to avoid doing find_package() again
# ## We already did this in top-level CMakeLists.txt and could pass it from there
# find_package (Threads       REQUIRED )
# find_package (FLAC    1.3.1 REQUIRED ) ## checks libFLAC and libFLAC++
#
# ## FIXME Should not know target name detail libarcsdec_readerflac_tests here
# ## We should pass the information about libs and headers to add_test_suite()
# if (TARGET FLAC::FLAC++ )
#
# 	target_link_libraries (libarcsdec_readerflac_tests PRIVATE FLAC::FLAC++ )
#
# else()
#
# 	target_include_directories (libarcsdec_readerflac_tests
# 		PRIVATE ${LIBFLACPP_INCLUDE_DIRS} )
#
# 	target_link_libraries (libarcsdec_readerflac_tests
# 		PRIVATE ${LIBFLACPP_LIBRARIES} )
# endif()

