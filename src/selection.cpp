/**
 * \file
 *
 * \brief Implementation of a selection toolkit for FileReaderDescriptors
 */

#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"
#endif

#include <algorithm>    // for find_if
#include <iterator>     // for begin, end
#include <memory>       // for unique_ptr, make_unique
#include <set>          // for set
#include <string>       // for string
#include <type_traits>  // for remove_reference
#include <utility>      // for pair, make_pair, move

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


// FileType


/**
 * \brief Determine a format and codec pair for a specified file.
 */
class FileType final
{
public:

	/**
	 * \brief Description.
	 *
	 * \param[in] filename File to read
	 */
	FileType(const std::string& filename);

	/**
	 * \brief Determine file type.
	 */
	std::pair<Format, Codec> type(const FormatList *formats) const;

	/**
	 * \brief Determines the format of a specified file.
	 */
	std::unique_ptr<Descriptor> format(const FormatList *formats) const;

	/**
	 * \brief Determines the Codec of a specified file.
	*
	 * Codec::UNKNOWN indicates that the codec could not be determined.
	 * Codec::NONE indicates that the file is not an audio file.
	 */
	Codec codec(const std::set<Codec>& codecs) const;

	/**
	 * \brief Filename.
	 *
	 * \return The filename of the specified file.
	 */
	std::string filename() const;

	/**
	 * \brief Bytes read from the file.
	 *
	 * \return The sequence of bytes read from the file.
	 */
	Bytes bytes() const;

protected:

	uint32_t offset() const
	{
		return 0;
	}

	uint32_t total_bytes() const
	{
		return TOTAL_BYTES_TO_READ;
	}

private:

	/**
	 * \brief Internal representation of the filename.
	 */
	std::string filename_;

	/**
	 * \brief Internal representation of the bytes read from the file.
	 */
	Bytes bytes_;
};


// FileType


FileType::FileType(const std::string& filename)
	: filename_ { filename }
	, bytes_    { details::read_bytes(filename, offset(), total_bytes()) }
{
	// empty
}


std::pair<Format, Codec> FileType::type(const FormatList *formats) const
{
	const auto format_d = format(formats);

	const auto format = format_d ? format_d->format() : Format::UNKNOWN;
	const auto codec  = format_d ? this->codec(format_d->codecs())
		: Codec::UNKNOWN;

	ARCS_LOG_INFO << "Filetype recognized: Format is '" << name(format)
		<< "', Codec is '" << name(codec) << "'";

	return std::make_pair(format, codec);
}


std::unique_ptr<Descriptor> FileType::format(
		const FormatList *formats) const
{
	ARCS_LOG_DEBUG << "Try to recognize file format: " << filename();

	for (const auto& accepted : *formats)
	{
		ARCS_LOG(DEBUG1) << "Check for format: " << accepted->name();

		if (accepted->matches(bytes()))
		{
			ARCS_LOG(DEBUG1) << accepted->name() << ": file bytes matched";

			if (accepted->matches(filename()))
			{
				ARCS_LOG(DEBUG1) << accepted->name() << ": filename matched";
				ARCS_LOG_DEBUG << "Recognized format: " << accepted->name();
				return accepted->clone();
			}

			ARCS_LOG(DEBUG1) << "But filename did not match!";
		}

		ARCS_LOG(DEBUG1) << "Check for format " << accepted->name()
			<< " failed";
	}

	ARCS_LOG_DEBUG << "File format is unknown. (Checked for "
		<< formats->size()
		<< " different formats.)";
	return nullptr;
}


Codec FileType::codec(const std::set<Codec>& codecs) const
{
	ARCS_LOG_DEBUG << "Try to recognize codec: " << filename();

	if (codecs.empty())
	{
		return Codec::NONE;
	}

	// TODO Implement codec analysis

	ARCS_LOG(DEBUG1) << "This is not yet implemented";

	// TODO Iterate over codecs and check each for a match with bytes
	if (codecs.size() == 1) // Make Codec::NONE work
	{
		const auto codec = *codecs.begin();

		ARCS_LOG(DEBUG1) << "Format supports only codec '" <<
			arcsdec::name(codec) << "', so just assume this";

		// TODO Activate/deactivate Validation?

		return codec;
	}

	ARCS_LOG_DEBUG << "Try to recognize codec: " << filename();

	return Codec::UNKNOWN;
}


