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

/* See the defined macro of yyFlexLexer in lexer.hpp */
%option prefix="Cuesheet_"
/* Note: each flex-generated scanner inherits from a class whose name is      */
/* specified by macro yyFlexLexer. This macro has to be present in each header*/
/* that defines the flex-generated lexer class. Thus there is no obvious way  */
/* to use different flex-generated lexers in the same compilation unit. The   */
/* only trick to achieve that is to put the definition of each lexer class in */
/* its own header file (for cuesheet: lexer_defs.hpp). Then, a wrapper header   */
/* is defined that redefines yyFlexLexer according to the prefix above before */
/* including the defs header (for cuesheet: lexer.hpp).                       */
/* Cf. Flex Manual, Ch. 18 'Generating C++-Scanners' (very end of the chapter)*/


%{
// from cuesheet.l

#include <iostream>       /* for cout */
#include <iomanip>        /* for setw, setfill, left, right */

#include "lexer_defs.hpp" /* for Lexer */
#include "driver.hpp"     /* for Driver */

#include "cuesheet.tab.hpp" /* auto-generated: Token Definitions of bison */
#include "location.hh"      /* auto-generated */


// Redefine calls of yyterminate():
// Original yyterminate() macro returns int. Since we're using Bison 3 variants
// as tokens, call context will expect `Parser::semantic_type` instead.
#define yyterminate() cuesheet::yycuesheet::Parser::make_END(driver_.current_token_location());

// Track current lexer location
#define YY_USER_ACTION shift_pos(yylineno, yyleng);

// Declare the symbols from yycuesheet that are to be used in yylex().
// (yylex() is in global namespace)
using cuesheet::yycuesheet::Parser;
using cuesheet::yycuesheet::location;
using cuesheet::yycuesheet::position;

// end cuesheet.l
%}


/* character tokens */
ENTRYMARKER @
CONC_OP		#
ASSIGN_OP	=
COMMA		,
LBRACE		\{
RBRACE		\}
LPAREN		\(
RPAREN		\)
DQUOTE		\"

/* predefined entry types */
DECL_STRING		(?i:STRING)
DECL_PREAMBLE	(?i:PREAMBLE)
DECL_COMMENT	(?i:COMMENT)

/* document type like "article", "book" etc. */
DECL_DOCTYPE	[A-Za-z][A-Za-z0-9]+

/* id of the cuesheet entry */
/* TODO may also [^\s\"#%'(){}]* be valid? */
ID				[A-Za-z][A-Za-z0-9\.:_-]*

/* key string of a key value pair, e.g. "booktitle", "crossref" etc. */
/* TODO may also [^\s\"#%'(){}]* be valid? */
KEY				[A-Za-z][A-Za-z0-9-]*

/* all legal variants of what can appear on the right side of the "=" */
/* TODO is something like 'page = -12' allowed? */
INTEGER			-?[0-9]+
STRINGREF		[A-Za-z][A-Za-z_0-9]+

WHITESPACE		[ \t\r]+


%x entry_type
%x id
%x start_string
%x start_preamble
%x start_entry
%x body
%x comment
%x tag
%x value
%x inside_bstring
%x inside_qstring
%x afterstringpart


%%


<afterstringpart>{CONC_OP} { /* Concatenation operator */
				notify("CONC_OP", yytext);

				BEGIN(value);
				return Parser::make_CONC_OP(driver_.current_token_location());
				}

<afterstringpart>{COMMA}|{RBRACE}|{RPAREN} { /* End of tag or end of entry */
				BEGIN(tag);
				unput(yytext[yyleng-1]);
				}



<inside_bstring,inside_qstring>{LBRACE} { /* '{' within a string */
				open_brace();

				notify("CHARACTERS", yytext);
				return Parser::make_CHARACTERS(yytext,
						driver_.current_token_location());
				}



<inside_qstring>[^"\{\}]+ { /* Part of string without DQUOTEs and braces */

				notify("CHARACTERS", yytext);
				return Parser::make_CHARACTERS(yytext,
						driver_.current_token_location());
				}

