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
// from cdrtoc.l

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

#include "cdrtoc.tab.hpp"       /* auto-generated: bison token definitions */
#include "cdrtoc_location.hpp"  /* auto-generated: position + location */

#ifndef __LIBARCSDEC_CDRTOC_LEXER_DEFS_HPP__
#include "cdrtoc_lexer_defs.hpp" /* for Lexer */
#endif
#ifndef __LIBARCSDEC_CDRTOC_DRIVER_HPP__
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

// Declare the symbols from yycdrtoc that are to be used in yylex().
// (yylex() is in global namespace)
using arcsdec::details::cdrtoc::yycdrtoc::Parser;
using arcsdec::details::cdrtoc::yycdrtoc::location;
using arcsdec::details::cdrtoc::yycdrtoc::position;

// remove quotes from quoted string
static std::string remove_quotes(const std::string& s)
{
	if (s.length() < 2)  { return s; };
	if (s.length() == 2) { return std::string{}; };

	return s.substr(1, s.length() - 2);
}

// end cdrtoc.l
%}

WHITESPACE      [ \t\r]+
DIGITS          [[:digit:]]+
GLYPHS          [[:graph:]]+
CTRLCHARS       [[:cntrl:]]+
QSTRING1        \'([^\'\n]|\\\')*\'
QSTRING2        \"([^\"\n]|\\\")*\"

COMMENT_START   //[[:blank:]]+
COLON           :
COMMA           ,
HASH            #
BRACE_OPEN      \{
BRACE_CLOSE     \}

%%

<*>{WHITESPACE} { /* ignore */ }

{DIGITS}       { return Parser::make_NUMBER(YYText(), get().loc()); }

CD_DA          { return Parser::make_CD_DA(get().loc());          }
CD_ROM         { return Parser::make_CD_ROM(get().loc());         }
CD_ROM_XA      { return Parser::make_CD_ROM_XA(get().loc());      }
CD_I           { return Parser::make_CD_I(get().loc());           }

CATALOG        { return Parser::make_CATALOG(get().loc());        }

TRACK          { return Parser::make_TRACK(get().loc());          }
ISRC           { return Parser::make_ISRC(get().loc());           }
AUDIOFILE      { return Parser::make_AUDIOFILE(get().loc());      }
FILE           { return Parser::make_FILE_TOKEN(get().loc());     }

NO                 { return Parser::make_NO(get().loc());                 }
COPY               { return Parser::make_COPY(get().loc());               }
PRE_EMPHASIS       { return Parser::make_PRE_EMPHASIS(get().loc());       }
TWO_CHANNEL_AUDIO  { return Parser::make_TWO_CHANNEL_AUDIO(get().loc());  }
FOUR_CHANNEL_AUDIO { return Parser::make_FOUR_CHANNEL_AUDIO(get().loc()); }
PREGAP             { return Parser::make_PREGAP(get().loc());             }
START              { return Parser::make_START(get().loc());              }
END                { return Parser::make_END_TOKEN(get().loc());          }
INDEX              { return Parser::make_INDEX(get().loc());              }

SWAP           { return Parser::make_SWAP(get().loc());           }
DATAFILE       { return Parser::make_DATAFILE(get().loc());       }
FIFO           { return Parser::make_FIFO(get().loc());           }
SILENCE        { return Parser::make_SILENCE(get().loc());        }
ZERO           { return Parser::make_ZERO(get().loc());           }

AUDIO          { return Parser::make_AUDIO(get().loc());          }
MODE0          { return Parser::make_MODE0(get().loc());          }
MODE1          { return Parser::make_MODE1(get().loc());          }
MODE1_RAW      { return Parser::make_MODE1_RAW(get().loc());      }
MODE2          { return Parser::make_MODE2(get().loc());          }
MODE2_RAW      { return Parser::make_MODE2_RAW(get().loc());      }
MODE2_FORM1    { return Parser::make_MODE2_FORM1(get().loc());    }
MODE2_FORM2    { return Parser::make_MODE2_FORM2(get().loc());    }
MODE2_FORM_MIX { return Parser::make_MODE2_FORM_MIX(get().loc()); }

RW             { return Parser::make_RW(get().loc());             }
RW_RAW         { return Parser::make_RW_RAW(get().loc());         }

CD_TEXT        { return Parser::make_CD_TEXT(get().loc());        }
LANGUAGE       { return Parser::make_LANGUAGE(get().loc());       }
LANGUAGE_MAP   { return Parser::make_LANGUAGE_MAP(get().loc());   }
TITLE          { return Parser::make_TITLE(get().loc());          }
PERFORMER      { return Parser::make_PERFORMER(get().loc());      }
SONGWRITER     { return Parser::make_SONGWRITER(get().loc());     }
COMPOSER       { return Parser::make_COMPOSER(get().loc());       }
ARRANGER       { return Parser::make_ARRANGER(get().loc());       }
MESSAGE        { return Parser::make_MESSAGE(get().loc());        }
DISC_ID        { return Parser::make_DISC_ID(get().loc());        }
GENRE          { return Parser::make_GENRE(get().loc());          }
TOC_INFO1      { return Parser::make_TOC_INFO1(get().loc());      }
TOC_INFO2      { return Parser::make_TOC_INFO2(get().loc());      }
RESERVED1      { return Parser::make_RESERVED1(get().loc());      }
RESERVED2      { return Parser::make_RESERVED2(get().loc());      }
RESERVED3      { return Parser::make_RESERVED3(get().loc());      }
RESERVED4      { return Parser::make_RESERVED4(get().loc());      }
CLOSED         { return Parser::make_CLOSED(get().loc());         }
UPC_EAN        { return Parser::make_UPC_EAN(get().loc());        }
SIZE_INFO      { return Parser::make_SIZE_INFO(get().loc());      }

EN                  { return Parser::make_EN(get().loc());                  }
ENCODING_ISO_8859_1 { return Parser::make_ENCODING_ISO_8859_1(get().loc()); }
ENCODING_ASCII      { return Parser::make_ENCODING_ASCII(get().loc());      }
ENCODING_MS_JIS     { return Parser::make_ENCODING_MS_JIS(get().loc());     }
ENCODING_KOREAN     { return Parser::make_ENCODING_KOREAN(get().loc());     }
ENCODING_MANDARIN   { return Parser::make_ENCODING_MANDARIN(get().loc());   }

{BRACE_OPEN}   { return Parser::make_BRACE_OPEN(get().loc());     }
{BRACE_CLOSE}  { return Parser::make_BRACE_CLOSE(get().loc());    }
{COLON}        { return Parser::make_COLON(get().loc());          }
{COMMA}        { return Parser::make_COMMA(get().loc());          }

{QSTRING1}|{QSTRING2} {
          return Parser::make_STRING(remove_quotes(YYText()), get().loc()); }

"//".*      { /* ignore */ }

<<EOF>>     { return yyterminate(); }

%%

#pragma GCC diagnostic pop
#pragma clang diagnostic pop

namespace arcsdec { namespace details {
namespace cdrtoc { namespace yycdrtoc {

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

} /* namespace yycdrtoc */ } /* namespace cdrtoc */
} /*details*/ } /*arcsdec*/

