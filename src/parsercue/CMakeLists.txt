## Root CMake file for parsercue
## vim:fdm=marker
##
## Prerequisites from PARENT
##
## - Variables: PROJECT_INCLUDE_BINARY_DIR, PROJECT_SOURCE_DIR, PROJECT_NAME

cmake_minimum_required (VERSION 3.10 )
## --- Policies {{{1

## Add link library cuesheet to target libarcsdec that is not built in this
## directory.
cmake_policy (SET CMP0079 NEW )

## 1}}}

## Build bison parser

add_subdirectory (cuesheet )


## --- Define target libarcsdec_parsercue {{{1

add_library (libarcsdec_parsercue OBJECT parsercue.cpp )

set_property (TARGET libarcsdec_parsercue
	PROPERTY POSITION_INDEPENDENT_CODE ON )

target_include_directories (libarcsdec_parsercue
	PRIVATE
		$<BUILD_INTERFACE:${PROJECT_INCLUDE_BINARY_DIR}> ## public headers
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>         ## private headers
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/cuesheet> ## gen. code
	)

target_link_libraries (libarcsdec_parsercue
	PRIVATE $<TARGET_OBJECTS:libarcsdec_parsercue_cuesheet>
	PRIVATE libarcstk::libarcstk
)

## 1}}}


target_link_libraries (${PROJECT_NAME} PRIVATE libarcsdec_parsercue )

