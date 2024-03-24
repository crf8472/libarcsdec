#ifndef CUESHEET_YYCUESHEET_LEXER_DEFS_HPP
#define CUESHEET_YYCUESHEET_LEXER_DEFS_HPP


#undef YY_DECL
/**
 * The signature of the yylex() function is defined by YY_DECL.
 *
 * Original yylex() returns int. Since Bison 3 uses symbol_type, we must change
 * the returned type.
 *
 * We also rename yylex() to Lexer::get_next_token().
*/
#define YY_DECL cuesheet::yycuesheet::Parser::symbol_type cuesheet::yycuesheet::Lexer::get_next_token()


#include <memory>
#include <ostream>


// From cuesheet.y:
// Include the Token definitions as well as the redefined yylex()
// in section "code top" (that calls get_next_token())
#include "cuesheet.tab.hpp"


namespace cuesheet {


class Driver;


namespace yycuesheet {


/**
 * \brief Print tokens.
 */
class Printer final
{
public:

	Printer(std::ostream &out) : out_(out) { /* empty */ }

	Printer() : Printer(std::cout) { /* empty */ };

	void token(const int state, const std::string &token_name,
			const std::string &token_value) const;

	std::ostream& error();

	std::ostream& warn();

	std::ostream& info();


private:

	std::ostream &out_;
};


/**
 * \brief BibTeX Lexer.
 */
class Lexer final : public Cuesheet_FlexLexer
{
public:

	/**
	 * \brief Constructor for BibTeX lexer.
	 *
	 * \param driver The cuesheet::Driver that constructed this lexer.
	 */
	explicit Lexer(Driver &driver)
		: driver_ { driver }
		, braces_opened_ { 0 }
		, current_pos_ {}
		, printer_ { std::make_unique<Printer>() }
	{
		/* empty */
	}

	/**
	 * \brief Destructor.
	 */
	~Lexer() noexcept override = default;

	/**
	 * \brief Return next token.
	 *
	 * This is the function signature declared by redefined YY_DECL. The
	 * implementation is yylex().
	 *
	 * Implementation will be provided by flex.
	 *
	 * \return Next token
	 */
	Parser::symbol_type get_next_token();


private:

	/**
	 * \brief Recognize an opening brace (LBRACE) within a quoted string.
	 *
	 * Called by yylex() to signal an open brace within a string.
	 */
	void open_brace();

	/**
	 * \brief Recognize a closing brace (RBRACE) within a quoted string.
	 *
	 * Called by yylex() to signal a closing brace within a string.
	 */
	void close_brace();

	/**
	 * \brief Returns TRUE iff <tt>braces_opened() > 0</tt> otherwise FALSE.
	 *
	 * Called by yylex() to check whether braces are currently opened.
	 *
	 * \return TRUE if position is between LBRACE and RBRACE.
	 */
	bool within_braces() const;

	/**
	 * \brief Sets the internal position to the end of the current token.
	 *
	 * Called by yylex() to update the position in the file.
	 *
	 * \param[in] line_no      Current line number (e.g. yylineno)
	 * \param[in] token_length Length of the current token (eg. yyleng)
	 */
	void shift_pos(const int line_no, const int token_length);

	/**
	 * \brief Notifies about the current token.
	 *
	 * Called by yylex() to notify Lexer about current token.
	 *
	 * \param[in] token_name Name of the token
	 * \param[in] characters Parsed characters (e.g. yytext)
	 */
	void notify(const std::string &token_name, const std::string &characters);

	/**
	 * \brief Hook for intercepting unexpected characters.
	 *
	 * Called by yylex() on unexpected characters.
	 *
	 * \param[in] chars The unexpected characters
	 * \param[in] loc   Location of the unexpected characters
	 */
	void unexpected(const std::string &chars, const location &loc);

	/**
	 * \brief Use the Printer instance.
	 *
	 * \return The Printer of this Lexer
	 */
	Printer& printer();



	Driver &driver_;

	int braces_opened_;

	position current_pos_;

	std::unique_ptr<Printer> printer_;
};

} // namespace yycuesheet
} // namespace cuesheet

#endif // CUESHEET_YYCUESHEET_LEXER_DEFS_HPP

