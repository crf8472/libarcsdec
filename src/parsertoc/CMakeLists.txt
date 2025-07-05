## Root CMake file for parsertoc
## vim:fdm=marker
##
## Prerequisites from PARENT
##
## - Variables: PROJECT_INCLUDE_BINARY_DIR, PROJECT_SOURCE_DIR, PROJECT_NAME

cmake_minimum_required (VERSION 3.10 )
## --- Policies {{{1

## Add link library cdrtoc to target libarcsdec that is not built in this
## directory.
cmake_policy (SET CMP0079 NEW )

## 1}}}

## Build bison parser

add_subdirectory (cdrtoc )


## --- Define target libarcsdec_parsertoc {{{1

add_library (libarcsdec_parsertoc OBJECT parsertoc.cpp )

set_property (TARGET libarcsdec_parsertoc
	PROPERTY POSITION_INDEPENDENT_CODE ON )

target_include_directories (libarcsdec_parsertoc
	PRIVATE
		$<BUILD_INTERFACE:${PROJECT_INCLUDE_SOURCE_DIR}> ## public headers
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>         ## private headers
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/cdrtoc> ## flex/bison code
	)

target_link_libraries (libarcsdec_parsertoc
	PRIVATE $<TARGET_OBJECTS:libarcsdec_parsertoc_cdrtoc>
	PRIVATE libarcstk::libarcstk
)

## 1}}}


target_link_libraries (${PROJECT_NAME} PRIVATE libarcsdec_parsertoc )

