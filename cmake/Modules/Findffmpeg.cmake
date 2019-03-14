## Try to find required ffmpeg components
##
## If no components are requested, the following components are set as default:
##
##  - libavcodec
##  - libavformat
##  - libavutil
##
## Defines the following variables:
##
##  FFMPEG_FOUND
##  FFMPEG_INCLUDE_DIRS
##  FFMPEG_LIBRARIES
##  FFMPEG_DEFINITIONS
##
## For each of the following components
##
##   - AVFORMAT
##   - AVCODEC
##   - AVUTIL
##   - AVDEVICE
##   - AVFILTER
##   - SWSCALE
##   - POSTPROC
##   - SWRESAMPLE
##
## the following variables will be defined
##
##  ${component}_FOUND
##  ${component}_INCLUDE_DIRS
##  ${component}_LIBRARIES
##  ${component}_DEFINITIONS
##  ${component}_VERSION


if (NOT ffmpeg_FIND_COMPONENTS )

	## libavformat, libavcodec and libavutil are the explicit dependencies

  set (ffmpeg_FIND_COMPONENTS avcodec avformat avutil )
endif()


## Macro: find_component
##
## Find the component _component with the pkgconfig-name _pkgconfig and the
## library name _library and the specific header _header
##
macro (find_component _component _version _header _pkgconfig )

	find_package (PkgConfig QUIET )

	if (PkgConfig_FOUND)

		if (_version)

			pkg_check_modules (PC_${_component}
				REQUIRED ${_pkgconfig}>=${_version} )

		else (_version)

			pkg_check_modules (PC_${_component}
				REQUIRED ${_pkgconfig} )

		endif (_version)

	endif()

	## Commented out, but kept for debugging
	## message (STATUS "pkg-config check for: ${_component}" )
	## message (STATUS "        found:   ${PC_${_component}_FOUND}" )
	## message (STATUS "        version: ${PC_${_component}_VERSION}" )
	## message (STATUS "        include: ${PC_${_component}_INCLUDEDIR}" )
	## message (STATUS "        library: ${PC_${_component}_LIBDIR}" )

	find_path (${_component}_INCLUDE_DIRS
		${_header}
		HINTS
			${PC_${_component}_INCLUDEDIR}
			${PC_${_component}_INCLUDE_DIRS}
		PATHS
			$ENV{FFMPEG_DIR}/include
			$ENV{OSGDIR}/include
			$ENV{OSG_ROOT}/include
			~/Library/Frameworks
			/Library/Frameworks
			/usr/local/include
			/usr/include
			/sw/include        ## Fink
			/opt/local/include ## DarwinPorts
			/opt/csw/include   ## Blastwave
			/opt/include
			/usr/freeware/include
		PATH_SUFFIXES
			ffmpeg
		DOC
			"Location of ffmpeg headers"
	)

	find_library (${_component}_LIBRARIES
		NAMES ${_component} ${_pkgconfig}
		HINTS
			${PC_${_component}_LIBDIR}
			${PC_${_component}_LIBRARY_DIRS}
	)

	if (${_component}_LIBRARIES AND ${_component}_INCLUDE_DIRS )

		set (${_component}_FOUND TRUE )
	endif()

	set (${_component}_VERSION ${PC_${_component}_VERSION}
			CACHE STRING "Version number of ${_component}" )

	set (${_component}_DEFINITIONS ${PC_${_component}_CFLAGS_OTHER}
			CACHE STRING "CFLAGS for binding ${_component}" )

	## Commented out, but kept for debugging
	## message (STATUS "${_component} found: ${${_component}_FOUND}" )
	## message (STATUS "              version: ${${_component}_VERSION}" )
	## message (STATUS "              include: ${${_component}_INCLUDE_DIRS}" )
	## message (STATUS "              library: ${${_component}_LIBRARIES}" )

	mark_as_advanced (
		${_component}_INCLUDE_DIRS
		${_component}_LIBRARIES
		${_component}_VERSION
		${_component}_DEFINITIONS )

endmacro()


## Check for cached results

if (NOT FFMPEG_LIBRARIES )

	## Check for the required components

	find_component(avcodec  0 libavcodec/avcodec.h   libavcodec  )
	find_component(avformat 0 libavformat/avformat.h libavformat )
	find_component(avutil   0 libavutil/avutil.h     libavutil   )

	## Add the includes, libraries and definitions of the required components
	## to the ffmpeg variables

	foreach (_component ${ffmpeg_FIND_COMPONENTS})

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
		CACHE STRING "ffmpeg: include directories" FORCE )

	set (FFMPEG_LIBRARIES    ${FFMPEG_LIBRARIES}
		CACHE STRING "ffmpeg: libraries" FORCE )

	set (FFMPEG_DEFINITIONS  ${FFMPEG_DEFINITIONS}
		CACHE STRING "ffmpeg: cflags" FORCE )

	mark_as_advanced (FFMPEG_INCLUDE_DIRS FFMPEG_LIBRARIES FFMPEG_DEFINITIONS )

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

