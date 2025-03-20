#ifndef __LIBARCSDEC_FLEXBISONDRIVER_HPP__
#define __LIBARCSDEC_FLEXBISONDRIVER_HPP__
/**
 * \file
 *
 * \brief A driver class for flex/bison based parsers.
 */

#include <fstream>   // for ifstream
#include <memory>    // for unique_ptr
#include <ostream>   // for ostream
#include <stdexcept> // for runtime_error
#include <string>    // for string
#include <vector>    // for vector

namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{


/**
 * \brief Interface: lexer handler defines reaction on the occurrence of tokens.
 */
class LexerHandler
{
	virtual void do_notify(const std::string& token_name,
			const std::string& chars)
	= 0;

public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~LexerHandler() noexcept;

	/**
	 * \brief Callback for lexers to notify the handler about a token.
	 *
	 * \param[in] token_name Name of the token
	 * \param[in] chars      Chars that form the token
	 */
	void notify(const std::string& token_name, const std::string& chars);
};


/**
 * \brief Default lexer handler that does not do anything when notified.
 */
class DefaultLexerHandler final : public LexerHandler
{
	void do_notify(const std::string& token_name, const std::string& chars)
		final;
};


/**
 * \brief Interface: parser handler defines reaction on the occurence of symbols.
 */
class ParserHandler
{
	// empty
};


/**
 * \brief Shift position \c current to new position with possible newline.
 *
 * \param[in] current The current position
 * \param[in] line_no Line number of new position
 * \param[in] col_no  Column number of new position
 *
 * \tparam POSITION Position type of lexer
 */
template<class POSITION>
auto shift_lexer_pos(POSITION current, const int line_no, const int col_no)
	-> POSITION
{
	// Current position is always the _end_ of the current token

	if (line_no != current.line) // newline occurred?
	{
		current.lines(line_no - current.line);
		current.column = 1; // Ignore newlines when stepping forward
	} else
	{
		current.columns(col_no);
	}

	return current;
}

/**
 * \brief Remove first and last character from input string.
 *
 * If input string has at least a length of 2, remove leading and trailing
 * character, otherwise return input string unaltered.
 *
 * \note
 * Does <b>not</b> test whether first or last character are really some kind of
 * quotes! This is intended to be used within a token rule where the token will
 * not be matched if it is not quoted, so the check will already be performed.
 *
 * \param[in] s Input string
 *
 * \return String without quotes
 */
std::string strip_quotes(const std::string& s);

/**
 * \brief Report a parser error to a specified output stream.
 *
 * \param[in] loc     Error location
 * \param[in] message Error message
 * \param[in] err     Error output stream
 */
template<class LOCATION>
void parser_error(const LOCATION& loc, const std::string& message,
		std::ostream& err)
{
	if (loc.begin.line == loc.end.line)
	{
		if (loc.end.column - 1 == loc.begin.column)
		{
			err << "Parser error at line " << loc.begin.line
				<< ", char " << loc.begin.column
				<< ": " << message << '\n';
		} else
		{
			err << "Parser error at line " << loc.begin.line
				<< " chars " << loc.begin.column
				<< "-" << loc.end.column - 1
				<< ": " << message << '\n';
		}
	} else
	{
		err << "Parser error from line " << loc.begin.line
			<< ", char " << loc.begin.column
			<< " till line " << loc.end.line
			<< ", char " << loc.end.column - 1
			<< ": " << message << '\n';
	}
}

/**
 * \brief Wrapper for the auto-generated class 'location'.
 */
template<class POSITION, class LOCATION>
class TokenLocation
{
	LOCATION current_token_location_;

	LOCATION create_initial_loc() const
	{
		return LOCATION( nullptr, 1, 1 );
	}

public:

	TokenLocation()
		: current_token_location_ { /* empty */ }
	{
		// empty
	}

	void reset()
	{
		this->current_token_location_ = this->create_initial_loc();
	}

	LOCATION loc() const
	{
		return this->current_token_location_;
	}

	// used as lexer callback

	void step_to(const POSITION& lexer_pos)
	{
		this->current_token_location_.step(); // set begin <- end
		this->current_token_location_.end = lexer_pos; // set end <- current
	}
};


#if defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#endif

/**
 * \brief .
 */
template<class LEXER, class PARSER, class LOCATION, class POSITION>
class Driver final
{
	LexerHandler* l_handler_;

	ParserHandler* p_handler_;

	TokenLocation<POSITION, LOCATION> current_loc_;

	std::unique_ptr<LEXER> lexer_;

	std::unique_ptr<PARSER> parser_;

public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] handler Parse handler
	 */
	explicit Driver(LexerHandler* l_handler, ParserHandler* p_handler)
		: l_handler_   { l_handler }
		, p_handler_   { p_handler }
		, current_loc_ { /* empty */ }
		, lexer_       { std::make_unique<LEXER>(&current_loc_, l_handler) }
		, parser_      { std::make_unique<PARSER>(
								&current_loc_, lexer_.get(), p_handler) }
	{
		// empty
	}

	/**
	 * \brief Set lexer input stream.
	 *
	 * Default is standard input. Stream has to be opened.
	 *
	 * Implies reset().
	 *
	 * \param[in] is Input stream to use
	 */
	void set_input(std::istream& is)
	{
		this->reset();
		lexer_->switch_streams(&is, nullptr);
	}

	/**
	 * \brief Set lexer debug level
	 *
	 * Passing '0' deactivates debug output, any other value sets the level.
	 */
	void set_lexer_debug_level(const int lvl) const
	{
		lexer_->set_debug(lvl);
	}

	/**
	 * \brief Set parser debug level
	 *
	 * Passing '0' deactivates debug output, any other value sets the level.
	 */
	void set_parser_debug_level(const int lvl) const
	{
		parser_->set_debug_level(lvl);
	}

	/**
	 * \brief Run parser.
	 *
	 * \param[in] filename File to parse
	 */
	void parse(const std::string& filename)
	{
		std::ifstream file;
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			file.open(filename, std::ifstream::in);
		} catch (const std::ifstream::failure& f)
		{
			throw std::runtime_error(std::string { "Failed to open file " }
					+ filename + ". Message: " + f.what());
		}
		this->set_input(file);

		if (this->parser_->parse() != 0)
		{
			file.close();
			throw std::runtime_error(
					std::string { "Failed to parse file " } + filename);
		}
	}

	/**
	 * \brief Clear parsed content and reset location.
	 */
	void reset()
	{
		this->current_loc_.reset();
	}

	/**
	 * \brief Returns the lexer handler used.
	 */
	const LexerHandler* lexer_handler()
	{
		return this->l_handler_;
	}

	/**
	 * \brief Returns the parser handler used.
	 */
	const ParserHandler* parser_handler()
	{
		return this->p_handler_;
	}

	/**
	 * \brief Return current location.
	 *
	 * \returns Current location
	 */
	LOCATION current_location() const
	{
		return this->current_loc_.loc();
	}

	// lexer callback

	void notify(const std::string& token_name, const std::string& chars)
	{
		this->l_handler_->notify(token_name, chars);
	}
};

#if defined(__GNUG__)
#pragma GCC diagnostic pop
#endif

} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

#endif

