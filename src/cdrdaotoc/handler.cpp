#ifndef __LIBARCSDEC_CDRDAOTOC_HANDLER_HPP__
#include "handler.hpp"
#endif

namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{
namespace cdrdaotoc
{


Handler::Handler() = default;


Handler::~Handler() noexcept = default;


void Handler::reset()
{
	do_reset();
}


void Handler::start_input()
{
	do_start_input();
}


void Handler::end_input()
{
	do_end_input();
}


void Handler::catalog(const std::string& mcn)
{
	do_catalog(mcn);
}


void Handler::cdtextfile(const std::string& name)
{
	do_cdtextfile(name);
}


void Handler::file(const std::string& name, const FILE_FORMAT& t)
{
	do_file(name, t);
}


void Handler::track_flags(const std::vector<TRACK_FLAG>& flags)
{
	do_track_flags(flags);
}


void Handler::index(const int i, const int m, const int s, const int f)
{
	do_index(i, m, s, f);
}


void Handler::isrc(const std::string& name)
{
	do_isrc(name);
}


void Handler::performer(const std::string& name)
{
	do_performer(name);
}


void Handler::postgap(const int m, const int s, const int f)
{
	do_postgap(m, s, f);
}


void Handler::pregap(const int m, const int s, const int f)
{
	do_pregap(m, s, f);
}


// rem


void Handler::songwriter(const std::string& name)
{
	do_songwriter(name);
}


void Handler::title(const std::string& title)
{
	do_title(title);
}


void Handler::track(const int i, const TRACK_MODE& m)
{
	do_track(i, m);
}


void Handler::do_reset()
{
	// empty
}


void Handler::do_start_input()
{
	// empty
}


void Handler::do_end_input()
{
	// empty
}

} // namespace cdrdaotoc
} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

