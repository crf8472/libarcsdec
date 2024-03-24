#ifndef CUESHEET_YYCUESHEET_LEXER_HPP
#define CUESHEET_YYCUESHEET_LEXER_HPP

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
 * [2] https://stackoverflow.com/questions/35606354/multiple-parsers-in-flex-bison-include-fails
 */
#define yyFlexLexer Cuesheet_FlexLexer
#include <FlexLexer.h>
#undef yyFlexLexer

#include "lexer_defs.hpp"

#endif // CUESHEET_YYCUESHEET_LEXER_HPP

