/* Bison grammar file for C++-based CDRDAO/TOC parser */

%skeleton "lalr1.cc" /* recommended newer parser skeleton for 3.2 */
%require  "3.2"
%language "c++"


/* Namespace for generated parser class */
%define api.namespace          {arcsdec::v_1_0_0::details::cdrtoc::yycdrtoc}

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
%define api.location.file "cdrtoc_location.hpp"

/* Location tracking */
%locations

/* Define parameters to pass to yylex() */
%lex-param { Lexer* lexer } // to apply token rules
%lex-param { arcsdec::details::TokenLocation<position, location>* loc }

/* Define parameters to pass to Parser::Parser() */
%parse-param { arcsdec::details::TokenLocation<position, location>* loc }
%parse-param { Lexer* lexer } // to call parser
%parse-param { ParserHandler* handler } // to implement parser rules


/* Goes to .tab.hpp header file (therefore included in source) */
%code requires
{
	#ifndef __LIBARCSDEC_FLEXBISONDRIVER_HPP__
	#include "../src/flexbisondriver.hpp"  // for ParserHandler
	#endif

	// Forward declare what we are about to use
	int yylex (void);
	int yyerror (const char *s);

	// Forward declarations
	namespace arcsdec { inline namespace v_1_0_0 { namespace details {
	namespace cdrtoc {

		/**
		 * \brief Namespace for flex- and bison generated sources for parsercue.
		 */
		namespace yycdrtoc {

			class Lexer;
		} // namespace yycdrtoc

	} // namespace cdrtoc
	} /*details*/ } /*v_1_0_0*/ } /*arcsdec*/
}


/* Goes to source file _before_ cdrtoc.tab.hpp is included */
%code top
{
	#ifndef __LIBARCSDEC_CDRTOC_LEXER_HPP__
	#include "cdrtoc_lexer.hpp"               // user-defined
	#endif

	#ifndef __LIBARCSDEC_FLEXBISONDRIVER_HPP__
	#include "../src/flexbisondriver.hpp"     // for TokenLocation
	#endif

	#include <cstdlib> // for atoi
	#include <string>  // for string
	#include <tuple>   // for tuple

	using arcsdec::details::cdrtoc::yycdrtoc::location;
	using arcsdec::details::cdrtoc::yycdrtoc::position;

	/**
	 * \brief Override yylex() to be called in Parser::parse()
	 */
	static auto yylex(arcsdec::details::cdrtoc::yycdrtoc::Lexer* lexer,
		arcsdec::details::TokenLocation<position, location>* /* loc */)
		-> arcsdec::details::cdrtoc::yycdrtoc::Parser::symbol_type
	{
		return lexer->next_token(); // renamed yylex()
	}
	// Only called inside bison, so make it static to limit visibility.
	// Namespaces are required since yylex() is in global namespace.
	// Note: could also be achieved by doing:
    // #define yylex(Lexer& l, Driver& d) lexer.next_token()
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

%token CD_DA
%token CD_ROM
%token CD_ROM_XA
%token CD_I

%token CATALOG

%token TRACK
%token ISRC
%token AUDIOFILE
%token FILE_TOKEN

%token NO
%token COPY
%token PRE_EMPHASIS
%token TWO_CHANNEL_AUDIO
%token FOUR_CHANNEL_AUDIO
%token PREGAP
%token START
%token END_TOKEN
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
%token BRACE_LEFT
%token BRACE_RIGHT

%token END 0 "End of file"

/* Non-Terminals */
%nterm <std::tuple<int, int, int>> msf_time


%start cdrtoc


%%


cdrtoc
	: opt_catalog_or_toctype_statements opt_global_cdtext_statement tracks
		{
			//handler->end_input();
		}
	;

opt_catalog_or_toctype_statements
	: opt_catalog_or_toctype_statements catalog_or_toctype_statement
	| /* none */
	;

catalog_or_toctype_statement
	: CATALOG NUMBER
		{   /* MCN: [0-9]{13} */
			//handler->catalog($2);
		}
	| toctype
	;

toctype
	: CD_DA | CD_ROM | CD_ROM_XA | CD_I
	;

opt_global_cdtext_statement
	: CD_TEXT  BRACE_LEFT  opt_language_map opt_cdtext_blocks  BRACE_RIGHT
	;

opt_language_map
	: LANGUAGE_MAP  BRACE_LEFT  language_mappings  BRACE_RIGHT
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
	: LANGUAGE NUMBER BRACE_LEFT  opt_cdtext_items  BRACE_RIGHT
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
	: BRACE_LEFT numbers BRACE_RIGHT
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
		opt_pregap subtrack_or_start_or_ends opt_index_statements
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
	: CD_TEXT BRACE_LEFT opt_cdtext_blocks BRACE_RIGHT
	;

opt_pregap
	: PREGAP msf_time
	| /* none */
	;

subtrack_or_start_or_ends
	: subtrack_or_start_or_ends subtrack_or_start_or_end
	| subtrack_or_start_or_end
	;

subtrack_or_start_or_end
	: subtrack_statement
	| START     opt_msf_time
	| END_TOKEN opt_msf_time
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
	| FILE_TOKEN
	;

audiofile_statement_corpus
	: samples opt_data_length
		{
			//handler->offset($1);
		}
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
	: opt_index_statements index_statement
	| /* none */
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


#if defined(__GNUG__)
#pragma GCC diagnostic pop
#endif

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

/* Bison expects us to provide implementation, otherwise linker complains */
namespace arcsdec { inline namespace v_1_0_0 { namespace details {
namespace cdrtoc {
namespace yycdrtoc {

void Parser::error(const location& loc, const std::string& message)
{
	parser_error(loc, message, std::cerr);
}

} // namespace yycdrtoc
} // namespace cdrtoc
} /*details*/ } /*v_1_0_0*/ } /*arcsdec*/

