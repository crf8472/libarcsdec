/**
 * \file
 *
 * \brief Implements a parser for CDRDAO/TOC files.
 */

#ifndef __LIBARCSDEC_PARSERTOC_HPP__
#include "parsertoc.hpp"
#endif
#ifndef __LIBARCSDEC_PARSERTOC_DETAILS_HPP__
#include "parsertoc_details.hpp"  // for TocParserImpl
#endif

#ifndef __LIBARCSDEC_CDRDAOTOC_DRIVER_HPP__
#include "cdrdaotoc/driver.hpp"
#endif
#ifndef __LIBARCSDEC_CDRDAOTOC_TOCHANDLER_HPP__
#include "cdrdaotoc/tochandler.hpp"
#endif
#ifndef __LIBARCSDEC_LIBINSPECT_HPP__
#include "libinspect.hpp"      // for first_libname_match
#endif
#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include "metaparser.hpp"      // for MetadataParseException
#endif
#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"       // for RegisterDescriptor
#endif

#ifndef __LIBARCSTK_METADATA_HPP__
#include <arcstk/metadata.hpp> // for ToC, make_toc
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#include <cstdint>   // for uint64_t
#include <iomanip>   // for setw
#include <fstream>   // for ifstream
#include <memory>    // for unique_ptr
#include <set>       // for set
#include <sstream>   // for ostringstream
#include <stdexcept> // for invalid_argument
#include <string>    // for string
#include <vector>    // for vector


namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{
namespace cdrdaotoc
{

using arcstk::ToC;
using arcstk::make_toc;


std::unique_ptr<ToC> TocParserImpl::do_parse(const std::string& filename)
{
	auto driver  = Driver{};
	auto handler = ToCHandler{};
	driver.set_handler(handler);

	driver.set_lexer_debug_level(1);
	driver.set_parser_debug_level(1);

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
	driver.set_input(file);

	if (driver.parse() != 0)
	{
		throw std::runtime_error(
				std::string { "Failed to parse file " } + filename);
	}

	return make_toc(handler.offsets(), handler.filenames());

	return nullptr;
}


std::unique_ptr<FileReaderDescriptor> TocParserImpl::do_descriptor() const
{
	return std::make_unique<DescriptorToc>();
}


} // namespace cdrdaotoc
} // namespace details


// DescriptorToc


DescriptorToc::~DescriptorToc() noexcept = default;


std::string DescriptorToc::do_id() const
{
	return "cdrtoc";
}


std::string DescriptorToc::do_name() const
{
	return "CDRDAO/TOC";
}


InputType DescriptorToc::do_input_type() const
{
	return InputType::TOC;
}


bool DescriptorToc::do_accepts_codec(Codec codec) const
{
	return codec == Codec::NONE;
}


std::set<Format> DescriptorToc::define_formats() const
{
	return { Format::CDRDAO };
}


LibInfo DescriptorToc::do_libraries() const
{
	return { { "-genuine-",
		details::first_libname_match(details::runtime_deps(""), LIBARCSDEC_NAME)
	} };
}


std::unique_ptr<FileReader> DescriptorToc::do_create_reader() const
{
	auto impl = std::make_unique<details::cdrdaotoc::TocParserImpl>();
	return std::make_unique<MetadataParser>(std::move(impl));
}


std::unique_ptr<FileReaderDescriptor> DescriptorToc::do_clone() const
{
	return std::make_unique<DescriptorToc>();
}


// Add this descriptor to the metadata descriptor registry

namespace {

const auto d = RegisterDescriptor<DescriptorToc>{};

} // namespace

} // namespace v_1_0_0
} // namespace arcsdec

