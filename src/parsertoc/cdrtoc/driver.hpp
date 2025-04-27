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
#include "tochandler.hpp"         // for ParserToCHandler
#endif

namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{
namespace cdrtoc
{

using Driver = FlexBisonDriver<
	yycdrtoc::Lexer,
	yycdrtoc::Parser,
	yycdrtoc::location,
	yycdrtoc::position,
	ParserToCHandler
>;

} // namespace cdrtoc
} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

#endif

