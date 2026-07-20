/**
 * \file
 *
 * \brief Implements a parser for CDRDAO/TOC files.
 */

#ifndef LIBARCSDEC_PARSERTOC_HPP_
#include "parsertoc.hpp"
#endif
#ifndef LIBARCSDEC_PARSERTOC_DETAILS_HPP_
#include "parsertoc_details.hpp"  // for TocParserImpl
#endif

#include <memory>    // for unique_ptr
#include <set>       // for set
#include <string>    // for string
#include <utility>   // for move

#ifndef LIBARCSTK_METADATA_HPP_
#include <arcstk/metadata.hpp> // for ToC
#endif
#ifndef LIBARCSTK_LOGGING_HPP_
#include <arcstk/logging.hpp>
#endif

#ifndef LIBARCSDEC_DESCRIPTOR_HPP_
#include "descriptor.hpp"      // for Codec, Format
#endif
#ifndef LIBARCSDEC_CDRTOC_DRIVER_HPP_
#include "cdrtoc/driver.hpp"
#endif
#ifndef LIBARCSDEC_LIBINSPECT_HPP_
#include "libinspect.hpp"      // for first_libname_match
#endif
#ifndef LIBARCSDEC_METAPARSER_HPP_
#include "metaparser.hpp"      // for MetadataParseException
#endif
#ifndef LIBARCSDEC_SELECTION_HPP_
#include "selection.hpp"       // for RegisterDescriptor
#endif
#ifndef LIBARCSDEC_VERSION_HPP_
#include "version.hpp"         // for LIBARCSDEC_NAME
#endif


namespace arcsdec
{
inline namespace v_1_0_0
{
namespace read
{

// forward declarations
class FileReader;
class FileReaderDescriptor;

namespace details::cdrtoc
{

using arcstk::ToC;


ToC TocParserImpl::do_parse(const std::string& filename)
{
	auto l_handler = DefaultLexerHandler {};
	auto driver    = Driver { &l_handler, handler() };

	driver.parse(filename);

	return handler()->get_toc();
}


std::unique_ptr<FileReaderDescriptor> TocParserImpl::do_descriptor() const
{
	using arcsdec::read::DescriptorToc;
	return std::make_unique<DescriptorToc>();
}

} // namespace details::cdrtoc


// DescriptorToc


DescriptorToc::~DescriptorToc() noexcept = default;


std::string DescriptorToc::do_id() const noexcept
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
		details::first_libname_match(
				details::runtime_deps(""), LIBARCSDEC_NAME) } };
}


std::unique_ptr<FileReader> DescriptorToc::do_create_reader() const
{
	auto impl = std::make_unique<details::cdrtoc::TocParserImpl>();
	return std::make_unique<MetadataParser>(std::move(impl));
}


std::unique_ptr<FileReaderDescriptor> DescriptorToc::do_clone() const
{
	return std::make_unique<DescriptorToc>();
}

} // namespace read


// Add this descriptor to the metadata descriptor registry

namespace {

using arcsdec::select::RegisterDescriptor;
using arcsdec::read::DescriptorToc;

const auto d = RegisterDescriptor<DescriptorToc>{};

} // namespace

} // namespace v_1_0_0
} // namespace arcsdec

