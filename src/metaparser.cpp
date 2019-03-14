/**
 * \file metaparser.cpp Implements interface for parsing TOC informations
 *
 */


#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include "metaparser.hpp"
#endif

#include <memory>
#include <string>

#ifndef __LIBARCSDEC_METAFORMATS_HPP__
#include "metaformats.hpp"
#endif
#ifndef __LIBARCS_LOGGING_HPP__
#include <arcs/logging.hpp>
#endif


namespace arcs
{

// MetadataParserImpl


MetadataParserImpl::~MetadataParserImpl() noexcept = default;


std::unique_ptr<TOC> MetadataParserImpl::parse(const std::string &filename)
{
	return this->do_parse(filename);
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
	return impl_->parse(filename);
}


// MetadataParseException


MetadataParseException::MetadataParseException(const std::string &what_arg)
	: std::runtime_error(what_arg)
{
	// empty
}


// MetadataParserCreator


MetadataParserCreator::MetadataParserCreator()
{
	// Provide tests

	std::unique_ptr<FileFormatTestSuffix> test =
		std::make_unique<FileFormatTestSuffix>();

	this->register_test(std::move(test));


	// Provide FileFormats

	// The constructor of AudioReaderCreator automagically introduces the
	// knowledge about what formats are available. This knowledge is
	// provided by the instance FileFormatsAudio that is populated at
	// buildtime based on the configuration of the build system.

	FileFormatsMetadata compiled_supported_metadata_formats;

	// We move all the actual formats to not access FileFormatsMetadata
	// beyond this particular block

	for (auto& f : compiled_supported_metadata_formats)
	{
		this->register_format(std::move(f));
	}
}


MetadataParserCreator::~MetadataParserCreator() noexcept = default;


std::unique_ptr<MetadataParser> MetadataParserCreator::create_metadata_parser(
	const std::string &filename)
{
	FileReader* file_reader_rptr = nullptr;

	// Create FileReader and release pointee

	auto file_reader_uptr = FileReaderCreator::create_reader(filename);

	if (not file_reader_uptr)
	{
		ARCS_LOG_ERROR << "FileReader could not be created";
		return nullptr;
	}

	auto metaparser_uptr = std::make_unique<MetadataParser>(nullptr);

	file_reader_rptr = file_reader_uptr.release();

	// Create AudioReader manually by (safe) downcasting and reassignment

	try
	{
		metaparser_uptr.reset(dynamic_cast<MetadataParser*>(file_reader_rptr));

		// Creation is correct iff the FileReader created is in fact a
		// MetadataParser. If not, the file is not a supported audio file, so
		// bail out.

	} catch (const std::bad_cast& e)
	{
		if (file_reader_rptr)
		{
			delete file_reader_rptr;
		}

		ARCS_LOG_ERROR <<
				"FileReader created, but failed to turn it to a MetadataParser";

		return nullptr;
	}

	return metaparser_uptr;
}

} // namespace arcs

