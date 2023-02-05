/**
 * \file
 *
 * \brief Implementation of a selection toolkit for FileReaderDescriptors
 */

#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"
#endif

#include <algorithm>    // for find_if
#include <cstddef>      // for size_t
#include <cstdint>      // for uint32_t, uint64_t, int64_t
#include <functional>   // for function
#include <memory>       // for unique_ptr
#include <string>       // for string, to_string
#include <utility>      // for move, make_unique

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp> // for ARCS_LOG, _WARNING, _DEBUG
#endif

#ifndef __LIBARCSDEC_DESCRIPTOR_HPP__
#include "descriptor.hpp"     // for FileReaderDescriptor
#endif

namespace arcsdec
{
inline namespace v_1_0_0
{

// DescriptorSet


DescriptorSet::DescriptorSet()
	: descriptors_ { /* empty */ }
{
	// empty
}


void DescriptorSet::insert(std::unique_ptr<FileReaderDescriptor> d)
{
	descriptors_.insert(std::make_pair(d->id(), std::move(d)));
}


std::unique_ptr<FileReaderDescriptor> DescriptorSet::get(const std::string &id)
	const
{
	auto p = descriptors_.find(id);
	if (p != descriptors_.end())
	{
		return p->second->clone();
	}

	return nullptr;
}


void DescriptorSet::traverse(
			std::function<void(const FileReaderDescriptor &)> func) const
{
	for (const auto& p : descriptors_)
	{
		func(*p.second);
	}
}


DescriptorSet::iterator DescriptorSet::begin()
{
	return descriptors_.begin();
}


DescriptorSet::iterator DescriptorSet::end()
{
	return descriptors_.end();
}


DescriptorSet::const_iterator DescriptorSet::begin() const
{
	return descriptors_.begin();
}


DescriptorSet::const_iterator DescriptorSet::end() const
{
	return descriptors_.end();
}


DescriptorSet::const_iterator DescriptorSet::cbegin() const
{
	return descriptors_.cbegin();
}


DescriptorSet::const_iterator DescriptorSet::cend() const
{
	return descriptors_.cend();
}


std::size_t DescriptorSet::size() const
{
	return descriptors_.size();
}


bool DescriptorSet::empty() const
{
	return descriptors_.empty();
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

	ARCS_LOG_DEBUG << "Descriptor '" << desc->name()
		<< "' does not match and is discarded";
	return false;
}


std::unique_ptr<FileReaderDescriptor> FileReaderSelector::select(
		const std::string &filename,
		const std::set<std::unique_ptr<FileTest>> &tests,
		const DescriptorSet &descs) const
{
	auto descriptor = do_select(filename, tests, descs);

	if (!descriptor)
	{
		ARCS_LOG_WARNING << "Could not select a matching descriptor";
		return nullptr;
	}

	ARCS_LOG_INFO << "Reader descriptor '" << descriptor->name()
		<< "' selected for file '" << filename << "'";

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
		const DescriptorSet &descs) const
{
	for (auto& desc : descs)
	{
		if (this->matches(filename, tests, desc.second))
		{
			ARCS_LOG(DEBUG1) << "First matching descriptor: '"
				<< desc.second->name()
				<< "'";

			return desc.second->clone();
		}
	}

	return nullptr;
}


/**
 * \internal
 *
 * \defgroup selectionImpl Implementation details for module 'selection'
 *
 * \ingroup selection
 * @{
 */

/**
 * \brief Implementation of FileReaderSelection.
 */
class FileReaderSelection::Impl final
{
public:

	Impl(std::unique_ptr<FileReaderSelector> selector)
		: selector_    { std::move(selector) }
		, tests_       { /* empty */ }
	{
		/* empty */
	}

	void set_selector(std::unique_ptr<FileReaderSelector> selector);

	const FileReaderSelector& selector() const;

	void register_test(std::unique_ptr<FileTest> testobj);

	std::unique_ptr<FileTest> unregister_test(
			const std::unique_ptr<FileTest> &test);

	std::size_t total_tests() const;

	bool no_tests() const;

	void remove_all_tests();

	std::unique_ptr<FileReaderDescriptor> get_descriptor(
			const std::string &filename, const DescriptorSet &set) const;

	std::unique_ptr<FileReader> get_reader(
			const std::string &filename, const DescriptorSet &set) const;

private:

	/**
	 * \brief Internal FileReaderSelector
	 */
	std::unique_ptr<FileReaderSelector> selector_;

	/**
	 * \brief Internal set of FileTests to be performed by the selector_
	 */
	std::set<std::unique_ptr<FileTest>> tests_;
};


void FileReaderSelection::Impl::set_selector(
		std::unique_ptr<FileReaderSelector> selector)
{
	selector_ = std::move(selector);
}


const FileReaderSelector& FileReaderSelection::Impl::selector() const
{
	return *selector_;
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


std::size_t FileReaderSelection::Impl::total_tests() const
{
	return tests_.size();
}


bool FileReaderSelection::Impl::no_tests() const
{
	return tests_.empty();
}


void FileReaderSelection::Impl::remove_all_tests()
{
	tests_.clear();
}


std::unique_ptr<FileReaderDescriptor>
FileReaderSelection::Impl::get_descriptor(const std::string &filename,
		const DescriptorSet &set) const
{
	if (filename.empty())
	{
		throw FileReadException("Filename must not be empty");
	}

	std::unique_ptr<FileReaderDescriptor> desc =
		selector_->select(filename, tests_, set);

	if (!desc)
	{
		ARCS_LOG_WARNING << "File format is unknown, no reader available.";

		return nullptr;
	}

	return desc;
}


std::unique_ptr<FileReader> FileReaderSelection::Impl::get_reader(
		const std::string &filename, const DescriptorSet &set) const
{
	auto d = this->get_descriptor(filename, set);
	return d ? d->create_reader() : nullptr;
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


void FileReaderSelection::set_selector(
		std::unique_ptr<FileReaderSelector> selector)
{
	impl_->set_selector(std::move(selector));
}


const FileReaderSelector& FileReaderSelection::selector() const
{
	return impl_->selector();
}


void FileReaderSelection::register_test(std::unique_ptr<FileTest> testobj)
{
	impl_->register_test(std::move(testobj));
}


std::unique_ptr<FileTest> FileReaderSelection::unregister_test(
		const std::unique_ptr<FileTest> &test)
{
	return impl_->unregister_test(test);
}


void FileReaderSelection::remove_all_tests()
{
	return impl_->remove_all_tests();
}


std::size_t FileReaderSelection::total_tests() const
{
	return impl_->total_tests();
}


bool FileReaderSelection::no_tests() const
{
	return impl_->no_tests();
}


std::unique_ptr<FileReaderDescriptor> FileReaderSelection::get_descriptor(
		const std::string &filename, const DescriptorSet &set) const
{
	return impl_->get_descriptor(filename, set);
}


std::unique_ptr<FileReader> FileReaderSelection::get_reader(
		const std::string &filename, const DescriptorSet &set) const
{
	return impl_->get_reader(filename, set);
}


// DefaultAudioSelection


struct DefaultAudioSelection final
{
	/**
	 * \brief Amount of bytes to read from the beginning of a file.
	 *
	 * This amount is sufficient to determine the file format and codec.
	 */
	const uint32_t TOTAL_BYTES_TO_READ = 44;
	// Why 44? => Enough for WAVE and every other metadata format.
	// We want to recognize container format, codec and CDDA format.
	// Consider RIFFWAVE/PCM: the first 12 bytes identify the container
	// format ('RIFF' + size + 'WAVE'), PCM format is encoded in bytes
	// 20+21, but validating CDDA requires to read the entire format
	// chunk (up to and including byte 36). Bytes 37-40 are the data
	// subchunk id and 41-44 the data subchunk size. This length is also
	// sufficient to identify all other formats currently supported.

	std::unique_ptr<FileReaderSelection> operator()() const;
};


std::unique_ptr<FileReaderSelection> DefaultAudioSelection::operator()()
	const
{
	auto selection = std::make_unique<FileReaderSelection>();
	selection->register_test(std::make_unique<FileTestName>());
	selection->register_test(std::make_unique<FileTestBytes>(0,
				TOTAL_BYTES_TO_READ));
	return selection;
}


// DefaultMetadataSelection


struct DefaultMetadataSelection final
{
	std::unique_ptr<FileReaderSelection> operator()() const;
};


std::unique_ptr<FileReaderSelection> DefaultMetadataSelection::operator()()
	const
{
	auto selection = std::make_unique<FileReaderSelection>();
	selection->register_test(std::make_unique<FileTestName>());
	return selection;
}


// FileReaderRegistry


FileReaderRegistry::FileReaderRegistry() = default;


const DescriptorSet* FileReaderRegistry::descriptors()
{
	return descriptors_.get();
}


const FileReaderSelection* FileReaderRegistry::default_audio_selection()
{
	if (!default_audio_selection_)
	{
		DefaultAudioSelection create;
		default_audio_selection_ = create();
	}

	return default_audio_selection_.get();
}


const FileReaderSelection* FileReaderRegistry::default_toc_selection()
{
	if (!default_toc_selection_)
	{
		DefaultMetadataSelection create;
		default_toc_selection_ = create();
	}

	return default_toc_selection_.get();
}


void FileReaderRegistry::add(std::unique_ptr<FileReaderDescriptor> d)
{
	if (!descriptors_)
	{
		descriptors_ = std::make_unique<DescriptorSet>();
	}

	descriptors_->insert(std::move(d));
}


std::unique_ptr<FileReaderDescriptor> FileReaderRegistry::call(
			FunctionReturningUniquePtr<FileReaderDescriptor> create)
{
	return create();
}


std::unique_ptr<DescriptorSet> FileReaderRegistry::descriptors_;


std::unique_ptr<FileReaderSelection>
	FileReaderRegistry::default_audio_selection_;


std::unique_ptr<FileReaderSelection>
	FileReaderRegistry::default_toc_selection_;


} // namespace v_1_0_0
} // namespace arcsdec

