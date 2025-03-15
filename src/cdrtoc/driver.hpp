#ifndef __LIBARCSDEC_CDRTOC_DRIVER_HPP__
#define __LIBARCSDEC_CDRTOC_DRIVER_HPP__

/**
 * \file
 *
 * \brief Public header for using the CDRDAO/TOC parsing driver.
 */

#include <memory>


namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{

/**
 * \brief Main namespace for CDRDAO/TOC tool classes.
 */
namespace cdrtoc
{

/**
 * \internal
 *
 * \brief Namespace for implementation details of '::cdrtoc'.
 */
namespace yycdrtoc
{

// We cannot include 'lexer.hpp' since this would import yyFlexLexer into
// cdrtoc.l/cdrtoc.yy.cpp (via this file, 'driver.hpp').
// This must be avoided since 'lexer.hpp' redefines yyFlexLexer.
class Lexer;

// Forward declare classes auto-generated by bison
class Parser;
class location;
class position;

} // namespace yycdrtoc

using yycdrtoc::Lexer;
using yycdrtoc::Parser;
using yycdrtoc::location;
using yycdrtoc::position;


class Handler;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"

/**
 * \brief Interface to bison-generated cdrtoc parser.
 *
 * It drives the lexer, keeps parsed content and is a good place to store
 * additional context data.
 *
 * Both parser and lexer have access to it via internal references.
 *
 * The AST for CDRDAO/TOC, however, is a simple vector with nodes.
 */
class Driver final
{
public:

	/**
	 * FIXME Scanner and Parser want to call the private member functions.
	 */
	friend class Parser;
	friend class Lexer;

	/**
	 * \brief Constructor.
	 */
	Driver();

	/**
	 * \brief Destructor.
	 */
	~Driver() noexcept;

	/**
	 * \brief Set lexer input stream.
	 *
	 * Default is standard input. Stream has to be opened.
	 *
	 * Implies reset().
	 *
	 * \param[in] is Input stream to use
	 */
	void set_input(std::istream& is);

	/**
	 * \brief Set lexer debug level
	 *
	 * Passing '0' deactivates debug output, any other value sets the level.
	 */
	void set_lexer_debug_level(const int lvl);

	/**
	 * \brief Set parser debug level
	 *
	 * Passing '0' deactivates debug output, any other value sets the level.
	 */
	void set_parser_debug_level(const int lvl);

	/**
	 * \brief Run parser.
	 *
	 * \returns 0 on success, 1 on failure
	 */
	int parse();

	/**
	 * \brief Clear parsed content.
	 */
	void reset();

	/**
	 * \brief Sets the parser handler to use.
	 *
	 * \param[in] handler The parser handler to use
	 */
	void set_handler(Handler& handler);

	/**
	 * \brief Returns the parser handler used.
	 */
	const Handler& handler();


private:

	/**
	 * \brief Callback for Lexer about current token.
	 */
	void notify(const int state, const std::string& token_name,
			const std::string& chars);

	/**
	 * \brief Callback for Lexer about unexpected characters.
	 */
	void unexpected(const std::string& chars, const location& loc);

	/**
	 * \brief Clear source location info.
	 *
	 * Called by parse() on start and by reset().
	 */
	void reset_loc();

	/**
	 * \brief Called by Lexer to update the internal location.
	 *
	 * \param p End position of current token
	 */
	void step_to(const position& p);

	/**
	 * \brief Returns last Lexer location.
	 *
	 * Used in error messages.
	 *
	 * \return Last Lexer location
	 */
	location loc() const;

	/**
	 * \brief Create initial location information.
	 *
	 * Called by reset_loc().
	 */
	std::unique_ptr<location> create_initial_loc() const;

	/**
	 * \brief Access driver handler.
	 *
	 * \return Handler used by this instance
	 */
	Handler* get_handler();



	std::unique_ptr<location> current_token_location_;

	std::unique_ptr<Lexer> lexer_;

	std::unique_ptr<Parser> parser_;

	Handler* handler_;
};

#pragma GCC diagnostic pop

} // namespace cdrtoc
} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

#endif // __LIBARCSDEC_CDRTOC_DRIVER_HPP__

