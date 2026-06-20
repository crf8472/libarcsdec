## CMake script for pkgconfig support
## vim:fdm=marker

## --- Configure + install libarcsdec.pc

string (REPLACE lib ""   PKG_BIND_NAME "${PROJECT_NAME}" )
string (REPLACE ";" ", " PKG_REQUIRE   "${LIBARCSDEC_PKG_REQUIRE_ARRAY}" )

configure_file (
	"${CMAKE_CURRENT_SOURCE_DIR}/libarcsdec.pc.in"
	"${CMAKE_CURRENT_BINARY_DIR}/libarcsdec.pc"
	@ONLY
)

unset (PKG_REQUIRE )
unset (PKG_BIND_NAME )

install (FILES "${CMAKE_CURRENT_BINARY_DIR}/libarcsdec.pc"
	DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" )

# 1}}}

