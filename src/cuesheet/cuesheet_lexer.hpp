#ifndef __LIBARCSDEC_CUESHEET_LEXER_HPP__
#define __LIBARCSDEC_CUESHEET_LEXER_HPP__

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
#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Weffc++"


#define yyFlexLexer Cuesheet_FlexLexer
#include <FlexLexer.h>
#undef yyFlexLexer

#include "cuesheet_lexer_defs.hpp"


#pragma GCC diagnostic pop

#endif // __LIBARCSDEC_CUESHEET_LEXER_HPP__

