/* Bison grammar file for C++-based Cuesheet parser */

%skeleton "lalr1.cc" /* recommended newer parser skeleton for 3.2 */
%require  "3.2"
%language "c++"

/* Avoid name conflicts with other parsers by prefixing this parser's names */
/* %define api.prefix             {toc} */
/* https://www.gnu.org/software/bison/manual/bison.html#Multiple-Parsers */

/* Define filename for location header (instead of location.hh) */
%define api.location.file      "cuesheet_location.hpp"

/* Namespace for generated parser class */
%define api.namespace          {arcsdec::v_1_0_0::details::cuesheet::yycuesheet}

/* Classname for generated parser class */
%define api.parser.class       {Parser}

/* Request symbols to be handled as a whole (type, value, location) in lexer */
%define api.token.constructor

/* Add prefix to token name */
%define api.token.prefix       {CUESHEET_}

/* Move tokens (instead of copy) when passing them to make_<TOKENNAME>() */
%define api.value.automove

/* Use bison variants as token types (requires token.constructor) */
%define api.value.type         variant

/* Runtime assertions to check proper destruction of tokens (requires RTTI) */
%define parse.assert

/* Report unexpected token as well as expected ones. */
%define parse.error            verbose

/* Location tracking */
%locations

/* Define parameters to pass to yylex() */
%lex-param   { Lexer* lexer } /* to apply token rules */

/* Define parameters to pass to Parser::Parser() */
%parse-param { arcsdec::details::TokenLocation<position,location>* loc }
%parse-param { Lexer* lexer } /* to call parser */
%parse-param { ParserToCHandler* handler } /* to implement parser rules */


/* Goes to .tab.hpp header file (therefore included in source) */
%code requires
{
	// Declaration of class 'Parser' refers to 'TokenLocation' in constructor
	#ifndef __LIBARCSDEC_FLEXBISONDRIVER_HPP__
	#include "flexbisondriver.hpp"   // for TokenLocation
	#endif

	// Declaration of class 'Parser' refers to 'ParserTocHandler' in constructor
	#ifndef __LIBARCSDEC_TOCHANDLER_HPP__
	#include "tochandler.hpp"        // for ParserTocHandler
	#endif

	// Forward declare what we are about to use
	int yylex (void);
	int yyerror (const char *s);

	// Forward declarations (require C++17)
	namespace arcsdec
	{
	inline namespace v_1_0_0
	{
	namespace details::cuesheet::yycuesheet
	{
		class Lexer;
	}
	} // namespace v_1_0_0
	} // namespace arcsdec
}


/* Goes to source file _before_ cuesheet.tab.hpp is included */
%code top
{
	// To use member functions of 'ParserTocHandler' in production rule actions
	// (Include also required in cuesheet.tab.hpp)
	#ifndef __LIBARCSDEC_TOCHANDLER_HPP__
	#include "tochandler.hpp"       // for ParserTocHandler, ...
	#endif

	// Re-declaration of yylex() below requires declaration of class 'Lexer'
	#ifndef __LIBARCSDEC_CDRTOC_LEXER_HPP__
	#include "cuesheet_lexer.hpp"            // for Lexer
	#endif

	#ifndef __LIBARCSTK_LOGGING_HPP__
	#include <arcstk/logging.hpp>
	#endif

	// Required in the implementation of production rules
	#include <cstdlib>     // for atoi
	#include <string>      // for string

	// Override yylex() to be called in Parser::parse().
	// Only called inside bison, so make it static to limit visibility to TU.
	// Namespaces are required since yylex() is in global namespace.
	static auto yylex(arcsdec::details::cuesheet::yycuesheet::Lexer* lexer)
		-> arcsdec::details::cuesheet::yycuesheet::Parser::symbol_type
	{
		return lexer->next_token(); // renamed yylex()
	}
	// Note: could also be achieved by doing something like:
    // #define yylex(Lexer* l, Location* loc) lexer.next_token()
	// But let's avoid macros if we can.
	//  for clang++
	#if defined(__clang__)
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Winline-namespace-reopened-noninline"
	#endif

	//  for g++
	#if defined(__GNUG__)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Weffc++"
	#endif
}


/* Terminals */

%token CATALOG
%token FILE_TOKEN
%token CDTEXTFILE
%token TITLE
%token PERFORMER
%token SONGWRITER
%token TRACK
%token FLAGS
%token PREGAP
%token POSTGAP
%token ISRC
%token INDEX
%token REM

%token BINARY
%token MOTOROLA
%token AIFF
%token WAVE
%token MP3
%token FLAC

%token TRACK_ISRC

%token AUDIO
%token MODE1_2048
%token MODE1_2352
%token MODE2_2336
%token MODE2_2048
%token MODE2_2342
%token MODE2_2332
%token MODE2_2352

%token PRE
%token DCP
%token FOUR_CH
%token SCMS

%token COMPOSER
%token ARRANGER
%token MESSAGE
%token DISC_ID
%token GENRE
%token TOC_INFO1
%token TOC_INFO2
%token UPC_EAN
%token SIZE_INFO

%token DATE
%token REMGENRE
%token RPG_ALBUM_GAIN
%token RPG_ALBUM_PEAK
%token RPG_TRACK_GAIN
%token RPG_TRACK_PEAK

