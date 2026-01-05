/**
 * \internal
 *
 * \file
 *
 * \brief Implements symbols from metaparser.hpp.
 */


#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include "metaparser.hpp"
#endif

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>  // for ARCS_LOG_DEBUG
#endif

#include <memory>   // for unique_ptr
#include <string>   // for string
#include <utility>  // for move


namespace arcsdec
{
inline namespace v_1_0_0
{

// MetadataParserImpl


MetadataParserImpl::MetadataParserImpl() = default;


MetadataParserImpl::~MetadataParserImpl() noexcept = default;


std::unique_ptr<ToC> MetadataParserImpl::parse(const std::string& filename)
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


std::unique_ptr<ToC> MetadataParser::parse(const std::string& filename)
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

long msf_to_frames(const int m, const int s, const int f)
{
	if (m < 0 || m > 99 || s < 0 || s >= 60 || f < 0 || f >= 75) {
		return -1;
	}

	return (m * 60 + s) * 75 + f;
}


void frames_to_msf(long frames, int* m, int* s, int* f)
{
	*f = frames % 75;
	frames /= 75;
	*s = frames % 60;
	frames /= 60;
	*m = frames;
}

} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

