/**
 * \file
 *
 * \brief Implementation of a descriptor for FileReaders
 */

#ifndef __LIBARCSDEC_DESCRIPTOR_HPP__
#include "descriptor.hpp"
#endif

#include <algorithm>    // for find, find_if, mismatch
#include <array>        // for array
#include <cstdint>      // for uint32_t, uint64_t, int64_t
#include <fstream>      // for ifstream
#include <ios>          // for ios, ios_base
#include <memory>       // for unique_ptr
#include <set>          // for set
#include <stdexcept>    // for runtime_error
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


std::string name(Format format)
{
	static const std::array<std::string, 11> names =
	{
		"unknown",
		"cue",
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
		// ... add more audio formats here
	};

	return names[std::underlying_type_t<Format>(format)];
}


std::string name(Codec codec)
{
	static const std::array<std::string, 14> names =
	{
		"unknown",
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
		"none" // Allows combination with a non-audio format
	};

	return names[std::underlying_type_t<Codec>(codec)];
}


// Bytes


bool operator == (const Bytes &lhs, const Bytes &rhs)
{
	return lhs.offset_ == rhs.offset_ && lhs.seq_ == rhs.seq_;
}


constexpr unsigned char Bytes::any;


Bytes::Bytes()
	: offset_ { 0 }
	, seq_    { /* empty */ }
{
	// empty
}


Bytes::Bytes(const uint32_t offset, const ByteSequence &bytes)
	: offset_ { offset }
	, seq_    { bytes }
{
	// empty
}


bool Bytes::match(const ByteSequence &bytes, const uint32_t &ioffset) const
{
	// No test bytes? => accept
	if (bytes.empty())
	{
		ARCS_LOG(DEBUG1) << "Test bytes empty, match";
		return true;
	}

	if (ref_bytes().empty())
	{
		ARCS_LOG(DEBUG1) << "Empty byte sequence, match";
		return true;
	}

	// Test Bytes Beyond Reference Part? => fail
	if (ioffset >= ref_bytes().size())
	{
		ARCS_LOG(DEBUG1) <<
			"Test bytes rejected since they are beyond reference";
		return false;
	}

	// determine start pointers

	auto in_current  = bytes.begin();
	auto ref_current = ref_bytes().begin();

	if (ioffset > offset())
	{
		ref_current += ioffset - offset();
	} else
	{
		in_current += offset() - ioffset;
	}

	// determine end pointers

	const auto ref_size     = ref_bytes().size() - ioffset;
	const bool longer_input = bytes.size() > ref_size;

	const auto in_stop  = longer_input
		? bytes.begin() + static_cast<long>(ref_size) + 1 /* past-the-end */
		: bytes.end();

	const auto ref_stop = longer_input
		? ref_bytes().end()
		: ref_current + static_cast<long>(bytes.size()) + 1 /* past-the-end */;

	do
	{
		const auto m = std::mismatch(in_current, in_stop, ref_current);

		// Reached the end? => Success
		if (m.first == in_stop or m.second == ref_stop)
		{
			break;
		}

		// Is it an actual mismatch (i.e. on a non-wildcard byte)?
		if (m.second != ref_stop and *m.second != wildcard())
		{
			return false;
		}

		// Mismatch was on a "wildcard byte", so skip all following bytes until
		// the wildcard sequence ends.

		in_current  = m.first;
		ref_current = m.second;

		// Skip all input bytes referring to 'any_byte' in the reference bytes
		while (in_current != in_stop and ref_current != ref_stop
				and *ref_current == wildcard())
		{
			++in_current;
			++ref_current;
		}

	} while (in_current != in_stop and ref_current != ref_stop);

	return true;
}


bool Bytes::match(const ByteSequence &bytes) const
{
	return this->match(bytes, 0);
}


uint32_t Bytes::offset() const
{
	return offset_;
}


ByteSequence Bytes::sequence() const
{
	return seq_;
}


ByteSequence::size_type Bytes::size() const
{
	return seq_.size();
}


ByteSequence::const_reference Bytes::operator[](ByteSequence::size_type i) const
{
	return seq_[i];
}


unsigned char Bytes::wildcard() const
{
	return Bytes::any;
}


