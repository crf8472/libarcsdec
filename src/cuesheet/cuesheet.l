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
%option nodefault

/* Assume only one input file: do not call yywrap on EOF, just stop. */
%option noyywrap

/* Specify name of the generated lexer class */
%option yyclass="cuesheet::yycue::Lexer"

/* Maintain the lineno */
%option yylineno

/* Write summary to stderr when generating the lexer. */
%option verbose

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

%option noinput
%option nounput
%option caseless

%{
// from cuesheet.l

#include <cstring>        /* for strncpy */
#include <iostream>       /* for cout */
#include <iomanip>        /* for setw, setfill, left, right */

#include "cuesheet_lexer_defs.hpp" /* for Lexer */
#include "driver.hpp"              /* for Driver */

#include "cuesheet.tab.hpp" /* auto-generated: Token Definitions of bison */
#include "location.hh"      /* auto-generated */


// Redefine calls of yyterminate():
// Original yyterminate() macro returns int. Since we're using Bison 3 variants
// as tokens, call context will expect `Parser::semantic_type` instead.
#define yyterminate() cuesheet::yycuesheet::Parser::make_END(get().loc());

// Track current lexer location
#define YY_USER_ACTION shift_pos(yylineno, yyleng);

// Declare the symbols from yycuesheet that are to be used in yylex().
// (yylex() is in global namespace)
using cuesheet::yycuesheet::Parser;
using cuesheet::yycuesheet::location;
using cuesheet::yycuesheet::position;

//char yy_buffer[PARSER_BUFFER];

// end cuesheet.l
%}

WHITESPACE	[ \t\r]+

%%

\'([^\']|\\\')*\' |
\"([^\"]|\\\")*\" {
				/*
				const auto length =
					(yyleng > sizeof(yy_buffer) ? sizeof(yy_buffer) : yyleng);
				yylval.sval = std::strncpy(yy_buffer, ++yytext, length);
				yylval.sval[length - 2] = '\0';
				*/
				return Parser::make_STRING(yytext, get().loc());
			}

[[:digit:]]+ {
				return Parser::make_NUMBER(atoi(yytext), get().loc());
			}

:           { return Parser::make_COLON(get().loc()); }

WHITESPACE	{ /* ignore */ }


CATALOG     { return Parser::make_CATALOG(get().loc()); }
FILETAG     { return Parser::make_FILETAG(get().loc()); }
CDTEXTFILE  { return Parser::make_CDTEXTFILE(get().loc()); }
TITLE       { return Parser::make_TITLE(get().loc()); }
PERFORMER   { return Parser::make_PERFORMER(get().loc()); }
SONGWRITER  { return Parser::make_SONGWRITER(get().loc()); }
TRACK       { return Parser::make_TRACK(get().loc()); }
FLAGS       { return Parser::make_FLAGS(get().loc()); }
PREGAP      { return Parser::make_PREGAP(get().loc()); }
POSTGAP     { return Parser::make_POSTGAP(get().loc()); }
ISRC        { return Parser::make_ISRC(get().loc()); }
INDEX       { return Parser::make_INDEX(get().loc()); }
REM         {  }

BINARY      { return Parser::make_BINARY(get().loc()); }
MOTOROLA    { return Parser::make_MOTOROLA(get().loc()); }
AIFF        { return Parser::make_AIFF(get().loc());     }
WAVE        { return Parser::make_WAVE(get().loc());     }
MP3         { return Parser::make_MP3(get().loc());      }
FLAC        { return Parser::make_FLAC(get().loc());     }

AUDIO       { return Parser::make_AUDIO(get().loc());      }
MODE1_2048  { return Parser::make_MODE1_2048(get().loc()); }
MODE1_2352  { return Parser::make_MODE1_2352(get().loc()); }
MODE2_2336  { return Parser::make_MODE2_2336(get().loc()); }
MODE2_2048  { return Parser::make_MODE2_2048(get().loc()); }
MODE2_2342  { return Parser::make_MODE2_2342(get().loc()); }
MODE2_2332  { return Parser::make_MODE2_2332(get().loc()); }
MODE2_2352  { return Parser::make_MODE2_2352(get().loc()); }

COMPOSER    { return Parser::make_COMPOSER(get().loc());      }
ARRANGER    { return Parser::make_ARRANGER(get().loc());      }
DISC_ID     { return Parser::make_DISC_ID(get().loc());      }
GENRE       { return Parser::make_GENRE(get().loc());      }
TOC_INFO1   { return Parser::make_TOC_INFO1(get().loc());      }
TOC_INFO2   { return Parser::make_TOC_INFO2(get().loc());      }
UPC_EAN     { return Parser::make_UPC_EAN(get().loc());      }
SIZE_INFO   { return Parser::make_SIZE_INFO(get().loc());      }

DATE                  { return Parser::make_DATE(get().loc()); }
GENRE                 { return Parser::make_GENRE(get().loc()); }
REPLAYGAIN_ALBUM_GAIN { return Parser::make_REPLAYGAIN_ALBUM_GAIN(get().loc()); }
REPLAYGAIN_ALBUM_PEAK { return Parser::make_REPLAYGAIN_ALBUM_PEAK(get().loc()); }
REPLAYGAIN_TRACK_GAIN { return Parser::make_REPLAYGAIN_TRACK_GAIN(get().loc()); }
REPLAYGAIN_TRACK_PEAK { return Parser::make_REPLAYGAIN_TRACK_PEAK(get().loc()); }

<<EOF>>     {
				return yyterminate();
			}


%%


namespace cuesheet { namespace yycuesheet {


void Printer::token(const int state,
		const std::string &token_name, const std::string &token_val) const
{
	if (true)
	{
	out_ << std::setw( 2) << std::right << state;
	out_ << "   ";
	out_ << std::setw(15) << std::left  << token_name;
	out_ << "   ";
	out_ << std::setw(15) << std::left  << token_val;
	out_ << std::endl;
	}
}

std::ostream& Printer::error()
{
	out_ << "Lexer error: ";
	return out_;
}

std::ostream& Printer::warn()
{
	out_ << "Lexer warning: ";
	return out_;
}

std::ostream& Printer::info()
{
	out_ << "Lexer info: ";
	return out_;
}

void Lexer::notify(const std::string &token_name, const std::string &characters)
{
	printer().token(YYSTATE, token_name, characters);
}

void Lexer::unexpected(const std::string &chars, const location &loc)
{
	printer().error() << "Unexpected character '" << chars
		<< "' in line "  << loc.begin.line
		<< " at column " << loc.begin.column
		<< std::endl;
}

void Lexer::shift_pos(const int line_no, const int token_length)
{
	// Current position is always the _end_ of the current token

	if (current_pos_.line != line_no) {
		current_pos_.line = line_no;
		current_pos_.column = 0;
	}
	current_pos_.columns(token_length - 1);

	driver_.update_loc(current_pos_);
}

Printer& Lexer::printer()
{
	return *printer_;
}

Driver& Lexer::get()
{
	return driver_;
}

} /* namespace yycuesheet */ } /* namespace cuesheet */