std::string FileType::filename() const
{
	return filename_;
}


Bytes FileType::bytes() const
{
	return bytes_;
}


// DescriptorPreference


DescriptorPreference::~DescriptorPreference() noexcept = default;


DescriptorPreference::type DescriptorPreference::preference(
		const Format format,
		const Codec codec,
		const FileReaderDescriptor &desc) const
{
	return this->do_preference(format, codec, desc);
}


// DefaultPreference


DescriptorPreference::type DefaultPreference::do_preference(
		const Format format, const Codec codec,
		const FileReaderDescriptor &desc) const
{
	constexpr static unsigned PENALTY_FOR_NONSPECIFICNESS = 2;

	if (!desc.accepts(format, codec))
	{
		return MIN_PREFERENCE;
	}

	// Prefer specific readers over multi-format readers.
	return MAX_PREFERENCE
			- ((desc.formats().size() - 1) * PENALTY_FOR_NONSPECIFICNESS
				- (desc.codecs().size()  - 1));
}


// FormatPreference


FormatPreference::type FormatPreference::do_preference(
		const Format format,
		const Codec /* codec */,
		const FileReaderDescriptor &desc) const
{
	if (!desc.accepts(format))
	{
		return MIN_PREFERENCE;
	}

	return MAX_PREFERENCE - (desc.formats().size() - 1) * 2
			- (desc.codecs().size() - 1);
}


// FileReaderSelector


FileReaderSelector::~FileReaderSelector() noexcept = default;


std::unique_ptr<FileReaderDescriptor> FileReaderSelector::select(
		const Format format, const Codec codec, const FileReaders &readers,
		const DescriptorPreference &strategy) const
{
	ARCS_LOG_DEBUG << "Try to select a FileReader for filetype "
		<< name(format) << "/" << name(codec);

	if (readers.empty())
	{
		ARCS_LOG_WARNING << "No FileReaderDescriptors to select from";
		return nullptr;
	}

	auto descriptor = do_select(format, codec, readers, strategy);

	if (!descriptor)
	{
		ARCS_LOG_WARNING << "Could not select a matching descriptor";
		return nullptr;
	}

	ARCS_LOG_DEBUG << "Reader descriptor '" << descriptor->name()
		<< "' selected";

	return descriptor;
}


// DefaultSelector


std::unique_ptr<FileReaderDescriptor> DefaultSelector::do_select(
		const Format format, const Codec codec, const FileReaders& readers,
		const DescriptorPreference &p) const
{
	FileReaderDescriptor* result    = nullptr;
	DescriptorPreference::type pref = DescriptorPreference::MIN_PREFERENCE;

	DescriptorPreference::type curr_pref = pref;

	for (auto& i : readers)
	{
		ARCS_LOG(DEBUG1) << "Check descriptor " << i.second->name();

		curr_pref = p.preference(format, codec, *i.second);

		ARCS_LOG(DEBUG1) << i.second->name() << ": preference for "
			<< name(format) << '/' << name(codec) << " is " << curr_pref;

		if (curr_pref > pref)
		{
			result = i.second.get();
			pref   = curr_pref;
		}
	}

	if (!result)
	{
		return nullptr;
	}

	ARCS_LOG(DEBUG1) << "First descriptor with highest preference: '"
			<< result->name()
			<< "'";

	return result->clone();
}


// FileReaderSelection


FileReaderSelection::~FileReaderSelection() noexcept = default;


std::unique_ptr<FileReaderDescriptor> FileReaderSelection::get(
		const Format format, const Codec codec, const FileReaders &descs) const
{
	return this->do_get(format, codec, descs);
}


