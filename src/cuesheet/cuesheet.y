/* Bison grammar file for C++-based Cuesheet parser */

%skeleton "lalr1.cc" /* recommended newer parser skeleton for 3.2 */
%require  "3.2"
%language "c++"


/* Namespace for generated parser class */
%define api.namespace          {cuesheet::yycuesheet}

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
	// Forward declare what we are about to use
	int yylex (void);
	int yyerror (const char *s);

	// Forward declarations
	namespace cuesheet {

		class Driver;

		namespace yycuesheet {

			class Lexer;
		} // namespace yycuesheet
	} // namespace cuesheet
}


/* Goes to source file _before_ cuesheet.tab.hpp is included */
%code top
{
	#pragma GCC diagnostic push

	#pragma GCC diagnostic ignored "-Weffc++"

	#include <iostream>
	#include <string>

	#include "cuesheet_lexer.hpp" // user-defined
	#include "cuesheet_location.hpp" // auto-generated

	#include "driver.hpp"  // user-defined
	//#include "handler.hpp" // user-defined

	/**
	 * \brief Override yylex() to be called in Parser::parse()
	 */
	static cuesheet::yycuesheet::Parser::symbol_type yylex(
			cuesheet::yycuesheet::Lexer &lexer, cuesheet::Driver& /*driver*/)
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
%token REPLAYGAIN_ALBUM_GAIN
%token REPLAYGAIN_ALBUM_PEAK
%token REPLAYGAIN_TRACK_GAIN
%token REPLAYGAIN_TRACK_PEAK

%token <std::string> NUMBER
%token <std::string> STRING
%token COLON

%nterm <std::string> time

%token END 0 "End of file"


%start cuesheet


%%

cuesheet
	: global_statements track_list
	;

global_statements
	: global_statements global_statement
	| /* empty */
	;

global_statement
	: CATALOG    NUMBER { /* MCN: [0-9]{13} */ }
		{
			std::cout << "CATALOG: " << $2 << '\n';
		}
	| CDTEXTFILE STRING
	| FILETAG    STRING file_format_tag
		{
			std::cout << "FILE: " << $2 << '\n';
		}
	| cdtext_statement
	| rem_statement
	;

file_format_tag
	: BINARY
	| MOTOROLA
	| AIFF
	| WAVE
	| MP3
	| FLAC
	;

cdtext_statement
	: cdtext_tag STRING

cdtext_tag
	: TITLE      { std::cout << "TITLE\n";  }
	| PERFORMER
	| SONGWRITER
	| COMPOSER
	| ARRANGER
	| MESSAGE
	| DISC_ID
	| GENRE
	| TOC_INFO1
	| TOC_INFO2
	| UPC_EAN
	| ISRC       { std::cout << "ISRC\n"; /* [[:alnum:]]{5}[0-9]{7} */ }
	| SIZE_INFO
	;

rem_statement
	: rem_item STRING
	;

rem_item
	: DATE
	| REMGENRE
	| REPLAYGAIN_ALBUM_GAIN
	| REPLAYGAIN_ALBUM_PEAK
	| REPLAYGAIN_TRACK_GAIN
	| REPLAYGAIN_TRACK_PEAK
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
			std::cout << "TRACK = " << $2 << '\n';
		}
	;

track_mode_tag
	: AUDIO
	| MODE1_2048
	| MODE1_2352
	| MODE2_2336
	| MODE2_2048
	| MODE2_2342
	| MODE2_2332
	| MODE2_2352
	;

track_statements
	: track_statement
	| track_statements track_statement
	;

track_statement
	: cdtext_statement
	| rem_statement
	| FLAGS track_flags
	| TRACK_ISRC STRING { std::cout << "  ISRC: " << $2 << '\n'; }
	| PREGAP       time { std::cout << "  PREGAP: " << $2 << '\n'; }
	| POSTGAP      time { std::cout << "  POSTGAP: " << $2 << '\n'; }
	| INDEX NUMBER time { std::cout << "  INDEX " << $2 << " " << $3 << '\n'; }
	/* | file_statment */   /* support EAC format variant */
	;

track_flags
	: track_flags track_flag
	| /* empty */
	;

track_flag
	: PRE
	| DCP
	| FOUR_CH
	| SCMS
	;

time
	: NUMBER
		{
			$$ = $1;
		}
	| NUMBER COLON NUMBER COLON NUMBER
		{
			$$ = $1 + ':' + $3 + ':' + $5;
		}
	;

%%

#pragma GCC diagnostic pop

/* Bison expects us to provide implementation, otherwise linker complains */
namespace cuesheet {
namespace yycuesheet {

void Parser::error(const location &loc, const std::string &message)
{
	std::cerr << "Parser error at " << loc << ": " << message << std::endl;
		//<< "Driver says:       "  << driver.loc()
}

} // namespace yycuesheet
} // namespace cuesheet

