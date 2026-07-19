## CMake file for feature parsertoc (builtin ToC parser for CDRTOC format)
## vim:fdm=marker

cmake_minimum_required (VERSION 3.13 )

## --- Request flex scanner + bison parser {{{1

set (CDRTOC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/cdrtoc )

## Input
set (CDRTOC_L       ${CDRTOC_SOURCE_DIR}/cdrtoc.l )
set (CDRTOC_Y       ${CDRTOC_SOURCE_DIR}/cdrtoc.y )

## Output
set (CDRTOC_TAB_CPP ${CMAKE_CURRENT_BINARY_DIR}/cdrtoc.tab.cpp )
set (CDRTOC_TAB_HPP ${CMAKE_CURRENT_BINARY_DIR}/cdrtoc.tab.hpp )
set (CDRTOC_YY_CPP  ${CMAKE_CURRENT_BINARY_DIR}/cdrtoc.yy.cpp  )

get_bison_flags (CDRTOC_BISON_FLAGS )
get_flex_flags  (CDRTOC_FLEX_FLAGS )

add_bison_command (
	NAME        "Cdrtoc"
	OUTPUT_CPP  ${CDRTOC_TAB_CPP}
	OUTPUT_HPP  ${CDRTOC_TAB_HPP}
	Y_FILE      ${CDRTOC_Y}
	FLAGS       ${CDRTOC_BISON_FLAGS}
	WORKING_DIR ${CMAKE_CURRENT_BINARY_DIR}
)

add_flex_command (
	NAME          "Cdrtoc"
	OUTPUT_CPP    ${CDRTOC_YY_CPP}
	L_FILE        ${CDRTOC_L}
	INPUT_HEADERS ${CDRTOC_TAB_HPP}
	FLAGS         ${CDRTOC_FLEX_FLAGS}
	WORKING_DIR   ${CMAKE_CURRENT_BINARY_DIR}
)

## 1}}}
## --- Define target libarcsdec_parsertoc {{{1

add_library (libarcsdec_parsertoc OBJECT
	src/parsertoc.cpp
)

set_property (TARGET libarcsdec_parsertoc
	PROPERTY POSITION_INDEPENDENT_CODE ON
)

target_sources (libarcsdec_parsertoc
	PRIVATE ${CDRTOC_TAB_CPP}
	PRIVATE ${CDRTOC_YY_CPP}
)

target_include_directories (libarcsdec_parsertoc
	PUBLIC $<BUILD_INTERFACE:${LIBARCSDEC_INCLUDE_SOURCE_DIR}> ## public headers
	PUBLIC $<BUILD_INTERFACE:${LIBARCSDEC_SOURCE_DIR}>    ## private headers
	PUBLIC $<BUILD_INTERFACE:${CDRTOC_SOURCE_DIR}>        ## private headers
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}> ## flex/bison output
)

target_link_libraries (libarcsdec_parsertoc
	PUBLIC libarcstk::libarcstk
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug" )

	target_compile_definitions (libarcsdec_parsertoc PRIVATE YYDEBUG )
	## YYDEBUG is a bison macro that affects the parser class interface.
	## Makes FlexBisonDriver subclasses activate debugging feature.
endif()

## 1}}}

## requires 3.13 for CMP0079 NEW
target_link_libraries (${PROJECT_NAME} PRIVATE libarcsdec_parsertoc )

if (WITH_TESTS )
	add_subdirectory (test )
endif()

