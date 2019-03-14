## Find libwavpack
##
## Defines the following variables:
##
## libwavpack_FOUND
## libwavpack_INCLUDE_DIRS
## libwavpack_LIBRARIES
## libwavpack_VERSION

if (libwavpack_INCLUDE_DIR AND libwavpack_LIBRARY )

	# Already in cache, so be quiet
	set(libwavpack_FIND_QUIETLY TRUE)
endif ()

## 1: use pkg-config to generate hints

find_package (PkgConfig QUIET )

if (PkgConfig_FOUND )

	PKG_CHECK_MODULES (PC_LIBWAVPACK QUIET wavpack )

	set (libwavpack_VERSION ${PC_LIBWAVPACK_VERSION} )
endif (PkgConfig_FOUND )

## 2: find includes

find_path (LIBWAVPACK_INCLUDE_DIR
	NAMES wavpack/wavpack.h wavpack.h
	HINTS
	${PC_LIBWAVPACK_INCLUDEDIR}
	${PC_LIBWAVPACK_INCLUDE_DIRS}
)

## 3: find library

find_library(LIBWAVPACK_LIBRARY
	NAMES wavpack
	HINTS
	${PC_LIBWAVPACK_LIBDIR}
	${PC_LIBWAVPACK_LIBRARY_DIRS}
)

## 4: handle REQUIRED and QUIET options, set _FOUND VARIABLE

include (FindPackageHandleStandardArgs )

find_package_handle_standard_args (libwavpack
	REQUIRED_VARS LIBWAVPACK_LIBRARY LIBWAVPACK_INCLUDE_DIR
	VERSION_VAR   libwavpack_VERSION
    FAIL_MESSAGE  DEFAULT_MSG )

## 5: set declared variables

set (libwavpack_LIBRARIES    ${LIBWAVPACK_LIBRARY} )
set (libwavpack_INCLUDE_DIRS ${LIBWAVPACK_INCLUDE_DIR} )

mark_as_advanced (LIBWAVPACK_INCLUDE_DIR LIBWAVPACK_LIBRARY )

