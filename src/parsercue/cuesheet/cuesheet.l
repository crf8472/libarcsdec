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
%option yyclass="arcsdec::details::cuesheet::yycuesheet::Lexer"

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
// start: from cuesheet.l

//  for clang++
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winline-namespace-reopened-noninline"
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#endif

//  for g++
#if defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wstrict-overflow"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif

#ifndef __LIBARCSDEC_CUESHEET_LEXER_DEFS_HPP__
#include "cuesheet_lexer_defs.hpp" /* for Lexer (user-defined) */
#endif

#ifndef __LIBARCSDEC_CUESHEET_TAB_HPP__
#include "cuesheet.tab.hpp"       /* auto-generated: bison token definitions */
#endif

#include "cuesheet_location.hpp"  /* auto-generated: position + location */

#ifndef __LIBARCSDEC_FLEXBISONDRIVER_HPP__
#include "flexbisondriver.hpp"  /* for LexerHandler,... */
#endif


// Redefine calls of yyterminate():
// Original yyterminate() macro returns int. Since we're using Bison 3 variants
// as tokens, call context will expect `Parser::semantic_type` instead.
#define yyterminate()       Parser::make_END(loc());

// Track current lexer position + update driver location with it
// (will be called from the inner scope of Lexer class)
#define YY_USER_ACTION \
		shift_pos(yylineno, yyleng); \
		current_loc_->step_to(current_pos_);

// end: from cuesheet.l
%}

WHITESPACE      [ \t\r]+
DIGITS          [[:digit:]]+
GLYPHS          [[:graph:]]+
CTRLCHARS       [[:cntrl:]]+
QSTRING1        \'([^\'\n]|\\\')*\'
QSTRING2        \"([^\"\n]|\\\")*\"
REM             REM[[:blank:]]+

%%

<*>{WHITESPACE} { /* ignore */ }

{DIGITS}    {
				return Parser::make_NUMBER(YYText(), loc());
			}

CATALOG     { return Parser::make_CATALOG(loc());             }
FILE        { return Parser::make_FILE_TOKEN(loc());          }
CDTEXTFILE  { return Parser::make_CDTEXTFILE(loc());          }
TITLE       { return Parser::make_TITLE(loc());               }
PERFORMER   { return Parser::make_PERFORMER(loc());           }
SONGWRITER  { return Parser::make_SONGWRITER(loc());          }
TRACK       { return Parser::make_TRACK(loc());               }
FLAGS       { return Parser::make_FLAGS(loc());               }
PREGAP      { return Parser::make_PREGAP(loc());              }
POSTGAP     { return Parser::make_POSTGAP(loc());             }
ISRC        { return Parser::make_ISRC(loc());                }
INDEX       { BEGIN(INDEX); return Parser::make_INDEX(loc()); }

BINARY      { return Parser::make_BINARY(loc());				}
MOTOROLA    { return Parser::make_MOTOROLA(loc());			}
AIFF        { return Parser::make_AIFF(loc());				}
WAVE        { return Parser::make_WAVE(loc());				}
MP3         { return Parser::make_MP3(loc());					}
FLAC        { return Parser::make_FLAC(loc());				}

AUDIO       { return Parser::make_AUDIO(loc());               }
MODE1_2048  { /**/ return Parser::make_MODE1_2048(loc());     }
MODE1_2352  { return Parser::make_MODE1_2352(loc());          }
MODE2_2048  { return Parser::make_MODE2_2048(loc());          }
MODE2_2324  { /* ? return Parser::make_MODE2_2324(loc()); */  }
MODE2_2332  { /* ? */ return Parser::make_MODE2_2332(loc());  }
MODE2_2336  { return Parser::make_MODE2_2336(loc());          }
MODE2_2342  { /* ? */ return Parser::make_MODE2_2342(loc());  }
MODE2_2352  { return Parser::make_MODE2_2352(loc());          }

PRE         { return Parser::make_PRE(loc());                 }
DCP         { return Parser::make_DCP(loc());                 }
4CH         { return Parser::make_FOUR_CH(loc());             }
SCMS        { return Parser::make_SCMS(loc());                }

COMPOSER    { return Parser::make_COMPOSER(loc());            }
ARRANGER    { return Parser::make_ARRANGER(loc());            }
MESSAGE     { return Parser::make_MESSAGE(loc());             }
DISC_ID     { return Parser::make_DISC_ID(loc());             }
GENRE       { return Parser::make_GENRE(loc());               }
TOC_INFO1   { return Parser::make_TOC_INFO1(loc());           }
TOC_INFO2   { return Parser::make_TOC_INFO2(loc());           }
UPC_EAN     { return Parser::make_UPC_EAN(loc());             }
SIZE_INFO   { return Parser::make_SIZE_INFO(loc());           }


{REM}DATE   { BEGIN(REMVAL); return Parser::make_DATE(loc());     }

{REM}GENRE  { BEGIN(REMVAL); return Parser::make_REMGENRE(loc()); }

{REM}REPLAYGAIN_ALBUM_GAIN {
			BEGIN(REMVAL);
			return Parser::make_RPG_ALBUM_GAIN(loc());
			}

{REM}REPLAYGAIN_ALBUM_PEAK {
			BEGIN(REMVAL);
			return Parser::make_RPG_ALBUM_PEAK(loc());
			}

{REM}REPLAYGAIN_TRACK_GAIN {
			BEGIN(REMVAL);
			return Parser::make_RPG_TRACK_GAIN(loc());
			}

{REM}REPLAYGAIN_TRACK_PEAK {
			BEGIN(REMVAL);
			return Parser::make_RPG_TRACK_PEAK(loc());
			}

<REMVAL>[^\n]* {
			return Parser::make_STRING(YYText(), loc());
			}

{REM}       { BEGIN(REM);     /* REM: discard everything from here */ }

<REM>[^\n]* { BEGIN(INITIAL); /* REM: discard rest of line */         }

{QSTRING1}|{QSTRING2} {
			return Parser::make_STRING(
				lexer_strip_quotes(YYText()), loc());
			}

:			{ return Parser::make_COLON(loc()); }

<INITIAL>{GLYPHS}  {
			return Parser::make_STRING(YYText(), loc());
			}

<INITIAL>{CTRLCHARS}  { /* ignore things like "ISRC ^\W" and other junk */ }

<*>\n       { BEGIN(INITIAL); /* acts as delimiter */ }

<INDEX>[^[:blank:][:digit:]:\n]+  {
			/* rule out from INDEX */
			BEGIN(INITIAL);
			return Parser::make_STRING(YYText(), loc());
			}

<<EOF>>     { return yyterminate(); }

%%

#if defined(__GNUG__)
#pragma GCC diagnostic pop
#endif

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

namespace arcsdec
{
namespace details
{
namespace cuesheet
{
namespace yycuesheet
{


LexerHandler* Lexer::handler()
{
	return lexer_handler_;
}

void Lexer::notify(const std::string& token_name, const std::string& chars)
{
	//driver_.notify(YYSTATE, token_name, chars);
}

void Lexer::unexpected(const std::string& chars, const location& loc)
{
	//driver_.unexpected(chars, loc);
}

void Lexer::shift_pos(const int line_no, const int token_length)
{
	// Current position is always the _end_ of the current token
	current_pos_ = lexer_shift_pos(current_pos_, line_no, token_length);

}

Parser::location_type Lexer::loc() const
{
	return current_loc_->loc();
}


} // namespace yycuesheet
} // namespace cuesheet
} // namespace details
} // namespace arcsdec

