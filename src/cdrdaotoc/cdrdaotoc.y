/* Bison grammar file for C++-based CDRDAO/TOC parser */

%skeleton "lalr1.cc" /* recommended newer parser skeleton for 3.2 */
%require  "3.2"
%language "c++"


/* Namespace for generated parser class */
%define api.namespace          {arcsdec::v_1_0_0::details::cdrdaotoc::yycdrdaotoc}

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
%define api.location.file "cdrdaotoc_location.hpp"

/* Location tracking */
%locations

/* Define parameters to pass to yylex() */
%lex-param   { Lexer&  lexer }
%lex-param   { Driver& driver }

/* Define parameters to pass to yyparse() */
%parse-param { Lexer&  lexer }
%parse-param { Driver& driver }


/* Goes to .tab.hpp header file (therefore included in source) */
%code requires
{
	#ifndef __LIBARCSDEC_VERSION_HPP__
	#include "version.hpp" // for v_1_0_0
	#endif

	#ifndef __LIBARCSDEC_CDRDAOTOC_HANDLER_HPP__
	#include "handler.hpp"                      // user-defined
	#endif

	// Forward declare what we are about to use
	int yylex (void);
	int yyerror (const char *s);

	// Forward declarations
	namespace arcsdec { inline namespace v_1_0_0 { namespace details {
	namespace cdrdaotoc {

		class Driver;

		/**
		 * \brief Namespace for flex- and bison generated sources for parsercue.
		 */
		namespace yycdrdaotoc {

			class Lexer;
		} // namespace yycdrdaotoc

	} // namespace cdrdaotoc
	} /*details*/ } /*v_1_0_0*/ } /*arcsdec*/
}


