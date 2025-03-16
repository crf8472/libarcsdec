#ifndef __LIBARCSDEC_CDRTOC_DRIVER_HPP__
#define __LIBARCSDEC_CDRTOC_DRIVER_HPP__
/**
 * \file
 *
 * \brief Driver for cdrtoc format.
 */

#ifndef __LIBARCSDEC_CDRTOC_LEXER_HPP__
#include "cdrtoc_lexer.hpp" // for Lexer declaration
#endif

#include "cdrtoc.tab.hpp" // auto-generated

#ifndef __LIBARCSDEC_FLEXBISONDRIVER_HPP__
#include "flexbisondriver.hpp"
#endif

namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{

// forward declase auto-generated classes
namespace cdrtoc {
namespace yycdrtoc {
	class Lexer;
	class Parser;
	class location;
	class position;
} /* namespace yycdrtoc */
} /* namespace cdrtoc */

// provide Driver class for namespace
using CdrtocDriver = Driver<
	cdrtoc::yycdrtoc::Lexer,
	cdrtoc::yycdrtoc::Parser,
	cdrtoc::yycdrtoc::location,
	cdrtoc::yycdrtoc::position
>;

} // namespace details
} // namespace V_1_0_0
} // namespace arcsdec

#endif

