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

#ifndef __LIBARCSTK_METADATA_HPP__
#include <arcstk/metadata.hpp>    // for ToC, make_toc
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __LIBARCSDEC_LIBINSPECT_HPP__
#include "libinspect.hpp"   // for first_libname_match
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
#ifndef __LIBARCSDEC_CUESHEET_TOCHANDLER_HPP__
#include "cuesheet/tochandler.hpp"
#endif

namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{
namespace cuesheet
{

using arcstk::ToC;
using arcstk::make_toc;
//using arcstk::InvalidMetadataException;


std::unique_ptr<ToC> CuesheetParserImpl::do_parse(const std::string& filename)
{
	Driver driver;
											//
	ToCHandler handler;
	driver.set_handler(handler);

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
		//throw InvalidMetadataException(
		throw std::invalid_argument(
				std::string { "Faild to parse file " } + filename);
	}

	//return make_toc(handler.total_tracks(), handler.offsets(),
	//		handler.lengths(), handler.filenames());
	return make_toc(handler.offsets(), handler.filenames());
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
	//return { /* empty */ };
	return { { "-genuine-",
		details::first_libname_match(details::runtime_deps(""), LIBARCSDEC_NAME)
	} };
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

