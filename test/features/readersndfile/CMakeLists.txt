## CMake build script for adding a test suite
## vim:fdm=marker

add_test_suite (libarcsdec_readersndfile
	LABEL "readersndfile"
	TIMEOUT 10 )

## redundant
find_package (SndFile 1.0.17 REQUIRED )

if (TARGET SndFile::sndfile )
	## found by libsndfile's cmake config
	target_link_libraries (${PROJECT_NAME} PRIVATE SndFile::sndfile )
else()
	## found by cmake/Modules/FindSndfile.cmake
	target_include_directories (libarcsdec_readersndfile_tests
		PRIVATE ${SndFile_INCLUDE_DIRS} )
	target_link_libraries (libarcsdec_readersndfile_tests
		PUBLIC ${SndFile_LIBRARIES} )
endif()

