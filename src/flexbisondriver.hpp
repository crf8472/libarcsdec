#ifndef __LIBARCSDEC_FLEXBISONDRIVER_HPP__
#define __LIBARCSDEC_FLEXBISONDRIVER_HPP__
/**
 * \file
 *
 * \brief A driver class for flex/bison based parsers.
 *
 * Provides some functionalities for reuse with different flex/bison parser
 * classes.
 */

#include <fstream>     // for ifstream
#include <memory>      // for unique_ptr
#include <ostream>     // for ostream
#include <stdexcept>   // for runtime_error
#include <string>      // for string
#include <type_traits> // for void_t


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

	// TODO Provide unexpected() which is dependent from location type
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
 * \brief Shift position \c current to new position with possible newline.
 *
 * \param[in] current The current position
 * \param[in] line_no Line number of new position
 * \param[in] col_no  Column number of new position
 *
 * \tparam POSITION Position type of lexer
 */
template<class POSITION>
auto lexer_shift_pos(POSITION current, const int line_no, const int col_no)
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
std::string lexer_strip_quotes(const std::string& s);


/**
 * \brief Interface: parser handler defines reaction on the occurence of symbols.
 */
class ParserHandler
{
	virtual void do_start_input()
	= 0;

	virtual void do_end_input()
	= 0;

public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~ParserHandler() noexcept;

	/**
	 * \brief To be called before the first token.
	 */
	void start_input();

	/**
	 * \brief To be called after the last token.
	 */
	void end_input();
};

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
 *
 * Provides facility to reset() and to step_to() a position.
 */
template<class POSITION, class LOCATION>
class TokenLocation
{
	/**
	 * \brief Internal location.
	 */
	LOCATION current_token_location_;

	/**
	 * \brief Create an initial location.
	 */
	LOCATION create_initial_loc() const
	{
		return LOCATION( nullptr, 1, 1 );
	}

public:

	/**
	 * \brief Constructor.
	 */
	TokenLocation()
		: current_token_location_ { /* empty */ }
	{
		// empty
	}

	/**
	 * \brief Reset this location to its initial value.
	 */
	void reset()
	{
		this->current_token_location_ = this->create_initial_loc();
	}

	/**
	 * \brief Provide this location in the lexer/parser specific format.
	 *
	 * \return Current location in the input file
	 */
	LOCATION loc() const
	{
		return this->current_token_location_;
	}


	// used as lexer callback


	/**
	 * \brief Step to the specified position.
	 *
	 * The specified position will become the new end position. The current end
	 * position becomes the new start position.
	 *
	 * \param[in] lexer_pos The new end position.
	 */
	void step_to(const POSITION& lexer_pos)
	{
		this->current_token_location_.step(); // set begin <- end
		this->current_token_location_.end = lexer_pos; // set end <- current
	}
};


template <typename PARSER, typename = std::void_t<>>
struct IsDebugEnabled : std::false_type
{
	using debug_level_type = int;

	debug_level_type debug_level(std::unique_ptr<PARSER>&) const
	{
		return 0;
	}

	void set_debug_level(std::unique_ptr<PARSER>&, const debug_level_type) const
	{
		// do nothing
	}
};


// Check whether parser class has function 'set_debug_level()' and
// defines a debug_level_type
template <typename PARSER>
struct IsDebugEnabled <PARSER,
		std::void_t<decltype(std::declval<PARSER>().set_debug_level(1))>
	> : std::true_type
{
	using debug_level_type = typename PARSER::debug_level_type;

	debug_level_type debug_level(std::unique_ptr<PARSER>& p) const
	{
		return p->debug_level();
	}

	void set_debug_level(std::unique_ptr<PARSER>& p, const debug_level_type l)
		const
	{
		p->set_debug_level(l);
	}
};


/**
 * \brief Wrapper for managing a bison parser instance.
 *
 * The wrapper provides default implementations for the member functions
 * of the bison parser that are only available when YYDEBUG is set.
 */
template <class PARSER>
class BisonParser
{
	/**
	 * \brief Internal bison parser instance.
	 */
	std::unique_ptr<PARSER> parser_;

	/**
	 * \brief Debug wrapper.
	 */
	IsDebugEnabled<PARSER>  debug_;

public:

	/**
	 * \brief Type to represent the internal parsers debug level.
	 */
	using debug_level_type = typename IsDebugEnabled<PARSER>::debug_level_type;

	/**
	 * \brief Constructor.
	 *
	 * \param[in] parser Parser to wrap
	 */
	explicit BisonParser(std::unique_ptr<PARSER> parser)
		: parser_ { std::move(parser) }
		, debug_  { /* default */ }
	{
		// empty
	}

	/**
	 * \brief TRUE iff debug capability is enabled, otherwise FALSE.
	 *
	 * \return TRUE iff debug capability is enabled
	 */
	bool debug_enabled() const
	{
		return IsDebugEnabled<PARSER>::value;
	}


	// pass-through functions

	void set_debug_level(const debug_level_type lvl)
	{
		this->debug_.set_debug_level(parser_, lvl);
	}

	debug_level_type debug_level() const
	{
		return this->debug_.debug_level(parser_);
	}

	int parse()
	{
		return this->parser_->parse();
	}
};


#if defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#endif

/**
 * \brief .
 */
template<class LEXER, class PARSER, class LOCATION, class POSITION,
	class HANDLER>
class FlexBisonDriver final
{
	LexerHandler* l_handler_;

	HANDLER* p_handler_;

	TokenLocation<POSITION, LOCATION> current_loc_;

	std::unique_ptr<LEXER> lexer_;

	BisonParser<PARSER>    parser_;

public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] handler Parse handler
	 */
	explicit FlexBisonDriver(LexerHandler* l_handler, HANDLER* p_handler)
		: l_handler_   { l_handler }
		, p_handler_   { p_handler }
		, current_loc_ { /* empty */ }
		, lexer_       { std::make_unique<LEXER>(&current_loc_, l_handler_) }
		, parser_      { std::make_unique<PARSER>(
								&current_loc_, lexer_.get(), p_handler_) }
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
		this->lexer_->switch_streams(&is, nullptr);
	}

	/**
	 * \brief Set lexer debug level
	 *
	 * Passing '0' deactivates debug output, any other value sets the level.
	 */
	void set_lexer_debug_level(const int lvl)
	{
		this->lexer_->set_debug(lvl);
	}

	/**
	 * \brief Set parser debug level
	 *
	 * Passing '0' deactivates debug output, any other value sets the level.
	 */
	void set_parser_debug_level(const int lvl)
	{
		this->parser_.set_debug_level(lvl);
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

		if (this->parser_.parse() != 0)
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

