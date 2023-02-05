#ifndef __LIBARCSDEC_SELECTION_HPP__
#define __LIBARCSDEC_SELECTION_HPP__

/**
 * \file
 *
 * \brief Toolkit for selecting file readers
 */

#include <cstddef>     // for size_t
#include <cstdint>     // for uint32_t, uint64_t, int64_t
#include <functional>  // for function
#include <map>         // for map
#include <memory>      // for unique_ptr, make_unique
#include <set>         // for set
#include <string>      // for string
#include <type_traits> // for is_convertible
#include <utility>     // for pair, move, make_pair, forward

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>   // for ARCS_LOG_WARNING, ARCS_LOG_DEBUG
#endif

#ifndef __LIBARCSDEC_DESCRIPTOR_HPP__
#include "descriptor.hpp"       // for FileReaderDescriptor
#endif


namespace arcsdec
{
inline namespace v_1_0_0
{


/**
 * \defgroup selction API for selecting FileReaders
 *
 * \brief API for selecting \link FileReader FileReaders\endlink for given
 * input files.
 *
 * Abstract class FileReaderSelection provides the API for the mechanism to
 * check a specified input file for a matching FileReaderDescriptor. If a
 * matching FileReaderDescriptor is found, an instance of this descriptor is
 * returned which is then used to create the concrete FileReader instance.
 *
 * A FileReaderSelection holds a a list of tests to perform on the input file
 * and a list of supported FileReaderDescriptors. Internally, it uses an
 * instance of FileReaderSelector to select a concrete FileReaderDescriptor.
 * FileReaderSelector performs the selection obeying a certain selection policy.
 * The default FileReaderSelector just selects the first FileReaderDescriptor in
 * the list of supported FileReaderDescriptors that passes all tests in the
 * provided list of tests.
 *
 * A FileTest implements a single test. It may or may not open the file to test.
 *
 * The \ref AudioReader and \ref MetadataParser APIs are built on this API.
 *
 * @{
 */

/**
 * \brief A set of FileReaderDescriptors.
 *
 * The set can be accessed via getting one of its members via an id. The set can
 * be traversed thereby applying a specified function to each member. The set
 * can be added a member. It provides iterators and size information as well as
 * an emptyness check.
 */
class DescriptorSet final
{
private:

	/**
	 * \brief Map with the descriptor ID as a key and the type as value
	 */
	std::map<std::string, std::unique_ptr<FileReaderDescriptor>> descriptors_;

public:

	DescriptorSet();

	/**
	 * \brief Add a descriptor to the set
	 *
	 * \param[in] d Descriptor to be added to the set
	 */
	void insert(std::unique_ptr<FileReaderDescriptor> d);

	/**
	 * \brief Get a descriptor by id
	 *
	 * \param[in] id The id of the descriptor to get
	 */
	std::unique_ptr<FileReaderDescriptor> get(const std::string &id) const;

	/**
	 * \brief Traverse all descriptors and apply the specified
	 * function \c func on each of them.
	 *
	 * This enables listing or querying the set of added descriptors.
	 *
	 * \param[in] func Function to apply to each descriptor.
	 */
	void traverse(std::function<void(const FileReaderDescriptor &)> func) const;

	/**
	 * \brief Total number of descriptors in the set.
	 *
	 * \return Total number of descriptors in the set.
	 */
	std::size_t size() const;

	/**
	 * \brief Returns \c TRUE iff the set is empty, otherwise \c FALSE.
	 *
	 * \return \c TRUE iff the set is empty, otherwise \c FALSE.
	 */
	bool empty() const;


	using iterator = decltype(descriptors_)::iterator;
	using const_iterator = decltype(descriptors_)::const_iterator;

	iterator begin();
	iterator end();

	const_iterator begin() const;
	const_iterator end() const;

	const_iterator cbegin() const;
	const_iterator cend() const;
};


class FileTest;
bool operator == (const FileTest &lhs, const FileTest &rhs);

/**
 * \brief A test whether a given FileReaderDescriptor matches a criterion.
 *
 * FileTest instances are polymorphically comparable to support their use
 * in containers.
 *
 * \note
 * Instances of this class are non-copyable and non-movable but provide
 * protected special members for copy and move that can be used in subclasses.
 */
class FileTest : public Comparable<FileTest>
{
public:

	friend bool operator == (const FileTest &lhs, const FileTest &rhs);

	/**
	 * \brief Default constructor.
	 */
	FileTest();

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~FileTest() noexcept;

	/**
	 * \brief Short description of this test.
	 *
	 * \returns Description of this test
	 */
	std::string description() const;

	/**
	 * \brief Perform test for a given pair of descriptor instance and filename.
	 *
	 * \param[in] desc     The FileReaderDescriptor to match
	 * \param[in] filename The filename to test
	 *
	 * \return TRUE iff the descriptor matches the criterion of this test
	 */
	bool passes(const FileReaderDescriptor &desc, const std::string &filename)
		const;

protected:

	FileTest(const FileTest &) = default;
	FileTest& operator = (const FileTest &) = default;

	FileTest(FileTest &&) noexcept = default;
	FileTest& operator = (FileTest &&) noexcept = default;

	/**
	 * \brief TRUE if instance equals \c rhs.
	 *
	 * \param[in] rhs Right hand side of the comparison
	 *
	 * \returns TRUE if instance equals \c rhs.
	 */
	virtual bool equals(const FileTest &rhs) const
	= 0;

private:

	/**
	 * \brief Implements FileTest::description().
	 */
	virtual std::string do_description() const
	= 0;

	/**
	 * \brief Implements FileTest::passes().
	 */
	virtual bool do_passes(const FileReaderDescriptor &desc,
			const std::string &filename) const
	= 0;
};


/**
 * \brief Test for matching a byte sequence from a file.
 */
class FileTestBytes final : public FileTest
{
public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] offset The offset in bytes where this sequence starts
	 * \param[in] length Number of bytes in the sequence
	 */
	FileTestBytes(const uint32_t &offset, const uint32_t &length);

	/**
	 * \brief Offset.
	 *
	 * \return Offset
	 */
	uint32_t offset() const;

	/**
	 * \brief Length.
	 *
	 * \return Length
	 */
	uint32_t length() const;

protected:

	bool equals(const FileTest &) const override;

private:

	std::string do_description() const override;

	bool do_passes(const FileReaderDescriptor &desc,
			const std::string &filename) const override;

	/**
	 * \brief Byte offset of the byte sequence in the file.
	 */
	uint32_t offset_;

	/**
	 * \brief Number of bytes to read from the start position.
	 */
	uint32_t length_;
};


/**
 * \brief Test for matching an actual filename.
 */
class FileTestName final : public FileTest
{
private:

