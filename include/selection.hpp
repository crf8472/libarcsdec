#ifndef __LIBARCSDEC_SELECTION_HPP__
#define __LIBARCSDEC_SELECTION_HPP__

/**
 * \file
 *
 * \brief Toolkit for selecting file readers by format and codec.
 */

#include <memory>        // for unique_ptr, make_unique
#include <set>           // for set
#include <string>        // for string
#include <type_traits>   // for is_convertible
#include <unordered_map> // for unordered_map
#include <utility>       // for pair, move, make_pair, forward
#include <vector>        // for vector

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
 * \defgroup selection API for selecting FileReaders
 *
 * \brief API for selecting \link FileReader FileReaders\endlink for given
 * input files.
 *
 * A concrete DescriptorPreference assigns a preference value to each descriptor
 * based on the specified Format and Codec. It represents a preference to use
 * the descriptor in question to read a file with this Format and Codec.
 *
 * A concrete FileReaderSelector can select a matching FileReaderDescriptor by
 * a pair of Format and Codec from a list of available FileReaderDescriptors.
 * The concrete selection mechanism is implemented by the subclass.
 *
 * FileReaderSelection provides an API for selecting a FileReaderDescriptor
 * from a set of available descriptors. It uses a concrete DescriptorPreference
 * and a concrete FileReaderSelector to actually select the descriptor based on
 * the preference. An instance of the selected descriptor is returned which can
 * then create the concrete FileReader instance.
 *
 * A convenience interface to this entire mechanism is provided by functions
 * select_descriptor() and select_reader().
 *
 * Class FileReaderRegistry holds the set of available FileReaderDescriptors as
 * well as the set of supported \link Format Formats\endlink. It also defines
 * default selections for Metadata/ToC formats as well as audio formats.
 *
 * @{
 */


/**
 * \brief Interface for a descriptor preference.
 *
 * A preference is a numerically expressed likeliness that a descriptor should
 * be selected for the given pair of Format and Codec.
 *
 * The actual \c type of a preference value is an unsigned integer type.
 * The minimal possible preference is expressed by MIN_PREFERENCE which can be
 * taken for 'no actual preference'. The maximal possible preference is
 * MAX_PREFERENCE.
 *
 * \see DefaultPreference
 */
class DescriptorPreference
{
public:

	/**
	 * \brief An unsigned integer type to express the actual preference value.
	 */
	using type = unsigned int;

	/**
	 * \brief Minimal preference value.
	 */
	constexpr static type MIN_PREFERENCE = 0;

	/**
	 * \brief Maximal preference value.
	 */
	constexpr static type MAX_PREFERENCE = 100;

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~DescriptorPreference() noexcept;

	/**
	 * \brief Get a preference of one descriptor for a specified pair
	 * of Format and Codec.
	 *
	 * \param[in] format  File format
	 * \param[in] codec   Audio codec
	 * \param[in] desc    The descriptor to check
	 *
	 * \return Preference value to read format and codec with this descriptor
	 */
	type preference(const Format format, const Codec codec,
		const FileReaderDescriptor &desc) const;

private:

	virtual type do_preference(const Format format, const Codec codec,
		const FileReaderDescriptor &desc) const
	= 0;
};


/**
 * \brief DescriptorPreference for the most specific descriptor (with least
 * supported Formats and Codecs).
 *
 * A default preference value for the given input. The more formats and
 * codecs a FileReader supports, the higher is the penalty subtracted from its
 * preference for a given Format and Codec.
 *
 * This prefers specialized readers (like libFLAC) over general multi-input
 * readers (like ffmpeg).
 *
 * \note
 * DefaultPreference is currently <b>not</b> the actual default preference.
 * DefaultPreference checks for acceptance of the Codec and therefore the
 * concrete descriptor decides whether it respects Codec::UNKNOWN. Since Codec
 * recognition is currently not implemented, the input will mostly be UNKNOWN
 * which currently has to be accepted. Therefore, the current default is
 * \ref FormatPreference.
 */
class DefaultPreference final : public DescriptorPreference
{
	type do_preference(const Format format, const Codec codec,
		const FileReaderDescriptor &desc) const override;
};


/**
 * \brief DescriptorPreference that is always
 * DescriptorPreference::MIN_PREFERENCE.
 *
 * \note
 * Any input is ignored. This preference model can be combined with selectors
 * that do not require a preference model, e.g. IdSelector.
 */
class MinPreference final : public DescriptorPreference
{
	type do_preference(const Format format, const Codec codec,
		const FileReaderDescriptor &desc) const override;
};


/**
 * \brief DescriptorPreference for the most specific descriptor that accepts the
 * Format.
 *
 * \note
 * The codec is ignored. This preference model is used as the default model
 * while codec recognition is not fully implemented. Once codec is fully
 * supported, DefaultPreference will be the default instead of FormatPreference.
 */
class FormatPreference final : public DescriptorPreference
{
	type do_preference(const Format format, const Codec codec,
		const FileReaderDescriptor &desc) const override;
};


/**
 * \brief Type for the container of available FileReaderDescriptor instances.
 *
 * A single FileReaderDescriptor can be requested by its id.
 * FileReaderDescriptor instances come without an inherent ordering.
 */
using FileReaders = std::unordered_map<std::string,
		std::unique_ptr<FileReaderDescriptor>>;


/**
 * \brief Interface for a selector on a set of FileReaderDescriptor instances.
 *
 * \see DefaultSelector
 */
class FileReaderSelector
{
public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~FileReaderSelector() noexcept;

	/**
	 * \brief Selects a descriptor for the specified Format and Codec.
	 *
	 * The concrete implementation is supposed to use \c pref_model to
	 * establish a preference ordering on the set of descriptors. Based on
	 * this ordering the implementation of select() is free to decide which
	 * position of the ordering is the one to pick.
	 *
	 * \param[in] format   File format
	 * \param[in] codec    Audio codec
	 * \param[in] descs    Set of descriptors to select from
	 * \param[in] pref_model Preference model for selecting descriptors
	 *
	 * \return A FileReaderDescriptor that accepts \c format and \c codec
	 */
	std::unique_ptr<FileReaderDescriptor> select(const Format format,
			const Codec codec, const FileReaders &descs,
			const DescriptorPreference &pref_model) const;

private:

	virtual std::unique_ptr<FileReaderDescriptor> do_select(
		const Format format, const Codec codec, const FileReaders &descs,
		const DescriptorPreference &pref_model) const
	= 0;
};


/**
 * \brief FileReaderSelector for first descriptor (in order of occurrence) with
 * highest preference.
 *
 * Assigns a preference to each FileReaderDescriptor and returns the first one
 * (in order of occurrence) that has the highest occurring preference based on
 * the actual DescriptorPreference.
 */
class DefaultSelector final : public FileReaderSelector
{
private:

	std::unique_ptr<FileReaderDescriptor> do_select(const Format format,
			const Codec codec, const FileReaders &descs,
			const DescriptorPreference &pref_model) const override;
};


/**
 * \brief FileReaderSelector for a specific descriptor id.
 *
 * Ignore any preference and return exactly either the reader with the specified
 * id if such a reader is available or otherwise nullptr.
 */
class IdSelector final : public FileReaderSelector
{
public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] reader_id Select reader with this id, if available
	 */
	IdSelector(const std::string &reader_id);

	/**
	 * \brief Reader id to select.
	 *
	 * \return Id of the reader selected by this selector.
	 */
	std::string reader_id() const;

private:

	/**
	 * \brief Internal id of the reader to select.
	 */
	std::string reader_id_;

	std::unique_ptr<FileReaderDescriptor> do_select(const Format format,
			const Codec codec, const FileReaders &descs,
			const DescriptorPreference &pref_model) const override;
};


