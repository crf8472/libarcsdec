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


/**
 * \brief Implementation of FileReaderSelection.
 */
class FileReaderSelection::Impl
{

public:

	/**
	 * \brief Default constructor.
	 */
	Impl();

	// class is non-copy-constructible
	Impl(const Impl &) = delete;

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~Impl() noexcept;

	/**
	 * \brief Add a file descriptor for which a reader can be created.
	 *
	 * \param[in] desc A FileReaderDescriptor
	 */
	void add_descriptor(std::unique_ptr<FileReaderDescriptor> desc);

	/**
	 * \brief Remove all descriptors that qualify as equivalent to the given
	 * descriptor by '==' from the list of descriptors.
	 *
	 * \param[in] desc The FileReaderDescriptor to be removed
	 *
	 * \return Number of descriptors instances removed.
	 */
	int remove_descriptor(const FileReaderDescriptor* desc);

	/**
	 * \brief Register a test for a FileReaderDescriptor for the specified
	 * filename.
	 *
	 * \param[in] testobj The test to be registered
	 */
	void register_test(std::unique_ptr<FileTest> testobj);

	/**
	 * \brief Remove all tests that qualify as equivalent to the given test by
	 * '==' from the list of test.
	 *
	 * \param[in] testobj The FileTest to be removed
	 *
	 * \return Number of test instances removed.
	 */
	int unregister_test(const FileTest * testobj);

	/**
	 * \brief Removes all tests registered to this instance.
	 */
	void remove_all_tests();

	/**
	 * \brief Sets the FileReaderSelector for this instance.
	 *
	 * \param[in] selector The FileReaderSelector for this instance
	 */
	void set_selector(std::unique_ptr<FileReaderSelector> selector);

	/**
	 * \brief Returns the internal FileReaderSelector of this instance.
	 *
	 * \return The FileReaderSelector of this instance
	 */
	const FileReaderSelector& selector() const;

	/**
	 * \brief Determine a matching FileReaderDescriptor for the specified file.
	 *
	 * \param[in] filename Name of the file to determine a descriptor for
	 *
	 * \return A FileReaderDescriptor for the specified file
	 */
	std::unique_ptr<FileReaderDescriptor> descriptor(
			const std::string &filename) const;

	/**
	 * \brief Create an opaque FileReader for the given file.
	 *
	 * Will return nullptr if the file does not exist or cannot be read or the
	 * filename is empty.
	 *
	 * \param[in] filename Name of the file to create the reader for
	 *
	 * \return A FileReader for the specified file
	 */
	std::unique_ptr<FileReader> for_file(const std::string &filename) const;

	/**
	 * \brief Return the FileReader specified by its name.
	 *
	 * If the selection does not contain a FileReader with the specified name,
	 * \c nullptr will be returned.
	 *
	 * \param[in] name The name of the FileReader.
	 *
	 * \return A FileReader with the specified name
	 */
	std::unique_ptr<FileReader> by_name(const std::string &name) const;

	/**
	 * \brief Traverse all available descriptors and apply the specified
	 * function \c func on each of them.
	 *
	 * \param[in] func Function to apply to each descriptor.
	 */
	void traverse_descriptors(
			std::function<void(const FileReaderDescriptor &)> func) const;

	/**
	 * \brief Reset this instance to its initial state, removing all tests and
	 * descriptors.
	 */
	void reset();

	/**
	 * \brief Number of descriptors.
	 *
	 * \return The number of descriptors in this selection.
	 */
	std::size_t size() const;

	/**
	 * \brief TRUE if this selection contains no descriptors.
	 *
	 * \return TRUE if this selection contains no descriptors.
	 */
	bool empty() const;

	// class is non-copy-assignable
	Impl& operator = (const Impl &) = delete;

protected:

	/**
	 * \brief Return the FileReaderSelector of this instance for use in
	 * subclasses.
	 *
	 * \return The FileReaderSelector of this instance
	 */
	FileReaderSelector& use_selector();

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
	std::list<std::unique_ptr<FileReaderDescriptor>> descriptors_;
};


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


bool FileReaderDescriptor::accepts(Codec codec) const
{
	return this->do_accepts(codec);
}


std::set<Codec> FileReaderDescriptor::codecs() const
{
	return this->do_codecs();
}


bool FileReaderDescriptor::accepts(Format format) const
{
	return this->do_accepts(format);
}