const ByteSequence& Bytes::ref_bytes() const
{
	return seq_;
}


namespace details
{


bool ci_match_suffix(const SuffixSet &suffices, const std::string &filename)
{
	const auto fname_suffix = details::get_suffix(filename, ".");

	if (fname_suffix.empty()) { return false; }
	if (fname_suffix.length() == filename.length()) { return true; }

	const auto ref_suffix = details::ci_string { fname_suffix.c_str() };
	auto result = std::find_if(suffices.begin(), suffices.end(),
			[ref_suffix](const SuffixSet::value_type &suffix)
			{
				return suffix == ref_suffix; // case insensitive comparison
			});

	return result != suffices.end();
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


Bytes read_bytes(const std::string &filename,
	const uint32_t &offset, const uint32_t &length)
{
	// Read a specified number of bytes from a file offset

	ByteSequence bytes(length);
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

	ARCS_LOG(DEBUG1) << "File successfully opened";

	try
	{
		ARCS_LOG(DEBUG1) << "Read " << length
			<< " bytes from offset " << offset;

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

	ARCS_LOG(DEBUG1) << "Successfully read bytes from file";

	return { offset, bytes };
}

} // namespace details


// Descriptor


Descriptor::~Descriptor() noexcept = default;


std::string Descriptor::name() const
{
	return do_name();
}


bool Descriptor::matches(const ByteSequence &bytes, const uint64_t &offset)
	const
{
	return do_matches(bytes, offset);
}


bool Descriptor::matches(const Bytes &bytes) const
{
	return do_matches(bytes.sequence(), bytes.offset());
}


bool Descriptor::matches(const std::string &filename) const
{
	return do_matches(filename);
}


Format Descriptor::format() const
{
	return do_format();
}


std::set<Codec> Descriptor::codecs() const
{
	return do_codecs();
}


Bytes Descriptor::reference_bytes() const
{
	return do_reference_bytes();
}


std::unique_ptr<Descriptor> Descriptor::clone() const
{
	return do_clone();
}


// FileReader


FileReader::FileReader() = default;


FileReader::~FileReader() noexcept = default;


std::unique_ptr<FileReaderDescriptor> FileReader::descriptor() const
{
	return this->do_descriptor();
}


// InputFormatException


InputFormatException::InputFormatException(const std::string &what_arg)
	: std::runtime_error(what_arg)
{
	/* empty */
};


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


// libinfo_entry


LibInfoEntry libinfo_entry(const std::string &libname)
{
	return { libname, details::libfile(libname) };
}


// FileReaderDescriptor


FileReaderDescriptor::~FileReaderDescriptor() noexcept = default;


std::string FileReaderDescriptor::id() const
{
	return this->do_id();
}


std::string FileReaderDescriptor::name() const
{
	return this->do_name();
}


bool FileReaderDescriptor::accepts(const Format f) const
{
	return this->do_accepts_format(f);
}


bool FileReaderDescriptor::accepts(const Codec c) const
{
	return this->do_accepts_codec(c);
}


bool FileReaderDescriptor::accepts(const Format f, const Codec c) const
{
	return this->do_accepts_format_and_codec(f, c);
}


const std::set<Format> FileReaderDescriptor::formats() const
{
	return do_formats();
}


const std::set<Codec> FileReaderDescriptor::codecs() const
{
	return do_codecs();
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


bool FileReaderDescriptor::do_accepts_format(const Format f) const
{
	const auto fs = formats();
	return std::find(fs.begin(), fs.end(), f) != fs.end();
}


bool FileReaderDescriptor::do_accepts_codec(const Codec c) const
{
	const auto cs = codecs();
	return std::find(cs.begin(), cs.end(), c) != cs.end();
}


bool FileReaderDescriptor::do_accepts_format_and_codec(const Format f,
		const Codec c) const
{
	return accepts(f) && accepts(c);
}


const std::set<Format> FileReaderDescriptor::do_formats() const
{
	return define_formats();
}


const std::set<Codec> FileReaderDescriptor::do_codecs() const
{
	return define_codecs();
}


std::set<Format> FileReaderDescriptor::define_formats() const
{
	return {};
}


std::set<Codec> FileReaderDescriptor::define_codecs() const
{
	return {};
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

