#ifndef __LIBARCSDEC_CUESHEET_DRIVER_HPP__
#define __LIBARCSDEC_CUESHEET_DRIVER_HPP__
/**
 * \file
 *
 * \brief Driver for cuesheet format.
 */

#ifndef __LIBARCSDEC_CUESHEET_LEXER_HPP__
#include "cuesheet_lexer.hpp"     // for Lexer declaration
#endif
#ifndef __LIBARCSDEC_CUESHEET_TAB_HPP__
#include "cuesheet_tab.hpp"       // auto-generated
#endif

#ifndef __LIBARCSDEC_FLEXBISONDRIVER_HPP__
#include "flexbisondriver.hpp"    // for FlexBisonDriver
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
namespace cuesheet
{

using Driver = FlexBisonDriver<
	yycuesheet::Lexer,
	yycuesheet::Parser,
	yycuesheet::location,
	yycuesheet::position,
	ParserToCHandler
>;

} // namespace cuesheet
} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

#endif

