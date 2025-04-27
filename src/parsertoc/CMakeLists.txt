## Root CMake file for parsertoc
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

bison_target (tocparser
	cdrtoc/cdrtoc.y
	${PROJECT_GEN_SOURCES_DIR}/cdrtoc.tab.cpp
	DEFINES_FILE ${PROJECT_GEN_SOURCES_DIR}/cdrtoc.tab.hpp
	#COMPILE_FLAGS "--verbose --debug -Wconflicts-sr -Wconflicts-rr -Wcounterexamples"
	COMPILE_FLAGS "--verbose -Wconflicts-sr -Wconflicts-rr -Wcounterexamples"
	)

flex_target (tocscanner
	cdrtoc/cdrtoc.l
	${PROJECT_GEN_SOURCES_DIR}/cdrtoc.yy.cpp
	#COMPILE_FLAGS "--verbose --debug"
	COMPILE_FLAGS "--verbose"
	)

add_flex_bison_dependency (tocscanner tocparser )


## --- Configure target for flex/bison parser {{{1

add_library (libarcsdec_cdrtoc OBJECT
	${BISON_tocparser_OUTPUTS}
	${FLEX_tocscanner_OUTPUTS}
	)

set_property (TARGET libarcsdec_cdrtoc PROPERTY POSITION_INDEPENDENT_CODE ON )

target_include_directories (libarcsdec_cdrtoc
	PRIVATE ${PROJECT_INCLUDE_SOURCE_DIR}       ## public headers
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/cdrtoc/ ## lexer.hpp + lexer_defs.hpp
	PRIVATE ${PROJECT_GEN_SOURCES_DIR}          ## generated sources
	)

if (WITH_SUBMODULES )
	# force to compile libarcstk before any cdrtoc
	target_link_libraries (libarcsdec_cdrtoc PRIVATE libarcstk::libarcstk )
endif ()


## --- Add to main target {{{1

target_link_libraries (${PROJECT_NAME} PRIVATE libarcsdec_cdrtoc )

target_sources        (${PROJECT_NAME}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/parsertoc.cpp )

