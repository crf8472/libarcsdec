## Root CMake file for parsercue
## vim:fdm=marker

cmake_minimum_required (VERSION 3.10 )
## --- Policies {{{1

## 3.13: Add link library cuesheet to target libarcsdec that is not built in
## this directory.
cmake_policy (SET CMP0079 NEW )

## 1}}}

## Build bison parser

add_subdirectory (cuesheet )

## --- Define target libarcsdec_parsercue {{{1

add_library (libarcsdec_parsercue OBJECT parsercue.cpp )

set_property (TARGET libarcsdec_parsercue
	PROPERTY POSITION_INDEPENDENT_CODE ON )

target_include_directories (libarcsdec_parsercue
	PUBLIC $<BUILD_INTERFACE:${LIBARCSDEC_INCLUDE_SOURCE_DIR}> ## public headers
	PUBLIC $<BUILD_INTERFACE:${LIBARCSDEC_SOURCE_DIR}>         ## private headers
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/cuesheet> ## gen. code
)

target_link_libraries (libarcsdec_parsercue
	PUBLIC $<TARGET_OBJECTS:libarcsdec_parsercue_cuesheet>
	PUBLIC libarcstk::libarcstk
)

## 1}}}

target_link_libraries (${PROJECT_NAME} PRIVATE libarcsdec_parsercue )