%token <std::string> NUMBER
%token <std::string> STRING
%token COLON

%token END 0 "End of file"

/* Non-Terminals */

%nterm <int32_t> msf_time
/* %nterm <FILE_FORMAT>               file_format_tag */
/* %nterm <TRACK_MODE>                track_mode_tag  */
/* %nterm <TRACK_FLAG>                track_flag      */
/* %nterm <std::vector<TRACK_FLAG>>   track_flags     */


%%

cuesheet
	:   /* start hook */
		{
			handler->start_input();
		}

		global_statements track_list
		{
			handler->end_input();
		}
	;

global_statements
	: global_statements global_statement
	| global_statement
	;

global_statement
	: CATALOG NUMBER
		{
			handler->set_mcn($2);
		}
	| FILE_TOKEN STRING file_format_tag
		{
			handler->append_filename($2);
		}
	| CDTEXTFILE STRING
	| cdtext_statement
	| rem_statement
	;

file_format_tag
	: BINARY    { /*$$ = FILE_FORMAT::BINARY;  */ /* cdrwin */ }
	| MOTOROLA  { /*$$ = FILE_FORMAT::MOTOROLA;*/ /* cdrwin */ }
	| AIFF      { /*$$ = FILE_FORMAT::AIFF;    */ /* cdrwin */ }
	| WAVE      { /*$$ = FILE_FORMAT::WAVE;    */ /* cdrwin */ }
	| MP3       { /*$$ = FILE_FORMAT::MP3;     */ /* cdrwin */ }
	| FLAC      { /*$$ = FILE_FORMAT::FLAC;    */ }
	;

cdtext_statement
	: TITLE STRING
	| PERFORMER STRING
	| SONGWRITER STRING
	| ISRC STRING
	| cdtext_tag STRING
	;

cdtext_tag
	: COMPOSER
	| ARRANGER
	| MESSAGE
	| GENRE
	| DISC_ID
	| TOC_INFO1
	| TOC_INFO2
	| UPC_EAN
	| SIZE_INFO
	;

rem_statement
	: rem_item STRING
	;

rem_item
	: DATE
	| REMGENRE
	| RPG_ALBUM_GAIN
	| RPG_ALBUM_PEAK
	| RPG_TRACK_GAIN
	| RPG_TRACK_PEAK
	;

track_list
	: track_list track
	| track
	;

track
	: track_header track_statements
	;

track_header
	: TRACK NUMBER track_mode_tag
		{
			handler->inc_current_track();
		}
	;

track_mode_tag
	: AUDIO       { /*$$ = TRACK_MODE::AUDIO;      */}
	| MODE1_2048  { /*$$ = TRACK_MODE::MODE1_2048; */}
	| MODE1_2352  { /*$$ = TRACK_MODE::MODE1_2352; */}
	| MODE2_2336  { /*$$ = TRACK_MODE::MODE2_2336; */}
	| MODE2_2048  { /*$$ = TRACK_MODE::MODE2_2048; */}
	| MODE2_2342  { /*$$ = TRACK_MODE::MODE2_2342; */}
	| MODE2_2332  { /*$$ = TRACK_MODE::MODE2_2332; */}
	| MODE2_2352  { /*$$ = TRACK_MODE::MODE2_2352; */}
	;

track_statements
	: track_statements track_statement
	| track_statement
	;

track_statement
	: cdtext_statement
	/* | file_statement */   /* support EAC format variant */
	| rem_statement
	| FLAGS track_flags
	| TRACK_ISRC STRING
		{
			handler->append_isrc($2);
		}
	| PREGAP       msf_time
	| POSTGAP      msf_time
	| INDEX NUMBER msf_time
		{
			const auto inumber { $2 };
			const auto index   { std::atoi(inumber.c_str()) };

			if (1 == index)
			{
				handler->append_offset($3);
			}

			if (0 == index)
			{
				// TODO pregap
			}
		}
	;

track_flags
	: track_flags track_flag
	| track_flag
	;

track_flag
	: PRE      { /*$$ = TRACK_FLAG::PRE;     */}
	| DCP      { /*$$ = TRACK_FLAG::DCP;     */}
	| FOUR_CH  { /*$$ = TRACK_FLAG::FOUR_CH; */}
	| SCMS     { /*$$ = TRACK_FLAG::SCMS;    */}
	;

msf_time
	: NUMBER COLON NUMBER COLON NUMBER
		{
			const auto m { $1 };
			const auto s { $3 };
			const auto f { $5 };

			$$ = to_sframes(
					std::atoi(m.c_str()),
					std::atoi(s.c_str()),
					std::atoi(f.c_str()));
		}
	;

%%


#if defined(__GNUG__)
#pragma GCC diagnostic pop
#endif

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

/* Bison expects us to provide implementation, otherwise linker complains */
namespace arcsdec { inline namespace v_1_0_0 { namespace details {
namespace cuesheet {
namespace yycuesheet {

void Parser::error(const location& loc, const std::string& message)
{
	parser_error(loc, message, std::cerr);
}

} // namespace yycuesheet
} // namespace cuesheet
} /*details*/ } /*v_1_0_0*/ } /*arcsdec*/

