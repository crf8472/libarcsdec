/**
 * \file descriptors.cpp Implementation of a selection toolkit for FileReaders
 */

#ifndef __LIBARCSDEC_DESCRIPTORS_HPP__
#include "descriptors.hpp"
#endif

extern "C"
{
#include <dlfcn.h>     // [glibc, Linux] for dlopen,dlclose
#include <link.h>      // [glibc, Linux] for link_map
}

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <list>
#include <memory>
#include <regex>
#include <string>
#include <vector>

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __LIBARCSDEC_VERSION_HPP__
#include "version.hpp"
#endif


namespace arcsdec
{

inline namespace v_1_0_0
{

namespace details
{

// TODO Linux/Unix only
std::vector<std::string> list_libs(const std::string &object_name)
{
	// C-Style stuff: Messing with glibc to get shared object paths
	// Use dlfcn.h and link.h. Do not know a better way yet.

	const auto* object = object_name.empty() ? nullptr : object_name.c_str();

	// TODO Hardcodes the SO we load on runtime and may completely fail
	auto* handle = ::dlopen(object, RTLD_LAZY);
	//auto* handle = ::dlopen(object, RTLD_LAZY);
	// If calles with NULL for first parameter, dlopen returns the list for
	// the main executable. Take this, then figure out libarcsdec.so, then load.

	if (!handle)
	{
		throw std::runtime_error(::dlerror());
	}

	using OpaqueStruct =
		struct opaque_struct
		{
			void*  pointers[3];
			struct opaque_struct* ptr;
		};

	auto* pter = reinterpret_cast<OpaqueStruct*>(handle)->ptr;

	if (!pter)
	{
		throw std::runtime_error("Got null instead of shared object handle");
	}

	using LinkMap = struct link_map;

	auto* lmap  = reinterpret_cast<LinkMap*>(pter->ptr);

	if (!lmap)
	{
		throw std::runtime_error("Shared object handle contained no link_map");
	}

	// Traverse link_map for names

	auto so_list = std::vector<std::string>{};

	while (lmap)
	{
		so_list.push_back(lmap->l_name); // FIXME Choose initial capacity

		lmap = lmap->l_next;
	}

	::dlclose(handle);

	return so_list;
}


void escape(std::string &input, const char c, const std::string &escape_seq)
{
	std::size_t lookHere = 0;
	std::size_t foundHere;

	auto replacement = escape_seq + c;
	while((foundHere = input.find(c, lookHere)) != std::string::npos)
	{
		input.replace(foundHere, 1, replacement);
		lookHere = foundHere + replacement.size();
	}
}


std::regex libname_pattern(const std::string &libname)
{
	auto e_name = libname;

	escape(e_name, '+', "\\");
	// TODO traverse string and escape every non-word (\W) character

	return std::regex(".*\\b" + e_name + "\\.so(\\.[0-9]+)*$",
			std::regex::icase);
}


const std::string& find_lib(const std::vector<std::string> &list,
		const std::string &name)
{
	static const auto empty_entry = std::string{};

	const auto pattern = libname_pattern(name);

	auto first_match = std::find_if(list.begin(), list.end(),
			[pattern](const std::string &lname)
			{
				return std::regex_match(lname, pattern);
			}
	);

	if (first_match == list.end())
	{
		return empty_entry;
	}

	return *first_match;
}


std::vector<std::string> acquire_libarcsdec_libs()
{
	// Runtime deps from main executable

	auto so_list = list_libs("");

	// Runtime deps of libarcsdec

	auto libarcsdec_so = find_lib(so_list, LIBARCSDEC_NAME);

	if (libarcsdec_so.empty())
	{
		ARCS_LOG_WARNING << "Could not retrieve any runtime dependencies from"
			" libarcsdec";

		return {}; // libarcsdec was not found
	}

	ARCS_LOG_DEBUG << "Inspect " << libarcsdec_so
		<< " for runtime dependencies";

	return list_libs(libarcsdec_so);
}


const std::vector<std::string>& libarcsdec_libs()
{
	static const std::vector<std::string> libarcsdec_libs =
		acquire_libarcsdec_libs();

	return libarcsdec_libs;
}

} // namespace details


std::string name(Format format)
{
	static const std::array<std::string, 12> names = {
		"Unknown",
		"CUE",
		"cdrdao",
		// ... add more metadata formats here
		"wave", // Audio formats from here on (is_audio_format relies on that)
		"fLaC",
		"APE",
		"CAF",
		"M4A",
		"OGG",
		"WV",
		"AIFF",
		"WMA"
		// ... add more audio formats here
	};

	return names[std::underlying_type_t<Format>(format)];
}


std::string name(Codec codec)
{
	static const std::array<std::string, 14> names = {
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
		"WMALOSSLESS"
	};

	return names[std::underlying_type_t<Codec>(codec)];
}


bool is_audio_format(Format format)
{
	return format >= Format::WAVE;
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
	: std::runtime_error(what_arg)
	, byte_pos_(-1)
{
	// empty
}


FileReadException::FileReadException(const std::string &what_arg,
		const int64_t &byte_pos)
	: std::runtime_error(what_arg)
	, byte_pos_(byte_pos)
{
	// empty
}


int64_t FileReadException::byte_pos() const
{
	return byte_pos_;
}


// FileReaderDescriptor


FileReaderDescriptor::~FileReaderDescriptor() noexcept = default;


std::string FileReaderDescriptor::name() const
{
	return this->do_name();
}


bool FileReaderDescriptor::accepts_bytes(const std::vector<char> &bytes,
			const uint64_t &offset) const
{
	return this->do_accepts_bytes(bytes, offset);
}


bool FileReaderDescriptor::accepts_name(const std::string &filename) const
{
	return this->do_accepts_name(filename);
}


bool FileReaderDescriptor::accepts(Format format) const
{
	return this->do_accepts(format);
}


bool FileReaderDescriptor::accepts(Codec codec) const
{
	return this->do_accepts(codec);
}


std::set<Format> FileReaderDescriptor::formats() const
{
	return this->do_formats();
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
	auto fname_suffix = get_suffix(filename, ".");

	if (fname_suffix.empty()) { return false; }

	if (fname_suffix.length() == filename.length()) { return false; }
	// Shouldn't this be TRUE?

	auto rc = std::find_if(suffices_.begin(), suffices_.end(),
			[fname_suffix,this](const decltype( *suffices_.begin() ) &suffix)
			{
				// Perform case insensitive comparison
				return suffix == details::ci_string { fname_suffix.c_str() };
			});

	return rc != suffices_.end();
}


std::string FileReaderDescriptor::get_suffix(const std::string &filename,
		const std::string &delimiter) const
{
	if (filename.empty()) { return filename; }

	auto pos = filename.find_last_of(delimiter);

	if (pos == std::string::npos) { return filename; }

	return pos < filename.length()
			? filename.substr(pos + 1, filename.length())
			: std::to_string(filename.back());
}


bool operator == (const FileReaderDescriptor &lhs,
			const FileReaderDescriptor &rhs)
{
	// FileReaderDescriptors are stateless and hence equal iff they are of the
	// same static type

	return typeid(lhs) == typeid(rhs);
}


// FileTest


FileTest::FileTest()
	: filename_()
{
	// empty
}


FileTest::FileTest(const std::string &filename)
	: filename_(filename)
{
	// empty
}


FileTest::~FileTest() noexcept = default;


void FileTest::set_filename(const std::string &filename)
{
	filename_ = filename;
}


const std::string& FileTest::filename() const
{
	return filename_;
}


bool FileTest::matches(const FileReaderDescriptor &desc) const
{
	return this->do_matches(desc);
}


// FileTestBytes


FileTestBytes::FileTestBytes(const uint64_t &offset,
		const uint32_t &length)
	: offset_(offset)
	, length_(length)
{
	// empty
}


bool FileTestBytes::do_matches(const FileReaderDescriptor &desc) const
{
	auto bytes = this->read_bytes(this->filename(), offset_, length_);
	return desc.accepts_bytes(bytes, offset_);
}


std::vector<char> FileTestBytes::read_bytes(const std::string &filename,
	const uint32_t &offset, const uint32_t &length) const
{
	// Read a number of bytes from the start of the file

	// Note: We close and reopen the file while analyzing its file type.
	// We 1 open the file, 2 read from it, 3 close it,
	// 4 open it again with the determined FileReader and 5 read the content.
	// This is inuitively two unnecessary operations (one open and one close).
	// On the other hand, it is clean, requires no tricks and the application
	// is not considered performance-critical. So for the moment, this is the
	// way to go.

	std::vector<char> bytes(length);
	//bytes.reserve(length); // TODO Fix vector usage

	std::ifstream in;

	in.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		in.open(filename, std::ifstream::in | std::ifstream::binary);

		in.ignore(offset);

		in.read(&bytes[0], length * sizeof(bytes[0]));

		in.close();
	}
	catch (const std::ifstream::failure& f)
	{
		int64_t total_bytes_read = in.gcount();

		bytes.resize(0);

		throw FileReadException(f.what(), total_bytes_read + 1);
	}

	return bytes;
}


// FileTestName


bool FileTestName::do_matches(const FileReaderDescriptor &desc) const
{
	return desc.accepts_name(filename());
}


// FileReaderSelector


FileReaderSelector::~FileReaderSelector() noexcept
= default;


std::unique_ptr<FileReaderDescriptor> FileReaderSelector::select(
		const std::set<std::unique_ptr<FileTest>> &tests,
		const std::list<std::unique_ptr<FileReaderDescriptor>> &descs) const
{
	return this->do_select(tests, descs);
}


bool FileReaderSelector::matches(
		const std::set<std::unique_ptr<FileTest>> &tests,
		const std::unique_ptr<FileReaderDescriptor> &desc) const
{
	ARCS_LOG_DEBUG << "Try to match descriptor: " << desc->name();

	return this->do_matches(tests, desc);
}


// DefaultSelector


std::unique_ptr<FileReaderDescriptor> DefaultSelector::do_select(
		const std::set<std::unique_ptr<FileTest>> &tests,
		const std::list<std::unique_ptr<FileReaderDescriptor>> &descs) const
{
	for (auto& desc : descs)
	{
		if (this->matches(tests, desc))
		{
			ARCS_LOG_DEBUG << "Select descriptor: '" << desc->name() << "'";

			return desc->clone();
		}
	}

	return nullptr;
}


bool DefaultSelector::do_matches(
		const std::set<std::unique_ptr<FileTest>> &tests,
		const std::unique_ptr<FileReaderDescriptor> &desc) const
{
	// The default implementation of matches() returns TRUE iff each test
	// passes (== AND). It could be overwritten by a version that returns TRUE
	// iff at least one test passes (== OR)

	for (const auto& test : tests)
	{
		ARCS_LOG_DEBUG << "Perform test"; // TODO test name?

		if (not test->matches(*desc))
		{
			ARCS_LOG_DEBUG << "Test failed";
			return false;
		}
	}

	// Note that if no tests are registered, each FileReaderDescriptor matches!
	// This means that whatever is first in enumerating the descriptors will be
	// matched and create the FileReader.

	ARCS_LOG_DEBUG << "Descriptor '" << desc->name() << "' matched";

	return true;
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

	Impl()
		: selector_(std::make_unique<DefaultSelector>())
		, tests_()
		, descriptors_()
	{ /* empty */ }

	void add_descriptor(std::unique_ptr<FileReaderDescriptor> desc);

	int remove_descriptor(const FileReaderDescriptor* desc);

	void remove_all_descriptors();

	void register_test(std::unique_ptr<FileTest> testobj);

	int unregister_test(const FileTest * testobj);

	void remove_all_tests();

	void reset();

	void set_selector(std::unique_ptr<FileReaderSelector> selector);

	const FileReaderSelector& selector() const;

	std::unique_ptr<FileReaderDescriptor> select_descriptor(
			const std::string &filename) const;

	std::unique_ptr<FileReader> for_file(const std::string &filename) const;

	std::unique_ptr<FileReader> by_name(const std::string &name) const;

	void traverse_descriptors(
			std::function<void(const FileReaderDescriptor &)> func) const;

	std::size_t size() const;

	bool empty() const;

	std::size_t total_tests() const;

	bool no_tests() const;

private:

	/**
	 * \brief Return the FileReaderSelector of this instance for use in
	 * subclasses.
	 *
	 * \return The FileReaderSelector of this instance
	 */
	FileReaderSelector& use_selector();

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
	std::list<std::unique_ptr<FileReaderDescriptor>> descriptors_;
};


void FileReaderSelection::Impl::add_descriptor(
		std::unique_ptr<FileReaderDescriptor> desc)
{
	descriptors_.push_back(std::move(desc));
}


int FileReaderSelection::Impl::remove_descriptor(
		const FileReaderDescriptor * desc)
{
	int counter { 0 };

	auto pos = descriptors_.begin();
	while ((pos = std::find_if(pos, descriptors_.end(),
			[desc](const std::unique_ptr<FileReaderDescriptor> &d)
				{ return d.get() == desc; }))
			!= descriptors_.end())
	{
		descriptors_.erase(pos);
		++counter;
	}

	return counter;
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


int FileReaderSelection::Impl::unregister_test(const FileTest * test)
{
	int counter { 0 };

	auto pos = tests_.begin();
	while ((pos = std::find_if(pos, tests_.end(),
			[test](const std::unique_ptr<FileTest> &t)
				{ return t.get() == test; }))
			!= tests_.end())
	{
		tests_.erase(pos);
		++counter;
	}

	return counter;
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

	for (auto& test : tests_)
	{
		test->set_filename(filename); // FIXME Remove this!
	}

	std::unique_ptr<FileReaderDescriptor> desc =
		selector_->select(tests_, descriptors_);

	if (not desc)
	{
		ARCS_LOG_WARNING << "Container format or codec unknown.";

		return nullptr;
	}

	ARCS_LOG_INFO << "Select reader '" << desc->name() << "'";

	return desc;
}


std::unique_ptr<FileReader> FileReaderSelection::Impl::for_file(
		const std::string &filename) const
{
	auto desc = this->select_descriptor(filename);

	return desc ? desc->create_reader() : nullptr;
}


std::unique_ptr<FileReader> FileReaderSelection::Impl::by_name(
		const std::string &name) const
{
	for (const auto& desc : descriptors_)
	{
		if (name == desc->name())
		{
			return desc->create_reader();
		}
	}

	return nullptr;
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


FileReaderSelector& FileReaderSelection::Impl::use_selector()
{
	return *selector_;
}

/// @}


// FileReaderSelection


FileReaderSelection::FileReaderSelection()
	: impl_(std::make_unique<FileReaderSelection::Impl>())
{
	// empty
}


FileReaderSelection::~FileReaderSelection() noexcept = default;


void FileReaderSelection::add_descriptor(
		std::unique_ptr<FileReaderDescriptor> desc)
{
	impl_->add_descriptor(std::move(desc));
}


int FileReaderSelection::remove_descriptor(const FileReaderDescriptor * desc)
{
	return impl_->remove_descriptor(desc);
}


void FileReaderSelection::register_test(std::unique_ptr<FileTest> testobj)
{
	impl_->register_test(std::move(testobj));
}


int FileReaderSelection::unregister_test(const FileTest * test)
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


std::unique_ptr<FileReader> FileReaderSelection::by_name(
		const std::string &name) const
{
	return impl_->by_name(name);
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


std::size_t FileReaderSelection::total_tests() const
{
	return impl_->total_tests();
}


bool FileReaderSelection::empty() const
{
	return impl_->empty();
}

} // namespace v_1_0_0

} // namespace arcsdec

