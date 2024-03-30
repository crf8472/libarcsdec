/* Bison grammar file for C++-based Cuesheet parser */

%skeleton "lalr1.cc" /* recommended newer parser skeleton for 3.2 */
%require  "3.2"
%language "c++"


/* Namespace for generated parser class */
%define api.namespace          {arcsdec::v_1_0_0::details::cuesheet::yycuesheet}

/* Classname for generated parser class */
%define api.parser.class       {Parser}

/* Request symbols to be handled as a whole (type, value, location) in lexer */
%define api.token.constructor

/* Add prefix to token name */
%define api.token.prefix       {TOKEN_}

/* Move tokens (instead of copy) when passing them to make_<TOKENNAME>() */
/* % define api.value.automove */

/* Use bison variants as token types (requires token.constructor) */
%define api.value.type variant

/* Runtime assertions to check proper destruction of tokens (requires RTTI) */
%define parse.assert

/* Report unexpected token as well as expected ones. */
%define parse.error verbose

/* Define filename for location header (instead of location.hh) */
%define api.location.file "cuesheet_location.hpp"

/* Location tracking */
%locations

/* Define parameters to pass to yylex() */
%lex-param   { Lexer  &lexer }
%lex-param   { Driver &driver }

/* Define parameters to pass to yyparse() */
%parse-param { Lexer  &lexer }
%parse-param { Driver &driver }


/* Goes to .tab.hpp header file (therefore included in source) */
%code requires
{
	#ifndef __LIBARCSDEC_VERSION_HPP__
	#include "version.hpp" // for v_1_0_0
	#endif

	#ifndef __LIBARCSDEC_CUESHEET_HANDLER_HPP__
	#include "handler.hpp"                      // user-defined
	#endif

	// Forward declare what we are about to use
	int yylex (void);
	int yyerror (const char *s);

	// Forward declarations
	namespace arcsdec { namespace v_1_0_0 { namespace details {
	namespace cuesheet {

		class Driver;

		/**
		 * \brief Namespace for flex- and bison generated sources for parsercue.
		 */
		namespace yycuesheet {

			class Lexer;
		} // namespace yycuesheet

	} // namespace cuesheet
	} /*details*/ } /*v_1_0_0*/ } /*arcsdec*/
}


/* Goes to source file _before_ cuesheet.tab.hpp is included */
%code top
{
	#pragma GCC diagnostic push

	#pragma GCC diagnostic ignored "-Weffc++"

	#include <cstdlib> // for atoi
	#include <string>  // for string
	#include <tuple>   // for tuple

	#ifndef __LIBARCSDEC_CUESHEET_LEXER_HPP__
	#include "cuesheet_lexer.hpp"               // user-defined
	#endif

	#include "cuesheet_location.hpp"            // auto-generated

	#ifndef __LIBARCSDEC_CUESHEET_DRIVER_HPP__
	#include "driver.hpp"                       // user-defined
	#endif

	/**
	 * \brief Override yylex() to be called in Parser::parse()
	 */
	static arcsdec::v_1_0_0::details::cuesheet::yycuesheet::Parser::symbol_type
	yylex(
			arcsdec::v_1_0_0::details::cuesheet::yycuesheet::Lexer &lexer,
			arcsdec::v_1_0_0::details::cuesheet::Driver& /*driver*/)
	{
		return lexer.next_token(); // renamed yylex()
	}
	// Only called inside bison, so make it static to limit visibility.
	// Namespaces are required since yylex() is in global namespace.
	// Note: could also be achieved by doing:
    // #define yylex(Lexer &l, Driver &d) lexer.get_next_token()
	// But let's avoid macros if we can.
}


/* Terminals */
%token CATALOG
%token FILETAG
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
%token GENRE
%token DISC_ID
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
%nterm <std::tuple<int, int, int>> time
%nterm <FILE_FORMAT>               file_format_tag
%nterm <TRACK_MODE>                track_mode_tag
%nterm <TRACK_FLAG>                track_flag
%nterm <std::vector<TRACK_FLAG>>   track_flags


%start cuesheet


%%


cuesheet
	: global_statements track_list
		{
			driver.get_handler()->end_input();
		}
	;

global_statements
	: global_statements global_statement
	| global_statement
	;

global_statement
	: CATALOG NUMBER
		{   /* MCN: [0-9]{13} */
			driver.get_handler()->catalog($2);
		}
	| CDTEXTFILE STRING
		{
			driver.get_handler()->cdtextfile($2);
		}
	| FILETAG STRING file_format_tag
		{
			driver.get_handler()->file($2, $3);
		}
	| cdtext_statement
	| rem_statement
	;

