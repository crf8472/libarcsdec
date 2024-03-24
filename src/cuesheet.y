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
	#include <iostream>
	#include <string>

	#include "lexer.hpp"
	#include "driver.hpp"  // user-defined
	//#include "handler.hpp" // user-defined

	#include "location.hh" // auto-generated

	/**
	 * \brief Override yylex() to be called in Parser::parse()
	 */
	static cuesheet::yycuesheet::Parser::symbol_type yylex(
			cuesheet::yycuesheet::Lexer &lexer, cuesheet::Driver &driver)
	{
		return lexer.get_next_token();
	}
	// Only called inside bison, so make it static to limit visibility.
	// Namespaces are required since yylex() is in global namespace.
	// Note: could also be achieved by doing:
    // #define yylex(Lexer &l, Driver &d) lexer.get_next_token()
	// But let's avoid macros if we can.
}


/* Terminals */
%token ENTRYMARKER
%token CONC_OP
%token ASSIGN_OP
%token COMMA
%token DQUOTE
%token LBRACE
%token RBRACE
%token LPAREN
%token RPAREN

/* Non-terminals with value: cuesheet-format objects */
%token <std::string> DECL_STRING
%token <std::string> DECL_PREAMBLE
%token <std::string> DECL_COMMENT
%token <std::string> DECL_DOCTYPE
%token <std::string> ID
%token <std::string> KEY
%token <std::string> INTEGER
%token <std::string> STRINGREF
%token <std::string> CHARACTERS

%token END 0 "End of file"


%start cuesheet_input


%%


/* Helper to signal start of the input */
start_input:
			{
				//driver.get_handler()->start_input();
			}
		;


/* Helper to hook in start_input() */
cuesheet_input: start_input entries { /* do nothing */ }
		;


/* Optional list of entries (entries: comments, preamble, strings or docs) */
entries:  entry entries   { /* do nothing */ }
		| /* empty occurrence */
			{
				//driver.get_handler()->end_input();
			}
		;


/* COMMENT */

comment: comment_start char_sequences
			{
				//driver.get_handler()->end_comment();
			}
		;

comment_start: DECL_COMMENT
			{
				//driver.get_handler()->start_comment();
			}
		;


/* PREAMBLE */

preamble: preamble_start preamble_body
			{
				//driver.get_handler()->end_preamble();
			}
		;

preamble_start: DECL_PREAMBLE
			{
				//driver.get_handler()->start_preamble();
			}
		;

preamble_body: LBRACE string_expr RBRACE { /* do nothing */ }
		| LPAREN string_expr RPAREN      { /* do nothing */ }
		;


/* STRING */

stringdef: stringdef_start stringdef_body
			{
				//driver.get_handler()->end_string();
			}
		;

stringdef_start: DECL_STRING
			{
				//driver.get_handler()->start_string();
			}
		;

stringdef_body: LBRACE tag RBRACE { /* do nothing */ }
		| LPAREN tag RPAREN       { /* do nothing */ }
		;


/* DOCUMENT-ENTRY */

document: document_start document_body
			{
				//driver.get_handler()->end_entry();
			}
		;

document_start: DECL_DOCTYPE
			{
				//driver.get_handler()->start_entry($1);
			}
		;

/* To signal ID separately */
document_id: ID COMMA
			{
				//driver.get_handler()->entry_id($1);
			}
		;

/* Body of a document entry */
document_body: LBRACE document_id tags RBRACE { /* do nothing */ }
		| LPAREN document_id tags RPAREN      { /* do nothing */ }
		;



/* Cuesheet-Entry (STRING, PREAMBLE, COMMENT or a Bibliographic Entry) */
entry: ENTRYMARKER document     { /* do nothing */ }
		| ENTRYMARKER stringdef { /* do nothing */ }
		| ENTRYMARKER preamble  { /* do nothing */ }
		| ENTRYMARKER comment   { /* do nothing */ }
		;


/* Non-Empty List of Tags, allowed to end with a COMMA */
tags:     tag COMMA tags { /* do nothing */ }
		| tag COMMA      { /* do nothing */ }
		| tag            { /* do nothing */ }
		;


/* Tag: left hand side of a tag (to signal 'key' before the value) */
tag_lhs: KEY ASSIGN_OP
			{
				//driver.get_handler()->key($1);
				//driver.get_handler()->start_value();
			}
		;

/* Tag: Assignment in the Body of a Bibliographic Entry */
tag:      tag_lhs INTEGER
			{
				//driver.get_handler()->integer($2);
				//driver.get_handler()->end_value();
			}
		| tag_lhs string_expr
			{
				//driver.get_handler()->end_value();
			}
		;

/* Reference to a named Cuesheet-string */
string_ref: STRINGREF
			{
				//driver.get_handler()->stringref($1);
			}
		;


/* Righthand side of a tag or content of a preamble */
string_expr: string_ref conc_vals      { /* do nothing */ }
		| quoted_string conc_vals      { /* do nothing */ }
		| LBRACE char_sequences RBRACE { /* do nothing */ }
		| LBRACE RBRACE                { /* do nothing */ }
		;


/* String in Double Quotes */
quoted_string: DQUOTE char_sequences DQUOTE { /* do nothing */ }
		| DQUOTE DQUOTE                     { /* do nothing */ }
		;


/* String: sequence of characters */
char_sequences: CHARACTERS char_sequences
			{
				//driver.get_handler()->characters($1);
			}
		| CHARACTERS
			{
				//driver.get_handler()->characters($1);
			}
		;


/* Value: concatenation operator */
conc_op: CONC_OP
			{
				//driver.get_handler()->concat_op();
			}
		;


/* Value: concatenated value parts */
conc_vals: conc_op quoted_string conc_vals { /* do nothing */ }
		|  conc_op string_ref    conc_vals { /* do nothing */ }
		|  /* empty occurrence */          { /* do nothing */ }
		;


%%


/* Bison expects us to provide implementation, otherwise linker complains */
namespace cuesheet {
namespace yycuesheet {

void Parser::error(const location &loc, const std::string &message)
{
	std::cout << "Parser error: " << message << std::endl
		<< "At token position: " << loc << std::endl
		<< "Driver says:       " << driver.current_token_location() << std::endl;
}

} // namespace yycuesheet
} // namespace cuesheet

