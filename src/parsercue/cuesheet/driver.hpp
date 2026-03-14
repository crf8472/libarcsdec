#ifndef LIBARCSDEC_CUESHEET_DRIVER_HPP_
#define LIBARCSDEC_CUESHEET_DRIVER_HPP_
/**
 * \file
 *
 * \brief Driver for cuesheet format.
 */

#ifndef LIBARCSDEC_CUESHEET_LEXER_HPP_
#include "cuesheet_lexer.hpp"     // for Lexer declaration
#endif
#ifndef LIBARCSDEC_CUESHEET_TAB_HPP_
#include "cuesheet_tab.hpp"       // auto-generated
#endif
#ifndef LIBARCSDEC_FLEXBISONDRIVER_HPP_
#include "flexbisondriver.hpp"    // for FlexBisonDriver
#endif
#ifndef LIBARCSDEC_TOCHANDLER_HPP_
#include "tochandler.hpp"         // for ParserToCHandler
#endif


namespace arcsdec
{
                                                  /** \cond NAMESPACE_v_1_0_0 */
inline namespace v_1_0_0
{
                                                                 /** \endcond */
namespace read
{
namespace details
{

/**
 * \brief Implementation for parsing Cuesheets.
 */
namespace cuesheet
{

/**
 * \brief Driver for parsercue.
 */
using Driver = FlexBisonDriver<yycuesheet::Lexer, yycuesheet::Parser,
	yycuesheet::location, yycuesheet::position, ParserToCHandler>;

/**
 * \brief Flex/Bison implementation of parsercue.
 */
namespace yycuesheet
{/*for doxygen*/}

} // namespace cuesheet
} // namespace details
} // namespace read
                                                  /** \cond NAMESPACE_v_1_0_0 */
} // namespace v_1_0_0
                                                                 /** \endcond */
} // namespace arcsdec

#endif