/**
 * \brief Interface to select a FileReaderDescriptor by Format and Codec.
 */
class FileReaderSelection
{
public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~FileReaderSelection() noexcept;

	/**
	 * \brief Selects a descriptor for the specified Format and Codec.
	 *
	 * \param[in] format   File format
	 * \param[in] codec    Audio codec
	 * \param[in] descs    Set of descriptors to select from
	 *
	 * \return A FileReaderDescriptor that accepts \c format and \c codec
	 */
	std::unique_ptr<FileReaderDescriptor> get(const Format format,
			const Codec codec, const FileReaders &descs) const;

private:

	virtual std::unique_ptr<FileReaderDescriptor> do_get(const Format format,
			const Codec codec, const FileReaders &descs) const
	= 0;
};


/**
 * \brief FileReaderSelection of FileReaderDescriptors.
 *
 * Use a concrete preference and selector type to determine a FileReader
 * for a specified input file.
 *
 * \tparam P Concrete DescriptorPreference type
 * \tparam S Concrete FileReaderSelector type
 */
template <typename P, typename S>
class FileReaderPreferenceSelection final : public FileReaderSelection
{
public:

	/**
	 * \brief The DescriptorPreference type.
	 */
	using preference_type = P;

	/**
	 * \brief The FileReaderSelector type.
	 */
	using selector_type = S;

