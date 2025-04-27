## Root CMake file for parsercue/cuesheet
## vim:fdm=marker
##
## Prerequisites from PARENT
##
## - Variables: PROJECT_INCLUDE_BINARY_DIR, WITH_SUBMODULES
##
## Provides:
##
## - Target: libarcsdec_parsercue_cuesheet (OBJECT)


find_package (BISON ) # 3.8.2
find_package (FLEX  ) # 2.6.4

set (PARSERTOC_CDRTOC_GEN_SOURCES_DIR "${CMAKE_CURRENT_BINARY_DIR}")


## --- Configure code generating flex/bison targets {{{1

bison_target (libarcsdec_parsercue_cuesheet_parser
	cuesheet.y
	${PARSERTOC_CDRTOC_GEN_SOURCES_DIR}/cuesheet.tab.cpp
	DEFINES_FILE ${PARSERTOC_CDRTOC_GEN_SOURCES_DIR}/cuesheet.tab.hpp
	#COMPILE_FLAGS "--verbose --debug -Wconflicts-sr -Wconflicts-rr -Wcounterexamples"
	COMPILE_FLAGS "--verbose -Wconflicts-sr -Wconflicts-rr -Wcounterexamples"
	)

flex_target (libarcsdec_parsercue_cuesheet_scanner
	cuesheet.l
	${PARSERTOC_CDRTOC_GEN_SOURCES_DIR}/cuesheet.yy.cpp
	#COMPILE_FLAGS "--verbose --debug"
	COMPILE_FLAGS "--verbose"
	)

add_flex_bison_dependency (libarcsdec_parsercue_cuesheet_scanner
	libarcsdec_parsercue_cuesheet_parser )


## --- Define target libarcsdec_parsercue_cuesheet {{{1

add_library (libarcsdec_parsercue_cuesheet OBJECT
	${BISON_libarcsdec_parsercue_cuesheet_parser_OUTPUTS}
	${FLEX_libarcsdec_parsercue_cuesheet_scanner_OUTPUTS}
	)

set_property (TARGET libarcsdec_parsercue_cuesheet
	PROPERTY POSITION_INDEPENDENT_CODE ON )

target_include_directories (libarcsdec_parsercue_cuesheet
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}         ## cuesheet private headers
	PRIVATE ${PARSERTOC_CDRTOC_GEN_SOURCES_DIR} ## flex/bison generated headers
	PRIVATE ${PROJECT_INCLUDE_BINARY_DIR}       ## libarcsdec public headers
	)


if (WITH_SUBMODULES )

	# force to compile libarcstk before any cuesheet
	target_link_libraries (libarcsdec_parsercue_cuesheet
		PRIVATE libarcstk::libarcstk )
endif ()

