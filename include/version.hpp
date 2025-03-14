#ifndef __LIBARCSDEC_VERSION_HPP__
#define __LIBARCSDEC_VERSION_HPP__

/**
 * \file
 *
 * \brief Provide version and name information for libarcsdec.
 */

#include <string>


namespace arcsdec
{
inline namespace v_1_0_0
{

/**
 * \defgroup version Version information
 *
 * \brief Access version information and check version
 *
 * @{
 */

/**
 * \brief Library name.
 */
extern const std::string LIBARCSDEC_NAME;

/**
 * \brief Major version number.
 */
extern const int LIBARCSDEC_VERSION_MAJOR;

/**
 * \brief Minor version number.
 */
extern const int LIBARCSDEC_VERSION_MINOR;

/**
 * \brief Patch level.
 */
extern const int LIBARCSDEC_VERSION_PATCH;

/**
 * \brief Version suffix (e.g. 'alpha.1', 'beta.6', 'rc.3' etc.).
 */
extern const std::string LIBARCSDEC_VERSION_SUFFIX;

/**
 * \brief Complete semantic version information.
 */
extern const std::string LIBARCSDEC_VERSION;

/**
 * \brief Version info as shown by `git describe --always HEAD`.
 */
extern const std::string LIBARCSDEC_GIT_VERSION;

/**
 * \brief Compares specified version to actual version
 *
 * \param[in] major Major version number to test
 * \param[in] minor Minor version number to test
 * \param[in] patch Patch level number to test
 *
 * \return TRUE if actual version is at least specified version
 */
bool api_version_is_at_least(const int major, const int minor, const int patch);

/** @} */

} // namespace v_1_0_0
} // namespace arcsdec

#endif

