## Try to find required ffmpeg components
##
## This module defines the following variables
##
##  FFMPEG_FOUND
##  FFMPEG_INCLUDE_DIRS
##  FFMPEG_LIBRARIES
##  FFMPEG_DEFINITIONS
##
## It tries to find the following components:
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
##  ${component}_DEFINITIONS
##
## The following components are ignored by default since libarcsdec does not
## use them, but can explicitly be requested as COMPONENTS:
##
##   - avdevice
##   - avfilter
##   - swscale
##   - postproc
##   - swresample


if (NOT ffmpeg_FIND_COMPONENTS )

	## libavformat, libavcodec and libavutil are the explicit dependencies

	set (ffmpeg_FIND_COMPONENTS avcodec avformat avutil )
endif()

if (ffmpeg_FIND_VERSION )

	message (WARNING
	"Find ffmpeg but ignore request for specific version ${ffmpeg_FIND_VERSION}."
	)
endif()


## Macro: find_component
##
## Find the component _component with the pkgconfig-name _pkgconfigname that you
## whish to include by the specific header _header (along with path)
##
macro (find_component _component _pkgconfigname _header)

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
		NAMES ${_header}
		PATHS
			${PC_${_component}_INCLUDEDIR}
			${PC_${_component}_INCLUDE_DIRS}
## Commented out: unnecessary, but won't loose the collection
#			$ENV{FFMPEG_DIR}/include
#			$ENV{OSGDIR}/include
#			$ENV{OSG_ROOT}/include
#			~/Library/Frameworks
#			/Library/Frameworks
#			/usr/local/include
#			/usr/include
#			/sw/include        ## Fink
#			/opt/local/include ## DarwinPorts
#			/opt/csw/include   ## Blastwave
#			/opt/include
#			/usr/freeware/include
		DOC
			"Location of lib${_component} headers"
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

	set (${_component}_VERSION ${PC_${_component}_VERSION}
			CACHE STRING "Version number of ${_component}" )

	if (NOT ${_component}_VERSION ) ## PKGConfig not available or didn't help

		## Magical knowledge about how ffmpeg provides its version number macros

		string(TOUPPER ${_component} TMP_NAME )

		set (${_component}_VERSION_INFO_REGEX
			"#define[ \t]+LIB${TMP_NAME}_VERSION_M[AJINORC]+[ \t]+([0-9]+)" )

		## Try to find the version.h header and parse version numbers

		find_path (${_component}_VERSIONPATH
			NAMES "version.h"
			PATHS ${${_component}_INCLUDE_DIRS}
			PATH_SUFFIXES ${_component} ${_pkgconfigname}
		)

		file (STRINGS "${${_component}_VERSIONPATH}/version.h"
			${_component}_VERSION_INFO
			LIMIT_COUNT 3
			REGEX ${${_component}_VERSION_INFO_REGEX}
		)

		## Format parsed version information

		string (REGEX REPLACE ${${_component}_VERSION_INFO_REGEX} "\\1"
			${_component}_TMP_VERSION "${${_component}_VERSION_INFO}" )

		string (REPLACE ";" "." ${_component}_VERSION
			"${${_component}_TMP_VERSION}" )

		mark_as_advanced (
			TMP_NAME
			${_component}_VERSION_INFO_REGEX
			${_component}_VERSIONPATH
			${_component}_VERSION_INFO
			${_component}_TMP_VERSION
		)

	endif()


	## Flags

	set (${_component}_DEFINITIONS ${PC_${_component}_CFLAGS_OTHER}
			CACHE STRING "CFLAGS for binding ${_component}" )


	if (${${_component}_FOUND} )
		if (NOT PC_${_component}_VERSION )
			message (STATUS "  lib${_component} found" )
			message (STATUS "    version: ${${_component}_VERSION}" )
		endif()
		message (STATUS "    library: ${${_component}_LIBRARIES}" )
		message (STATUS "    include: ${${_component}_INCLUDE_DIRS}" )
		message (STATUS "    flags:   ${${_component}_DEFINITIONS}" )
	else()
		message (WARNING "${_component} not found!" )
	endif()

	mark_as_advanced (
		${_component}_INCLUDE_DIRS
		${_component}_LIBRARIES
		${_component}_VERSION
		${_component}_DEFINITIONS )

endmacro()


## Check for cached results

if (NOT FFMPEG_LIBRARIES )

	find_package (PkgConfig QUIET )

	## Check for the required components

	## ffmpeg 3.1:
	## API Change: deprecated avcodec_decode_audio4() from ffmpeg 0.9 in favor
	## of avcodec_send_packet()/avcodec_receive_frame()
	#set (MIN_AVCODEC_VERSION  "57.37.100" ) ## 2016-04-21
	#set (MIN_AVFORMAT_VERSION "57.33.100" ) ## 2016-04-11
	#set (MIN_AVUTIL_VERSION   "55.22.100" ) ## 2016-04-14
	#find_component(avcodec  "57.37.100" libavcodec/avcodec.h   libavcodec  )
	#find_component(avformat "57.33.100" libavformat/avformat.h libavformat )
	#find_component(avutil   "55.22.100" libavutil/avutil.h     libavutil   )

	## Add the includes, libraries and definitions of the required components
	## to the ffmpeg variables

	foreach (_component ${ffmpeg_FIND_COMPONENTS})

		find_component(${_component}
			"lib${_component}"
			"lib${_component}/${_component}.h" )

		if (${_component}_FOUND)

			list (APPEND FFMPEG_INCLUDE_DIRS ${${_component}_INCLUDE_DIRS} )

			set (FFMPEG_LIBRARIES
				${FFMPEG_LIBRARIES} ${${_component}_LIBRARIES} )

			set (FFMPEG_DEFINITIONS
				${FFMPEG_DEFINITIONS} ${${_component}_DEFINITIONS} )

		endif()

	endforeach()

	if (FFMPEG_INCLUDE_DIRS )

		list (REMOVE_DUPLICATES FFMPEG_INCLUDE_DIRS )

	endif()

	# Cache the ffmpeg variables

	set (FFMPEG_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIRS}
		CACHE STRING "ffmpeg: include directories" )

	set (FFMPEG_LIBRARIES    ${FFMPEG_LIBRARIES}
		CACHE STRING "ffmpeg: libraries" )

	set (FFMPEG_DEFINITIONS  ${FFMPEG_DEFINITIONS}
		CACHE STRING "ffmpeg: cflags" )

	mark_as_advanced (
		FFMPEG_INCLUDE_DIRS
		FFMPEG_LIBRARIES
		FFMPEG_DEFINITIONS )

endif (NOT FFMPEG_LIBRARIES )


## Compile the list of required variables

set (_ffmpeg_REQUIRED_VARS FFMPEG_LIBRARIES FFMPEG_INCLUDE_DIRS )

foreach (_component ${ffmpeg_FIND_COMPONENTS} )

	list (APPEND _ffmpeg_REQUIRED_VARS ${_component}_VERSION )

	## One could also add ${_component}_INCLUDE_DIRS and ${_component}_LIBRARIES

endforeach()


## Handle QUIET and REQUIRED parameters and give an error message if necessary

include (FindPackageHandleStandardArgs )

find_package_handle_standard_args (ffmpeg DEFAULT_MSG ${_ffmpeg_REQUIRED_VARS} )

