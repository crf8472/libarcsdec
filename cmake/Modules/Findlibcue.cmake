## Find libcue
##
## Defines the following variables:
##
## libcue_FOUND
## libcue_INCLUDE_DIRS
## libcue_LIBRARIES
## libcue_VERSION

if (LIBCUE_INCLUDE_DIR AND LIBCUE_LIBRARY )

	# Already in cache, so be quiet
	set (libcue_FIND_QUIETLY TRUE )
endif ()

## 1: use pkg-config to generate hints

find_package (PkgConfig QUIET )

if (PkgConfig_FOUND )

	PKG_CHECK_MODULES (PC_LIBCUE QUIET libcue cue )

	set (libcue_VERSION ${PC_LIBCUE_VERSION} )
endif (PkgConfig_FOUND )

## 2: find includes

file (GLOB PATHS_WITH_VERSION "/usr/include/libcue-*" )

find_path (LIBCUE_INCLUDE_DIR
	NAMES libcue.h
	HINTS
	${PC_LIBCUE_INCLUDEDIR}
	${PC_LIBCUE_INCLUDE_DIRS}
	${PATHS_WITH_VERSION}
	PATH_SUFFIXES libcue
)

## 3: find library

find_library (LIBCUE_LIBRARY
	NAMES cue libcue
	HINTS
	${PC_LIBCUE_LIBDIR}
	${PC_LIBCUE_LIBRARY_DIRS}
)

## 4: handle REQUIRED and QUIET options, set _FOUND VARIABLE

include (FindPackageHandleStandardArgs )

find_package_handle_standard_args (libcue
	REQUIRED_VARS LIBCUE_INCLUDE_DIR LIBCUE_LIBRARY
    VERSION_VAR   libcue_VERSION
    FAIL_MESSAGE  DEFAULT_MSG )

## 5: set declared variables

set (libcue_LIBRARIES    ${LIBCUE_LIBRARY} )
set (libcue_INCLUDE_DIRS ${LIBCUE_INCLUDE_DIR} )

mark_as_advanced (LIBCUE_INCLUDE_DIR LIBCUE_LIBRARY )

