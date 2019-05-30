## Try to find required ffmpeg components
##
## This module accepts a REQUIRED ffmpeg version but cannot detect the ffmpeg
## package version (e.g. 3.1). Only versions of the component libs are relevant
## and can be detected (quite) safely. The find_package command does not
## accept required versions for components. So the module just returns the lib
## versions and the caller has to check them on her own.
##
## This module defines the following variables
##
##  FFMPEG_FOUND
##  FFMPEG_INCLUDE_DIRS
##  FFMPEG_LIBRARIES
##
## It tries to find the following components by default:
##
##  - libavcodec
##  - libavformat
##  - libavutil
##
## And defines the following variables for each of them:
##
##  ${component}_FOUND
##  ${component}_VERSION
##  ${component}_INCLUDE_DIRS
##  ${component}_LIBRARIES
##
## The following components are ignored by default since libarcsdec does not
## use them, but can explicitly be requested as COMPONENTS:
##
##   - avdevice
##   - avfilter
##   - swscale
##   - postproc
##   - swresample


## Macro: find_component
##
## Finds a component by its name.
##
## Arguments:
##		_component: filename without "lib"
##		_pkgconfigname: component name used by pkg-config
##
## Sets the variables:
##		${_component}_VERSION
##		${_component}_INCLUDE_DIRS
##		${_component}_LIBRARIES
##
macro (find_component _component _pkgconfigname )

	if (PkgConfig_FOUND)

		pkg_check_modules (PC_${_component} REQUIRED ${_pkgconfigname} )

	endif()

	## Commented out, but kept for debugging
	## message (STATUS "pkg-config check for: ${_component}" )
	## message (STATUS "        found:   ${PC_${_component}_FOUND}" )
	## message (STATUS "        version: ${PC_${_component}_VERSION}" )
	## message (STATUS "        include: ${PC_${_component}_INCLUDEDIR}" )
	## message (STATUS "        library: ${PC_${_component}_LIBDIR}" )

	find_path (${_component}_INCLUDE_DIRS
		NAMES "lib${_component}/${_component}.h"
		PATHS
			${PC_${_component}_INCLUDEDIR}
			${PC_${_component}_INCLUDE_DIRS}
		DOC
			"Header path required to include <lib${_component}/${_component}.h>"
	)

	find_library (${_component}_LIBRARIES
		NAMES ${_component} ${_pkgconfigname}
		PATHS
			${PC_${_component}_LIBDIR}
			${PC_${_component}_LIBRARY_DIRS}
		DOC
			"Location of lib${_component} libraries"
	)

	if (${_component}_LIBRARIES AND ${_component}_INCLUDE_DIRS )

		set (${_component}_FOUND TRUE )
	endif()


	## Version number

	set (${_component}_VERSIONVAR_TEXT "Version number of lib${_component}" )

	set (${_component}_VERSION ${PC_${_component}_VERSION}
			CACHE STRING "${${_component}_VERSIONVAR_TEXT}" )

	## If we have everything but the version, PkgConfig is not
	## available or couldn't help. Get the version "manually".
	if (${_component}_FOUND AND NOT ${_component}_VERSION )

		## Magical knowledge about how ffmpeg provides its version number macros

		string (TOUPPER ${_component} TMP_NAME )

		set (${_component}_VERSION_INFO_REGEX
			"#define[ \t]+LIB${TMP_NAME}_VERSION_M[AJINORC]+[ \t]+([0-9]+)" )

		unset (TMP_NAME )


		## Try to find the version.h header and parse version numbers

		find_path (${_component}_VERSIONFILE_PATH
			NAMES "version.h"
			PATHS ${${_component}_INCLUDE_DIRS}
			PATH_SUFFIXES ${_component} ${_pkgconfigname}
			DOC "Path to ${_component}'s 'version.h'"
		)

		file (STRINGS "${${_component}_VERSIONFILE_PATH}/version.h"
			${_component}_VERSION_INFO
			LIMIT_COUNT 3
			REGEX ${${_component}_VERSION_INFO_REGEX}
		)

		## Format parsed version information

		string (REGEX REPLACE ${${_component}_VERSION_INFO_REGEX} "\\1"
			${_component}_TMP_VERSION "${${_component}_VERSION_INFO}" )

		string (REPLACE ";" "." ${_component}_VERSION
			"${${_component}_TMP_VERSION}" )

		unset (${_component}_VERSION_INFO_REGEX )
		unset (${_component}_VERSION_INFO )
		unset (${_component}_TMP_VERSION )


		## Force the updated version value to cache
		set (${_component}_VERSION "${${_component}_VERSION}"
			CACHE STRING "${${_component}_VERSIONVAR_TEXT}" FORCE )

		mark_as_advanced (
			${_component}_VERSIONFILE_PATH
		)

	endif()


	## Debug output

	if (${${_component}_FOUND} )

		if (NOT PC_${_component}_VERSION )

			message (STATUS "  lib${_component} found" )
			message (STATUS "    version: ${${_component}_VERSION}" )
		endif()

		message (STATUS "    library: ${${_component}_LIBRARIES}" )
		message (STATUS "    include: ${${_component}_INCLUDE_DIRS}" )
	else()

		message (WARNING "${_component} not found!" )
	endif()

	mark_as_advanced (
		${_component}_INCLUDE_DIRS
		${_component}_LIBRARIES
		${_component}_VERSION
	)

