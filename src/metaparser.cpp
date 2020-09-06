/**
 * \file
 *
 * \brief Implements API for implementing MetadataParsers.
 */


#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include "metaparser.hpp"
#endif

#include <limits>    // for numeric_limits
#include <memory>
#include <string>

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif


namespace arcsdec
{
inline namespace v_1_0_0
{

using arcstk::InvalidMetadataException;


// MetadataParserImpl


MetadataParserImpl::MetadataParserImpl() = default;


MetadataParserImpl::~MetadataParserImpl() noexcept = default;


std::unique_ptr<TOC> MetadataParserImpl::parse(const std::string &filename)
{
	return this->do_parse(filename);
}


std::unique_ptr<FileReaderDescriptor> MetadataParserImpl::descriptor() const
{
	return this->do_descriptor();
}


// MetadataParser


MetadataParser::MetadataParser(std::unique_ptr<MetadataParserImpl> impl)
	: impl_(std::move(impl))
{
	// empty
}


MetadataParser::MetadataParser(MetadataParser &&) noexcept = default;


MetadataParser& MetadataParser::operator = (MetadataParser &&) noexcept
= default;


std::unique_ptr<TOC> MetadataParser::parse(const std::string &filename)
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


MetadataParseException::MetadataParseException(const std::string &what_arg)
	: std::runtime_error(what_arg)
{
	// empty
}

} // namespace v_1_0_0
} // namespace arcsdec

