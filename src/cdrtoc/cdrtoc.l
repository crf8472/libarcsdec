/* Flex specification file for C++-based CDRDAO/TOC lexer */

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
%option yyclass="arcsdec::details::cdrtoc::yycdrtoc::Lexer"

/* Maintain the lineno */
%option yylineno

/* We do not manipulate yytext */
%option noinput nounput

/* Write summary to stderr when generating the lexer. */
/* %option verbose */

/* See the defined macro of yyFlexLexer in cdrtoc_lexer.hpp */
%option prefix="CDRTOC_"
/* Note: each flex-generated scanner inherits from a class whose name is      */
/* specified by macro yyFlexLexer. This macro has to be present in each header*/
/* that defines the flex-generated lexer class. Thus there is no obvious way  */
/* to use different flex-generated lexers in the same compilation unit. The   */
/* only trick to achieve that is to put the definition of each lexer class in */
/* its own header file (for cdrtoc: cdrtoc_lexer_defs.hpp). Then, a     */
/* wrapper header is defined that redefines yyFlexLexer according to the      */
/* prefix above before including the defs header                              */
/* (for cdrtoc: cdrtoc_lexer.hpp).                                      */
/* */
/* Cf. Flex Manual, Ch. 18 'Generating C++-Scanners' (very end of the chapter)*/

%{
// start: from cdrtoc.l

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

#ifndef __LIBARCSDEC_CDRTOC_LEXER_DEFS_HPP__
#include "cdrtoc_lexer_defs.hpp"   /* for Lexer (user-defined) */
#endif

#ifndef __LIBARCSDEC_CDRTOC_TAB_HPP__
#include "cdrtoc_tab.hpp"       /* auto-generated: bison token definitions */
#endif

#include "cdrtoc_location.hpp"  /* auto-generated: position + location */

#ifndef __LIBARCSDEC_FLEXBISONDRIVER_HPP__
#include "flexbisondriver.hpp"  /* for LexerHandler,... */
#endif

//#include <iostream>       /* for cout */
//#include <iomanip>        /* for setw, setfill, left, right */

// Redefine calls of yyterminate():
// Original yyterminate() macro returns int. Since we're using Bison 3 variants
// as tokens, call context will expect `Parser::semantic_type` instead.
#define yyterminate()       Parser::make_END(loc());

// Track current lexer position + update driver location with it
// (will be called from the inner scope of Lexer class)
#define YY_USER_ACTION \
		shift_pos(yylineno, yyleng); \
		current_loc_->step_to(current_pos_);

// end: from cdrtoc.l
%}

WHITESPACE      [ \t\r]+
DIGITS          [[:digit:]]+
GLYPHS          [[:graph:]]+
CTRLCHARS       [[:cntrl:]]+

DQ_STRING       \"([^\"\n]|\\\")+\"
EMPTY_DQ_STRING \"\"
SQ_STRING       \'([^\'\n]|\\\')+\'
EMPTY_SQ_STRING \'\'

COMMENT_START   //[[:blank:]]+
COLON           :
COMMA           ,
HASH            #
BRACE_LEFT      \{
BRACE_RIGHT     \}

%%

<*>{WHITESPACE}     { /* ignore */ }

{DIGITS}            { return Parser::make_NUMBER(YYText(), loc());    }

CD_DA               { return Parser::make_CD_DA(loc());               }
CD_ROM              { return Parser::make_CD_ROM(loc());              }
CD_ROM_XA           { return Parser::make_CD_ROM_XA(loc());           }
CD_I                { return Parser::make_CD_I(loc());                }

CATALOG             { return Parser::make_CATALOG(loc());             }

TRACK               { return Parser::make_TRACK(loc());               }

RW                  { return Parser::make_RW(loc());                  }
RW_RAW              { return Parser::make_RW_RAW(loc());              }

ISRC                { return Parser::make_ISRC(loc());                }
NO                  { return Parser::make_NO(loc());                  }
COPY                { return Parser::make_COPY(loc());                }
PRE_EMPHASIS        { return Parser::make_PRE_EMPHASIS(loc());        }
TWO_CHANNEL_AUDIO   { return Parser::make_TWO_CHANNEL_AUDIO(loc());   }
FOUR_CHANNEL_AUDIO  { return Parser::make_FOUR_CHANNEL_AUDIO(loc());  }
AUDIOFILE           { return Parser::make_AUDIOFILE(loc());           }
FILE                { return Parser::make_FILE_TOKEN(loc());          }
PREGAP              { return Parser::make_PREGAP(loc());              }
SWAP                { return Parser::make_SWAP(loc());                }
DATAFILE            { return Parser::make_DATAFILE(loc());            }
FIFO                { return Parser::make_FIFO(loc());                }
SILENCE             { return Parser::make_SILENCE(loc());             }
ZERO                { return Parser::make_ZERO(loc());                }
START               { return Parser::make_START(loc());               }
END                 { return Parser::make_END_TOKEN(loc());           }
INDEX               { return Parser::make_INDEX(loc());               }

