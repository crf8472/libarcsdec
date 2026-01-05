/**
 * \internal
 *
 * \file
 *
 * \brief Implements symbols from descriptor.hpp.
 */

#ifndef __LIBARCSDEC_DESCRIPTOR_HPP__
#include "descriptor.hpp"
#endif

#ifndef __LIBARCSDEC_LIBINSPECT_HPP__
#include "libinspect.hpp"     // for libfile
#endif

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp> // for ARCS_LOG, _WARNING, _DEBUG
#endif

#include <algorithm>    // for find, find_if, mismatch
#include <array>        // for array
#include <cstdint>      // for uint32_t, uint64_t, int64_t
#include <fstream>      // for ifstream
#include <initializer_list>
#include <ios>          // for ios, ios_base
#include <iterator>     // for distance
#include <memory>       // for unique_ptr
#include <set>          // for set
#include <stdexcept>    // for runtime_error
#include <string>       // for string, to_string
#include <type_traits>  // for underlying_type_t
#include <vector>       // for vector


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
		"PCM_S16BE(planar)",
		"PCM_S16LE",
		"PCM_S16LE(planar)",
		"PCM_S32BE",
		"PCM_S32BE(planar)",
		"PCM_S32LE",
		"PCM_S32LE(planar)",
		"fLaC",
		"Wavepack",
		"Monkey",
		"ALAC",
		"none" // Allows combination with a non-audio format
	};

	return names[std::underlying_type_t<Codec>(codec)];
}


// ByteSeq


ByteSeq::ByteSeq(ByteSeq::sequence_type::size_type length)
	: sequence_(length)
	, wildcards_ { /* empty */ }
{
	// empty
}


ByteSeq::ByteSeq(std::initializer_list<unsigned> values)
	: sequence_  { /* empty */ }
	, wildcards_ { /* empty */ }
{
	if (values.size() == 0) // TODO empty() only since C++17
	{
		return;
	}

	auto pos = unsigned { 0 };
	sequence_.reserve(values.size());

	// Initialize sequence_ while collecting positions with wildcards
	std::transform(values.begin(), values.end(),
			std::back_inserter(sequence_),
			[&](const unsigned& v) -> byte_type
			{
				if (v > max_byte_value)
				{
					// Everything bigger than max_byte_value will count
					// as undefined resp. wildcard
					wildcards_.insert(pos);
				}
				++pos;

				// Actual value on undefined positions will be max_byte_value
				return v > max_byte_value ? max_byte_value : v;
			});
}


bool ByteSeq::matches(sequence_type::size_type i, byte_type b) const
{
	return sequence_[i] == b || is_wildcard(i);
}


bool ByteSeq::is_wildcard(sequence_type::size_type i) const
{
	return wildcards_.find(i) != wildcards_.end();
}


ByteSeq& ByteSeq::swap(ByteSeq& rhs)
{
	using std::swap;
	swap(this->sequence_,  rhs.sequence_);
	swap(this->wildcards_, rhs.wildcards_);
	return *this;
}


ByteSeq::size_type ByteSeq::size() const
{
	return sequence_.size();
}


bool ByteSeq::empty() const
{
	return sequence_.empty();
}


ByteSeq::const_reference ByteSeq::operator[](ByteSeq::size_type i) const
{
	return sequence_[i];
}


ByteSeq::reference ByteSeq::operator[](ByteSeq::size_type i)
{
	return sequence_[i];
}


ByteSeq::sequence_type::iterator ByteSeq::begin()
{
	return sequence_.begin();
}


ByteSeq::sequence_type::iterator ByteSeq::end()
{
	return sequence_.end();
}


ByteSeq::sequence_type::const_iterator ByteSeq::begin() const
{
	return sequence_.begin();
}


ByteSeq::sequence_type::const_iterator ByteSeq::end() const
{
	return sequence_.end();
}


ByteSeq::sequence_type::const_iterator ByteSeq::cbegin() const
{
	return sequence_.cbegin();
}


ByteSeq::sequence_type::const_iterator ByteSeq::cend() const
{
	return sequence_.cend();
}


ByteSeq::byte_type* ByteSeq::data()
{
	return sequence_.data();
}


bool operator == (const ByteSeq& lhs, const ByteSeq& rhs)
{
	return lhs.sequence_ == rhs.sequence_ && lhs.wildcards_ == rhs.wildcards_;
}


void swap(ByteSeq& lhs, ByteSeq& rhs)
{
	lhs.swap(rhs);
}


// Bytes


bool operator == (const Bytes& lhs, const Bytes& rhs)
{
	return lhs.offset_ == rhs.offset_ && lhs.seq_ == rhs.seq_;
}


constexpr unsigned int Bytes::any;


Bytes::Bytes()
	: offset_ { 0 }
	, seq_    { /* empty */ }
{
	// empty
}


Bytes::Bytes(const uint32_t offset, const ByteSequence& bytes)
	: offset_ { offset }
	, seq_    { bytes }
{
	// empty
}


bool Bytes::match(const Bytes& bytes) const
{
	return match(bytes.sequence(), bytes.offset());
}


