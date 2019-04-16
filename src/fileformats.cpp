/**
 * \file fileformats.cpp Implementation of a selection toolkit for FileReaders
 */


#ifndef __LIBARCSDEC_FILEFORMATS_HPP__
#include "fileformats.hpp"
#endif

#include <cstdint>
#include <fstream>
#include <list>
#include <memory>
#include <string>
#include <vector>

#ifndef __LIBARCS_CALCULATE_HPP__
#include <arcs/calculate.hpp>
#endif
#ifndef __LIBARCS_LOGGING_HPP__
#include <arcs/logging.hpp>
#endif


namespace arcs
{


// FileReader


FileReader::FileReader() = default;


FileReader::~FileReader() noexcept = default;


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


bool FileReaderDescriptor::accepts_suffix(const std::string &suffix) const
{
	return this->do_accepts_suffix(suffix);
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
	const uint64_t &offset, const uint32_t &length) const
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

		ARCS_LOG_ERROR << "Failed to read from file: " << f.what();
		bytes.resize(0);

		throw FileReadException(f.what(), total_bytes_read + 1);
	}

	return bytes;
}


// FileTestSuffix


bool FileTestSuffix::do_matches(const FileReaderDescriptor &desc) const
{
	auto suffix = this->get_suffix(this->filename());
	return desc.accepts_suffix(suffix);
}


std::string FileTestSuffix::get_suffix(const std::string &filename) const
{
	auto pos = filename.find_last_of(".");

	return pos == std::string::npos
		? filename
		: (pos < filename.length())
			? filename.substr(pos + 1, filename.length())
			: std::to_string(filename.back());
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
 * \cond IMPL_ONLY
 *
 * \internal \defgroup fileformatsImpl Implementation details for building and selecting file types
 *
 * \ingroup fileformats
 * @{
 */


/**
 * Implementation of FileReaderSelection
 */
class FileReaderSelection::Impl
{

public:

	/**
	 * Default constructor
	 */
	Impl();

	// class is non-copy-constructible
	Impl(const Impl &) = delete;

	/**
	 * Virtual default destructor
	 */
	virtual ~Impl() noexcept;

	/**
	 * Add a file descriptor for which a reader can be created
	 *
	 * \param[in] desc A FileReaderDescriptor
	 */
	void add_descriptor(std::unique_ptr<FileReaderDescriptor> desc);

	/**
	 * Remove all descriptors that qualify as equivalent to the given descriptor
	 * by '==' from the list of descriptors.
	 *
	 * \param[in] desc The FileReaderDescriptor to be removed
	 *
	 * \return Number of descriptors instances removed.
	 */
	int remove_descriptor(const FileReaderDescriptor* desc);

	/**
	 * Register a test for a FileReaderDescriptor for the specified filename.
	 *
	 * \param[in] testobj The test to be registered
	 */
	void register_test(std::unique_ptr<FileTest> testobj);

	/**
	 * Remove all tests that qualify as equivalent to the given test by
	 * '==' from the list of test.
	 *
	 * \param[in] testobj The FileTest to be removed
	 *
	 * \return Number of test instances removed.
	 */
	int unregister_test(const FileTest * testobj);

	/**
	 * Removes all tests registered to this instance.
	 */
	void remove_all_tests();

	/**
	 * Sets the FileReaderSelector for this instance
	 *
	 * \param[in] selector The FileReaderSelector for this instance
	 */
	void set_selector(std::unique_ptr<FileReaderSelector> selector);

	/**
	 * Returns the internal FileReaderSelector of this instance
	 *
	 * \return The FileReaderSelector of this instance
	 */
	const FileReaderSelector& selector() const;

	/**
	 * Determine a matching FileReaderDescriptor for the specified file.
	 *
	 * \param[in] filename Name of the file to determine a descriptor for
	 *
	 * \return A FileReaderDescriptor for the specified file
	 */
	std::unique_ptr<FileReaderDescriptor> descriptor(
			const std::string &filename) const;

	/**
	 * Create an opaque FileReader for the given file.
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
	 * Return the FileReader specified by its name.
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
	 * Reset this instance to its initial state, removing all tests and
	 * descriptors.
	 */
	void reset();

	// class is non-copy-assignable
	Impl& operator = (const Impl &) = delete;


protected:

	/**
	 * Return the FileReaderSelector of this instance for use in subclasses.
	 *
	 * \return The FileReaderSelector of this instance
	 */
	FileReaderSelector& use_selector();


private:

	/**
	 * Internal FileReaderSelector
	 */
	std::unique_ptr<FileReaderSelector> selector_;

	/**
	 * Internal set of FileTests to performed by the selector_
	 */
	std::set<std::unique_ptr<FileTest>> tests_;

	/**
	 * Internal list of FileReaderDescriptors to match by the selector_
	 */
	std::list<std::unique_ptr<FileReaderDescriptor>> descriptors_;
};


/// @}
/// \endcond IMPL_ONLY


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
	for (auto ptr = descriptors_.begin(); ptr != end; ++ptr) // TODO correct
	{
		if (ptr->get() == desc)
		{
			descriptors_.erase(ptr);
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

	for (auto& t: tests_)
	{
		if (t.get() == test)
		{
			tests_.erase(t);
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


void FileReaderSelection::Impl::reset()
{
	tests_.clear();
	descriptors_.clear();
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


void FileReaderSelection::reset()
{
	impl_->reset();
}

} // namespace arcs

