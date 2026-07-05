## CMake file for feature parsercue (builtin ToC parser for Cuesheet format)
## vim:fdm=marker

cmake_minimum_required (VERSION 3.13 )

## --- Request flex scanner + bison parser {{{1

set (CUESHEET_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/cuesheet )

## Input
set (CUESHEET_L       ${CUESHEET_SOURCE_DIR}/cuesheet.l )
set (CUESHEET_Y       ${CUESHEET_SOURCE_DIR}/cuesheet.y )

## Output
set (CUESHEET_TAB_CPP ${CMAKE_CURRENT_BINARY_DIR}/cuesheet.tab.cpp )
set (CUESHEET_TAB_HPP ${CMAKE_CURRENT_BINARY_DIR}/cuesheet.tab.hpp )
set (CUESHEET_YY_CPP  ${CMAKE_CURRENT_BINARY_DIR}/cuesheet.yy.cpp  )

add_bison_command (
	OUTPUT_CPP  ${CUESHEET_TAB_CPP}
	OUTPUT_HPP  ${CUESHEET_TAB_HPP}
	Y_FILE      ${CUESHEET_Y}
	FLAGS       "--verbose"
	WORKING_DIR ${CMAKE_CURRENT_BINARY_DIR}
)
## for analysis use:
## FLAGS "--verbose --debug -Wconflicts-sr -Wconflicts-rr -Wcounterexamples"

add_flex_command (
	OUTPUT_CPP    ${CUESHEET_YY_CPP}
	L_FILE        ${CUESHEET_L}
	INPUT_HEADERS ${CUESHEET_TAB_HPP}
	FLAGS         "--verbose"
	WORKING_DIR   ${CMAKE_CURRENT_BINARY_DIR}
)
## for analysis use:
## FLAGS "--verbose --debug"

## 1}}}
## --- Define target libarcsdec_parsercue {{{1

add_library (libarcsdec_parsercue OBJECT
	src/parsercue.cpp
)

set_property (TARGET libarcsdec_parsercue
	PROPERTY POSITION_INDEPENDENT_CODE ON )

target_sources (libarcsdec_parsercue
	PRIVATE ${CUESHEET_TAB_CPP}
	PRIVATE ${CUESHEET_YY_CPP}
)

target_include_directories (libarcsdec_parsercue
	PUBLIC $<BUILD_INTERFACE:${LIBARCSDEC_INCLUDE_SOURCE_DIR}> ## public headers
	PUBLIC $<BUILD_INTERFACE:${LIBARCSDEC_SOURCE_DIR}>    ## private headers
	PUBLIC $<BUILD_INTERFACE:${CUESHEET_SOURCE_DIR}>      ## private headers
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}> ## flex/bison output
)

target_link_libraries (libarcsdec_parsercue
	PUBLIC libarcstk::libarcstk
)
## 1}}}

## requires 3.13 for CMP0079 NEW
target_link_libraries (${PROJECT_NAME} PRIVATE libarcsdec_parsercue )

if (WITH_TESTS )
	add_subdirectory (test )
endif()