endmacro()


## Check arguments

if (NOT ffmpeg_FIND_COMPONENTS )

	## libavformat, libavcodec and libavutil are the explicit dependencies

	set (ffmpeg_FIND_COMPONENTS "avcodec" "avformat" "avutil" )
endif()

if (ffmpeg_FIND_VERSION )

	message (WARNING
	"Ignore constraint for specific ffmpeg version ${ffmpeg_FIND_VERSION}."
	)
endif()


## Check for cached results

if (NOT FFMPEG_LIBRARIES )

	find_package (PkgConfig QUIET )

	if (NOT PkgConfig_FOUND )

		message (WARNING
		"Consider installing pkg-config to find ffmpeg libraries correctly" )
	endif()

	## Add the includes, libraries and definitions of the required components
	## to the ffmpeg variables

	foreach (_component ${ffmpeg_FIND_COMPONENTS})

		find_component(${_component} "lib${_component}" )

		if (${_component}_FOUND)

			list (APPEND FFMPEG_INCLUDE_DIRS ${${_component}_INCLUDE_DIRS} )

			list (APPEND FFMPEG_LIBRARIES    ${${_component}_LIBRARIES} )

		endif()

	endforeach()

	if (FFMPEG_INCLUDE_DIRS )

		list (REMOVE_DUPLICATES FFMPEG_INCLUDE_DIRS )

	endif()

	# Cache the ffmpeg variables

	set (FFMPEG_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIRS}
		CACHE STRING "Header path required to include <libav.../av....h>" )

	set (FFMPEG_LIBRARIES    ${FFMPEG_LIBRARIES}
		CACHE STRING "Location of ffmpeg libraries" )

	mark_as_advanced (
		FFMPEG_INCLUDE_DIRS
		FFMPEG_LIBRARIES
	)

endif (NOT FFMPEG_LIBRARIES )


## List of required variables' names

set (_ffmpeg_REQUIRED_VARS FFMPEG_LIBRARIES FFMPEG_INCLUDE_DIRS )

foreach (_component ${ffmpeg_FIND_COMPONENTS} )

	list (APPEND _ffmpeg_REQUIRED_VARS ${_component}_VERSION )
	list (APPEND _ffmpeg_REQUIRED_VARS ${_component}_INCLUDE_DIRS )
	list (APPEND _ffmpeg_REQUIRED_VARS ${_component}_LIBRARIES )

endforeach()


## Handle QUIET and REQUIRED parameters and give an error message if necessary

include (FindPackageHandleStandardArgs )

find_package_handle_standard_args (ffmpeg
	REQUIRED_VARS ${_ffmpeg_REQUIRED_VARS}
    FAIL_MESSAGE  DEFAULT_MSG
)

unset (_ffmpeg_REQUIRED_VARS )

