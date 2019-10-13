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

#ifndef __LIBARCSDEC_DESCRIPTORS_METADATA_HPP__
#include "descriptors_metadata.hpp"
#endif


namespace arcsdec
{

inline namespace v_1_0_0
{


/// \cond UNDOC_FUNCTION_BODIES


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


// MetadataParserSelection


MetadataParserSelection::MetadataParserSelection()
{
	// Provide tests

	std::unique_ptr<FileTestSuffix> test =
		std::make_unique<FileTestSuffix>();

	this->register_test(std::move(test));


	// Provide FileReaderDescriptors

	// The constructor of MetadataParserSelection automagically introduces the
	// knowledge about what formats are available. This knowledge is
	// provided by the instance FileReaderDescriptorsAudio that is populated at
	// buildtime based on the configuration of the build system.

	FileReaderDescriptorsMetadata compiled_supported_metadata_formats;

	// We move all the actual formats to not access FileReaderDescriptorsMetadata
	// beyond this particular block

	for (auto& f : compiled_supported_metadata_formats)
	{
		this->add_descriptor(std::move(f));
	}
}


MetadataParserSelection::~MetadataParserSelection() noexcept = default;


std::unique_ptr<MetadataParser> MetadataParserSelection::for_file(
	const std::string &filename) const
{
	return this->safe_cast(std::move(
				FileReaderSelection::for_file(filename)));
}


std::unique_ptr<MetadataParser> MetadataParserSelection::by_name(
	const std::string &name) const
{
	return this->safe_cast(std::move(FileReaderSelection::by_name(name)));
}


std::unique_ptr<MetadataParser> MetadataParserSelection::safe_cast(
		std::unique_ptr<FileReader> file_reader_uptr) const
{
	if (not file_reader_uptr)
	{
		return nullptr;
	}

	// Create MetadataParser manually by (safe) downcasting and reassignment

	auto metaparser_uptr = std::make_unique<MetadataParser>(nullptr);

	FileReader* file_reader_rptr = nullptr;
	file_reader_rptr = file_reader_uptr.release();
	// This is definitively NOT nice since file_reader_rptr is now an
	// owning raw pointer. We will fix that with the following reset() to
	// a unique_ptr. If thereby something goes wrong, we immediately destroy
	// the raw pointer accurately.

	try
	{
		metaparser_uptr.reset(dynamic_cast<MetadataParser*>(file_reader_rptr));

		// Creation is correct iff the FileReader created is in fact a
		// MetadataParser. If not, the file is not a supported audio file, so
		// bail out.

	} catch (...) // std::bad_cast is possible, but we play it safe
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

/// \endcond

} // namespace v_1_0_0

} // namespace arcsdec