	std::string do_description() const override;

	bool do_passes(const FileReaderDescriptor &desc,
			const std::string &filename) const override;

protected:

	bool equals(const FileTest &) const override;
};


/**
 * \brief A selection mechanism for a FileReaderSelection.
 *
 * A FileReaderSelector applies FileTests to FileReaderDescriptors to select
 * a descriptor with a certain test result.
 *
 * It implements two different decisions. Implementing matches() defines which
 * descriptors are candidates to be selected. Implementing select() defines
 * which of the matching candidates is concretely selected.
 *
 * \note
 * Instances of this class are non-copyable and non-movable but provide
 * protected special members for copy and move that can be used in subclasses.
 *
 * \see DefaultSelector
 */
class FileReaderSelector
{
public:

	/**
	 * \brief Default constructor.
	 */
	FileReaderSelector();

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~FileReaderSelector() noexcept;

	/**
	 * \brief Decide whether a descriptor matches the given set of tests.
	 *
	 * This defines the set of selection candidates.
	 *
	 * \param[in] filename Name of the file to perform the tests on
	 * \param[in] tests    Set of tests to perform
	 * \param[in] desc     The descriptor to check
	 *
	 * \return TRUE iff the descriptor matches the given set of tests
	 */
	bool matches(
		const std::string &filename,
		const std::set<std::unique_ptr<FileTest>> &tests,
		const std::unique_ptr<FileReaderDescriptor> &desc) const;