AUDIO               { return Parser::make_AUDIO(loc());               }
MODE0               { return Parser::make_MODE0(loc());               }
MODE1               { return Parser::make_MODE1(loc());               }
MODE1_RAW           { return Parser::make_MODE1_RAW(loc());           }
MODE2               { return Parser::make_MODE2(loc());               }
MODE2_RAW           { return Parser::make_MODE2_RAW(loc());           }
MODE2_FORM1         { return Parser::make_MODE2_FORM1(loc());         }
MODE2_FORM2         { return Parser::make_MODE2_FORM2(loc());         }
MODE2_FORM_MIX      { return Parser::make_MODE2_FORM_MIX(loc());      }

CD_TEXT             { return Parser::make_CD_TEXT(loc());             }
LANGUAGE            { return Parser::make_LANGUAGE(loc());            }
LANGUAGE_MAP        { return Parser::make_LANGUAGE_MAP(loc());        }
TITLE               { return Parser::make_TITLE(loc());               }
PERFORMER           { return Parser::make_PERFORMER(loc());           }
SONGWRITER          { return Parser::make_SONGWRITER(loc());          }
COMPOSER            { return Parser::make_COMPOSER(loc());            }
ARRANGER            { return Parser::make_ARRANGER(loc());            }
MESSAGE             { return Parser::make_MESSAGE(loc());             }
DISC_ID             { return Parser::make_DISC_ID(loc());             }
GENRE               { return Parser::make_GENRE(loc());               }
TOC_INFO1           { return Parser::make_TOC_INFO1(loc());           }
TOC_INFO2           { return Parser::make_TOC_INFO2(loc());           }
RESERVED1           { return Parser::make_RESERVED1(loc());           }
RESERVED2           { return Parser::make_RESERVED2(loc());           }
RESERVED3           { return Parser::make_RESERVED3(loc());           }
RESERVED4           { return Parser::make_RESERVED4(loc());           }
UPC_EAN             { return Parser::make_UPC_EAN(loc());             }
SIZE_INFO           { return Parser::make_SIZE_INFO(loc());           }

EN                  { return Parser::make_EN(loc());                  }
ENCODING_ISO_8859_1 { return Parser::make_ENCODING_ISO_8859_1(loc()); }
ENCODING_ASCII      { return Parser::make_ENCODING_ASCII(loc());      }
ENCODING_MS_JIS     { return Parser::make_ENCODING_MS_JIS(loc());     }
ENCODING_KOREAN     { return Parser::make_ENCODING_KOREAN(loc());     }
ENCODING_MANDARIN   { return Parser::make_ENCODING_MANDARIN(loc());   }

{COLON}             { return Parser::make_COLON(loc());               }
{COMMA}             { return Parser::make_COMMA(loc());               }
{HASH}              { return Parser::make_HASH(loc());                }
{BRACE_LEFT}        { return Parser::make_BRACE_LEFT(loc());          }
{BRACE_RIGHT}       { return Parser::make_BRACE_RIGHT(loc());         }

{DQ_STRING}|{SQ_STRING} {
                      return Parser::make_STRING(strip_quotes(YYText()), loc());
					}

{EMPTY_DQ_STRING}|{EMPTY_SQ_STRING} {
                      return Parser::make_EMPTY_STRING(loc());
					}

"//".*              { /* ignore line comments */ }
\n                  { /* ignore, but required for line count */ }
<<EOF>>             { return yyterminate(); }

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
namespace cdrtoc
{
namespace yycdrtoc
{


LexerHandler* Lexer::handler()
{
	return lexer_handler_;
}

void Lexer::notify(const std::string& token_name, const std::string& chars)
{
	// Commented out, currently not required
	//handler()->notify(token_name, chars);
	// YYSTATE denotes lexer state
}

void Lexer::unexpected(const std::string& chars, const location& loc)
{
	// Commented out, not implemented in handler
	//handler()->unexpected(chars, loc);
}

void Lexer::shift_pos(const int line_no, const int token_length)
{
	// Current position is always the _end_ of the current token
	current_pos_ = shift_lexer_pos(current_pos_, line_no, token_length);

	//std::cout << "Line " << std::setw(3) << line_no;
	//std::cout << ", token_length: " << std::setw(2) << token_length;
	//std::cout << " - pos is now: "  << std::setw(2) << current_pos_ << '\n';
}

Parser::location_type Lexer::loc() const
{
	return current_loc_->loc();
}


} // namespace yycdrtoc
} // namespace cdrtoc
} // namespace details
} // namespace arcsdec