<inside_qstring>{RBRACE} { /* '}' inside a string: decreases nesting depth */
				close_brace();

				notify("CHARACTERS", yytext);
				return Parser::make_CHARACTERS(yytext,
						driver_.current_token_location());
				}

<inside_qstring>{DQUOTE} { /* DQUOTE: close the string or maybe inside as well*/
				if (within_braces()) {
					/* DQUOTE in braces is part of text */

					notify("CHARACTERS", yytext);
					return Parser::make_CHARACTERS(yytext,
							driver_.current_token_location());
				} else {
					/* DQUOTE not escaped by braces ends string */

					BEGIN(afterstringpart);

					notify("DQUOTE", yytext);
					return
						Parser::make_DQUOTE(driver_.current_token_location());
					}
				}



<inside_bstring>[^\{\}]+ { /* Part of string without braces */

				notify("CHARACTERS", yytext);
				return Parser::make_CHARACTERS(yytext,
					driver_.current_token_location());
				}

<inside_bstring>{RBRACE} { /* RBRACE: close the string or maybe inside as well*/
				if (within_braces()) {
					/* RBRACE in positive nesting is part of text */
					close_brace();

					notify("CHARACTERS", yytext);
					return Parser::make_CHARACTERS(yytext,
						driver_.current_token_location());
				} else {
					/* Outmost RBRACE _always_ ends string */
					/* (No need to test for escaping)        */

					BEGIN(afterstringpart);

					notify("RBRACE", yytext);
					return
						Parser::make_RBRACE(driver_.current_token_location());
					}
				}




<value>{INTEGER} { /* Bare integer value, ends the value */
				BEGIN(tag);

				notify("INTEGER", yytext);
				return Parser::make_INTEGER(yytext,
						driver_.current_token_location());
				}

<value>{LBRACE}	{ /* '{' starting a braced string */
				BEGIN(inside_bstring);

				notify("LBRACE", yytext);
				return Parser::make_LBRACE(driver_.current_token_location());
				}

<value>{DQUOTE} { /* DQUOTE starts quoted string (maybe a concatenation) */
				BEGIN(inside_qstring);

				notify("DQUOTE", yytext);
				return Parser::make_DQUOTE(driver_.current_token_location());
				}

<value>{STRINGREF} { /* Reference to predefined string (maybe a concatenation)*/
				BEGIN(afterstringpart);

				notify("STRINGREF", yytext);
				return Parser::make_STRINGREF(yytext,
						driver_.current_token_location());
				}

<value>{RBRACE} { /* RBRACE closing an empty entry body */
				BEGIN(INITIAL);
				return Parser::make_RBRACE(driver_.current_token_location());
				}



<tag>{ASSIGN_OP} { /* Assignment Operator */
				BEGIN(value);

				notify("ASSIGN_OP", yytext);
				return Parser::make_ASSIGN_OP(driver_.current_token_location());
				}

<tag>{KEY}		{ /* KEY, left side of the tag */

				notify("KEY", yytext);
				return Parser::make_KEY(yytext,
						driver_.current_token_location());
				}

<tag>{COMMA}	{ /* The COMMA that ends the tag */
				notify("COMMA", yytext);
				return Parser::make_COMMA(driver_.current_token_location());
				}

<tag>{RBRACE}|{RPAREN} { /* The token that closes the entry */
				BEGIN(body);

				/* Pass RBRACE to the enclosing body state (braced or */
				/* parenthed) to be processed there                   */
				unput(yytext[yyleng-1]);
				}



<id>{ID}		{ /* the entry's ID */

				notify("ID", yytext);
				return Parser::make_ID(yytext,
						driver_.current_token_location());
				}

<id>{COMMA}		{ /* the COMMA between the ID and the taglist */
				BEGIN(tag);

				notify("COMMA", yytext);
				return Parser::make_COMMA(driver_.current_token_location());
				}



<body>{RBRACE} { /* The RBRACE that closes the entry */
				BEGIN(INITIAL);

				notify("RBRACE", yytext);
				return Parser::make_RBRACE(driver_.current_token_location());
				}

<body>{RPAREN} { /* The RPAREN that closes the entry */
				BEGIN(INITIAL);

				notify("RPAREN", yytext);
				return Parser::make_RPAREN(driver_.current_token_location());
				}



