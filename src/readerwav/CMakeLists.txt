## Root CMake file for readerwav
## vim:fdm=marker

cmake_minimum_required (VERSION 3.10 )
## --- Policies {{{1

## 3.13: Add link library cuesheet to target libarcsdec that is not built in
## this directory.
cmake_policy (SET CMP0079 NEW )

## 1}}}

#target_sources (${PROJECT_NAME}
#	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/readerwav.cpp )

add_library (libarcsdec_readerwav OBJECT readerwav.cpp )

set_property (TARGET libarcsdec_readerwav
	PROPERTY POSITION_INDEPENDENT_CODE ON )

target_include_directories (libarcsdec_readerwav
	PUBLIC $<BUILD_INTERFACE:${LIBARCSDEC_INCLUDE_SOURCE_DIR}>## public headers
	PUBLIC $<BUILD_INTERFACE:${LIBARCSDEC_SOURCE_DIR}>        ## private headers
)

target_link_libraries (libarcsdec_readerwav
	PUBLIC libarcstk::libarcstk
)

target_link_libraries (${PROJECT_NAME} PRIVATE libarcsdec_readerwav )