	/**
	 * \brief Constructor.
	 *
	 * \tparam Args Arguments passed to the selector's constructor.
	 */
	template <typename... Args>
	inline FileReaderPreferenceSelection(Args&&... args)
		: preference_ { /* empty */ }
		, selector_   { std::forward<Args>(args)... }
	{
		// empty
	}

	/**
	 * \brief Set preference model for this selection.
	 *
	 * \param[in] preference The preference model to use
	 */
	inline void set_preference(const preference_type& preference)
	{
		preference_ = preference;
	}

	/**
	 * \brief Preference model for this selection.
	 *
	 * \return Preference model for this selection.
	 */
	inline const DescriptorPreference* preference() const
	{
		return &preference_;
	}

	/**
	 * \brief Set the selector for this selection.
	 *
	 * \param[in] selector The selector to use
	 */
	inline void set_selector(const selector_type& selector)
	{
		selector_ = selector;
	}

	/**
	 * \brief Selector for this selection.
	 *
	 * \return Selector for this selection.
	 */
	inline const FileReaderSelector* selector() const
	{
		return &selector_;
	}

private:

	/**
	 * \brief Internal preference model.
	 */
	preference_type preference_;

	/**
	 * \brief Internal selector.
	 */
	selector_type selector_;


	inline std::unique_ptr<FileReaderDescriptor> do_get(const Format format,
			const Codec codec, const FileReaders &descs) const final
	{
		return selector()->select(format, codec, descs, *preference());
	}
};


/**
 * \brief An unordered list of \link Matcher Matchers\endlink for Formats.
 */
using FormatList = std::vector<std::unique_ptr<Matcher>>;


/**
 * \internal
 * \brief Function pointer to function returning std::unique_ptr<T>.
 *
 * \tparam T The type the return unique_ptr holds
 */
template <class T>
using FunctionReturningUniquePtr = std::unique_ptr<T>(*)();


/**
 * \brief Registry holding all available FileReaderDescriptors
 * and all supported Formats.
 *
 * A FileReaderDescriptor instance is registered via instantiating the template
 * subclass RegisterDescriptor with the appropriate Descriptor type.
 *
 * A Format along with its byte and filename characteristics is registered via
 * instantiating the template subclass RegisterFormat with the appropriate
 * Format type.
 *
 * \note
 * This class is non-final but does not support polymorphic deletion.
 *
 * \see RegisterDescriptor
 * \see RegisterFormat
 */
class FileReaderRegistry
{
	/**
	 * \brief List of supported formats.
	 */
	static FormatList formats_;

	/**
	 * \brief Set of available readers.
	 */
	static std::unique_ptr<FileReaders> readers_;

