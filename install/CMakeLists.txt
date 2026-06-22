## CMake script for installation
## vim:fdm=marker


set (LIBARCSDEC_EXPORT_SET "${PROJECT_NAME}-targets" )


## --- Install binary and public headers

install (TARGETS ${PROJECT_NAME}
	EXPORT "${LIBARCSDEC_EXPORT_SET}"
	LIBRARY       DESTINATION lib
	ARCHIVE       DESTINATION lib ## for .a/.lib files
	RUNTIME       DESTINATION bin ## for .dll files (Windows)
	PUBLIC_HEADER DESTINATION "${LIBARCSDEC_INCLUDE_INSTALL_SUBDIR}"
)


## --- Install cmake config, version and targets file {{{1

if (NOT SKIP_INSTALL_FILES )

	include (CMakePackageConfigHelpers )

	set (INSTALL_CMAKECONFIG_DIR
		"${CMAKE_INSTALL_PREFIX}/lib/cmake/${PROJECT_NAME}"
		CACHE PATH "Installation directory for cmake package files" )

	## libarcsdec-config.cmake

	set (LIBARCSDEC_CONFIG_FILE "${PROJECT_NAME}-config.cmake" )

	configure_package_config_file (
		"${CMAKE_CURRENT_SOURCE_DIR}/${LIBARCSDEC_CONFIG_FILE}.in"
		"${CMAKE_CURRENT_BINARY_DIR}/${LIBARCSDEC_CONFIG_FILE}"
		INSTALL_DESTINATION "${INSTALL_CMAKECONFIG_DIR}"
	)

	## libarcsdec-config-version.cmake

	set (LIBARCSDEC_CONFIG_VERSION_FILE "${PROJECT_NAME}-config-version.cmake")

	write_basic_package_version_file (
		"${CMAKE_CURRENT_BINARY_DIR}/${LIBARCSDEC_CONFIG_VERSION_FILE}"
		VERSION        ${PROJECT_VERSION}
		COMPATIBILITY  SameMajorVersion
	)

	## libarcsdec-targets.cmake

	set (LIBARCSDEC_TARGETS_FILE "${LIBARCSDEC_EXPORT_SET}.cmake" )

	install (EXPORT ${LIBARCSDEC_EXPORT_SET}
		FILE "${LIBARCSDEC_TARGETS_FILE}"
		NAMESPACE libarcsdec::
		DESTINATION "${INSTALL_CMAKECONFIG_DIR}" )

	# export targets file to build dir for debugging + convenience
	export (EXPORT ${LIBARCSDEC_EXPORT_SET}
		FILE "${LIBARCSDEC_TARGETS_FILE}"
		NAMESPACE libarcsdec:: )


	## install all 3 files

	install (FILES
		"${CMAKE_CURRENT_BINARY_DIR}/${LIBARCSDEC_CONFIG_FILE}"
		"${CMAKE_CURRENT_BINARY_DIR}/${LIBARCSDEC_CONFIG_VERSION_FILE}"
		"${CMAKE_CURRENT_BINARY_DIR}/${LIBARCSDEC_TARGETS_FILE}"
		DESTINATION "${INSTALL_CMAKECONFIG_DIR}"
	)

endif ()
# 1}}}


## --- pkgconfig

if (NOT SKIP_INSTALL_FILES )

	add_subdirectory (pkgconfig )

endif ()