file_format_tag
	: BINARY    { $$ = FILE_FORMAT::BINARY;   /* cdrwin */ }
	| MOTOROLA  { $$ = FILE_FORMAT::MOTOROLA; /* cdrwin */ }
	| AIFF      { $$ = FILE_FORMAT::AIFF;     /* cdrwin */ }
	| WAVE      { $$ = FILE_FORMAT::WAVE;     /* cdrwin */ }
	| MP3       { $$ = FILE_FORMAT::MP3;      /* cdrwin */ }
	| FLAC      { $$ = FILE_FORMAT::FLAC;     }
	;

cdtext_statement
	: TITLE STRING
		{
			driver.get_handler()->title($2);
		}
	| PERFORMER STRING
		{
			driver.get_handler()->performer($2);
		}
	| SONGWRITER STRING
		{
			driver.get_handler()->songwriter($2);
		}
	| ISRC STRING
		{
			/* ISRC: [[:alnum:]]{5}[0-9]{7} */
			driver.get_handler()->isrc($2);
		}
	| cdtext_tag STRING
	;

cdtext_tag
	: COMPOSER
	| ARRANGER
	| MESSAGE
	| DISC_ID
	| GENRE
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
	: track
	| track_list track
	;

track
	: track_header track_statements
	;

track_header
	: TRACK NUMBER track_mode_tag
		{
			driver.get_handler()->track(std::atoi(&$2[0]), $3);
		}
	;

track_mode_tag
	: AUDIO       { $$ = TRACK_MODE::AUDIO;      }
	| MODE1_2048  { $$ = TRACK_MODE::MODE1_2048; }
	| MODE1_2352  { $$ = TRACK_MODE::MODE1_2352; }
	| MODE2_2336  { $$ = TRACK_MODE::MODE2_2336; }
	| MODE2_2048  { $$ = TRACK_MODE::MODE2_2048; }
	| MODE2_2342  { $$ = TRACK_MODE::MODE2_2342; }
	| MODE2_2332  { $$ = TRACK_MODE::MODE2_2332; }
	| MODE2_2352  { $$ = TRACK_MODE::MODE2_2352; }
	;

track_statements
	: track_statement
	| track_statements track_statement
	;

track_statement
	: cdtext_statement
	/* | file_statment */   /* support EAC format variant */
	| rem_statement
	| FLAGS track_flags
		{
			driver.get_handler()->track_flags($2);
		}
	| TRACK_ISRC STRING
		{
			/* ISRC: [[:alnum:]]{5}[0-9]{7} */
			driver.get_handler()->isrc($2);
		}
	| PREGAP       time
		{
			auto msf { $2 };
			driver.get_handler()->pregap(
				std::get<0>(msf), std::get<1>(msf), std::get<2>(msf));

		}
	| POSTGAP      time
		{
			auto msf { $2 };
			driver.get_handler()->postgap(
				std::get<0>(msf), std::get<1>(msf), std::get<2>(msf));
		}
	| INDEX NUMBER time
		{
			auto msf { $3 };
			driver.get_handler()->index(std::atoi(&$2[0]),
				std::get<0>(msf), std::get<1>(msf), std::get<2>(msf));
		}
	;

track_flags
	: track_flags track_flag
		{
			$$.emplace_back($2);
		}
	| track_flag
		{
			$$.emplace_back($1);
		}
	;

track_flag
	: PRE      { $$ = TRACK_FLAG::PRE;     }
	| DCP      { $$ = TRACK_FLAG::DCP;     }
	| FOUR_CH  { $$ = TRACK_FLAG::FOUR_CH; }
	| SCMS     { $$ = TRACK_FLAG::SCMS;    }
	;

time
	: NUMBER COLON NUMBER COLON NUMBER
		{
			$$ = std::make_tuple(
					std::atoi(&$1[0]),
					std::atoi(&$3[0]),
					std::atoi(&$5[0]));
		}
	;


%%


#pragma GCC diagnostic pop

/* Bison expects us to provide implementation, otherwise linker complains */
namespace arcsdec { namespace v_1_0_0 { namespace details {
namespace cuesheet {
namespace yycuesheet {

void Parser::error(const location &loc, const std::string &message)
{
	if (loc.begin.line == loc.end.line)
	{
		if (loc.end.column - 1 == loc.begin.column)
		{
			std::cerr << "Parser error at line " << loc.begin.line
				<< ", char " << loc.begin.column
				<< ": " << message << std::endl;
		} else
		{
			std::cerr << "Parser error at line " << loc.begin.line
				<< " chars " << loc.begin.column
				<< "-" << loc.end.column - 1
				<< ": " << message << std::endl;
		}
	} else
	{
		std::cerr << "Parser error from line " << loc.begin.line
			<< ", char " << loc.begin.column
			<< " till line " << loc.end.line
			<< ", char " << loc.end.column - 1
			<< ": " << message << std::endl;

	}
}

} // namespace yycuesheet
} // namespace cuesheet
} /*details*/ } /*v_1_0_0*/ } /*arcsdec*/

