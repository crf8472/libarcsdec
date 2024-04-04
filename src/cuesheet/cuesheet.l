/* Flex specification file for C++-based Cuesheet lexer */

/* Supports location tracking and can be combined with other flex C++-lexers */
/* in the same compilation unit (due to using prefixes and namespaces).      */

/* Generate a lexer in C++ */
%option c++
/* option 8bit is default except using options -Cf or -CF which we do not do */

/* Warn about certain things. */
%option warn

/* Generate equivalence classes for characters with equivalent lexical props. */
%option ecs

/* Do not generate a default rule. A non-matching character is an error. */
/* %option nodefault */

/* Assume only one input file: do not call yywrap on EOF, just stop. */
%option noyywrap

/* Specify name of the generated lexer class */
%option yyclass="arcsdec::details::cuesheet::yycue::Lexer"

/* Maintain the lineno */
%option yylineno

/* We do not manipulate yytext */
%option noinput nounput

/* Write summary to stderr when generating the lexer. */
/* %option verbose */

/* See the defined macro of yyFlexLexer in cuesheet_lexer.hpp */
%option prefix="Cuesheet_"
/* Note: each flex-generated scanner inherits from a class whose name is      */
/* specified by macro yyFlexLexer. This macro has to be present in each header*/
/* that defines the flex-generated lexer class. Thus there is no obvious way  */
/* to use different flex-generated lexers in the same compilation unit. The   */
/* only trick to achieve that is to put the definition of each lexer class in */
/* its own header file (for cuesheet: cuesheet_lexer_defs.hpp). Then, a       */
/* wrapper header is defined that redefines yyFlexLexer according to the      */
/* prefix above before including the defs header                              */
/* (for cuesheet: cuesheet_lexer.hpp).                                        */
/* */
/* Cf. Flex Manual, Ch. 18 'Generating C++-Scanners' (very end of the chapter)*/

%s INDEX
%s REMVAL

%x REM

%{
// from cuesheet.l

//  for clang++
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winline-namespace-reopened-noninline"
#pragma clang diagnostic ignored "-Wunknown-warning-option"

//  for g++
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wstrict-overflow"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"


#include <cstring>        /* for strncpy */
#include <cstdlib>        /* for atoi */
#include <iostream>       /* for cout */
#include <iomanip>        /* for setw, setfill, left, right */

#include "cuesheet.tab.hpp"       /* auto-generated: bison token definitions */
#include "cuesheet_location.hpp"  /* auto-generated: position + location */

#ifndef __LIBARCSDEC_CUESHEET_LEXER_DEFS_HPP__
#include "cuesheet_lexer_defs.hpp" /* for Lexer */
#endif
#ifndef __LIBARCSDEC_CUESHEET_DRIVER_HPP__
#include "driver.hpp"              /* for Driver */
#endif


// Redefine calls of yyterminate():
// Original yyterminate() macro returns int. Since we're using Bison 3 variants
// as tokens, call context will expect `Parser::semantic_type` instead.
#define yyterminate()       Parser::make_END(get().loc());

// Track current lexer position + update driver location with it
#define YY_USER_ACTION \
		shift_pos(yylineno, yyleng); \
		driver_.step_to(current_pos_);

// Declare the symbols from yycuesheet that are to be used in yylex().
// (yylex() is in global namespace)
using arcsdec::details::cuesheet::yycuesheet::Parser;
using arcsdec::details::cuesheet::yycuesheet::location;
using arcsdec::details::cuesheet::yycuesheet::position;

// end cuesheet.l
%}

WHITESPACE      [ \t\r]+
DIGITS          [[:digit:]]+
GLYPHS          [[:graph:]]+
QSTRING1        \'([^\'\n]|\\\')*\'
QSTRING2        \"([^\"\n]|\\\")*\"
REM             REM[[:blank:]]+

%%

<*>{WHITESPACE} { /* ignore */ }

{DIGITS}    {
				return Parser::make_NUMBER(YYText(), get().loc());
			}

CATALOG     { return Parser::make_CATALOG(get().loc());    }
FILE        { return Parser::make_FILETAG(get().loc());    }
CDTEXTFILE  { return Parser::make_CDTEXTFILE(get().loc()); }
TITLE       { return Parser::make_TITLE(get().loc());      }
PERFORMER   { return Parser::make_PERFORMER(get().loc());  }
SONGWRITER  { return Parser::make_SONGWRITER(get().loc()); }
TRACK       { return Parser::make_TRACK(get().loc());      }
FLAGS       { return Parser::make_FLAGS(get().loc());      }
PREGAP      { return Parser::make_PREGAP(get().loc());     }
POSTGAP     { return Parser::make_POSTGAP(get().loc());    }
ISRC        { return Parser::make_ISRC(get().loc());       }
INDEX       { BEGIN(INDEX); return Parser::make_INDEX(get().loc()); }

