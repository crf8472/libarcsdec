## CMake file for libflac based FLAC audio reader
## vim:fdm=marker

cmake_minimum_required (VERSION 3.14.0 )

message (STATUS "Build with fLaC reading capability for fLaC audio" )

function (readerflac_pkg DEPS )
	set  (${DEPS} "flac++ >= 1.3.1" PARENT_SCOPE )
endfunction()

find_package (Threads       REQUIRED )
find_package (FLAC    1.3.1 REQUIRED ) ## checks libFLAC and libFLAC++

if (TARGET FLAC::FLAC++ )
	## found by flac's cmake config
	target_link_libraries (${PROJECT_NAME} PRIVATE FLAC::FLAC++ FLAC::FLAC )
else()
	## found by cmake/Modules/FindFLAC
	target_include_directories (${PROJECT_NAME}
		PRIVATE ${LIBFLACPP_INCLUDE_DIRS} )
	target_link_libraries (${PROJECT_NAME} PRIVATE ${LIBFLACPP_LIBRARIES} )
endif()

target_sources (${PROJECT_NAME} PRIVATE readerflac.cpp )

