/**
 * \file
 *
 * \brief Implementation of a selection toolkit for FileReaders
 */

#ifndef __LIBARCSDEC_DESCRIPTORS_HPP__
#include "descriptors.hpp"
#endif

#include <algorithm>    // for find_if
#include <array>        // for array
#include <cstddef>      // for size_t
#include <cstdint>      // for uint32_t, uint64_t, int64_t
#include <fstream>      // for ifstream
#include <functional>   // for function
#include <memory>       // for unique_ptr
#include <string>       // for string, to_string
#include <type_traits>  // for underlying_type_t
#include <utility>      // for move, make_unique
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
		"wav", // Audio formats from here on (is_audio_format relies on that)
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
				return suffix == details::ci_string { fname_suffix.c_str() };
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


// FileTest


FileTest::FileTest() = default;


FileTest::~FileTest() noexcept = default;


std::string FileTest::description() const
{
	return do_description();
}


bool FileTest::passes(const FileReaderDescriptor &desc,
		const std::string &filename) const
{
	ARCS_LOG_DEBUG << "Perform test: " << description();

	if (filename.empty())
	{
		ARCS_LOG_WARNING << "Test for empty filename failed";
		return false;
	}

	if (do_passes(desc, filename))
	{
		ARCS_LOG_DEBUG << "Test passed";
		return true;
	}

	ARCS_LOG_DEBUG << "Test failed";
	return false;
}


bool operator == (const FileTest &lhs, const FileTest &rhs)
{
	return lhs.equals(rhs);
}


// FileTestBytes


FileTestBytes::FileTestBytes(const uint32_t &offset,
		const uint32_t &length)
	: offset_(offset)
	, length_(length)
{
	// empty
}


uint32_t FileTestBytes::offset() const
{
	return offset_;
}


uint32_t FileTestBytes::length() const
{
	return length_;
}


std::string FileTestBytes::do_description() const
{
	return "Is byte sequence on offset " + std::to_string(offset_)
		+ " of length " + std::to_string(length_)
		+ " accepted?";
}


bool FileTestBytes::do_passes(const FileReaderDescriptor &desc,
		const std::string &filename) const
{
	ARCS_LOG(DEBUG1) << "Do the " << length_ << " bytes starting on offset "
		<< offset_ << " in file " << filename <<
		" match the definition in descriptor " << desc.name() << "?";

	auto bytes = details::read_bytes(filename, offset_, length_);
	return desc.accepts_bytes(bytes, offset_);
}


bool FileTestBytes::equals(const FileTest &rhs) const
{
	auto rhs_ftb = dynamic_cast<const FileTestBytes*>(&rhs);

	return rhs_ftb != nullptr
		and offset_ == rhs_ftb->offset_
		and length_ == rhs_ftb->length_;
}


// FileTestName


std::string FileTestName::do_description() const
{
	return "Filename analysis";
}


bool FileTestName::do_passes(const FileReaderDescriptor &desc,
		const std::string &filename) const
{
	ARCS_LOG(DEBUG1) << "Does descriptor " << desc.name()
		<< " accept file with name '" << filename << "'?";

	return desc.accepts_name(filename);
}


bool FileTestName::equals(const FileTest &rhs) const
{
	return typeid(*this) == typeid(rhs);
}


// FileReaderSelector


FileReaderSelector::FileReaderSelector() = default;


FileReaderSelector::~FileReaderSelector() noexcept = default;


bool FileReaderSelector::matches(
		const std::string &filename,
		const std::set<std::unique_ptr<FileTest>> &tests,
		const std::unique_ptr<FileReaderDescriptor> &desc) const
{
	ARCS_LOG_DEBUG << "Try to match descriptor: " << desc->name();

	if (do_matches(filename, tests, desc))
	{
		ARCS_LOG_DEBUG << "Descriptor '" << desc->name() << "' matched";
		return true;
	}

	ARCS_LOG_DEBUG << "Descriptor '" << desc->name() << "' does not match";
	return false;
}


std::unique_ptr<FileReaderDescriptor> FileReaderSelector::select(
		const std::string &filename,
		const std::set<std::unique_ptr<FileTest>> &tests,
		const std::set<std::unique_ptr<FileReaderDescriptor>> &descs) const
{
	auto descriptor = do_select(filename, tests, descs);

	if (!descriptor)
	{
		ARCS_LOG_WARNING << "Could not select a matching descriptor";
		return nullptr;
	}

	ARCS_LOG_DEBUG << "Descriptor '" << descriptor->name() << "' selected";

	return descriptor;
}


// DefaultSelector


