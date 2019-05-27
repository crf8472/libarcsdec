## Find libcue
##
## Defines the following variables:
##
## libcue_FOUND
## libcue_VERSION
## libcue_INCLUDE_DIRS
## libcue_LIBRARIES

if (LIBCUE_INCLUDE_DIR AND LIBCUE_LIBRARY )

	# Already in cache, so be quiet
	set (libcue_FIND_QUIETLY TRUE )
endif ()


## 1: use pkg-config to generate path hints for finding includes and libs

find_package (PkgConfig QUIET )

if (PkgConfig_FOUND )

	PKG_CHECK_MODULES (PC_LIBCUE QUIET libcue )

	set (LIBCUE_FOUND        ${PC_LIBCUE_FOUND} )
	set (LIBCUE_VERSION      ${PC_LIBCUE_VERSION} )
	set (LIBCUE_INCLUDE_DIRS ${PC_LIBCUE_INCLUDE_DIRS} )
	set (LIBCUE_LIBRARIES    ${PC_LIBCUE_LIBRARIES} )

endif (PkgConfig_FOUND )

## 2: find includes

## If pkgconfig is not available, we try to get the version number from
## the include path.
if (NOT LIBCUE_VERSION )

	message (STATUS "Guess version" )

	## Search for subpaths containing a version number, e.g.
	## /usr/include/libcue-2.0/libcue/libcue.h
	set (PATHS_WITH_VERSION )
	foreach(_path IN LISTS CMAKE_SYSTEM_PREFIX_PATH )

		file (GLOB PREFIX_SUBPATH "${_path}/include/libcue-*" )
		list (APPEND PATHS_WITH_VERSION ${PREFIX_SUBPATH} )
	endforeach()
	unset (PREFIX_SUBPATH )

	list (REMOVE_DUPLICATES PATHS_WITH_VERSION )

	## Prefix every path with its version to make path list sortable in
	## descending order
	set (SUITABLE_PATHS )
	foreach(_path IN LISTS PATHS_WITH_VERSION )

		## Get version number (hopefully)
		string (REGEX REPLACE ".+-\([0-9\.]+\)$" "\\1" PATH_VERSION ${_path} )

		## Create a list of version paths with entries formatted as
		## 'version:path' that is easily sortable by the version prefix
		#if (PATH_VERSION VERSION_GREATER_EQUAL libcue_FIND_VERSION )

			list (APPEND SUITABLE_PATHS "${PATH_VERSION}:${_path}" )

		#endif()
	endforeach()
	unset (PATH_VERSION )
	unset (PATHS_WITH_VERSION )

	if (SUITABLE_PATHS ) ## Avoids misleading error message

		## Sort paths by version number in descending order
		list (SORT SUITABLE_PATHS )
		list (REVERSE SUITABLE_PATHS )

		## Remove the Sorting Prefixes
		string (REGEX REPLACE "[0-9\.]+:" "" CUSTOM_PATHS "${SUITABLE_PATHS}" )
	endif ()
	unset (SUITABLE_PATHS )

endif()

find_path (LIBCUE_INCLUDE_DIR
	NAMES "libcue/libcue.h"
	PATHS
	${LIBCUE_INCLUDE_DIRS}
	${CUSTOM_PATHS}
)

## Get actual version found, if required, but do not overwrite found version
if (LIBCUE_INCLUDE_DIR AND NOT LIBCUE_VERSION )

	string (REGEX REPLACE ".+-\([0-9\.]+\)$" "\\1" LIBCUE_VERSION
		${LIBCUE_INCLUDE_DIR} )
endif()

## 3: find library

## TODO In case we guessed the include dir correctly by dirname, we must
## guarantee to find the exact corresponding lib (since we probably have several
## versions installed)

find_library (LIBCUE_LIBRARY
	NAMES cue libcue
	PATHS
	${PC_LIBCUE_LIBDIR}
	${PC_LIBCUE_LIBRARY_DIRS}
)

## 4: handle REQUIRED and QUIET options, set _FOUND VARIABLE

if (LIBCUE_VERSION AND LIBCUE_INCLUDE_DIRS AND LIBCUE_LIBRARIES )
	set (LIBCUE_FOUND TRUE )
endif ()

include (FindPackageHandleStandardArgs )

find_package_handle_standard_args (libcue
	REQUIRED_VARS LIBCUE_LIBRARY LIBCUE_VERSION LIBCUE_INCLUDE_DIR
	VERSION_VAR   LIBCUE_VERSION
	FAIL_MESSAGE  DEFAULT_MSG )

## 5: set declared variables

set (libcue_FOUND        ${LIBCUE_FOUND} )
set (libcue_VERSION      ${LIBCUE_VERSION} )
set (libcue_INCLUDE_DIRS ${LIBCUE_INCLUDE_DIR} )
set (libcue_LIBRARIES    ${LIBCUE_LIBRARY} )

mark_as_advanced (
	LIBCUE_FOUND
	LIBCUE_VERSION
	LIBCUE_INCLUDE_DIR
	LIBCUE_LIBRARY
)

