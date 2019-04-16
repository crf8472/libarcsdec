/**
 * \file fileformats.cpp Implementation of a selection toolkit for FileReaders
 *
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


// FileFormat


FileFormat::~FileFormat() noexcept = default;


std::string FileFormat::name() const
{
	return this->do_name();
}


bool FileFormat::can_have_bytes(const std::vector<char> &bytes,
			const uint64_t &offset) const
{
	return this->do_can_have_bytes(bytes, offset);
}


bool FileFormat::can_have_suffix(const std::string &suffix) const
{
	return this->do_can_have_suffix(suffix);
}


std::unique_ptr<FileReader> FileFormat::create_reader() const
{
	return this->do_create_reader();
}


std::unique_ptr<FileFormat> FileFormat::clone() const
{
	return this->do_clone();
}


bool FileFormat::operator == (const FileFormat &rhs) const
{
	// FileFormats are stateless and hence equal iff they are of the same static
	// type

	return typeid(this) == typeid(rhs);
}


bool FileFormat::operator != (const FileFormat &rhs) const
{
	return not(this == &rhs);
}


// FileFormatTest


FileFormatTest::FileFormatTest()
	: filename_()
{
	// empty
}


FileFormatTest::FileFormatTest(const std::string &filename)
	: filename_(filename)
{
	// empty
}


FileFormatTest::~FileFormatTest() noexcept = default;


void FileFormatTest::set_filename(const std::string &filename)
{
	filename_ = filename;
}


const std::string& FileFormatTest::filename() const
{
	return filename_;
}


bool FileFormatTest::matches(const FileFormat &format) const
{
	return this->do_matches(format);
}


// FileFormatTestBytes


FileFormatTestBytes::FileFormatTestBytes(const uint64_t &offset,
		const uint32_t &length)
	: offset_(offset)
	, length_(length)
{
	// empty
}


bool FileFormatTestBytes::do_matches(const FileFormat &format) const
{
	auto bytes = this->read_bytes(this->filename(), offset_, length_);
	return format.can_have_bytes(bytes, offset_);
}


std::vector<char> FileFormatTestBytes::read_bytes(const std::string &filename,
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


// FileFormatTestSuffix


bool FileFormatTestSuffix::do_matches(const FileFormat &format) const
{
	auto suffix = this->get_suffix(this->filename());
	return format.can_have_suffix(suffix);
}


std::string FileFormatTestSuffix::get_suffix(const std::string &filename) const
{
	auto pos = filename.find_last_of(".");

	return pos == std::string::npos
		? filename
		: (pos < filename.length())
			? filename.substr(pos + 1, filename.length())
			: std::to_string(filename.back());
}


// FileFormatTestFormatname


FileFormatTestFormatname::FileFormatTestFormatname(
		const std::string &formatname)
	: formatname_(formatname)
{
	// empty
}


bool FileFormatTestFormatname::do_matches(const FileFormat &format) const
{
	return formatname_ == format.name();
}


// FileFormatSelector


FileFormatSelector::~FileFormatSelector() noexcept = default;


std::unique_ptr<FileFormat> FileFormatSelector::select(
		const std::set<std::unique_ptr<FileFormatTest>> &tests,
		const std::list<std::unique_ptr<FileFormat>> &formats) const
{
	return this->do_select(tests, formats);
}


std::unique_ptr<FileFormat> FileFormatSelector::do_select(
		const std::set<std::unique_ptr<FileFormatTest>> &tests,
		const std::list<std::unique_ptr<FileFormat>> &formats) const
{
	for (auto& format : formats)
	{
		ARCS_LOG_DEBUG << "Testing format: " << format->name();

		if (this->matches(tests, format))
		{
			ARCS_LOG_DEBUG << "Format matched";

			return format->clone();
			// Moving would Nullify the FileFormat in formats.
			// Copying using decltype would be okay since FileFormats are
			// defined to be stateless but cloning is cleanest.
		}
	}

	return nullptr;
}


bool FileFormatSelector::matches(
		const std::set<std::unique_ptr<FileFormatTest>> &tests,
		const std::unique_ptr<FileFormat> &format) const
{
	// The default implementation of matches() returns TRUE iff each test
	// passes (== AND). It could be overwritten by a version that returns TRUE
	// iff at least one test passes (== OR)

	for (const auto& test : tests)
	{
		ARCS_LOG_DEBUG << "Perform test";

		if (not test->matches(*format))
		{
			ARCS_LOG_DEBUG << "Test failed";
			return false;
		}
	}

	// Note that if no tests are registered, each FileFormat matches!
	// This means that whatever is first in enumerating the formats will be
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
	 * Add a file format for which a reader can be created
	 *
	 * \param[in] format A FileFormat
	 */
	void register_format(std::unique_ptr<FileFormat> format);

	/**
	 * Remove all formats that qualify as equivalent to the given format by
	 * '==' from the list of formats.
	 *
	 * \param[in] format The FileFormat to be removed
	 *
	 * \return Number of format instances removed.
	 */
	int remove_format(const FileFormat * format);

	/**
	 * Register a test for a FileFormat for the specified filename.
	 *
	 * \param[in] format_test The test to be registered
	 */
	void register_test(std::unique_ptr<FileFormatTest> format_test);

	/**
	 * Remove all tests that qualify as equivalent to the given test by
	 * '==' from the list of test.
	 *
	 * \param[in] format_test The FileFormatTest to be removed
	 *
	 * \return Number of test instances removed.
	 */
	int remove_test(const FileFormatTest * format_test);

	/**
	 * Removes all tests registered to this instance.
	 */
	void remove_all_tests();

	/**
	 * Sets the FileFormatSelector for this instance
	 *
	 * \param[in] selector The FileFormatSelector for this instance
	 */
	void set_selector(std::unique_ptr<FileFormatSelector> selector);

	/**
	 * Returns the internal FileFormatSelector of this instance
	 *
	 * \return The FileFormatSelector of this instance
	 */
	const FileFormatSelector& selector() const;

	/**
	 * Determine a matching FileFormat for the specified file.
	 *
	 * \param[in] filename Name of the file to determine a FileFormat for
	 *
	 * \return A FileFormat for the specified file
	 */
	std::unique_ptr<FileFormat> get_format(const std::string &filename) const;

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
	 * formats.
	 */
	void reset();

	// class is non-copy-assignable
	Impl& operator = (const Impl &) = delete;