	/**
	 * \brief Selects a descriptor using tests.
	 *
	 * This defines the selection of a concrete selection candidate.
	 *
	 * The concrete implementation is supposed to use \c matches() to
	 * decide whether a descriptor is matched.
	 *
	 * \param[in] filename Name of the file to select a descriptor for
	 * \param[in] tests    Set of tests to perform
	 * \param[in] descs    Set of descriptors to select from
	 *
	 * \return A FileReaderDescriptor
	 */
	std::unique_ptr<FileReaderDescriptor> select(
			const std::string &filename,
			const std::set<std::unique_ptr<FileTest>> &tests,
			const DescriptorSet &descs)
		const;

protected:

	FileReaderSelector(const FileReaderSelector &rhs) = default;
	FileReaderSelector& operator = (const FileReaderSelector &rhs) = default;

	FileReaderSelector(FileReaderSelector &&rhs) = default;
	FileReaderSelector& operator = (FileReaderSelector &&rhs) = default;

private:

	/**
	 * \brief Implements FileReaderSelector::matches().
	 */
	virtual bool do_matches(
		const std::string &filename,
		const std::set<std::unique_ptr<FileTest>> &tests,
		const std::unique_ptr<FileReaderDescriptor> &desc) const
	= 0;

	/**
	 * \brief Implements FileReaderSelector::select().
	 */
	virtual std::unique_ptr<FileReaderDescriptor> do_select(
			const std::string &filename,
			const std::set<std::unique_ptr<FileTest>> &tests,
			const DescriptorSet &descs)
		const
	= 0;
};


/**
 * \brief Default selector.
 *
 * Selects the first descriptor from the descriptor list that passes all tests.
 *
 * Note that if no tests are passed, each FileReaderDescriptor matches!
 * This means that whatever is first descriptor in the sequence of descriptors
 * will be matched and create the FileReader.
 */
class DefaultSelector final : public FileReaderSelector
{
private:

	std::unique_ptr<FileReaderDescriptor> do_select(
			const std::string &filename,
			const std::set<std::unique_ptr<FileTest>> &tests,
			const DescriptorSet &descs)
		const override;

	bool do_matches(
		const std::string &filename,
		const std::set<std::unique_ptr<FileTest>> &tests,
		const std::unique_ptr<FileReaderDescriptor> &desc) const override;
};


/**
 * \brief Traversable selection of available FileReader descriptors.
 *
 * Default constructor initializes the selection with a DefaultSelector.
 *
 * \note
 * Instances of this class are non-copyable but movable.
 */
class FileReaderSelection final
{
public:

	/**
	 * \brief Constructor.
	 */
	FileReaderSelection();

	FileReaderSelection(FileReaderSelection &&) noexcept = default;
	FileReaderSelection& operator = (FileReaderSelection &&) noexcept = default;

	/**
	 * \brief Default destructor.
	 */
	~FileReaderSelection() noexcept; // Required by pimpl

	/**
	 * \brief Set the FileReaderSelector for this instance.
	 *
	 * \param[in] selector The FileReaderSelector for this instance
	 */
	void set_selector(std::unique_ptr<FileReaderSelector> selector);

	/**
	 * \brief Return the FileReaderSelector of this instance.
	 *
	 * \return The FileReaderSelector of this instance
	 */
	const FileReaderSelector& selector() const;

	/**
	 * \brief Register a test.
	 *
	 * \param[in] testobj The test to be registered
	 */
	void register_test(std::unique_ptr<FileTest> testobj);

	/**
	 * \brief Remove all matching tests.
	 *
	 * Removes all tests from the selection that qualify as equivalent to
	 * \c test by testing equality with '=='.
	 *
	 * \param[in] test The FileTest to be removed
	 *
	 * \return Number of test instances removed.
	 */
	std::unique_ptr<FileTest> unregister_test(
			const std::unique_ptr<FileTest> &test);

	/**
	 * \brief Removes all tests registered to this instance.
	 */
	void remove_all_tests();

	/**
	 * \brief Number of registered tests.
	 *
	 * \return The number of registered tests in this selection.
	 */
	std::size_t total_tests() const;

	/**
	 * \brief TRUE if this selection has no tests registered.
	 *
	 * \return TRUE if this selection has no tests registered.
	 */
	bool no_tests() const;

