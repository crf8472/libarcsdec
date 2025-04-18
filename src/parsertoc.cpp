/**
 * \file
 *
 * \brief Implements libcdio-based parser for CDRDAO/TOC files.
 */

#ifndef __LIBARCSDEC_PARSERTOC_HPP__
#include "parsertoc.hpp"
#endif
#ifndef __LIBARCSDEC_PARSERTOC_DETAILS_HPP__
#include "parsertoc_details.hpp"
#endif

#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include "metaparser.hpp"  // for MetadataParseException
#endif
#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"   // for RegisterDescriptor
#endif

#ifndef __LIBARCSTK_METADATA_HPP__
#include <arcstk/metadata.hpp>    // for ToC, make_toc
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#include <cdio++/cdio.hpp> // libcdio

#include <cstdint>   // for uint64_t
#include <cstdio>    // for fopen, fclose, FILE
#include <iomanip>   // for setw
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
namespace cdrdao
{


// TODO Place implementation of TocParserImpl here
// cdio/device.h: cdio_open_cdrdao


std::unique_ptr<ToC> TocParserImpl::do_parse(const std::string& filename)
{
	// TODO Return a ToC
	return nullptr;
}


std::unique_ptr<FileReaderDescriptor> TocParserImpl::do_descriptor() const
{
	return std::make_unique<DescriptorToc>();
}


} // namespace cdrdao
} // namespace details


// DescriptorToc


DescriptorToc::~DescriptorToc() noexcept = default;


std::string DescriptorToc::do_id() const
{
	return "cdrdaotoc";
}


std::string DescriptorToc::do_name() const
{
	return "CDRDAO";
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
	return { libinfo_entry_filepath("libcdio") };
}


std::unique_ptr<FileReader> DescriptorToc::do_create_reader() const
{
	auto impl = std::make_unique<details::cdrdao::TocParserImpl>();
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

