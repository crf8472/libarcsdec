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

#include <cstdint>      // for int64_t, uintmax_t
#include <filesystem>   // for file_size
#include <fstream>      // for ifstream
#include <limits>       // for numeric_limits
#include <memory>       // for unique_ptr
#include <stdexcept>    // for runtime_error
#include <string>       // for string
#include <system_error> // for error_code
#include <utility>      // for move

#ifndef LIBARCSTK_LOGGING_HPP_
#include <arcstk/logging.hpp>  // for ARCS_LOG_DEBUG
#endif

#ifndef LIBARCSDEC_TOCHANDLER_HPP_
#include "tochandler.hpp"      // for ParserToCHandler
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


void MetadataParserImpl::set_handler(ParserToCHandler* handler)
{
	handler_ = handler;
}


ParserToCHandler* MetadataParserImpl::handler() const
{
	return handler_;
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


void MetadataParser::set_handler(ParserToCHandler* handler)
{
	impl_->set_handler(handler);
}


ParserToCHandler* MetadataParser::handler() const
{
	return impl_->handler();
}


// MetadataParseException


MetadataParseException::MetadataParseException(const std::string& what_arg)
	: std::runtime_error { what_arg }
{
	// empty
}


namespace details
{


std::uintmax_t file_size_or_throw(const std::string &filepath)
{
	namespace fs = std::filesystem;

	// Check existence

	if (!fs::exists(filepath))
	{
        throw std::runtime_error("File not found");
    }

	// Check file size

	std::error_code rc;
    const auto file_size { fs::file_size(filepath, rc) };

	if (rc)
	{
		auto msg = std::ostringstream{};

		msg << "Unable to determine file size for file '"
			<< filepath
			<< "'. Original error message: '" << rc.message() << "'";

		throw std::runtime_error(msg.str());
	}

	return file_size;
}


std::optional<std::string> file_content(const std::string &filepath,
		const std::uintmax_t max_size)
{
	// Get file size

	auto file_size = file_size_or_throw(filepath);

	if (file_size == 0)
	{
		return std::nullopt;
	}

	if (file_size > max_size)
	{
		auto msg = std::ostringstream{};

		msg << "File too large, more than maximum of "
			<< max_size
			<< " bytes";

		throw std::runtime_error(msg.str());
	}

	// Check before casting to signed type when passing it to ifstream::read()
	// Note that this also covers the necessary check for:
	// if (file_size == std::numeric_limits<std::uintmax_t>::max()) throw;
	if (file_size > static_cast<std::uintmax_t>(
				std::numeric_limits<std::streamsize>::max()))
	{
		throw std::runtime_error(
				"File too large, is not readable in a single read operation");
	}

	// Open file

    auto input = std::ifstream { filepath };

    if (!input)
	{
		auto msg = std::ostringstream{};

		msg << "Unable to correctly open file '"
			<< filepath
			<< "'";

		throw std::runtime_error(msg.str());
    }

	input.exceptions(std::ios::failbit | std::ios::badbit);

	// Load file content into string

	std::string content (file_size, '\0'); // parentheses
	input.read(content.data(), static_cast<std::streamsize>(file_size));

    return content;
}


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

