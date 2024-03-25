#include "driver.hpp"

//#include "handler.hpp"
#include "lexer.hpp"   // includes also cuesheet.tab.hpp


namespace cuesheet {


Driver::Driver()
	: current_token_location_{}
	, lexer_ (std::make_unique<cuesheet::yycuesheet::Lexer>(*this))
	, parser_(std::make_unique<cuesheet::yycuesheet::Parser>(*lexer_, *this))
	//, handler_ {nullptr}
{
	// empty
}


Driver::~Driver() = default;


void Driver::set_input(std::istream &is)
{
	reset();
	lexer_->switch_streams(&is, nullptr);
}


void Driver::set_lexer_debug_level(const int lvl)
{
	lexer_->set_debug(lvl);
}


void Driver::set_parser_debug_level(const int lvl)
{
	parser_->set_debug_level(lvl);
}


int Driver::parse()
{
	current_token_location_ = std::make_unique<location>(nullptr, 1, 1);
	return parser_->parse();
}


void Driver::reset()
{
	current_token_location_ = std::make_unique<location>(nullptr, 1, 1);
	//handler_->reset();
}

/*
void Driver::set_handler(Handler *handler)
{
	handler_ = handler;
}


const Handler& Driver::handler()
{
	return *handler_;
}
*/

void Driver::update_loc(const yycuesheet::position &p)
{
	current_token_location_->step();
	current_token_location_->end = p;
}


yycuesheet::location Driver::loc() const
{
	return *current_token_location_;
}

/*
Handler* Driver::get_handler()
{
	return handler_;
}
*/

} // namespace cuesheet