	/**
	 * \brief Default selection for acquiring audio readers.
	 */
	static std::unique_ptr<FileReaderSelection> default_audio_selection_;

	/**
	 * \brief Default selection for acquiring metadata parsers.
	 */
	static std::unique_ptr<FileReaderSelection> default_toc_selection_;

public:

	/**
	 * \brief Return TRUE iff at least one descriptor is registered that
	 * accepts \c f.
	 *
	 * \param[in] f The format to check for
	 *
	 * \return TRUE iff descriptors for \c are registered, otherwise FALSE
	 */
	static bool has_format(const Format f);

	/**
	 * \brief Get a FileReaderDescriptor by its id.
	 *
	 * \param[in] id Id of the FileReaderDescriptor to lookup
	 *
	 * \return The FileReaderDescriptor with the specified \c id or nullptr.
	 */
	static std::unique_ptr<FileReaderDescriptor> reader(const std::string &id);

	/**
	 * \brief List of supported \link Format Formats\endlink.
	 *
	 * \return List of supported formats
	 */
	static const FormatList* formats();

	/**
	 * \brief Set of available \link FileReader FileReaderDescriptors\endlink.
	 *
	 * \return Set of available descriptors for FileReaders
	 */
	static const FileReaders* readers();

	/**
	 * \brief Default selection for \link AudioReader AudioReaders\endlink.
	 *
	 * This is used to initialize ToCParser and the Calculators with the same
	 * default selection setup for audio readers.
	 *
	 * \return The default selector for determining an AudioReader
	 */
	static const FileReaderSelection* default_audio_selection();

	/**
	 * \brief Default selection for
	 * \link MetadataParser MetatdataParsers\endlink.
	 *
	 * This is used to initialize ToCParser and the Calculators with the same
	 * default selection setup for ToCs.
	 *
	 * \return The default selector for determining a MetadataParser
	 */
	static const FileReaderSelection* default_toc_selection();

protected:

	/**
	 * \brief Add a Matcher for a Format to this registry.
	 *
	 * \param[in] m Matcher to add.
	 */
	static void add_format(std::unique_ptr<Matcher> m);

	/**
	 * \brief Add a descriptor to this registry.
	 *
	 * \param[in] d Descriptor to add.
	 */
	static void add_reader(std::unique_ptr<FileReaderDescriptor> d);

	/**
	 * \brief Instantiate the concrete Matcher with the given name.
	 *
	 * \param[in] create Function pointer to create the instance
	 *
	 * \return Instance returned by \c create
	 */
	static std::unique_ptr<Matcher> call_maker(
			FunctionReturningUniquePtr<Matcher> create);

	/**
	 * \brief Instantiate the concrete FileReaderDescriptor with the given name.
	 *
	 * \param[in] create Function pointer to create the instance
	 *
	 * \return Instance returned by \c create
	 */
	static std::unique_ptr<FileReaderDescriptor> call_maker(
			FunctionReturningUniquePtr<FileReaderDescriptor> create);
};


