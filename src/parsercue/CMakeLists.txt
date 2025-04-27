## Root CMake file for parsercue
## vim:fdm=marker
##
## Prerequisites from PARENT
##
## - Variables: PROJECT_GEN_SOURCES_DIR, PROJECT_INCLUDE_SOURCE_DIR,
##              PROJECT_NAME

cmake_minimum_required (VERSION 3.10 )
## --- Policies {{{1

## Add link library cdrtoc to target libarcsdec that is not built in this
## directory.
cmake_policy (SET CMP0079 NEW )

## 1}}}

find_package (BISON ) # 3.8.2
find_package (FLEX  ) # 2.6.4


## --- Configure flex/bison targets {{{1

bison_target (cueparser
	cuesheet/cuesheet.y
	${PROJECT_GEN_SOURCES_DIR}/cuesheet.tab.cpp
	DEFINES_FILE ${PROJECT_GEN_SOURCES_DIR}/cuesheet.tab.hpp
	#COMPILE_FLAGS "--verbose --debug -Wconflicts-sr -Wconflicts-rr -Wcounterexamples"
	COMPILE_FLAGS "--verbose -Wconflicts-sr -Wconflicts-rr -Wcounterexamples"
	)

flex_target (cuescanner
	cuesheet/cuesheet.l
	${PROJECT_GEN_SOURCES_DIR}/cuesheet.yy.cpp
	#COMPILE_FLAGS "--verbose --debug"
	COMPILE_FLAGS "--verbose"
	)

add_flex_bison_dependency (cuescanner cueparser )


## --- Configure target for flex/bison parser {{{1

add_library (libarcsdec_cuesheet OBJECT
	${BISON_cueparser_OUTPUTS}
	${FLEX_cuescanner_OUTPUTS}
	)

set_property (TARGET libarcsdec_cuesheet PROPERTY POSITION_INDEPENDENT_CODE ON )

target_include_directories (libarcsdec_cuesheet
	PRIVATE ${PROJECT_INCLUDE_SOURCE_DIR}         ## public headers
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/cuesheet/ ## lexer.hpp + lexer_defs.hpp
	PRIVATE ${PROJECT_GEN_SOURCES_DIR}            ## generated sources
	)

if (WITH_SUBMODULES )
	# force to compile libarcstk before cuesheet
	target_link_libraries (libarcsdec_cuesheet PRIVATE libarcstk::libarcstk )
endif ()


## --- Add to main target {{{1

target_link_libraries (${PROJECT_NAME} PRIVATE libarcsdec_cuesheet )

target_sources        (${PROJECT_NAME}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/parsercue.cpp )

