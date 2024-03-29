/**
 * \file
 *
 * \brief Implements a parser for CueSheets.
 */

#ifndef __LIBARCSDEC_PARSERCUE_HPP__
#include "parsercue.hpp"
#endif
#ifndef __LIBARCSDEC_PARSERCUE_DETAILS_HPP__
#include "parsercue_details.hpp"  // for CuesheetParserImpl
#endif

#include <cstdio>    // for fopen, fclose, FILE
#include <iomanip>   // for setw
#include <fstream>
#include <memory>    // for unique_ptr
#include <set>       // for set
#include <sstream>   // for ostringstream
#include <stdexcept> // for invalid_argument
#include <string>    // for string
#include <vector>    // for vector

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>  // for TOC, make_toc, InvalidMetadataException
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include "metaparser.hpp"  // for MetadataParseException
#endif
#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"   // for RegisterDescriptor
#endif

#ifndef __LIBARCSDEC_CUESHEET_DRIVER_HPP__
#include "cuesheet/driver.hpp"
#endif

namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{
namespace cuesheet
{

using arcstk::TOC;
using arcstk::make_toc;
using arcstk::InvalidMetadataException;


std::unique_ptr<TOC> CuesheetParserImpl::do_parse(const std::string &filename)
{
	Driver driver;

	std::ifstream file;
	file.open(filename, std::ifstream::in);
	driver.set_input(file);

	const int res { driver.parse() };

	return nullptr; // TODO
}

std::unique_ptr<FileReaderDescriptor> CuesheetParserImpl::do_descriptor() const
{
	return std::make_unique<DescriptorCuesheet>();
}

} // namespace cuesheet
} // namespace details


// DescriptorCuesheet


DescriptorCuesheet::~DescriptorCuesheet() noexcept = default;


std::string DescriptorCuesheet::do_id() const
{
	return "cuesheet";
}


std::string DescriptorCuesheet::do_name() const
{
	return "CueSheet";
}


InputType DescriptorCuesheet::do_input_type() const
{
	return InputType::TOC;
}


bool DescriptorCuesheet::do_accepts_codec(Codec codec) const
{
	ARCS_LOG(DEBUG1) << "Is Codec NONE?";
	return codec == Codec::NONE;
}


std::set<Format> DescriptorCuesheet::define_formats() const
{
	return { Format::CUE };
}


LibInfo DescriptorCuesheet::do_libraries() const
{
	return { /* empty */ };
}


std::unique_ptr<FileReader> DescriptorCuesheet::do_create_reader() const
{
	auto impl = std::make_unique<details::cuesheet::CuesheetParserImpl>();
	return std::make_unique<MetadataParser>(std::move(impl));
}


std::unique_ptr<FileReaderDescriptor> DescriptorCuesheet::do_clone() const
{
	return std::make_unique<DescriptorCuesheet>();
}


// Add this descriptor to the metadata descriptor registry

namespace {

const auto d = RegisterDescriptor<DescriptorCuesheet>{};

} // namespace

} // namespace v_1_0_0
} // namespace arcsdec

