#ifndef __LIBARCSDEC_LIBINSPECT_HPP__
#define __LIBARCSDEC_LIBINSPECT_HPP__

/**
 * \file
 *
 * \brief Inspect the libarcsdec library.
 */

#include <regex>       // for regex
#include <string>      // for string
#include <vector>      // for vector

namespace arcsdec
{
inline namespace v_1_0_0
{

namespace details
{


/**
 * \internal
 *
 * \defgroup libinfoImpl API for implementing info functions about descriptors
 *
 * \ingroup descriptors
 *
 * \brief API for implementing info functions about descriptors.
 *
 * \warning
 * This API is currently *nix-only. It uses only dlopen and operates only on
 * sonames.
 *
 * @{
 */

/**
 * \brief Escape every occurrence of a character with a string.
 *
 * \param[in,out] input Input string to modify
 * \param[in]     c     Character to escape
 * \param[in]     seq   Escape string
 */
void escape(std::string& input, const char c, const std::string& seq);


/**
 * \internal
 * \brief Service: construct soname search pattern from library name.
 *
 * The library name should be the first part of the soname without any
 * suffices, e.g. 'libfoo', 'libFLAC++' but not 'libwavpack.so.4' or 'quux.dll'.
 *
 * \warning
 * This function is *nix-specific. It constructs a search pattern for shared
 * objects.
 *
 * \param[in] libname The library name to turn into a pattern
 *
 * \return A regex matching concrete sonames for this library
 */
std::regex to_libname_pattern(const std::string& libname);


/**
 * \internal
 * \brief Find shared object in the list of libarcsdec runtime dependencies.
 *
 * List \c list is a list of sonames, it can be created by using runtime_deps.
 * The \c name is the same format as the input for libname_pattern.
 *
 * \param[in] list List of library so filepaths
 * \param[in] name Name of the lib (e.g. libFLAC++, libavformat etc.)
 *
 * \return Filepath for the object or empty string.
 */
const std::string& first_libname_match(const std::vector<std::string>& list,
		const std::string& name);


/**
 * \internal
 * \brief Service: load runtime dependencies of an object.
 *
 * If \c object_name is empty, the runtime dependencies of the main executable
 * are loaded.
 *
 * \warning
 * This function is *nix-specific. It inspects binaries with dlopen.
 *
 * \param[in] object_name Name of the object to get dependencies for.
 *
 * \return List of runtime dependencies of an object
 */
std::vector<std::string> runtime_deps(const std::string& object_name);


/**
 * \internal
 * \brief Acquire list of runtime dependencies of libarcsdec.
 *
 * \return List of runtime dependencies of libarcsdec
 */
std::vector<std::string> acquire_libarcsdec_deps();


/**
 * \brief Comprehensive list of libarcsdec runtime dependency libraries.
 *
 * \return Comprehensive list of libarcsdec runtime dependency libraries.
 */
const std::vector<std::string>& libarcsdec_deps();


/**
 * \brief Return the library filepath for the runtime dependency.
 */
const std::string& libfile(const std::string& libname);

/// @}

} // details


} // namespace v_1_0_0
} // namespace arcsdec

#endif

