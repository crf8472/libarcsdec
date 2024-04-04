#ifndef __LIBARCSDEC_CUESHEET_LEXER_DEFS_HPP__
#define __LIBARCSDEC_CUESHEET_LEXER_DEFS_HPP__

/**
 * \file
 *
 * \brief Declaration of the FlexLexer subclass for Cuesheet lexing.
 */

#undef YY_DECL
/**
 * The signature of the yylex() function is defined by YY_DECL.
 *
 * Original yylex() returns int. Since Bison 3 uses symbol_type, we must change
 * the returned type.
 *
 * We also rename yylex() to Lexer::next_token().
*/
#define YY_DECL arcsdec::details::cuesheet::yycuesheet::Parser::symbol_type arcsdec::details::cuesheet::yycuesheet::Lexer::next_token()


#include <memory>
#include <ostream>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winline-namespace-reopened-noninline"
#endif

// From cuesheet.y:
// Include the Token definitions as well as the redefined yylex()
// in section "code top" (that calls get_next_token())
#include "cuesheet.tab.hpp"

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

/* NOTE that Lexer is part of v_1_0_0! File cuesheet.l does always refer to */
/* the Lexer class without specifiying the inline namespace for version!    */

namespace arcsdec { inline namespace v_1_0_0 { namespace details {
namespace cuesheet
{


class Driver;


namespace yycuesheet
{

// NOTE What is declared here receives its implementation from the third
// section in cuesheet.l.


/**
 * \brief Cuesheet Lexer.
 *
 * Subclass of the flex provided lexer interface. Provides interface to the
 * flex-generated lexing routines.
 */
class Lexer final : public Cuesheet_FlexLexer
{
	/**
	 * \brief Internal token position.
	 */
	position current_pos_;

	/**
	 * \brief Class for interfacing the lexer from calling code.
	 */
	Driver &driver_;

public:

	/**
	 * \brief Constructor for Cuesheet lexer.
	 *
	 * \param[in] driver The cuesheet::Driver that constructed this lexer.
	 */
	explicit Lexer(Driver &driver)
		: current_pos_ {}
		, driver_ { driver }
	{
		/* empty */
	}

	/**
	 * \brief Destructor.
	 */
	~Lexer() noexcept = default;

	/**
	 * \brief Called by yylex() to notify Lexer about current token.
	 *
	 * \param[in] token_name Token name
	 * \param[in] chars      Characters in token
	 */
	void notify(const std::string& token_name, const std::string& chars);

	/**
	 * \brief Called by yylex() on unexpected characters.
	 *
	 * \param[in] chars Characters
	 * \param[in] loc   Location
	 */
	void unexpected(const std::string& chars, const location& loc);

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
	Parser::symbol_type next_token();

private:

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
	 * \brief Get the driver to start some user action.
	 *
	 * \return The Driver of this Lexer
	 */
	Driver& get();
};

} // namespace yycuesheet
} // namespace cuesheet
} /*details*/ } /*v_1_0_0*/ } /*arcsdec*/

#endif // __LIBARCSDEC_CUESHEET_LEXER_DEFS_HPP__

