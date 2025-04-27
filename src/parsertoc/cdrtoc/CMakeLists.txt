## Root CMake file for parsertoc/cdrtoc
## vim:fdm=marker
##
## Prerequisites from PARENT
##
## - Variables: PROJECT_INCLUDE_BINARY_DIR, WITH_SUBMODULES
##
## Provides:
##
## - Target: libarcsdec_parsertoc_cdrtoc (OBJECT)


find_package (BISON ) # 3.8.2
find_package (FLEX  ) # 2.6.4

set (PARSERTOC_CDRTOC_GEN_SOURCES_DIR "${CMAKE_CURRENT_BINARY_DIR}")


## --- Configure code generating flex/bison targets {{{1

bison_target (libarcsdec_parsertoc_cdrtoc_parser
	cdrtoc.y
	${PARSERTOC_CDRTOC_GEN_SOURCES_DIR}/cdrtoc.tab.cpp
	DEFINES_FILE ${PARSERTOC_CDRTOC_GEN_SOURCES_DIR}/cdrtoc.tab.hpp
	#COMPILE_FLAGS "--verbose --debug -Wconflicts-sr -Wconflicts-rr -Wcounterexamples"
	COMPILE_FLAGS "--verbose -Wconflicts-sr -Wconflicts-rr -Wcounterexamples"
	)

flex_target (libarcsdec_parsertoc_cdrtoc_scanner
	cdrtoc.l
	${PARSERTOC_CDRTOC_GEN_SOURCES_DIR}/cdrtoc.yy.cpp
	#COMPILE_FLAGS "--verbose --debug"
	COMPILE_FLAGS "--verbose"
	)

add_flex_bison_dependency (libarcsdec_parsertoc_cdrtoc_scanner
	libarcsdec_parsertoc_cdrtoc_parser )


## --- Define target libarcsdec_parsertoc_cdrtoc {{{1

add_library (libarcsdec_parsertoc_cdrtoc OBJECT
	${BISON_libarcsdec_parsertoc_cdrtoc_parser_OUTPUTS}
	${FLEX_libarcsdec_parsertoc_cdrtoc_scanner_OUTPUTS}
	)

set_property (TARGET libarcsdec_parsertoc_cdrtoc
	PROPERTY POSITION_INDEPENDENT_CODE ON )

target_include_directories (libarcsdec_parsertoc_cdrtoc
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}         ## cdrtoc private headers
	PRIVATE ${PARSERTOC_CDRTOC_GEN_SOURCES_DIR} ## flex/bison generated headers
	PRIVATE ${PROJECT_INCLUDE_BINARY_DIR}       ## libarcsdec public headers
	)


if (WITH_SUBMODULES )

	# force to compile libarcstk before any cdrtoc
	target_link_libraries (libarcsdec_parsertoc_cdrtoc
		PRIVATE libarcstk::libarcstk )
endif ()

