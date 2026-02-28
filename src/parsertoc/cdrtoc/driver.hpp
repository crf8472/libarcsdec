#ifndef __LIBARCSDEC_CDRTOC_DRIVER_HPP__
#define __LIBARCSDEC_CDRTOC_DRIVER_HPP__
/**
 * \file
 *
 * \brief Driver for cdrtoc format.
 */

#ifndef __LIBARCSDEC_CDRTOC_LEXER_HPP__
#include "cdrtoc_lexer.hpp"     // for Lexer declaration
#endif
#ifndef __LIBARCSDEC_CDRTOC_TAB_HPP__
#include "cdrtoc_tab.hpp"       // auto-generated
#endif

#ifndef __LIBARCSDEC_FLEXBISONDRIVER_HPP__
#include "flexbisondriver.hpp"  // for FlexBisonDriver
#endif
#ifndef __LIBARCSDEC_TOCHANDLER_HPP__
#include "tochandler.hpp"       // for ParserToCHandler
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
 * \brief Implementation for parsing CDRTOC documents.
 */
namespace cdrtoc
{

/**
 * \brief Driver for parsertoc.
 */
using Driver = FlexBisonDriver<yycdrtoc::Lexer, yycdrtoc::Parser,
	yycdrtoc::location, yycdrtoc::position, ParserToCHandler>;

/**
 * \brief Flex/Bison implementation of parsertoc.
 */
namespace yycdrtoc
{/*for doxygen*/}

} // namespace cdrtoc
} // namespace details
} // namespace read
                                                  /** \cond NAMESPACE_v_1_0_0 */
} // namespace v_1_0_0
                                                                 /** \endcond */
} // namespace arcsdec

#endif

