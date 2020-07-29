/**
 * \file metaparser.cpp Implements interface for parsing TOC informations
 *
 */


#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include "metaparser.hpp"
#endif

#include <memory>
#include <string>

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif


namespace arcsdec
{

inline namespace v_1_0_0
{


// MetadataParserImpl


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


MetadataParser::~MetadataParser() noexcept = default;


std::unique_ptr<TOC> MetadataParser::parse(const std::string &filename)
{
	ARCS_LOG_DEBUG << "Parse metadata file '" << filename << "'";

	return impl_->parse(filename);
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

