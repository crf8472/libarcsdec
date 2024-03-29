#ifndef __LIBARCSDEC_CUESHEET_TOCHANDLER_HPP__
#define __LIBARCSDEC_CUESHEET_TOCHANDLER_HPP__

/**
 * \file
 *
 * \brief Public header for a handler that constructs a TOC.
 */

#ifndef __LIBARCSDEC_CUESHEET_HANDLER_HPP__
#include "handler.hpp"
#endif

#include <string>

namespace cuesheet
{

class TOCHandler : public Handler
{
	virtual void do_start_input();

	virtual void do_end_input();

};

} // namespace cuesheet

#endif

