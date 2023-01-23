## Find libcdio
##
## Defines the following variables:
##
## libcdio_FOUND
## libcdio_INCLUDE_DIRS
## libcdio_LIBRARIES
## libcdio_VERSION

if (LIBCDIO_INCLUDE_DIR AND LIBCDIO_LIBRARY )

	# Already in cache, so be quiet
	set (libcdio_FIND_QUIETLY TRUE )
endif ()

## 1: use pkg-config to generate hints

find_package (PkgConfig QUIET )

if (PkgConfig_FOUND )

	PKG_CHECK_MODULES (PC_LIBCDIO QUIET libcdio )

	set (libcdio_VERSION ${PC_LIBCDIO_VERSION} )
endif (PkgConfig_FOUND )

## 2: find includes

find_path (LIBCDIO_INCLUDE_DIR
	NAMES cdio/cdio.h cdio.h
	HINTS
	${PC_LIBCDIO_INCLUDEDIR}
	${PC_LIBCDIO_INCLUDE_DIRS}
)

## 3: find library

find_library (LIBCDIO_LIBRARY
	NAMES cdio libcdio
	HINTS
	${PC_LIBCDIO_LIBDIR}
	${PC_LIBCDIO_LIBRARY_DIRS}
)

## 4: handle REQUIRED and QUIET options, set _FOUND VARIABLE

include (FindPackageHandleStandardArgs )

find_package_handle_standard_args (libcdio
	REQUIRED_VARS LIBCDIO_INCLUDE_DIR LIBCDIO_LIBRARY
    VERSION_VAR   libcdio_VERSION
    FAIL_MESSAGE  DEFAULT_MSG )

## 5: set declared variables

set (libcdio_LIBRARIES    ${LIBCDIO_LIBRARY} )
set (libcdio_INCLUDE_DIRS ${LIBCDIO_INCLUDE_DIR} )

mark_as_advanced (LIBCDIO_INCLUDE_DIR LIBCDIO_LIBRARY )

