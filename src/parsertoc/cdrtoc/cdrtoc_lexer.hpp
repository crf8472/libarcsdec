#ifndef __LIBARCSDEC_CDRTOC_LEXER_HPP__
#define __LIBARCSDEC_CDRTOC_LEXER_HPP__

/**
 * Generated Flex class name is yyFlexLexer by default.
 *
 * If we want to use more flex-generated classes each of them must have a
 * unique class name. This can be achieved by specifying the 'prefix' option
 * in the .l-file.
 *
 * Unfortunately the implementation relies on this trick with redefining class
 * name with a preprocessor macro.
 *
 * See:
 * [1] GNU Flex manual, section: "Generating C++ Scanners".
 * [2] https://stackoverflow.com/q/35606354
 */

#if defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#endif


#define yyFlexLexer CDRTOC_FlexLexer
#include <FlexLexer.h>
#undef yyFlexLexer

#ifndef __LIBARCSDEC_CDRTOC_LEXER_DEFS_HPP__
#include "cdrtoc_lexer_defs.hpp"
#endif


#if defined(__GNUG__)
#pragma GCC diagnostic pop
#endif

#endif // __LIBARCSDEC_CDRTOC_LEXER_HPP__