// FileReaderRegistry


FormatList FileReaderRegistry::formats_;


std::unique_ptr<FileReaders> FileReaderRegistry::readers_;


std::unique_ptr<FileReaderSelection>
	FileReaderRegistry::default_audio_selection_ =
		std::make_unique<FileReaderPreferenceSelection<
			FormatPreference, DefaultSelector>
		>();


std::unique_ptr<FileReaderSelection>
	FileReaderRegistry::default_toc_selection_ =
		std::make_unique<FileReaderPreferenceSelection<
			FormatPreference, DefaultSelector>
		>();


bool FileReaderRegistry::has_format(const Format f)
{
	const auto fs = formats();

	return std::end(*fs) != std::find_if(std::begin(*fs), std::end(*fs),
			[f](const auto& p){ return p->format() == f; });
}


std::unique_ptr<FileReaderDescriptor> FileReaderRegistry::reader(
		const std::string &id)
{
	auto p = readers_->find(id);
	return p != readers_->end() ? p->second->clone() : nullptr;
}


const FormatList* FileReaderRegistry::formats()
{
	return &formats_;
}


const FileReaders* FileReaderRegistry::readers()
{
	return readers_.get();
}


const FileReaderSelection* FileReaderRegistry::default_audio_selection()
{
	return default_audio_selection_.get();
}


const FileReaderSelection* FileReaderRegistry::default_toc_selection()
{
	return default_toc_selection_.get();
}


void FileReaderRegistry::add_format(std::unique_ptr<Descriptor> f)
{
	// ... does not seem to require any further static initialization
	if (f) { formats_.push_back(std::move(f)); }
}


void FileReaderRegistry::add_reader(std::unique_ptr<FileReaderDescriptor> d)
{
	static const bool r_guard = []{
		FileReaderRegistry::readers_ = std::make_unique<FileReaders>();
		return true;
	}();
	// add_reader() is called several times via RegisterDescriptor before
	// entering main(), so readers_ will be initialized when reader() is called
	// for the first time. Ugly, nonetheless.

	if (d)
	{
		readers_->emplace(std::make_pair(d->id(), std::move(d)));
	}

	if (r_guard){} /* avoid -Wunused-variable firing */
}


std::unique_ptr<Descriptor> FileReaderRegistry::call_maker(
			FunctionReturningUniquePtr<Descriptor> create)
{
	return create();
}


std::unique_ptr<FileReaderDescriptor> FileReaderRegistry::call_maker(
			FunctionReturningUniquePtr<FileReaderDescriptor> create)
{
	return create();
}


// non-member functions

namespace details {

std::unique_ptr<FileReaderDescriptor> select_descriptor(
		const std::string &filename,
		const FileReaderSelection &selection,
		const FormatList &formats,
		const FileReaders &readers)
{
	if (filename.empty())
	{
		throw FileReadException("Filename must not be empty");
	}

	const auto type { FileType(filename).type(&formats) };
	auto reader = selection.get(type.first, type.second, readers);

	if (!reader)
	{
		ARCS_LOG_WARNING << "File format is unknown, no reader available.";

		return nullptr;
	}

	return reader;
}


std::unique_ptr<FileReader> select_reader(
		const std::string &filename,
		const FileReaderSelection &selection,
		const FormatList &formats,
		const FileReaders &readers)
{
	auto d = select_descriptor(filename, selection, formats, readers);
	return d ? d->create_reader() : nullptr;
}

} // namespace details


