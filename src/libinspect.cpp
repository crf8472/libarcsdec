/**
 * \internal
 *
 * \file
 *
 * \brief Implements symbols from libinspect.hpp.
 */

#ifndef __LIBARCSDEC_LIBINSPECT_HPP__
#include "libinspect.hpp"
#endif

#ifndef __LIBARCSDEC_VERSION_HPP__
#include "version.hpp"        // for LIBARCSDEC_NAME
#endif

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp> // for ARCS_LOG_WARNING, ARCS_LOG_DEBUG
#endif

extern "C"
{
#include <dlfcn.h>     // [glibc, Linux] for dlopen, dlclose, dlerror, RTLD_LAZY
#include <link.h>      // [glibc, Linux] for link_map
}

#include <cstddef>     // for size_t
#include <regex>       // for regex, regex_match
#include <string>      // for string


namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{

void escape(std::string& input, const char c, const std::string& escape_seq)
{
	std::size_t lookHere = 0;
	std::size_t foundHere;

	const auto replacement = escape_seq + c;
	while((foundHere = input.find(c, lookHere)) != std::string::npos)
	{
		input.replace(foundHere, 1, replacement);
		lookHere = foundHere + replacement.size();
	}
}


std::regex to_libname_pattern(const std::string& libname)
{
	auto e_name = libname;

	escape(e_name, '+', "\\");
	// NOTE We allow SONAMEs to contain '+', which targets libflac++.
	// We could escape any character that is allowed in a SONAME/libfilename
	// but has special meaning in a regex. Currently, only libs with
	// ASCII-characters are recognized.

	// TODO Escape possible "special" characters in SONAMEs

	return std::regex(".*\\b" + e_name + "\\.so(\\.[0-9]+)*$",
			std::regex::icase);
}


const std::string& first_libname_match(const std::vector<std::string>& list,
		const std::string& name)
{
	static const auto empty_entry = std::string{};

	const auto pattern = to_libname_pattern(name);

	using std::cbegin;
	using std::cend;

	const auto first_match = std::find_if(cbegin(list), cend(list),
			[pattern](const std::string& lname)
			{
				return std::regex_match(lname, pattern);
			}
	);

	if (first_match == cend(list))
	{
		return empty_entry;
	}

	return *first_match;
}


std::vector<std::string> runtime_deps(const std::string& object_name)
{
	//std::cerr << "Runtime deps of " << object_name << '\n';

	// C-Style stuff: Messing with glibc to get shared object paths
	// Use dlfcn.h and link.h. Do not know a better way yet.

	const auto* object = object_name.empty() ? nullptr : object_name.c_str();

	auto* handle = ::dlopen(object, RTLD_LAZY);
	// If called with NULL for first parameter, dlopen returns the list for
	// the main executable. Take this, then figure out libarcsdec.so, then load.

	if (!handle)
	{
		throw std::runtime_error(::dlerror());
	}

	using OpaqueStruct =
		struct opaque_struct
		{
			void*  pointers[3];
			struct opaque_struct* ptr;
		};

	auto* pter = reinterpret_cast<OpaqueStruct*>(handle)->ptr;

	if (!pter)
	{
		::dlclose(handle);
		throw std::runtime_error("Got null instead of shared object handle");
	}

	using LinkMap = struct link_map;

	auto* lmap  = reinterpret_cast<LinkMap*>(pter->ptr);

	if (!lmap)
	{
		::dlclose(handle);
		throw std::runtime_error("Shared object handle contained no link_map");
	}

	// Traverse link_map for names

	auto so_list = std::vector<std::string>{};

	while (lmap)
	{
		so_list.push_back(lmap->l_name);
		//std::cerr << "Add: " << lmap->l_name << '\n';

		lmap = lmap->l_next;
	}
	//std::cerr << "List completed" << '\n';

	::dlclose(handle);

	return so_list;
}


std::vector<std::string> acquire_libarcsdec_deps()
{
	ARCS_LOG_DEBUG << "Acquire runtime dependencies for libarcsdec";

	// Runtime deps from main executable

	auto so_list = runtime_deps("");

	// Runtime deps of libarcsdec

	auto libarcsdec_so = first_libname_match(so_list, LIBARCSDEC_NAME);

	if (libarcsdec_so.empty())
	{
		ARCS_LOG_WARNING <<
			"Could not retrieve any runtime dependencies from libarcsdec";

		return {}; // libarcsdec was not found
	}

	ARCS_LOG_DEBUG << "Inspect " << libarcsdec_so
		<< " for runtime dependencies";

	return runtime_deps(libarcsdec_so);
}


const std::vector<std::string>& libarcsdec_deps()
{
	static const std::vector<std::string> libarcsdec_deps =
		acquire_libarcsdec_deps();

	return libarcsdec_deps;
}


const std::string& libfile(const std::string& libname)
{
	return first_libname_match(libarcsdec_deps(), libname);
}

} // namespace details

} // namespace v_1_0_0
} // namespace arcsdec