std::set<Format> FileReaderDescriptor::formats() const
{
	return this->do_formats();
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


bool FileReaderDescriptor::operator == (const FileReaderDescriptor &rhs) const
{
	return this->do_operator_equals(rhs);
}


bool FileReaderDescriptor::operator != (const FileReaderDescriptor &rhs) const
{
	return not(this == &rhs);
}


bool FileReaderDescriptor::do_operator_equals(const FileReaderDescriptor &rhs)
	const
{
	// FileReaderDescriptors are stateless and hence equal iff they are of the
	// same static type

	return typeid(this) == typeid(rhs);
}


bool FileReaderDescriptor::do_accepts_name(const std::string &filename) const
{
	auto rc = std::find_if(suffices_.begin(), suffices_.end(),
			[filename,this](const decltype( suffices_[0] ) &suffix)
			{
				return this->has_suffix(filename, suffix);
			});

	return rc != suffices_.end();
}


std::string FileReaderDescriptor::get_suffix(const std::string &filename,
		const std::string &delimiter) const
{
	auto pos = filename.find_last_of(delimiter);

	if (pos == std::string::npos) { return filename; }

	return pos < filename.length()
			? filename.substr(pos + 1, filename.length())
			: std::to_string(filename.back());
}


bool FileReaderDescriptor::has_suffix(const std::string &name,
		const details::ci_string &suffix) const
{
	auto name_suffix = get_suffix(name, ".");

	if (name_suffix == name)
	{
		return false;
	}

	return suffix == details::ci_string { name_suffix.c_str() };
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


std::unique_ptr<FileReaderDescriptor> FileReaderSelector::do_select(
		const std::set<std::unique_ptr<FileTest>> &tests,
		const std::list<std::unique_ptr<FileReaderDescriptor>> &descs) const
{
	for (auto& desc : descs)
	{
		ARCS_LOG_DEBUG << "Testing reader: " << desc->name();

		if (this->matches(tests, desc))
		{
			ARCS_LOG_DEBUG << "Reader descriptor matched";

			return desc->clone();
		}
	}

	return nullptr;
}


bool FileReaderSelector::matches(
		const std::set<std::unique_ptr<FileTest>> &tests,
		const std::unique_ptr<FileReaderDescriptor> &desc) const
{
	// The default implementation of matches() returns TRUE iff each test
	// passes (== AND). It could be overwritten by a version that returns TRUE
	// iff at least one test passes (== OR)

	for (const auto& test : tests)
	{
		ARCS_LOG_DEBUG << "Perform test";

		if (not test->matches(*desc))
		{
			ARCS_LOG_DEBUG << "Test failed";
			return false;
		}
	}

	// Note that if no tests are registered, each FileReaderDescriptor matches!
	// This means that whatever is first in enumerating the descriptors will be
	// matched and create the FileReader.

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


// FileReaderSelection


FileReaderSelection::Impl::Impl()
	: selector_(std::make_unique<FileReaderSelector>())
	, tests_()
	, descriptors_()
{
	// empty
}


FileReaderSelection::Impl::~Impl() noexcept = default;


void FileReaderSelection::Impl::add_descriptor(
		std::unique_ptr<FileReaderDescriptor> desc)
{
	descriptors_.push_back(std::move(desc));
}


int FileReaderSelection::Impl::remove_descriptor(
		const FileReaderDescriptor * desc)
{
	int counter { 0 };

	auto end { descriptors_.end() };
	for (auto ptr = descriptors_.begin(); ptr != end; ++ptr) // FIXME repair
	{
		if (ptr->get() == desc)
		{
			descriptors_.erase(ptr); // <= THIS invalidate ptr!
			++counter;
		}
	}

	return counter;
}


void FileReaderSelection::Impl::register_test(
		std::unique_ptr<FileTest> testobj)
{
	tests_.insert(std::move(testobj));
}


int FileReaderSelection::Impl::unregister_test(const FileTest * test)
{
	int counter { 0 };

	for (auto& t: tests_) // FIXME repair
	{
		if (t.get() == test)
		{
			tests_.erase(t); // <= THIS invalidate t
			++counter;
		}
	}

	return counter;
}


void FileReaderSelection::Impl::remove_all_tests()
{
	tests_.clear();
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


std::unique_ptr<FileReaderDescriptor> FileReaderSelection::Impl::descriptor(
		const std::string &filename) const
{
	if (filename.empty())
	{
		throw FileReadException("Filename must not be empty");
	}

	for (auto& test : tests_)
	{
		test->set_filename(filename);
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
	auto desc = this->descriptor(filename);

	if (not desc)
	{
		return nullptr;
	}

	return desc->create_reader();
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


void FileReaderSelection::Impl::reset()
{
	tests_.clear();
	descriptors_.clear();
}


std::size_t FileReaderSelection::Impl::size() const
{
	return descriptors_.size();
}


bool FileReaderSelection::Impl::empty() const
{
	return descriptors_.empty();
}


FileReaderSelector& FileReaderSelection::Impl::use_selector()
{
	return *selector_;
}


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


std::unique_ptr<FileReaderDescriptor> FileReaderSelection::descriptor(
		const std::string &filename) const
{
	return impl_->descriptor(filename);
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


bool FileReaderSelection::empty() const
{
	return impl_->empty();
}

/// @}

} // namespace v_1_0_0

} // namespace arcsdec