<start_string>{LBRACE} { /* LBRACE after DECL_DOCTYPE that opens the entry */
				BEGIN(tag);

				notify("LBRACE", yytext);
				return Parser::make_LBRACE(driver_.current_token_location());
				}

<start_string>{LPAREN} { /* LPAREN after DECL_DOCTYPE that opens the entry */
				BEGIN(tag);

				notify("LPAREN", yytext);
				return Parser::make_LPAREN(driver_.current_token_location());
				}



<start_preamble>{LBRACE} { /* LBRACE after DECL_DOCTYPE that opens the entry */
				BEGIN(value);

				notify("LBRACE", yytext);
				return Parser::make_LBRACE(driver_.current_token_location());
				}

<start_preamble>{LPAREN} { /* LPAREN after DECL_DOCTYPE that opens the entry */
				BEGIN(value);

				notify("LPAREN", yytext);
				return Parser::make_LPAREN(driver_.current_token_location());
				}



<start_entry>{LBRACE} { /* LBRACE after DECL_DOCTYPE that opens the entry */
				BEGIN(id);

				notify("LBRACE", yytext);
				return Parser::make_LBRACE(driver_.current_token_location());
				}

<start_entry>{LPAREN} { /* LPAREN after DECL_DOCTYPE that opens the entry */
				BEGIN(id);

				notify("LPAREN", yytext);
				return Parser::make_LPAREN(driver_.current_token_location());
				}



<entry_type>{DECL_STRING} { /* "STRING"/i */
				BEGIN(start_string);

				notify("DECL_STRING", yytext);
				return Parser::make_DECL_STRING(yytext,
						driver_.current_token_location());
				}

<entry_type>{DECL_PREAMBLE} { /* "PREAMBLE"/i */
				BEGIN(start_preamble);

				notify("DECL_PREAMBLE", yytext);
				return Parser::make_DECL_PREAMBLE(yytext,
						driver_.current_token_location());
				}

<entry_type>{DECL_COMMENT} { /* "COMMENT"/i */
				BEGIN(comment);

				notify("DECL_COMMENT", yytext);
				return Parser::make_DECL_COMMENT(yytext,
						driver_.current_token_location());
				}

<entry_type>{DECL_DOCTYPE}	{ /* BibTeX document type Entry name */

				BEGIN(start_entry);

				notify("DECL_DOCTYPE", yytext);
				return Parser::make_DECL_DOCTYPE(yytext,
						driver_.current_token_location());
				}




<comment>{ENTRYMARKER} { /* ends the comment */
				BEGIN(INITIAL);
				unput(yytext[yyleng-1]);
				}

<comment>\n		{ /* ends the comment */
				BEGIN(INITIAL);
				}

<comment>[^@\n]* { /* ignore any character except \n and ENTRYMARKER */

				notify("CHARACTERS", yytext);
				return Parser::make_CHARACTERS(yytext,
						driver_.current_token_location());
				}



<INITIAL>{ENTRYMARKER} { /* '@', the start marker for an entry in EVERY state */
				BEGIN(entry_type);

				notify("ENTRYMARKER", yytext);
				return
					Parser::make_ENTRYMARKER(driver_.current_token_location());
				}

<INITIAL>[^@\n] ; /* Ignore every character outside of an entry or comment */

<INITIAL>\n     ; /* Ignore every newline outside of an entry or comment */



<*>{WHITESPACE} ; /* Ignore every whitespace */

<*>\n           ; /* Ignore every newline outside of tag values */

<*>.			{ /* Any unexpected non-whitespace character */
				unexpected(yytext, driver_.current_token_location());

				return Parser::make_YYUNDEF(driver_.current_token_location());
				}

<*><<EOF>>      {
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

void Lexer::open_brace()
{
	++braces_opened_;
}

void Lexer::close_brace()
{
	--braces_opened_;
}

bool Lexer::within_braces() const
{
	return braces_opened_ > 0;
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

	driver_.update_token_location(current_pos_);
}

Printer& Lexer::printer()
{
	return *printer_;
}

} /* namespace yycuesheet */ } /* namespace cuesheet */

