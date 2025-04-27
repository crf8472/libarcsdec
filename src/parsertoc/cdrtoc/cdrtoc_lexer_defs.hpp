#ifndef __LIBARCSDEC_CDRTOC_LEXER_DEFS_HPP__
#define __LIBARCSDEC_CDRTOC_LEXER_DEFS_HPP__

/**
 * \file
 *
 * \brief Declaration of the FlexLexer subclass for CDRDAO/TOC lexing.
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
#define YY_DECL arcsdec::details::cdrtoc::yycdrtoc::Parser::symbol_type arcsdec::details::cdrtoc::yycdrtoc::Lexer::next_token()


#include <string>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winline-namespace-reopened-noninline"
#endif

// From cdrtoc.y:
// Include the Token definitions as well as the redefined yylex()
// in section "code top" (that calls get_next_token())
#ifndef __LIBARCSDEC_CDRTOC_TAB_HPP__
#include "cdrtoc_tab.hpp"
#endif

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

/* NOTE that Lexer is part of v_1_0_0! File cdrtoc.l does always refer to */
/* the Lexer class without specifiying the inline namespace for version!    */

namespace arcsdec { inline namespace v_1_0_0 { namespace details {

// forward declared from flexbisondriver.hpp

class LexerHandler;

template <class POSITION, class LOCATION>
class TokenLocation;

using cdrtoc::yycdrtoc::position;
using cdrtoc::yycdrtoc::location;
using LocationClass = TokenLocation<position, location>;

namespace cdrtoc
{
namespace yycdrtoc
{

// NOTE What is declared here receives its implementation from the third
// section in cdrtoc.l.


/**
 * \brief CDRDAO/TOC Lexer.
 *
 * Subclass of the flex provided lexer interface. Provides interface to the
 * flex-generated lexing routines.
 */
class Lexer final : public CDRTOC_FlexLexer
{
	/**
	 * \brief Internal token position.
	 *
	 * Always the _end_ of the current token!
	 */
	position current_pos_;

	/**
	 * \brief Internal token location.
	 */
	LocationClass* current_loc_;

	/**
	 * \brief Class for interfacing the lexer from calling code.
	 */
	LexerHandler* lexer_handler_;

public:

	/**
	 * \brief Constructor for CDRDAO/TOC lexer.
	 *
	 * \param[in] driver The cdrtoc::Driver that constructed this lexer.
	 */
	explicit Lexer(LocationClass* loc, LexerHandler* handler)
		: current_pos_   { /* empty */ }
		, current_loc_   { loc }
		, lexer_handler_ { handler }
	{
		/* empty */
	}

	/**
	 * \brief Destructor.
	 */
	~Lexer() noexcept = default;

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

	/**
	 * \brief Provide current location.
	 *
	 * \return Current token location
	 */
	Parser::location_type loc() const;

	/**
	 * \brief Get the driver to start some user action.
	 *
	 * \return The Driver of this Lexer
	 */
	LexerHandler* handler();

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
	 * \brief Called by yylex() to update the position in the file.
	 *
	 * \param[in] line_no      Current line number (e.g. yylineno)
	 * \param[in] token_length Length of the current token (eg. yyleng)
	 */
	void shift_pos(const int line_no, const int token_length);
};

} // namespace yycdrtoc
} // namespace cdrtoc
} /*details*/ } /*v_1_0_0*/ } /*arcsdec*/

#endif // __LIBARCSDEC_CDRTOC_LEXER_DEFS_HPP__

