## CMake script for installation
## vim:fdm=marker

## --- Variables {{{1

set (INSTALL_CMAKECONFIG_DIR "${CMAKE_INSTALL_PREFIX}/lib/cmake/${PROJECT_NAME}"
	CACHE PATH "Installation directory for cmake package files" )

set (LIBARCSDEC_EXPORT_SET "${PROJECT_NAME}-targets" )
# 1}}}

## --- Install binary and exported header files {{{1

install (TARGETS ${PROJECT_NAME}
	EXPORT "${LIBARCSDEC_EXPORT_SET}"
	LIBRARY       DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
	PUBLIC_HEADER DESTINATION
			"${CMAKE_INSTALL_PREFIX}/${LIBARCSDEC_INCLUDE_INSTALL_SUBDIR}" )

install (EXPORT ${LIBARCSDEC_EXPORT_SET}
	NAMESPACE libarcsdec::
	DESTINATION "${INSTALL_CMAKECONFIG_DIR}" )
# 1}}}

## --- Install *.cmake files {{{1

if (NOT SKIP_INSTALL_FILES )

	## -- Support CMake Packaging

	include (CMakePackageConfigHelpers )

	set (LIBARCSDEC_CONFIG_FILE "${PROJECT_NAME}-config.cmake" )

	set (LIBARCSDEC_CONFIG_VERSION_FILE "${PROJECT_NAME}-config-version.cmake")

	configure_package_config_file (
		"${CMAKE_CURRENT_SOURCE_DIR}/${LIBARCSDEC_CONFIG_FILE}.in"
		"${CMAKE_CURRENT_BINARY_DIR}/${LIBARCSDEC_CONFIG_FILE}"
		INSTALL_DESTINATION "${INSTALL_CMAKECONFIG_DIR}"
	)

	write_basic_package_version_file (
		"${CMAKE_CURRENT_BINARY_DIR}/${LIBARCSDEC_CONFIG_VERSION_FILE}"
		VERSION        ${PROJECT_VERSION}
		COMPATIBILITY  SameMajorVersion
	)

	install (FILES
		"${CMAKE_CURRENT_BINARY_DIR}/${LIBARCSDEC_CONFIG_FILE}"
		"${CMAKE_CURRENT_BINARY_DIR}/${LIBARCSDEC_CONFIG_VERSION_FILE}"
		DESTINATION "${INSTALL_CMAKECONFIG_DIR}"
	)

endif ()
# 1}}}

## --- pkgconfig {{{1

if (NOT SKIP_INSTALL_FILES )
	add_subdirectory (pkgconfig )
endif ()

# 1}}}

