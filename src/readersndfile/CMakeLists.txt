## CMake file for libsndfile based audio reader
## vim:fdm=marker

cmake_minimum_required (VERSION 3.14.0 )

message (STATUS "Build with libsndfile support for audio" )

function (readersndfile_pkg DEPS )
	set  (${DEPS} "sndfile >= 1.0.17" PARENT_SCOPE )
endfunction()

find_package (SndFile 1.0.17 REQUIRED )

#target_sources (${PROJECT_NAME} PRIVATE readersndfile.cpp )

add_library (libarcsdec_readersndfile OBJECT readersndfile.cpp )

set_property (TARGET libarcsdec_readersndfile
	PROPERTY POSITION_INDEPENDENT_CODE ON )

target_include_directories (libarcsdec_readersndfile
	PUBLIC $<BUILD_INTERFACE:${LIBARCSDEC_INCLUDE_SOURCE_DIR}>## public headers
	PUBLIC $<BUILD_INTERFACE:${LIBARCSDEC_SOURCE_DIR}>        ## private headers
)

target_link_libraries      (libarcsdec_readersndfile
	PUBLIC libarcstk::libarcstk # TODO Should be propagated by main target
)

if (TARGET SndFile::sndfile )
	## found by libsndfile's cmake config
	target_link_libraries (libarcsdec_readersndfile
		PUBLIC SndFile::sndfile )
else()
	## found by cmake/Modules/FindSndfile.cmake
	target_include_directories (libarcsdec_readersndfile
		PUBLIC $<BUILD_INTERFACE:${SndFile_INCLUDE_DIRS}> )
	target_link_libraries (libarcsdec_readersndfile
		PUBLIC ${SndFile_LIBRARIES} )
endif()

## Put this into libarcsdec
target_link_libraries (${PROJECT_NAME} PRIVATE libarcsdec_readersndfile )

