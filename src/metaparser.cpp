/**
 * \internal
 *
 * \file
 *
 * \brief Implements symbols from metaparser.hpp.
 */


#ifndef LIBARCSDEC_METAPARSER_HPP_
#include "metaparser.hpp"
#endif
#ifndef LIBARCSDEC_METAPARSER_DETAILS_HPP_
#include "metaparser_details.hpp"
#endif

#include <cstdint>      // for int64_t
#include <memory>       // for unique_ptr
#include <stdexcept>    // for runtime_error
#include <string>       // for string
#include <utility>      // for move

#ifndef LIBARCSTK_LOGGING_HPP_
#include <arcstk/logging.hpp>  // for ARCS_LOG_DEBUG
#endif


namespace arcsdec
{
inline namespace v_1_0_0
{
namespace read
{

// forward declarations
class FileReaderDescriptor;


// MetadataParserImpl


MetadataParserImpl::MetadataParserImpl() = default;


MetadataParserImpl::~MetadataParserImpl() noexcept = default;


ToC MetadataParserImpl::parse(const std::string& filename)
{
	return this->do_parse(filename);
}


std::unique_ptr<FileReaderDescriptor> MetadataParserImpl::descriptor() const
{
	return this->do_descriptor();
}


// MetadataParser


MetadataParser::MetadataParser(std::unique_ptr<MetadataParserImpl> impl)
	: impl_ { std::move(impl) }
{
	// empty
}


MetadataParser::MetadataParser(MetadataParser&&) noexcept = default;


MetadataParser& MetadataParser::operator = (MetadataParser&&) noexcept
= default;


ToC MetadataParser::parse(const std::string& filename)
{
	ARCS_LOG_DEBUG << "Try to read metadata file '" << filename << "'";

	auto toc = impl_->parse(filename);

	ARCS_LOG_DEBUG << "Metadata file '" << filename << "' successfully read";

	return toc;
}


std::unique_ptr<FileReaderDescriptor> MetadataParser::do_descriptor() const
{
	return impl_->descriptor();
}


// MetadataParseException


MetadataParseException::MetadataParseException(const std::string& what_arg)
	: std::runtime_error { what_arg }
{
	// empty
}


namespace details
{

int64_t msf_to_frames(const int m, const int s, const int f)
{
	if (m < 0 || m > 99 || s < 0 || s >= 60 || f < 0 || f >= 75) {
		return -1;
	}

	return (m * 60 + s) * 75 + f;
}


void frames_to_msf(int64_t frames, int64_t* m, int64_t* s, int64_t* f)
{
	*f = frames % 75;
	frames /= 75;
	*s = frames % 60;
	frames /= 60;
	*m = frames;
}

} // namespace details

} // namespace read
} // namespace v_1_0_0
} // namespace arcsdec