namespace {

// Register all supported file formats.
// This will not guarantee that a matching reader will be available!

// TOC/Meta

const auto dm1 = RegisterFormat<Format::CUE>({ "cue" }, { Codec::NONE} );

const auto dm2 = RegisterFormat<Format::CDRDAO>({ "toc" }, { Codec::NONE } );

// Audio

const auto da1 = RegisterFormat<Format::WAV>({ "wav", "wave" },
		{0, {
		0x52, 0x49, 0x46, 0x46, // BE: 'R','I','F','F'
		Bytes::any, Bytes::any, Bytes::any, Bytes::any,
		// LE: filesize in bytes - 8
		0x57, 0x41, 0x56, 0x45, // BE: 'W','A','V','E'
		0x66, 0x6D, 0x74, 0x20, // BE: Format Subchunk Header: 'f','m','t',' '
		0x10, 0x00, 0x00, 0x00, // LE: Format Subchunk Size: '16'
		0x01, 0x00,             // LE: wFormatTag: 1 (means: PCM),
		0x02, 0x00,             // LE: wChannels: 2 (means: stereo)
		0x44, 0xAC, 0x00, 0x00, // LE: dwSamplesPerSec:  44100
		0x10, 0xB1, 0x02, 0x00, // LE: dwAvgBytesPerSec: 176400
		0x04, 0x00,             // LE: wBlockAlign: 4 (bytes per block)
		0x10, 0x00,             // LE: wBitsPerSample: 16
		0x64, 0x61, 0x74, 0x61  // BE: Data Subchunk Header: 'd','a','t','a'
	                            // LE: Data Subchunk Size
		}},
		{ Codec::PCM_S16BE, Codec::PCM_S16BE_PLANAR,
		  Codec::PCM_S16LE, Codec::PCM_S16LE_PLANAR,
		  Codec::PCM_S32BE, Codec::PCM_S32BE_PLANAR,
		  Codec::PCM_S32LE, Codec::PCM_S32LE_PLANAR });

const auto da2 = RegisterFormat<Format::FLAC>({ "flac" },
		{0, { 0x66 /* f */, 0x4C /* L */, 0x61 /* a */, 0x43 /* C */}},
		{ Codec::FLAC } );

const auto da3 = RegisterFormat<Format::APE>({ "ape" },
		{0, { 0x4D /* M */, 0x41 /* A */, 0x43 /* C */, 0x20 /*   */}},
		{ Codec::MONKEY } );

const auto da4 = RegisterFormat<Format::CAF>({ "caf" },
		{0, { 0x63 /* c */, 0x61 /* a */, 0x66 /* f */, 0x66 /* f */,
			  0x00,         0x01,         0x00,         0x00        }},
		{ Codec::ALAC } );

const auto da5 = RegisterFormat<Format::M4A>({ "m4a" },
		{4, { 0x66 /* f */, 0x74 /* t */, 0x79 /* y */, 0x70 /* p */,
			  0x4D /* M */, 0x34 /* 4 */, 0x41 /* A */, 0x20 /*   */}},
		{ Codec::ALAC } );

const auto da6 = RegisterFormat<Format::OGG>({ "ogg", "oga" },
		{0, { 0x4F /* O */, 0x67 /* g */, 0x67 /* g */, 0x53 /* S */}},
		{ Codec::FLAC } );

const auto da7 = RegisterFormat<Format::WV>({ "wv" },
		{0, { 0x77 /* w */, 0x76 /* v */, 0x70 /* p */, 0x6B /* k */}},
		{ Codec::WAVPACK} );

const auto da8 = RegisterFormat<Format::AIFF>({ "aiff" },
		{0, { 0x46 /* F */, 0x4F /* O */, 0x52 /* R */, 0x4D /* M */,
			  Bytes::any,   Bytes::any,   Bytes::any,   Bytes::any,
			  0x41 /* A */, 0x49 /* I */, 0x46 /* F */, 0x46 /* F */}},
		{ Codec::PCM_S16BE, Codec::PCM_S16BE_PLANAR,
		  Codec::PCM_S16LE, Codec::PCM_S16LE_PLANAR,
		  Codec::PCM_S32BE, Codec::PCM_S32BE_PLANAR,
		  Codec::PCM_S32LE, Codec::PCM_S32LE_PLANAR });

} // namespace

} // namespace v_1_0_0
} // namespace arcsdec

