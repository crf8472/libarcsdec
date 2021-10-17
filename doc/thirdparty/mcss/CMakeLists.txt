## CMake file for m.css
##
## Use HTML5/CSS3 from m.css for generating HTML from Doxygen XML output

## Prerequisites from parent:
## - 'doc' target must exist
## - Variables: DOC_GEN_DIR, DOC_COMMENT

if (WIN32 )

	message (FATAL_ERROR
		"m.css support is currently not available for Windows" )
endif()


cmake_minimum_required (VERSION 3.0.2 )

find_package (Git REQUIRED )

message (STATUS "Will build documentation with m.css" )



## --- Overwrite default values from parent

set (DOXYGEN_OUTPUT_DIRECTORY "${DOC_GEN_DIR}/mcss" )
set (DOC_COMMENT              "${DOC_COMMENT}, using m.css" )

## --- Some variables

set (OUTPUT_MAINPAGE   "${DOXYGEN_OUTPUT_DIRECTORY}/html/index.html" )
set (OUTPUT_INDEX      "${DOXYGEN_OUTPUT_DIRECTORY}/xml/index.xml" )
set (DOXYFILE_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in" )
set (DOXYFILE          "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile" )
set (CONFPY            "${CMAKE_CURRENT_BINARY_DIR}/conf.py" )

set (MCSS_REPO_URL       "https://github.com/mosra/m.css.git" )
set (MCSS_SCRIPT_RELPATH "documentation/doxygen.py" )
## Commented out:
## Before commit 3f568563a8b70fc49a479250d51a2fdc71a66943:
## set (MCSS_SCRIPT_RELPATH "doxygen/dox2html5.py" )


add_custom_target (mcss_doc DEPENDS ${OUTPUT_MAINPAGE} )


## --- Define m.css to be checked out/updated while configuring

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.11.0" )

	## --- Use FetchContent if possible

	## This will NOT update the cloned m.css on build!

	include (FetchContent )

	FetchContent_Declare (m_css
		GIT_REPOSITORY    "${MCSS_REPO_URL}"
		#GIT_TAG          ... use always HEAD of default branch
		UPDATE_COMMAND    ${GIT_EXECUTABLE} pull
	)

	if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.14" )

		FetchContent_MakeAvailable (m_css )

	else()

		if (NOT m_css_POPULATED )

			FetchContent_Populate (m_css )

		endif (NOT m_css_POPULATED )
	endif()

	## Patch m.css for doxygen >= 1.9.0
	if (DOXYGEN_VERSION VERSION_GREATER_EQUAL 1.9.0 )

		execute_process (
			COMMAND ${GIT_EXECUTABLE} apply ${CMAKE_CURRENT_SOURCE_DIR}/mcss-doxygen19x.patch
			WORKING_DIRECTORY ${m_css_SOURCE_DIR}
			RESULT_VARIABLE PATCH_EXIT_CODE
		)

		if (PATCH_EXIT_CODE EQUAL 0)
			message (STATUS "Patching m.css for doxygen >= 1.9.0 was successful" )
		else()
			message (WARNING "Patching m.css for doxygen >= 1.9.0 failed." )
		endif()
	endif()

	set (MCSS_CMD "${m_css_SOURCE_DIR}/${MCSS_SCRIPT_RELPATH}" )

else() ## No FetchContent module available

	## --- Define m.css as target (will be updated on build)

	include (ExternalProject )

	ExternalProject_Add (mcss_download
		GIT_REPOSITORY    ${MCSS_REPO_URL}
		#GIT_TAG          ... use always HEAD of default branch
		GIT_SHALLOW       true
		GIT_PROGRESS      true
		TIMEOUT           10
		# DOWNLOAD_COMMAND use default
		PATCH_COMMAND     ""
		UPDATE_COMMAND    ${GIT_EXECUTABLE} pull
		CONFIGURE_COMMAND ""
		BUILD_COMMAND     ""
		INSTALL_COMMAND   ""
		TEST_COMMAND      ""
		LOG_DOWNLOAD      ON
	)

	ExternalProject_Get_Property (mcss_download SOURCE_DIR )

	## Patch m.css for doxygen >= 1.9.0
	if (DOXYGEN_VERSION VERSION_GREATER_EQUAL 1.9.0 )

		add_custom_command (
			OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mcss.patched
			PRE_BUILD
			COMMAND ${GIT_EXECUTABLE} apply ${CMAKE_CURRENT_SOURCE_DIR}/mcss-doxygen19x.patch
			COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/mcss.patched
			WORKING_DIRECTORY ${SOURCE_DIR}
		)

		add_custom_target (mcss_provide
			DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/mcss.patched )

		add_dependencies (mcss_provide mcss_download )
	endif()

	add_dependencies (mcss_doc mcss_provide )

	set (MCSS_CMD "${SOURCE_DIR}/${MCSS_SCRIPT_RELPATH}" )

	unset(SOURCE_DIR )
endif()


## --- Set up python environment

add_requirements_target (mcss_requirements
	"${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt"
)

add_dependencies (mcss_doc mcss_requirements )


## --- Build rule for running m.css

configure_file ("${DOXYFILE_TEMPLATE}"                   "${DOXYFILE}" @ONLY )
configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in" "${CONFPY}"   @ONLY )



if (DOXYGEN_NEEDS_XML_DEDUP )

	## Doxygen needs to run separately from m.css since we want to post-process
	## its XML output.

	add_doxygen_target (mcss_doxygen ${DOXYFILE} ${OUTPUT_INDEX} )

	add_dependencies (mcss_doc mcss_doxygen )

	add_custom_command (
		OUTPUT  "${OUTPUT_MAINPAGE}"
		COMMAND "${PYTHON_CMD}" "${MCSS_CMD}" --no-doxygen "${CONFPY}"
				2> "${PROJECT_LOG_DIR}/MCSS.log"
		DEPENDS ${OUTPUT_INDEX} ${DOC_SOURCES}
		WORKING_DIRECTORY "${DOC_GEN_DIR}"
		COMMENT "${DOC_COMMENT}"
		VERBATIM
	)
else()

	## Doxygen is called by m.css.

	add_custom_command (
		OUTPUT  "${OUTPUT_MAINPAGE}"
		COMMAND "${PYTHON_CMD}" "${MCSS_CMD}" "${CONFPY}"
				2> "${PROJECT_LOG_DIR}/MCSS.log"
		DEPENDS "${DOC_DOXYFILE_BASE}"
				"${DOXYFILE}"
				${DOC_SOURCES}
		WORKING_DIRECTORY "${DOC_GEN_DIR}"
		COMMENT "${DOC_COMMENT}"
		VERBATIM
	)
endif()

## Claim prerequisites for mcss_doc
add_dependencies (mcss_doc doc_prerequisites )

add_dependencies (doc mcss_doc )