namespace details
{

/** \brief Downcast a FileReader to a specialized ReaderType.
 *
 * The first element of the returned pair is a pointer to the requested input
 * object with the requested cast performed. If the cast fails, the first
 * element is a nullptr. The unaltered input pointer is returned as a second
 * element in any case.
 *
 * \tparam Concrete FileReader type to cast to
 *
 * \param[in] file_reader The pointer to cast
 *
 * \return Pair of casting result and the original input pointer.
 */
template<class ReaderType>
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
 * \brief Select a FileReaderDescriptor.
 *
 * Select a reader that is guaranteed to accept the current input file or return
 * a nullptr.
 *
 * \param[in] filename Name of the file to read
 * \param[in] selector Selector to choose a reader
 * \param[in] formats  Set of file formats to check \c filename for
 * \param[in] readers  Set of available file readers
 *
 * \return Descriptor that accepts the input file.
 */
std::unique_ptr<FileReaderDescriptor> select_descriptor(
		const std::string &filename,
		const FileReaderSelection &selection,
		const FormatList &formats,
		const FileReaders &readers);


/**
 * \brief Select a FileReader.
 *
 * Select a reader that is guaranteed to accept the current input file or return
 * a null pointer.
 *
 * \param[in] filename Name of the file to read
 * \param[in] selector Selector to choose a reader
 * \param[in] formats  Set of file formats to check \c filename for
 * \param[in] readers  Set of available file readers
 *
 * \return FileReader that accepts the input file.
 */
std::unique_ptr<FileReader> select_reader(
		const std::string &filename,
		const FileReaderSelection &selection,
		const FormatList &formats,
		const FileReaders &readers);


/**
 * \brief Functor to safely create a unique_ptr to a downcasted FileReader.
 *
 * It will either provide a valid FileReader of the requested type or will
 * throw. It will never silently fail nor provide a nullptr.
 *
 * \warning
 * This class is non-abstract and non-final and does not support polymorphic
 * deletion. It is intended to be used for private inheritance to stateless
 * subclasses exclusively.
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
	 * \brief Default destructor.
	 */
	virtual ~CreateReader() noexcept = default;

	/**
	 * \brief Return a unique_ptr to an instance of the specified \c ReaderType.
	 *
	 * \param[in] selector  The FileReaderSelector to choose from
	 * \param[in] filename  The name of the file to choose a FileReader
	 * \param[in] formats   Set of supported formats
	 * \param[in] readers   Set of available file readers
	 */
	auto operator()(const std::string &filename,
			const FileReaderSelection &selection,
			const FormatList &formats,
			const FileReaders &readers) const
	-> std::unique_ptr<ReaderType>
	{
		ARCS_LOG_DEBUG << "Input file: " << filename << "";

		auto reader = select_reader(filename, selection, formats, readers);

		if (!reader)
		{
			throw InputFormatException("Failed to select a reader for file: '"
					+ filename + "'");
		}

		auto p = details::cast_reader<ReaderType>(std::move(reader));

		if (!p.first)
		{
			throw InputFormatException("Failed to acquire reader for file: "
					+ filename);
		}

		auto file_reader { std::move(p.first) };

		// XXX Return std::move(p.first) is quirky since it prevents RVO.
		// Omitting the move and return p.first directly causes compile error
		// since we try in fact to copy-construct a MetadataParser or
		// AudioReader. Both classes are abstract.

		return file_reader;
	}
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
 * \brief Register a \ref Format.
 *
 * Register a set of supported filename suffices and a byte sequence for a
 * Format \c F.
 *
 * \tparam F The Format to register
 */
template <enum Format F>
struct RegisterFormat final : private FileReaderRegistry
{
	/**
	 * \brief Constructor
	 *
	 * \param[in] suffices Set of supported filename suffices for \c F
	 * \param[in] codecs   Set of codecs supported with \c F
	 */
	RegisterFormat(const SuffixSet &suffices, const std::set<Codec> &codecs)
	{
		add_format(std::make_unique<FormatMatcher<F>>(suffices, codecs));
	}

	/**
	 * \brief Constructor
	 *
	 * \param[in] suffices Set of supported filename suffices for \c F
	 * \param[in] bytes    Reference byte sequence to determine \c F
	 * \param[in] codecs   Set of codecs supported with \c F
	 */
	RegisterFormat(const SuffixSet &suffices, const Bytes &bytes,
			const std::set<Codec> &codecs)
	{
		add_format(std::make_unique<FormatMatcher<F>>(suffices, bytes, codecs));
	}
};


/**
 * \brief Register a FileReaderDescriptor type.
 *
 * \tparam D The descriptor type to register
 */
template <class D>
struct RegisterDescriptor final : private FileReaderRegistry
{
	/**
	 * \brief Registers a descriptor of the template type \c D.
	 */
	RegisterDescriptor()
	{
		add_reader(call_maker(&details::make_descriptor<D>));
	}
};

/// @}

} // namespace v_1_0_0
} // namespace arcsdec

#endif

