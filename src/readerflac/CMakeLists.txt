## CMake file for libflac based FLAC audio reader
## vim:fdm=marker

## - Variables: PKG_REQUIRE_ARRAY, PROJECT_NAME, PROJECT_SOURCE_DIR

cmake_minimum_required (VERSION 3.14.0 )


message (STATUS "Build with fLaC reading capability for fLaC audio" )

find_package (Threads       REQUIRED )
find_package (FLAC    1.3.1 REQUIRED ) ## checks libFLAC and libFLAC++

list (APPEND PKG_REQUIRE_ARRAY "flac++ >= 1.3.1" )



## --- Define testcases {{{1

if (TARGET FLAC::FLAC++ )
	## found by flac's cmake config
	target_link_libraries (${PROJECT_NAME} PRIVATE FLAC::FLAC++ )
else()
	## found by cmake/Modules/FindFLAC
	target_include_directories (${PROJECT_NAME}
		PRIVATE ${LIBFLACPP_INCLUDE_DIRS} )
	target_link_libraries (${PROJECT_NAME}
		PRIVATE ${LIBFLACPP_LIBRARIES} )
endif()

target_sources (${PROJECT_NAME} PRIVATE readerflac.cpp )

