/* Bison grammar file for C++-based CDRDAO/TOC parser */

/* Sources: */
/* [1] $ man 1 cdrdao */

%require  "3.2"
%language "c++"
%skeleton "lalr1.cc" /* recommended newer parser skeleton for 3.2 */

/* Avoid name conflicts with other parsers by prefixing this parser's names */
/* %define api.prefix             {toc} */
/* https://www.gnu.org/software/bison/manual/bison.html#Multiple-Parsers */

/* Define filename for location header (instead of location.hh) */
%define api.location.file      "cdrtoc_location.hpp"

/* Namespace for generated parser class */
%define api.namespace          {arcsdec::v_1_0_0::details::cdrtoc::yycdrtoc}

/* Classname for generated parser class */
%define api.parser.class       {Parser}

/* Request symbols to be handled as a whole (type, value, location) in lexer */
%define api.token.constructor

/* Add prefix to token name */
%define api.token.prefix       {TOKEN_}

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
	#include "../src/flexbisondriver.hpp"   // for TokenLocation
	#endif

	// Declaration of class 'Parser' refers to 'ParserTocHandler' in constructor
	#ifndef __LIBARCSDEC_TOCHANDLER_HPP__
	#include "../src/tochandler.hpp"        // for ParserTocHandler
	#endif

	// Forward declare what we are about to use
	int yylex (void);
	int yyerror (const char *s);

	// Forward declarations (require C++17)
	namespace arcsdec
	{
	inline namespace v_1_0_0
	{
	namespace details::cdrtoc::yycdrtoc
	{
		class Lexer;
	}
	} // namespace v_1_0_0
	} // namespace arcsdec
}


