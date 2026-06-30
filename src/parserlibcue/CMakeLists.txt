## CMake file for libcue based Cuesheet parser
## vim:fdm=marker

cmake_minimum_required (VERSION 3.14.0 )

message (STATUS "Build with libcue parsing capability for cuesheets" )

function (parserlibcue_pkg DEPS )
	set  (${DEPS} "libcue >= 2.0.0" PARENT_SCOPE )
endfunction()

find_package (libcue 2.0.0 REQUIRED )

# Debugging the build, commented out
#message (STATUS "libcue includes: ${libcue_INCLUDE_DIRS}" )
#message (STATUS "libcue libraries: ${libcue_LIBRARIES}" )

add_library (libarcsdec_parserlibcue OBJECT parserlibcue.cpp )

set_property (TARGET libarcsdec_parserlibcue
	PROPERTY POSITION_INDEPENDENT_CODE ON )

target_include_directories (libarcsdec_parserlibcue
	PUBLIC $<BUILD_INTERFACE:${LIBARCSDEC_INCLUDE_SOURCE_DIR}>## public headers
	PUBLIC $<BUILD_INTERFACE:${LIBARCSDEC_SOURCE_DIR}>        ## private headers
	PUBLIC $<BUILD_INTERFACE:${libcue_INCLUDE_DIRS}>
)

target_link_libraries      (libarcsdec_parserlibcue
	PUBLIC ${libcue_LIBRARIES}
	PUBLIC libarcstk::libarcstk # TODO Should be propagated by main target
)

## Put this into libarcsdec
target_link_libraries (${PROJECT_NAME} PRIVATE libarcsdec_parserlibcue )