BINARY      { return Parser::make_BINARY(get().loc());   }
MOTOROLA    { return Parser::make_MOTOROLA(get().loc()); }
AIFF        { return Parser::make_AIFF(get().loc());     }
WAVE        { return Parser::make_WAVE(get().loc());     }
MP3         { return Parser::make_MP3(get().loc());      }
FLAC        { return Parser::make_FLAC(get().loc());     }

AUDIO       { return Parser::make_AUDIO(get().loc());      }
MODE1_2048  { /**/ return Parser::make_MODE1_2048(get().loc()); }
MODE1_2352  { return Parser::make_MODE1_2352(get().loc()); }
MODE2_2048  { return Parser::make_MODE2_2048(get().loc()); }
MODE2_2324  { /* ? return Parser::make_MODE2_2324(get().loc()); */ }
MODE2_2332  { /* ? */ return Parser::make_MODE2_2332(get().loc()); }
MODE2_2336  { return Parser::make_MODE2_2336(get().loc()); }
MODE2_2342  { /* ? */ return Parser::make_MODE2_2342(get().loc()); }
MODE2_2352  { return Parser::make_MODE2_2352(get().loc()); }

PRE         { return Parser::make_PRE(get().loc());        }
DCP         { return Parser::make_DCP(get().loc());        }
4CH         { return Parser::make_FOUR_CH(get().loc());    }
SCMS        { return Parser::make_SCMS(get().loc());       }

COMPOSER    { return Parser::make_COMPOSER(get().loc());   }
ARRANGER    { return Parser::make_ARRANGER(get().loc());   }
MESSAGE     { return Parser::make_MESSAGE(get().loc());   }
DISC_ID     { return Parser::make_DISC_ID(get().loc());    }
GENRE       { return Parser::make_GENRE(get().loc());      }
TOC_INFO1   { return Parser::make_TOC_INFO1(get().loc());  }
TOC_INFO2   { return Parser::make_TOC_INFO2(get().loc());  }
UPC_EAN     { return Parser::make_UPC_EAN(get().loc());    }
SIZE_INFO   { return Parser::make_SIZE_INFO(get().loc());  }


{REM}DATE                  { BEGIN(REMVAL); return Parser::make_DATE(get().loc());                  }
{REM}GENRE                 { BEGIN(REMVAL); return Parser::make_REMGENRE(get().loc());              }
{REM}REPLAYGAIN_ALBUM_GAIN { BEGIN(REMVAL); return Parser::make_RPG_ALBUM_GAIN(get().loc()); }
{REM}REPLAYGAIN_ALBUM_PEAK { BEGIN(REMVAL); return Parser::make_RPG_ALBUM_PEAK(get().loc()); }
{REM}REPLAYGAIN_TRACK_GAIN { BEGIN(REMVAL); return Parser::make_RPG_TRACK_GAIN(get().loc()); }
{REM}REPLAYGAIN_TRACK_PEAK { BEGIN(REMVAL); return Parser::make_RPG_TRACK_PEAK(get().loc()); }

<REMVAL>[^\n]* { return Parser::make_STRING(YYText(), get().loc()); }


{REM}       { BEGIN(REM);     /* REM: discard everything from here */ }
<REM>[^\n]* { BEGIN(INITIAL); /* REM: discard rest of line */  }


{QSTRING1}|{QSTRING2} { return Parser::make_STRING(YYText(), get().loc()); }

:			{ return Parser::make_COLON(get().loc()); }

<INITIAL>{GLYPHS}     { return Parser::make_STRING(YYText(), get().loc()); }

<*>\n        { BEGIN(INITIAL); /* acts as delimiter */ }

<INDEX>[^[:blank:][:digit:]:\n]+   { /* rule out from INDEX */ BEGIN(INITIAL); return Parser::make_STRING(YYText(), get().loc()); }

<<EOF>>      { return yyterminate(); }

%%

#pragma GCC diagnostic pop
#pragma clang diagnostic pop

namespace arcsdec { namespace details {
namespace cuesheet { namespace yycuesheet {

void Lexer::notify(const std::string& token_name, const std::string& chars)
{
	driver_.notify(YYSTATE, token_name, chars);
}

void Lexer::unexpected(const std::string& chars, const location& loc)
{
	driver_.unexpected(chars, loc);
}

void Lexer::shift_pos(const int line_no, const int token_length)
{
	// Current position is always the _end_ of the current token

	if (line_no != current_pos_.line) // newline occurred?
	{
		current_pos_.lines(line_no - current_pos_.line);
		current_pos_.column = 1; // Ignore newlines when stepping forward
	} else
	{
		current_pos_.columns(token_length);
	}

	//std::cout << "Line " << line_no << ", token_length: " << token_length;
	//std::cout << " - pos is now: " << current_pos_ << '\n';
}

Driver& Lexer::get()
{
	return driver_;
}

} /* namespace yycuesheet */ } /* namespace cuesheet */
} /*details*/ } /*arcsdec*/