protected:

	/**
	 * Return the FileFormatSelector of this instance for use in subclasses.
	 *
	 * \return The FileFormatSelector of this instance
	 */
	FileFormatSelector& use_selector();


private:

	/**
	 * Internal FileFormatSelector
	 */
	std::unique_ptr<FileFormatSelector> selector_;

	/**
	 * Internal set of FileFormatTests to performed by the selector_
	 */
	std::set<std::unique_ptr<FileFormatTest>> tests_;

	/**
	 * Internal list of FileFormats to match by the selector_
	 */
	std::list<std::unique_ptr<FileFormat>> file_formats_;
};


/// @}
/// \endcond IMPL_ONLY


FileReaderSelection::Impl::Impl()
	: selector_(std::make_unique<FileFormatSelector>())
	, tests_()
	, file_formats_()
{
	// empty
}


FileReaderSelection::Impl::~Impl() noexcept = default;


void FileReaderSelection::Impl::register_format(
		std::unique_ptr<FileFormat> format)
{
	file_formats_.push_back(std::move(format));
}


int FileReaderSelection::Impl::remove_format(const FileFormat * format)
{
	int counter = 0;

	auto end = file_formats_.end();
	for (auto ptr = file_formats_.begin(); ptr != end; ++ptr) // TODO correct
	{
		if (ptr->get() == format)
		{
			file_formats_.erase(ptr);
			++counter;
		}
	}

	return counter;
}


void FileReaderSelection::Impl::register_test(
		std::unique_ptr<FileFormatTest> format_test)
{
	tests_.insert(std::move(format_test));
}


int FileReaderSelection::Impl::remove_test(const FileFormatTest * test)
{
	int counter = 0;

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
		std::unique_ptr<FileFormatSelector> selector)
{
	selector_ = std::move(selector);
}


const FileFormatSelector& FileReaderSelection::Impl::selector() const
{
	return *selector_;
}


std::unique_ptr<FileFormat> FileReaderSelection::Impl::get_format(
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

	std::unique_ptr<FileFormat> format =
		selector_->select(tests_, file_formats_);

	if (not format)
	{
		ARCS_LOG_WARNING << "Container format or codec unknown.";

		return nullptr;
	}

	ARCS_LOG_INFO << "Input file format seems to be " << format->name();

	return format;
}


std::unique_ptr<FileReader> FileReaderSelection::Impl::for_file(
		const std::string &filename) const
{
	auto format = this->get_format(filename);

	if (not format)
	{
		return nullptr;
	}

	return format->create_reader();
}


std::unique_ptr<FileReader> FileReaderSelection::Impl::by_name(
		const std::string &name) const
{
	for (const auto& format : file_formats_)
	{
		if (name == format->name())
		{
			return format->create_reader();
		}
	}

	return nullptr;
}


void FileReaderSelection::Impl::reset()
{
	tests_.clear();
	file_formats_.clear();
}


FileFormatSelector& FileReaderSelection::Impl::use_selector()
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


void FileReaderSelection::register_format(std::unique_ptr<FileFormat> format)
{
	impl_->register_format(std::move(format));
}


int FileReaderSelection::remove_format(const FileFormat * format)
{
	return impl_->remove_format(format);
}


void FileReaderSelection::register_test(
		std::unique_ptr<FileFormatTest> format_test)
{
	impl_->register_test(std::move(format_test));
}


int FileReaderSelection::remove_test(const FileFormatTest * test)
{
	return impl_->remove_test(test);
}


void FileReaderSelection::remove_all_tests()
{
	return impl_->remove_all_tests();
}


void FileReaderSelection::set_selector(
		std::unique_ptr<FileFormatSelector> selector)
{
	impl_->set_selector(std::move(selector));
}


const FileFormatSelector& FileReaderSelection::selector() const
{
	return impl_->selector();
}


std::unique_ptr<FileFormat> FileReaderSelection::get_format(
		const std::string &filename) const
{
	return impl_->get_format(filename);
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