bool DefaultSelector::do_matches(
		const std::string &filename,
		const std::set<std::unique_ptr<FileTest>> &tests,
		const std::unique_ptr<FileReaderDescriptor> &desc) const
{
	// The default implementation of matches() returns TRUE iff each test
	// passes (== AND). It could be overwritten by a version that returns TRUE
	// iff at least one test passes (== OR).

	for (const auto& test : tests)
	{
		// Note that if no tests are registered, each descriptor matches!
		// In this case, just the first descriptor wins.

		if (not test->passes(*desc, filename))
		{
			ARCS_LOG(DEBUG1) << "Descriptor '" << desc->name()
				<< "' failed test '"
				<< test->description()
				<< "' and is discarded";
			return false;
		}
	}

	// Note: We close and reopen the file while analyzing its file type.
	// We 1 open the file, 2 read from it, 3 close it,
	// 4 open it again with the determined FileReader and 5 read the content.
	// This is inuitively two unnecessary operations (one open and one close).
	// On the other hand, it is clean, requires no tricks and the application
	// is not considered performance-critical. So for the moment, this is the
	// way to go.

	ARCS_LOG(DEBUG1) << "Descriptor '" << desc->name() << "' passed all tests";
	return true;
}


std::unique_ptr<FileReaderDescriptor> DefaultSelector::do_select(
		const std::string &filename,
		const std::set<std::unique_ptr<FileTest>> &tests,
		const std::set<std::unique_ptr<FileReaderDescriptor>> &descs) const
{
	for (auto& desc : descs)
	{
		if (this->matches(filename, tests, desc))
		{
			ARCS_LOG(DEBUG1) << "First matching descriptor: '" << desc->name()
				<< "'";

			return desc->clone();
		}
	}

	return nullptr;
}


/**
 * \internal
 *
 * \defgroup descriptorsImpl Implementation details for module 'descriptors'
 *
 * \ingroup descriptors
 * @{
 */

/**
 * \brief Implementation of FileReaderSelection.
 */
class FileReaderSelection::Impl final
{
public:

	Impl(std::unique_ptr<FileReaderSelector> selector)
		: selector_ { std::move(selector) }
		, tests_{}
		, descriptors_{}
	{ /* empty */ }

	void add_descriptor(std::unique_ptr<FileReaderDescriptor> desc);

	std::unique_ptr<FileReaderDescriptor> remove_descriptor(
			const std::unique_ptr<FileReaderDescriptor> &desc);

	void remove_all_descriptors();

	void register_test(std::unique_ptr<FileTest> testobj);

	std::unique_ptr<FileTest> unregister_test(
			const std::unique_ptr<FileTest> &test);

	void remove_all_tests();

	void reset();

	void set_selector(std::unique_ptr<FileReaderSelector> selector);

	const FileReaderSelector& selector() const;

	std::unique_ptr<FileReaderDescriptor> select_descriptor(
			const std::string &filename) const;

	std::unique_ptr<FileReader> for_file(const std::string &filename) const;

	void traverse_descriptors(
			std::function<void(const FileReaderDescriptor &)> func) const;

	std::size_t size() const;

	bool empty() const;

	std::size_t total_tests() const;

	bool no_tests() const;

private:

	/**
	 * \brief Internal FileReaderSelector
	 */
	std::unique_ptr<FileReaderSelector> selector_;

	/**
	 * \brief Internal set of FileTests to performed by the selector_
	 */
	std::set<std::unique_ptr<FileTest>> tests_;

	/**
	 * \brief Internal list of FileReaderDescriptors to match by the selector_
	 */
	std::set<std::unique_ptr<FileReaderDescriptor>> descriptors_;
};


void FileReaderSelection::Impl::add_descriptor(
		std::unique_ptr<FileReaderDescriptor> desc)
{
	descriptors_.insert(std::move(desc));
}


std::unique_ptr<FileReaderDescriptor>
	FileReaderSelection::Impl::remove_descriptor(
		const std::unique_ptr<FileReaderDescriptor> &desc)
{
	auto desc_ptr = desc.get();
	auto pos = std::find_if(descriptors_.begin(), descriptors_.end(),
			[desc_ptr](const std::unique_ptr<FileReaderDescriptor> &d)
			{
				return desc_ptr && *d == *desc_ptr;
			});

	if (pos != descriptors_.end())
	{
		pos = descriptors_.erase(pos);
	}

	if (pos == descriptors_.end())
	{
		return nullptr;
	}

	return std::move(const_cast<std::unique_ptr<FileReaderDescriptor>&>(*pos));
}


void FileReaderSelection::Impl::remove_all_descriptors()
{
	descriptors_.clear();
}


void FileReaderSelection::Impl::register_test(
		std::unique_ptr<FileTest> testobj)
{
	tests_.insert(std::move(testobj));
}


