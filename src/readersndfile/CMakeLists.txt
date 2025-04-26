## CMake file for libsndfile based audio reader
## vim:fdm=marker

## - Variables: PKG_REQUIRE_ARRAY, PROJECT_NAME, PROJECT_SOURCE_DIR

cmake_minimum_required (VERSION 3.14.0 )


message (STATUS "Build with libsndfile support for audio" )

find_package (SndFile 1.0.17 REQUIRED )

list (APPEND PKG_REQUIRE_ARRAY "sndfile >= 1.0.17" )

if (TARGET SndFile::sndfile )
	## found by libsndfile's cmake config
	target_link_libraries (${PROJECT_NAME} PRIVATE SndFile::sndfile )
else()
	## found by cmake/Modules/Findlibsndfile.cmake

	if (NOT libsndfile_LIBRARIES )
		find_package (libsndfile 1.0.17 REQUIRED )
	endif()

	target_include_directories (${PROJECT_NAME}
		PRIVATE ${libsndfile_INCLUDE_DIRS} )

	target_link_libraries (${PROJECT_NAME}
		PUBLIC ${libsndfile_LIBRARIES} )
endif()

target_sources (${PROJECT_NAME}
	PRIVATE ${PROJECT_SOURCE_DIR}/readersndfile.cpp )