/* Goes to source file _before_ cdrdaotoc.tab.hpp is included */
%code top
{
	//  for clang++
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Winline-namespace-reopened-noninline"

	//  for g++
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Weffc++"


	#include <cstdlib> // for atoi
	#include <string>  // for string
	#include <tuple>   // for tuple

	#ifndef __LIBARCSDEC_CDRDAOTOC_LEXER_HPP__
	#include "cdrdaotoc_lexer.hpp"               // user-defined
	#endif

	#include "cdrdaotoc_location.hpp"            // auto-generated

	#ifndef __LIBARCSDEC_CDRDAOTOC_DRIVER_HPP__
	#include "driver.hpp"                        // user-defined
	#endif

	/**
	 * \brief Override yylex() to be called in Parser::parse()
	 */
	static arcsdec::v_1_0_0::details::cdrdaotoc::yycdrdaotoc::Parser::symbol_type
	yylex(
			arcsdec::v_1_0_0::details::cdrdaotoc::yycdrdaotoc::Lexer& lexer,
			arcsdec::v_1_0_0::details::cdrdaotoc::Driver& /*driver*/)
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

%token CD_DA
%token CD_ROM
%token CD_ROM_XA
%token CD_I

%token CATALOG

%token TRACK
%token ISRC
%token AUDIOFILE
%token FILETAG

%token NO
%token COPY
%token PRE_EMPHASIS
%token TWO_CHANNEL_AUDIO
%token FOUR_CHANNEL_AUDIO
%token PREGAP
%token START
%token ENDTAG
%token INDEX

%token SWAP
%token DATAFILE
%token FIFO
%token SILENCE
%token ZERO

%token AUDIO
%token MODE0
%token MODE1
%token MODE1_RAW
%token MODE2
%token MODE2_RAW
%token MODE2_FORM1
%token MODE2_FORM2
%token MODE2_FORM_MIX

%token RW
%token RW_RAW

%token CD_TEXT
%token LANGUAGE
%token LANGUAGE_MAP
%token TITLE
%token PERFORMER
%token SONGWRITER
%token COMPOSER
%token ARRANGER
%token MESSAGE
%token DISC_ID
%token GENRE
%token TOC_INFO1
%token TOC_INFO2
%token RESERVED1
%token RESERVED2
%token RESERVED3
%token RESERVED4
%token CLOSED
%token UPC_EAN
%token SIZE_INFO

%token EN
%token ENCODING_ISO_8859_1
%token ENCODING_ASCII
%token ENCODING_MS_JIS
%token ENCODING_KOREAN
%token ENCODING_MANDARIN

%token <std::string> NUMBER
%token <std::string> STRING
%token COMMENT_START
%token COLON
%token COMMA
%token HASH
%token BRACE_OPEN
%token BRACE_CLOSE

%token END 0 "End of file"

/* Non-Terminals */
%nterm <std::tuple<int, int, int>> msf_time


%start cdrtoc


%%


cdrtoc
	: opt_global_statements opt_global_cdtext_statement tracks
		{
			driver.get_handler()->end_input();
		}
	;

opt_global_statements
	: global_statements
	| /* none */
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
	| toctype
	;

toctype
	: CD_DA | CD_ROM | CD_ROM_XA | CD_I
	;

opt_global_cdtext_statement
	: CD_TEXT  BRACE_OPEN  opt_language_map opt_cdtext_blocks  BRACE_CLOSE
	;

opt_language_map
	: LANGUAGE_MAP  BRACE_OPEN  language_mappings  BRACE_CLOSE
	| /* none */
	;

language_mappings
	: language_mappings language_mapping
	| language_mapping
	;

language_mapping
	: NUMBER COLON NUMBER
	| NUMBER COLON EN
	;

opt_cdtext_blocks
	: cdtext_blocks
	| /* none */
	;

cdtext_blocks
	: cdtext_blocks cdtext_block
	| cdtext_block
	;

cdtext_block
	: LANGUAGE NUMBER BRACE_OPEN  opt_cdtext_items  BRACE_CLOSE
	;

opt_cdtext_items
	: cdtext_items
	| /* none */
	;

cdtext_items
	: cdtext_items cdtext_item
	| cdtext_item
	;

cdtext_item
	: pack_type cdtext_item_value
	;

pack_type
	: TITLE
	| PERFORMER
	| SONGWRITER
	| COMPOSER
	| ARRANGER
	| MESSAGE
	| DISC_ID
	| GENRE
	| TOC_INFO1
	| TOC_INFO2
	| RESERVED1
	| RESERVED2
	| RESERVED3
	| RESERVED4
	| UPC_EAN
	| ISRC
	| SIZE_INFO
	;

cdtext_item_value
	: BRACE_OPEN numbers BRACE_CLOSE
	| STRING
	;

numbers
	: numbers COMMA NUMBER
	| NUMBER
	;

tracks
	: tracks track
	| track
	;

track
	:  TRACK track_mode opt_subchannel_mode opt_track_flags opt_cdtext_track
		opt_pregap trailing_track_info
	;

track_mode
	: AUDIO          { /* $$ = MODE::BINARY;   */ }
	| MODE1          { /* $$ = MODE::AIFF;     */ }
	| MODE1_RAW      { /* $$ = MODE::WAVE;     */ }
	| MODE2          { /* $$ = MODE::MP3;      */ }
	| MODE2_RAW      { /* $$ = MODE::FLAC;     */ }
	| MODE2_FORM1
	| MODE2_FORM2
	| MODE2_FORM_MIX
	;

subchannel_mode
	: RW
	| RW_RAW
	;

opt_subchannel_mode
	: subchannel_mode
	| /* none */
	;

opt_track_flags
	: track_flags
	| /* none */
	;

track_flags
	: track_flags track_flag
	| track_flag
	;

track_flag
	: ISRC STRING
	| TWO_CHANNEL_AUDIO
	| FOUR_CHANNEL_AUDIO
	| boolean_attr
	| NO boolean_attr
	;

boolean_attr
	: COPY
	| PRE_EMPHASIS
	;

opt_cdtext_track
	: cdtext_track
	| /* none */
	;

cdtext_track
	: CD_TEXT BRACE_OPEN opt_cdtext_blocks BRACE_CLOSE
	;

opt_pregap
	: PREGAP msf_time
	| /* none */
	;

trailing_track_info
	: subtrack_or_start_or_ends /* FIXME Must be optional => */ index_statements
	;

subtrack_or_start_or_ends
	: subtrack_or_start_or_ends subtrack_or_start_or_end
	| subtrack_or_start_or_end
	;

subtrack_or_start_or_end
	: subtrack_statement
	| START opt_msf_time
	| END   opt_msf_time
	;

subtrack_statement
	: audiofile STRING opt_swap audiofile_statement_corpus
	| DATAFILE STRING opt_time_spec
	| FIFO STRING data_length
	| SILENCE samples
	| ZERO opt_data_mode opt_subchannel_mode data_length
	;

audiofile
	: AUDIOFILE
	| FILETAG
	;

audiofile_statement_corpus
	: samples opt_data_length
	| number_tag samples
	;

opt_swap
	: SWAP
	| /* none */
	;

opt_time_spec
	: number_tag opt_data_length
	| /* none */
	;

number_tag
	: HASH NUMBER
	;

samples
	: msf_time
	| NUMBER
	;

opt_data_length
	: data_length
	| /* none */
	;

data_length
	: msf_time
	| NUMBER
	;

opt_data_mode
	: data_mode
	| /* none */
	;

data_mode
	: track_mode
	| MODE0
	;

opt_index_statements
	: index_statements
	| /* none */
	;

index_statements
	: index_statements index_statement
	| index_statement
	;

index_statement
	: INDEX msf_time
	;

opt_msf_time
	: msf_time
	| /* none */
	;

msf_time
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
#pragma clang diagnostic pop

/* Bison expects us to provide implementation, otherwise linker complains */
namespace arcsdec { inline namespace v_1_0_0 { namespace details {
namespace cdrdaotoc {
namespace yycdrdaotoc {

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

} // namespace yycdrdaotoc
} // namespace cdrdaotoc
} /*details*/ } /*v_1_0_0*/ } /*arcsdec*/