std::unique_ptr<FileTest>
	FileReaderSelection::Impl::unregister_test(
			const std::unique_ptr<FileTest> &test)
{
	auto test_ptr = test.get();
	auto pos = std::find_if(tests_.begin(), tests_.end(),
			[test_ptr](const std::unique_ptr<FileTest> &t)
			{
				return test_ptr && *t == *test_ptr;
			});

	if (pos != tests_.end())
	{
		pos = tests_.erase(pos);
	}

	if (pos == tests_.end())
	{
		return nullptr;
	}

	return std::move(const_cast<std::unique_ptr<FileTest>&>(*pos));
}


void FileReaderSelection::Impl::remove_all_tests()
{
	tests_.clear();
}


void FileReaderSelection::Impl::reset()
{
	tests_.clear();
	descriptors_.clear();
}


void FileReaderSelection::Impl::set_selector(
		std::unique_ptr<FileReaderSelector> selector)
{
	selector_ = std::move(selector);
}


const FileReaderSelector& FileReaderSelection::Impl::selector() const
{
	return *selector_;
}


std::unique_ptr<FileReaderDescriptor>
FileReaderSelection::Impl::select_descriptor(const std::string &filename) const
{
	if (filename.empty())
	{
		throw FileReadException("Filename must not be empty");
	}

	std::unique_ptr<FileReaderDescriptor> desc =
		selector_->select(filename, tests_, descriptors_);

	if (!desc)
	{
		ARCS_LOG_WARNING << "File format is unknown, no reader available.";

		return nullptr;
	}

	return desc;
}


std::unique_ptr<FileReader> FileReaderSelection::Impl::for_file(
		const std::string &filename) const
{
	auto desc = this->select_descriptor(filename);

	return desc ? desc->create_reader() : nullptr;
}


void FileReaderSelection::Impl::traverse_descriptors(
			std::function<void(const FileReaderDescriptor &)> func) const
{
	for (const auto& desc : descriptors_)
	{
		func(*desc);
	}
}


std::size_t FileReaderSelection::Impl::size() const
{
	return descriptors_.size();
}


bool FileReaderSelection::Impl::empty() const
{
	return descriptors_.empty();
}


std::size_t FileReaderSelection::Impl::total_tests() const
{
	return tests_.size();
}


bool FileReaderSelection::Impl::no_tests() const
{
	return tests_.empty();
}

/// @}


// FileReaderSelection


FileReaderSelection::FileReaderSelection()
	: impl_ { std::make_unique<FileReaderSelection::Impl>(
				std::make_unique<DefaultSelector>()) }
{
	// empty
}


FileReaderSelection::~FileReaderSelection() noexcept = default;


void FileReaderSelection::add_descriptor(
		std::unique_ptr<FileReaderDescriptor> desc)
{
	impl_->add_descriptor(std::move(desc));
}


std::unique_ptr<FileReaderDescriptor> FileReaderSelection::remove_descriptor(
		const std::unique_ptr<FileReaderDescriptor> &desc)
{
	return impl_->remove_descriptor(desc);
}


void FileReaderSelection::register_test(std::unique_ptr<FileTest> testobj)
{
	impl_->register_test(std::move(testobj));
}


std::unique_ptr<FileTest>  FileReaderSelection::unregister_test(
		const std::unique_ptr<FileTest> &test)
{
	return impl_->unregister_test(test);
}


void FileReaderSelection::remove_all_tests()
{
	return impl_->remove_all_tests();
}


void FileReaderSelection::set_selector(
		std::unique_ptr<FileReaderSelector> selector)
{
	impl_->set_selector(std::move(selector));
}


const FileReaderSelector& FileReaderSelection::selector() const
{
	return impl_->selector();
}


std::unique_ptr<FileReaderDescriptor> FileReaderSelection::select_descriptor(
		const std::string &filename) const
{
	return impl_->select_descriptor(filename);
}


std::unique_ptr<FileReader> FileReaderSelection::for_file(
		const std::string &filename) const
{
	return impl_->for_file(filename);
}


void FileReaderSelection::traverse_descriptors(
			std::function<void(const FileReaderDescriptor &)> func) const
{
	impl_->traverse_descriptors(func);
}


void FileReaderSelection::reset()
{
	impl_->reset();
}


std::size_t FileReaderSelection::size() const
{
	return impl_->size();
}


bool FileReaderSelection::empty() const
{
	return impl_->empty();
}


std::size_t FileReaderSelection::total_tests() const
{
	return impl_->total_tests();
}


bool FileReaderSelection::no_tests() const
{
	return impl_->no_tests();
}


// FileReaderRegistry


FileReaderRegistry::FileReaderRegistry() = default;


std::unique_ptr<FileReaderSelection> FileReaderRegistry::audio_selection_;


std::unique_ptr<FileReaderSelection> FileReaderRegistry::toc_selection_;


} // namespace v_1_0_0
} // namespace arcsdec