bool Bytes::match(const ByteSequence& bytes, const uint32_t& ioffset) const
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

	auto on_wildcard = bool { false };

	do
	{
		const auto m = std::mismatch(in_current, in_stop, ref_current);

		// Reached the end? => Success
		if (m.first == in_stop or m.second == ref_stop)
		{
			return true;
		}

		// Has reference or input sequence a wildcard on their respective
		// current position?
		auto dist_r = std::distance(ref_bytes().begin(), m.second);
		auto dist_i = std::distance(bytes.begin(), m.first);
		on_wildcard = ref_bytes().is_wildcard(static_cast<unsigned>(dist_r))
			|| bytes.is_wildcard(static_cast<unsigned>(dist_i));

		// Is it an actual mismatch (i.e. on a non-wildcard byte)?
		if (m.second != ref_stop and !on_wildcard)
		{
			return false;
		}

		// Mismatch was on a "wildcard byte", so skip all following bytes until
		// the wildcard sequence ends.
		in_current  = m.first;
		ref_current = m.second;
		while (in_current != in_stop && ref_current != ref_stop && on_wildcard)
		{
			++in_current;
			++ref_current;
		}

	} while (in_current != in_stop and ref_current != ref_stop);

	return true;
}


bool Bytes::match(const ByteSequence& bytes) const
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


ByteSequence::const_reference Bytes::operator[](
		ByteSequence::size_type i) const
{
	return seq_[i];
}


Bytes& Bytes::swap(Bytes& b) // noexcept
{
	using std::swap;
	swap(this->seq_,    b.seq_); // noexcept only since C++17
	swap(this->offset_, b.offset_);
	return *this;
}


const ByteSequence& Bytes::ref_bytes() const
{
	return seq_;
}


namespace details
{


bool ci_match_suffix(const SuffixSet& suffices, const std::string& filename)
{
	const auto fname_suffix = details::get_suffix(filename, ".");

	if (fname_suffix.empty()) { return false; }
	if (fname_suffix.length() == filename.length()) { return true; }

	const auto ref_suffix = details::ci_string { fname_suffix.c_str() };
	auto result = std::find_if(suffices.begin(), suffices.end(),
			[ref_suffix](const SuffixSet::value_type& suffix)
			{
				return suffix == ref_suffix; // case insensitive comparison
			});

	return result != suffices.end();
}


std::string get_suffix(const std::string& filename, const std::string& delim)
{
	// TODO Use std::filesystem of C++17

	if (filename.empty()) { return filename; }

	auto pos = filename.find_last_of(delim);

	if (pos == std::string::npos) { return filename; }

	return pos < filename.length()
			? filename.substr(pos + 1, filename.length())
			: std::to_string(filename.back());
}


Bytes read_bytes(const std::string& filename,
	const uint32_t& offset, const uint32_t& length)
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


// Matcher


Matcher::~Matcher() noexcept = default;


std::string Matcher::name() const
{
	return do_name();
}


bool Matcher::matches(const Bytes& bytes) const
{
	return do_matches(bytes);
}


bool Matcher::matches(const std::string& filename) const
{
	return do_matches(filename);
}


Format Matcher::format() const
{
	return do_format();
}


std::set<Codec> Matcher::codecs() const
{
	return do_codecs();
}


Bytes Matcher::reference_bytes() const
{
	return do_reference_bytes();
}


std::unique_ptr<Matcher> Matcher::clone() const
{
	return do_clone();
}


// libinfo_entry_filepath


LibInfoEntry libinfo_entry_filepath(const std::string& libname)
{
	return { libname, details::libfile(libname) };
}


// FileReader


FileReader::~FileReader() noexcept = default;


std::unique_ptr<FileReaderDescriptor> FileReader::descriptor() const
{
	return this->do_descriptor();
}


// InputFormatException


InputFormatException::InputFormatException(const std::string& what_arg)
	: std::runtime_error(what_arg)
{
	/* empty */
};


// FileReadException


FileReadException::FileReadException(const std::string& what_arg)
	: std::runtime_error { what_arg }
	, byte_pos_ { -1 }
{
	// empty
}


FileReadException::FileReadException(const std::string& what_arg,
		const int64_t& byte_pos)
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


InputType FileReaderDescriptor::input_type() const
{
	return this->do_input_type();
}


std::unique_ptr<FileReader> FileReaderDescriptor::create_reader() const
{
	return this->do_create_reader();
}


std::unique_ptr<FileReaderDescriptor> FileReaderDescriptor::clone() const
{
	return this->do_clone();
}


InputType FileReaderDescriptor::do_input_type() const
{
	return InputType::AUDIO;
}


bool FileReaderDescriptor::do_accepts_format(const Format f) const
{
	using std::cbegin;
	using std::cend;

	const auto fs = formats();
	return std::find(cbegin(fs), cend(fs), f) != cend(fs);
}


bool FileReaderDescriptor::do_accepts_codec(const Codec c) const
{
	using std::cbegin;
	using std::cend;

	const auto cs = codecs();
	return std::find(cbegin(cs), cend(cs), c) != cend(cs);
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


bool operator == (const FileReaderDescriptor& lhs,
			const FileReaderDescriptor& rhs)
{
	// FileReaderDescriptors are stateless and hence equal iff they are of the
	// same static type

	return typeid(lhs) == typeid(rhs);
}


void swap(Bytes& lhs, Bytes& rhs)
{
	lhs.swap(rhs);
}

} // namespace v_1_0_0
} // namespace arcsdec

