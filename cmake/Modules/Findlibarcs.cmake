# libarcs_FOUND
# libarcs_INCLUDE_DIR
# libarcs_LIBRARIES
# libarcs_DEFINITIONS

if (LIBARCS_INCLUDE_DIR AND LIBARCS_LIBRARY )

	## already in cache
	set(libarcs_FIND_QUIETLY TRUE )
endif ()

if (NOT WIN32)

	## try using pkg-config to get the directories and then use these values
	## in the find_path() and find_library() calls
	## also fills in LIBARCS_DEFINITIONS, although that isn't normally useful
	find_package (PkgConfig QUIET )
	pkg_check_modules (PC_LIBARCS QUIET libarcs )
	set (LIBARCS_DEFINITIONS    ${PC_LIBARCS_CFLAGS_OTHER} )
	set (LIBARCS_VERSION_STRING ${PC_LIBARCS_VERSION} )
endif ()

find_path (LIBARCS_INCLUDE_DIR arcs/calculate.hpp
	HINTS
	${PC_LIBARCS_INCLUDEDIR}
	${PC_LIBARCS_INCLUDE_DIRS}
)

find_library (LIBARCS_LIBRARY NAMES libarcs arcs
	HINTS
	${PC_LIBARCS_LIBDIR}
	${PC_LIBARCS_LIBRARY_DIRS}
)

mark_as_advanced (LIBARCS_INCLUDE_DIR LIBARCS_LIBRARY )

include (FindPackageHandleStandardArgs )

find_package_handle_standard_args (libarcs
	REQUIRED_VARS LIBARCS_LIBRARY LIBARCS_INCLUDE_DIR
	VERSION_VAR   LIBARCS_VERSION_STRING
)

if (libarcs_FOUND )
    set (libarcs_LIBRARIES    ${LIBARCS_LIBRARY} )
    set (libarcs_INCLUDE_DIRS ${LIBARCS_INCLUDE_DIR} )
endif()

