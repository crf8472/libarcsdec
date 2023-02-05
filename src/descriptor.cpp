/**
 * \file
 *
 * \brief Implementation of a descriptor for FileReaders
 */

#ifndef __LIBARCSDEC_DESCRIPTOR_HPP__
#include "descriptor.hpp"
#endif

#include <algorithm>    // for find_if
#include <array>        // for array
#include <cstdint>      // for uint32_t, uint64_t, int64_t
#include <fstream>      // for ifstream
#include <memory>       // for unique_ptr
#include <string>       // for string, to_string
#include <type_traits>  // for underlying_type_t
#include <vector>       // for vector

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp> // for ARCS_LOG, _WARNING, _DEBUG
#endif

#ifndef __LIBARCSDEC_LIBINSPECT_HPP__
#include "libinspect.hpp"   // for libfile
#endif

namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{

std::vector<unsigned char> read_bytes(const std::string &filename,
	const uint32_t &offset, const uint32_t &length)
{
	// Read a specified number of bytes from a file offset

	std::vector<unsigned char> bytes(length);
	const auto byte_size = sizeof(bytes[0]);

	std::ifstream in;

	// Do not consume new lines in binary mode
	in.unsetf(std::ios::skipws);

	std::ios_base::iostate exception_mask = in.exceptions()
		| std::ios::failbit | std::ios::badbit | std::ios::eofbit;

	in.exceptions(exception_mask);

	try
	{
		ARCS_LOG(DEBUG1) << "Open file: " << filename;

		in.open(filename, std::ifstream::in | std::ifstream::binary);
	}
	catch (const std::ios_base::failure& f)
	{
		auto msg = std::string { "Failed to open file: " };
		msg += filename;

		throw FileReadException(msg, 0);
	}

	try
	{
		in.ignore(offset);

		in.read(reinterpret_cast<char*>(&bytes[0]), length * byte_size);
	}
	catch (const std::ios_base::failure& f)
	{
		int64_t total_bytes_read = 1 + in.gcount();

		in.close();

		if (in.bad())
		{
			auto msg = std::string { "Failed while reading file: " };
			msg += filename;
			throw FileReadException(msg, total_bytes_read);
		} else if (in.eof())
		{
			auto msg = std::string { "Unexpected end while reading file: " };
			msg += filename;
			throw FileReadException(msg, total_bytes_read);
		} else
		{
			auto msg = std::string { "Content failure on file: " };
			msg += filename;
			msg += ", message: ";
			msg += f.what();
			msg += ", read ";
			msg += total_bytes_read;
			msg += " bytes";

			throw InputFormatException(msg);
		}
	}

	return bytes;
}


std::string get_suffix(const std::string &filename, const std::string &delim)
{
	// TODO Use std::filesystem of C++17

	if (filename.empty()) { return filename; }

	auto pos = filename.find_last_of(delim);

	if (pos == std::string::npos) { return filename; }

	return pos < filename.length()
			? filename.substr(pos + 1, filename.length())
			: std::to_string(filename.back());
}

} // namespace details


LibInfoEntry libinfo_entry(const std::string &libname)
{
	return { libname, details::libfile(libname) };
}


std::string name(Format format)
{
	static const std::array<std::string, 12> names =
	{
		"Unknown",
		"CUE",
		"cdrdao",
		// ... add more metadata formats here
		"wav", // Audio formats from here on
		"fLaC",
		"APE",
		"CAF",
		"M4A",
		"OGG",
		"WV", // TODO Should we also read WVC?
		"AIFF",
		"WMA" // TODO Implement and test this
		// ... add more audio formats here
	};

	return names[std::underlying_type_t<Format>(format)];
}


std::string name(Codec codec)
{
	static const std::array<std::string, 14> names =
	{
		"Unknown",
		"PCM_S16BE",
		"PCM_S16BE_PLANAR",
		"PCM_S16LE",
		"PCM_S16LE_PLANAR",
		"PCM_S32BE",
		"PCM_S32BE_PLANAR",
		"PCM_S32LE",
		"PCM_S32LE_PLANAR",
		"FLAC",
		"WAVEPACK",
		"MONKEY",
		"ALAC",
		"WMALOSSLESS" // TODO Implement and test this
	};

	return names[std::underlying_type_t<Codec>(codec)];
}


