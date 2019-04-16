# libarcstk_FOUND
# libarcstk_INCLUDE_DIR
# libarcstk_LIBRARIES
# libarcstk_DEFINITIONS

if (LIBARCSTK_INCLUDE_DIR AND LIBARCSTK_LIBRARY )

	## already in cache
	set(libarcstk_FIND_QUIETLY TRUE )
endif ()

if (NOT WIN32)

	## try using pkg-config to get the directories and then use these values
	## in the find_path() and find_library() calls
	## also fills in LIBARCSTK_DEFINITIONS, although that isn't normally useful
	find_package (PkgConfig QUIET )
	pkg_check_modules (PC_LIBARCSTK QUIET libarcstk )
	set (LIBARCSTK_DEFINITIONS    ${PC_LIBARCSTK_CFLAGS_OTHER} )
	set (LIBARCSTK_VERSION_STRING ${PC_LIBARCSTK_VERSION} )
endif ()

find_path (LIBARCSTK_INCLUDE_DIR arcstk/calculate.hpp
	HINTS
	${PC_LIBARCSTK_INCLUDEDIR}
	${PC_LIBARCSTK_INCLUDE_DIRS}
)

find_library (LIBARCSTK_LIBRARY NAMES libarcstk arcstk
	HINTS
	${PC_LIBARCSTK_LIBDIR}
	${PC_LIBARCSTK_LIBRARY_DIRS}
)

mark_as_advanced (LIBARCSTK_INCLUDE_DIR LIBARCSTK_LIBRARY )

include (FindPackageHandleStandardArgs )

find_package_handle_standard_args (libarcstk
	REQUIRED_VARS LIBARCSTK_LIBRARY LIBARCSTK_INCLUDE_DIR
	VERSION_VAR   LIBARCSTK_VERSION_STRING
)

if (libarcstk_FOUND )
    set (libarcstk_LIBRARIES    ${LIBARCSTK_LIBRARY} )
    set (libarcstk_INCLUDE_DIRS ${LIBARCSTK_INCLUDE_DIR} )
endif()

