#ifndef LIBARCSDEC_CDRTOC_DRIVER_HPP_
#define LIBARCSDEC_CDRTOC_DRIVER_HPP_
/**
 * \file
 *
 * \brief Driver for cdrtoc format.
 */

#ifndef LIBARCSDEC_CDRTOC_LEXER_HPP_
#include "cdrtoc_lexer.hpp"     // for Lexer declaration
#endif
#ifndef LIBARCSDEC_CDRTOC_TAB_HPP_
#include "cdrtoc_tab.hpp"       // auto-generated
#endif
#ifndef LIBARCSDEC_FLEXBISONDRIVER_HPP_
#include "flexbisondriver.hpp"  // for FlexBisonDriver
#endif
#ifndef LIBARCSDEC_TOCHANDLER_HPP_
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