bool is_audio_format(Format format)
{
	return format >= Format::WAV;
}


// FileReader


FileReader::FileReader() = default;


FileReader::~FileReader() noexcept = default;


std::unique_ptr<FileReaderDescriptor> FileReader::descriptor() const
{
	return this->do_descriptor();
}


// FileReadException


FileReadException::FileReadException(const std::string &what_arg)
	: std::runtime_error { what_arg }
	, byte_pos_ { -1 }
{
	// empty
}


FileReadException::FileReadException(const std::string &what_arg,
		const int64_t &byte_pos)
	: std::runtime_error { what_arg }
	, byte_pos_ { byte_pos }
{
	// empty
}


int64_t FileReadException::byte_pos() const
{
	return byte_pos_;
}


// FileReaderDescriptor


FileReaderDescriptor::FileReaderDescriptor()
		: suffices_ { }
{
	// empty
}


FileReaderDescriptor::~FileReaderDescriptor() noexcept = default;


std::string FileReaderDescriptor::id() const
{
	return this->do_id();
}


std::string FileReaderDescriptor::name() const
{
	return this->do_name();
}


bool FileReaderDescriptor::accepts_bytes(
		const std::vector<unsigned char> &bytes, const uint64_t &offset) const
{
	return this->do_accepts_bytes(bytes, offset);
}


bool FileReaderDescriptor::accepts_name(const std::string &filename) const
{
	return this->do_accepts_name(filename);
}


bool FileReaderDescriptor::accepts(Format format) const
{
	return this->do_accepts_format(format);
}


std::set<Format> FileReaderDescriptor::formats() const
{
	return this->do_formats();
}


bool FileReaderDescriptor::accepts(Codec codec) const
{
	return this->do_accepts_codec(codec);
}


std::set<Codec> FileReaderDescriptor::codecs() const
{
	return this->do_codecs();
}


LibInfo FileReaderDescriptor::libraries() const
{
	return this->do_libraries();
}


std::unique_ptr<FileReader> FileReaderDescriptor::create_reader() const
{
	return this->do_create_reader();
}


std::unique_ptr<FileReaderDescriptor> FileReaderDescriptor::clone() const
{
	return this->do_clone();
}


bool FileReaderDescriptor::do_accepts_name(const std::string &filename) const
{
	auto fname_suffix = details::get_suffix(filename, ".");

	if (fname_suffix.empty()) { return false; }

	if (fname_suffix.length() == filename.length()) { return false; }
	// XXX Shouldn't this be TRUE?

	auto rc = std::find_if(suffices_.begin(), suffices_.end(),
			[fname_suffix](const decltype( suffices_ )::value_type &suffix)
			{
				// Perform case insensitive comparison
				if (suffix == details::ci_string { fname_suffix.c_str() })
				{
					ARCS_LOG(DEBUG1) << "Suffix '" << fname_suffix
						<< "' accepted" ;
					return true;
				}
				return false;
			});

	return rc != suffices_.end();
}


bool FileReaderDescriptor::do_accepts_format(Format format) const
{
	const auto format_set = formats();
	return format_set.find(format) != format_set.end();
}


std::set<Format> FileReaderDescriptor::do_formats() const
{
	static const auto formats = define_formats();
	return formats;
}


std::set<Format> FileReaderDescriptor::define_formats() const
{
	return { /* empty */ };
}


bool FileReaderDescriptor::do_accepts_codec(Codec codec) const
{
	const auto codec_set = codecs();
	return codec_set.find(codec) != codec_set.end();
}


std::set<Codec> FileReaderDescriptor::do_codecs() const
{
	static const auto codecs = define_codecs();
	return codecs;
}


std::set<Codec> FileReaderDescriptor::define_codecs() const
{
	return { /* empty */ };
}


bool operator == (const FileReaderDescriptor &lhs,
			const FileReaderDescriptor &rhs)
{
	// FileReaderDescriptors are stateless and hence equal iff they are of the
	// same static type

	return typeid(lhs) == typeid(rhs);
}

} // namespace v_1_0_0
} // namespace arcsdec