	/**
	 * \brief Determine a matching FileReaderDescriptor for the specified file.
	 *
	 * \param[in] filename Name of the file to determine a descriptor for
	 *
	 * \return A FileReaderDescriptor for the specified file
	 */
	std::unique_ptr<FileReaderDescriptor> get_descriptor(
			const std::string &filename, const DescriptorSet &set) const;

	/**
	 * \brief Create an opaque FileReader for the given file.
	 *
	 * Will return \c nullptr if the file cannot be read or the filename is
	 * empty. The FileReader returned is selected by \c select_descriptor().
	 *
	 * \param[in] filename Name of the file to create the reader for
	 *
	 * \return A FileReader for the specified file
	 */
	std::unique_ptr<FileReader> get_reader(
			const std::string &filename, const DescriptorSet &set) const;

private:

	// forward declaration
	class Impl;

	/**
	 * \brief Private implementation of this FileReaderSelection.
	 */
	std::unique_ptr<FileReaderSelection::Impl> impl_;
};


/**
 * \internal
 * \brief Function pointer to function returning std::unique_ptr<T>.
 *
 * \tparam T The type the return unique_ptr holds
 */
template <class T>
using FunctionReturningUniquePtr = std::unique_ptr<T>(*)();


/**
 * \brief A global registry holding all compiled-in FileReaderDescriptors.
 *
 * A FileReaderDescriptor instance is registered via instantiating the template
 * subclass RegisterDescriptor with the appropriate FileReaderDescriptor type.
 *
 * \note
 * Instances of this class are non-copyable and non-movable but provide
 * protected special members for copy and move that can be used in subclasses.
 *
 * \note
 * This class is non-final but does not support polymorphic deletion.
 *
 * \see RegisterDescriptor
 */
class FileReaderRegistry
{
public:

	/**
	 * \brief Default constructor.
	 */
	FileReaderRegistry();

	/**
	 * \brief Set of available descriptors for FileReaders.
	 *
	 * \return Set of available descriptors for FileReaders.
	 */
	static const DescriptorSet* descriptors();

	/**
	 * \brief Create a default audio selection.
	 *
	 * This is used to initialize TOCParser and the Calculators with the same
	 * default selection setup.
	 *
	 * \return The default selection for determining an AudioReader
	 */
	static const FileReaderSelection* default_audio_selection();

	/**
	 * \brief Create a default toc selection
	 *
	 * This is used to initialize TOCParser and the Calculators with the same
	 * default selection setup.
	 *
	 * \return The default selection for determining a MetadataParser
	 */
	static const FileReaderSelection* default_toc_selection();

protected:

	~FileReaderRegistry() noexcept = default;

	FileReaderRegistry(const FileReaderRegistry &) = default;
	FileReaderRegistry& operator = (const FileReaderRegistry &) = default;

	FileReaderRegistry(FileReaderRegistry &&) noexcept = default;
	FileReaderRegistry& operator = (FileReaderRegistry &&) noexcept = default;

	/**
	 * \brief Add a descriptor to this registry.
	 *
	 * \param[in] d Descriptor to add.
	 */
	static void add(std::unique_ptr<FileReaderDescriptor> d);

	/**
	 * \brief Instantiate the FileReader with the given name.
	 *
	 * \param[in] create Function pointer to create the instance
	 *
	 * \return Instance returned by \c create
	 */
	static std::unique_ptr<FileReaderDescriptor> call(
			FunctionReturningUniquePtr<FileReaderDescriptor> create);

private:

	static std::unique_ptr<DescriptorSet> descriptors_;

	static std::unique_ptr<FileReaderSelection> default_audio_selection_;

