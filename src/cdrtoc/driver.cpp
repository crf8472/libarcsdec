#ifndef __LIBARCSDEC_CDRTOC_DRIVER_HPP__
#include "driver.hpp"
#endif

#ifndef __LIBARCSDEC_CDRTOC_HANDLER_HPP__
#include "handler.hpp"
#endif

#ifndef __LIBARCSDEC_CDRTOC_LEXER_HPP__
#include "cdrtoc_lexer.hpp"   // includes also cdrtoc.tab.hpp
#endif


namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{
namespace cdrtoc
{


Driver::Driver()
	: current_token_location_ { /* empty */ }
	, lexer_   { std::make_unique<yycdrtoc::Lexer>(*this) }
	, parser_  { std::make_unique<yycdrtoc::Parser>(*lexer_, *this) }
	, handler_ { nullptr }
{
	// empty
}


Driver::~Driver() noexcept = default;


void Driver::set_input(std::istream& is)
{
	reset();
	lexer_->switch_streams(&is, nullptr);
}


void Driver::set_lexer_debug_level(const int lvl)
{
	lexer_->set_debug(lvl);
}


void Driver::set_parser_debug_level(const int /*lvl*/)
{
	// Commented out since parser_ does not have function set_debug_level()
	// when bison is not called with parameter --debug.
	// This parameter is deliberately missing for CMAKE_BUILD_TYPE=Release.
	// For debugging, add --debug to COMPILE_FLAGS in CMakeLists.txt and
	// uncomment the line below as well as parameter 'lvl'.

	//parser_->set_debug_level(lvl); // %define parse.trace
}


int Driver::parse()
{
	reset_loc();
	return parser_->parse();
}


void Driver::reset()
{
	reset_loc();
	//handler_->reset();
}


void Driver::set_handler(Handler& handler)
{
	handler_ = &handler;
}


const Handler& Driver::handler()
{
	return *handler_;
}


void Driver::notify(const int /*state*/, const std::string& /*token_name*/,
			const std::string& /*chars*/)
{
	// TODO
}


void Driver::unexpected(const std::string& /*chars*/, const location& /*loc*/)
{
	// TODO
}


void Driver::reset_loc()
{
	current_token_location_ = create_initial_loc();
}


void Driver::step_to(const yycdrtoc::position& lexer_pos)
{
	current_token_location_->step(); // set begin to end
	current_token_location_->end = lexer_pos; // set end to current
}


yycdrtoc::location Driver::loc() const
{
	return *current_token_location_;
}


std::unique_ptr<location> Driver::create_initial_loc() const
{
	return std::make_unique<location>(nullptr, 1, 1);
}


Handler* Driver::get_handler()
{
	return handler_;
}

} // namespace cdrtoc
} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

