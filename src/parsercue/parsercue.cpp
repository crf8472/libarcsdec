/**
 * \file
 *
 * \brief Implements a parser for CueSheets.
 */

#ifndef LIBARCSDEC_PARSERCUE_HPP_
#include "parsercue.hpp"
#endif
#ifndef LIBARCSDEC_PARSERCUE_DETAILS_HPP_
#include "parsercue_details.hpp"  // for CuesheetParserImpl
#endif

#include <memory>    // for unique_ptr
#include <set>       // for set
#include <string>    // for string

#ifndef LIBARCSTK_METADATA_HPP_
#include <arcstk/metadata.hpp> // for ToC, make_toc
#endif
#ifndef LIBARCSTK_LOGGING_HPP_
#include <arcstk/logging.hpp>
#endif

#ifndef LIBARCSDEC_CUESHEET_DRIVER_HPP_
#include "cuesheet/driver.hpp" // for Driver
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
namespace details
{
namespace cuesheet
{

using arcstk::ToC;
using arcstk::make_toc;


std::unique_ptr<ToC> CuesheetParserImpl::do_parse(const std::string& filename)
{
	auto p_handler = ParserToCHandler{};

	{
		auto l_handler = DefaultLexerHandler{} ;
		auto driver    = Driver { &l_handler, &p_handler };

#ifdef YYDEBUG
		const auto lexer_level  = 1;
		ARCS_LOG_DEBUG << "Set lexer debug level: " << lexer_level;

		const auto parser_level = 1;
		ARCS_LOG_DEBUG << "Set parser debug level: " << parser_level;
#else
		const auto lexer_level  = 0;
		ARCS_LOG_DEBUG << "Lexer debug info is deactivated";

		const auto parser_level = 0;
		ARCS_LOG_DEBUG << "Parser debug info is deactivated";
#endif

		driver.set_lexer_debug_level(lexer_level);
		driver.set_parser_debug_level(parser_level);

		driver.parse(filename);
	}

	return p_handler.get_toc();
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

} // namespace read

// Add this descriptor to the metadata descriptor registry

namespace {

using arcsdec::select::RegisterDescriptor;
using arcsdec::read::DescriptorCuesheet;

const auto d = RegisterDescriptor<DescriptorCuesheet>{};

} // namespace

} // namespace v_1_0_0
} // namespace arcsdec