/* Goes to source file _before_ cdrtoc.tab.hpp is included */
%code top
{
	// To use member functions of 'ParserTocHandler' in production rule actions
	// (Include also required in cdrtoc.tab.hpp)
	#ifndef __LIBARCSDEC_TOCHANDLER_HPP__
	#include "../src/tochandler.hpp"       // for ParserTocHandler, ...
	#endif

	// Re-declaration of yylex() below requires declaration of class 'Lexer'
	#ifndef __LIBARCSDEC_CDRTOC_LEXER_HPP__
	#include "cdrtoc_lexer.hpp"            // for Lexer
	#endif

	// Required in the implementation of production rules
	#include <string>  // for string, stol, stoul

	// Override yylex() to be called in Parser::parse().
	// Only called inside bison, so make it static to limit visibility to TU.
	// Namespaces are required since yylex() is in global namespace.
	static auto yylex(arcsdec::details::cdrtoc::yycdrtoc::Lexer* lexer)
		-> arcsdec::details::cdrtoc::yycdrtoc::Parser::symbol_type
	{
		return lexer->next_token(); // renamed yylex()
	}
	// Note: could also be achieved by doing something like:
    // #define yylex(Lexer* l, Location* loc) lexer.next_token()
	// But let's avoid macros if we can.

	// Convert to the type of non-terminal 'u_long'.
	// Inteded for converting NUMBER tokens.
	// Requires 'api.value.automove'
	static auto to_unsigned_long(std::string&& s) -> uint64_t
	{
		return std::stoul(s.c_str());
	}

	// Convert to the type of non-terminal 's_long'.
	// Inteded for converting NUMBER tokens.
	// Requires 'api.value.automove'
	static auto to_signed_long(std::string&& s) -> int64_t
	{
		return std::stol(s.c_str());
	}

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
%token UPC_EAN
%token SIZE_INFO

%token EN
%token ENCODING_ISO_8859_1
%token ENCODING_ASCII
%token ENCODING_MS_JIS
%token ENCODING_KOREAN
%token ENCODING_MANDARIN

%token <std::string> STRING
%token <std::string> NUMBER
%token COMMENT_START
%token COLON
%token COMMA
%token HASH
%token BRACE_LEFT
%token BRACE_RIGHT
%token EMPTY_STRING

%token END 0 "End of file"

/* Non-Terminals */

/* unsigned long number format */
%nterm <uint64_t> u_long msf_time opt_msf_time data_length opt_data_length
						samples opt_samples

/* signed long number format */
%nterm  <int64_t> s_long tagged_number opt_start_offset


%%

cdrtoc
	: opt_catalog_or_toctype_statements opt_global_cdtext_statement tracks
		{
			handler->end_input();
		}
	;

opt_catalog_or_toctype_statements
	: opt_catalog_or_toctype_statements catalog_or_toctype_statement
	| /* none */
	;

catalog_or_toctype_statement
	: CATALOG NUMBER
		{
			handler->set_mcn($2);
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
	: pack_type cdtext_item_value /* default for all pack_values */
	| DISC_ID STRING /* DISC_ID is the only pack_value we want to react on */
		{
			handler->set_disc_id($2);
		}
	;

pack_type
	: pack_type_str
	| pack_type_bin
	| RESERVED1
	| RESERVED2
	| RESERVED3
	| RESERVED4
	;

pack_type_str /* requires value as STRING or EMPTY_STRING */
	: TITLE
	| PERFORMER
	| SONGWRITER
	| COMPOSER
	| ARRANGER
	| MESSAGE
	/* | DISC_ID */ /* by separate rule in production 'cdtext_item' */
	| GENRE
	| UPC_EAN
	| ISRC
	;

pack_type_bin /* requires value as binary_data */
	: TOC_INFO1
	| TOC_INFO2
	| SIZE_INFO
	;

cdtext_item_value
	: binary_data
	| STRING
	| EMPTY_STRING
	;

binary_data
	: BRACE_LEFT  sequence_of_numbers  BRACE_RIGHT
	;

sequence_of_numbers
	: sequence_of_numbers COMMA NUMBER
	| NUMBER
	;

tracks
	: tracks track
	| track
	;

track
	:  TRACK track_mode opt_subchannel_mode opt_track_flags
		opt_track_cdtext_statement opt_pregap subtrack_or_start_or_ends
		opt_index_statements
		{
			handler->inc_current_track();
		}
	;

track_mode
	: AUDIO
	| MODE1
	| MODE1_RAW
	| MODE2
	| MODE2_RAW
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
		{
			const auto isrc { $2 };

			handler->append_isrc(isrc);
		}
	| TWO_CHANNEL_AUDIO
	| FOUR_CHANNEL_AUDIO
	| boolean_attr
	| NO boolean_attr
	;

boolean_attr
	: COPY
	| PRE_EMPHASIS
	;

opt_track_cdtext_statement
	: CD_TEXT BRACE_LEFT opt_cdtext_blocks BRACE_RIGHT
	| /* none */
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
	| START opt_msf_time
		{
			const auto start { $2 };

			if (start)
			{
				const auto track  { handler->current_track() };

				try
				{
					const auto offset { handler->offset(track) };
					handler->set_offset(track, offset + start);

				} catch (const std::exception& e)
				{
					using std::to_string;
					auto message = std::string("Error: Tried to ")
						+ " update offset for track " + to_string(track)
						+ " but no offset available.";

					// TODO Use combination of yyerror() + YYERROR
				}
			}
		}
	| END_TOKEN opt_msf_time
	;

subtrack_statement
	: audiofile STRING opt_swap audiofile_offset_and_length
		{
			const auto filename { $2 };

			handler->append_filename(filename);
		}
	| DATAFILE STRING opt_start_length
	| FIFO STRING data_length
	| SILENCE samples
	| ZERO opt_data_mode opt_subchannel_mode data_length
	;

audiofile
	: AUDIOFILE
	| FILE_TOKEN
	;

audiofile_offset_and_length
	: opt_start_offset samples /* FIXME $3 is not in grammar */ opt_samples
		{
			// This rule parses things like
			// AUDIOFILE 00:00:00 00:00:33
			// AUDIOFILE 00:00:00
			// (audiofile followed by offset + optional length)
			// which occurr in the wild, e.g. produce by cdrdao's own cue2toc.
			// However, it is a deviation from the grammar, that specifies
			// AUDIOFILE #0 00:00:33
			// (audiofile followed by optional offset + mandatory length).
			// (Or I just do not understand the grammar.)

			handler->append_offset($2);

			//const auto length { $3 }; /* commented: unused, documentation */
		}
	;

opt_swap
	: SWAP
	| /* none */
	;

opt_start_length
	: tagged_number opt_data_length
	| /* none */
	;

opt_start_offset
	: tagged_number
	| /* none */
		{
			$$ = 0;
		}
	;

tagged_number
	: HASH s_long
		{
			$$ = $2;
		}
	;

s_long
	: NUMBER
		{
			$$ = to_signed_long($1);
		}
	;

opt_data_length
	: data_length
	| /* none */
		{
			$$ = 0u;
		}
	;

data_length
	: msf_time
	| u_long
	;

opt_samples
	: samples
	| /* none */
		{
			$$ = 0u;
		}
	;

samples
	: msf_time
	| u_long
	;

u_long
	: NUMBER
		{
			$$ = to_unsigned_long($1);
		}
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
	: opt_index_statements INDEX msf_time
	| /* none */
	;

opt_msf_time
	: msf_time
	| /* none */
		{
			$$ = 0u;
		}
	;

msf_time
	: NUMBER COLON NUMBER COLON NUMBER
		{
			$$ = to_uframes(
					to_unsigned_long($1),
					to_unsigned_long($3),
					to_unsigned_long($5));
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