	static std::unique_ptr<FileReaderSelection> default_toc_selection_;
};


namespace details
{

/**
 * \brief Downcast a FileReader to a specialized ReaderType.
 *
 * The operation is safe: if the cast fails, the input pointer is returned
 * unaltered as second element of the pair, together with a nullptr as casting
 * result. If the cast succeeds, the casted pointer is returned together with
 * a nullptr as second element.
 */
template<typename ReaderType>
auto cast_reader(std::unique_ptr<FileReader> file_reader) noexcept
	-> std::pair<std::unique_ptr<ReaderType>, std::unique_ptr<FileReader>>
{
	if (!file_reader)
	{
		return std::make_pair(nullptr, std::move(file_reader));
	}

	// Create ReaderType manually by downcasting and reassignment

	FileReader *file_reader_rptr = file_reader.get();
	ReaderType *reader_type_rptr = nullptr;

	// Dry run:
	// Casting succeeds iff the FileReader created is in fact a ReaderType.
	// If not, the file is not supported by file_reader, so bail out.
	try
	{
		reader_type_rptr = dynamic_cast<ReaderType*>(file_reader_rptr);

	} catch (...) // std::bad_cast is possible
	{
		ARCS_LOG_WARNING <<
				"Failed to safely cast FileReader pointer to ReaderType";

		return std::make_pair(nullptr, std::move(file_reader));
	}

	if (!reader_type_rptr)
	{
		ARCS_LOG_WARNING <<
				"Casting FileReader pointer to ReaderType resulted in nullptr";

		return std::make_pair(nullptr, std::move(file_reader));
	}

	auto readertype_uptr = std::make_unique<ReaderType>(nullptr);

	// file_reader was left unmodified until now

	readertype_uptr.reset(dynamic_cast<ReaderType*>(file_reader.release()));
	// release() + reset() are both 'noexcept'

	return std::make_pair(std::move(readertype_uptr), nullptr);
}


/**
 * \brief Functor to safely create a unique_ptr to a downcasted FileReader.
 *
 * It will either provide a valid FileReader of the requested type or will
 * throw. It will never silently fail nor provide a nullptr.
 *
 * \warning
 * This class is non-abstract and non-final and does not support polymorphic
 * deletion. It is intended to be used for private inheritance to stateless
 * subclasses.
 *
 * \tparam ReaderType Concrete type of the required FileReader
 *
 * \see CreateAudioReader
 * \see CreateMetadataParser
 */
template <class ReaderType>
struct CreateReader
{
	/**
	 * \brief Return a unique_ptr to an instance of the specified \c ReaderType.
	 *
	 * \param[in] selection The FileReaderSelection to choose from
	 * \param[in] filename  The name of the file to choose a FileReader
	 */
	auto operator()(const FileReaderSelection &selection,
			const DescriptorSet &descriptors, const std::string &filename) const
		-> std::unique_ptr<ReaderType>
	{
		ARCS_LOG_DEBUG << "Recognize format of input file '" << filename << "'";

		auto file_reader = selection.get_reader(filename, descriptors);

		if (!file_reader)
		{
			throw InputFormatException("Could not identify file format: '"
					+ filename + "'");
		}

		auto p = details::cast_reader<ReaderType>(std::move(file_reader));

		if (!p.first)
		{
			throw InputFormatException("Could not acquire reader for file: "
					+ filename);
		}

		auto reader { std::move(p.first) };

		// XXX Return std::move(p.first) is quirky since it prevents RVO.
		// Omitting the move and return p.first directly causes compile error
		// since we try in fact to copy-construct a MetadataParser or
		// AudioReader. Both classes are abstract.

		return reader;
	}

protected:

	/**
	 * \brief Default destructor.
	 */
	~CreateReader() noexcept = default;
};


/**
 * \brief Instantiate FileReaderDescriptor.
 *
 * \tparam T    The type to instantiate
 * \tparam Args The constructor arguments
 */
template <class T, typename... Args>
std::unique_ptr<FileReaderDescriptor> make_descriptor(Args&&... args)
{
	static_assert(std::is_convertible<T*, FileReaderDescriptor*>::value,
			"Cannot convert type to FileReaderDescriptor");

	return std::make_unique<T>(std::forward<Args>(args)...);
}

} // namespace details


/**
 * \brief Register a FileReaderDescriptor type.
 *
 * \note This is a quite convenient way to register FileReaderDescriptors
 * without having to keep a global singleton.
 *
 * \tparam D The descriptor type to register
 */
template <class D>
class RegisterDescriptor final : private FileReaderRegistry
{
public:

	/**
	 * \brief Constructor
	 *
	 * Registers a descriptor of the template type
	 */
	RegisterDescriptor()
	{
		add(call(&details::make_descriptor<D>));
	}
};

/// @}

} // namespace v_1_0_0
} // namespace arcsdec

#endif

