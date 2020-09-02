#ifndef __LIBARCSDEC_READERWAV_HPP__
#error "Do not include readerwav_details.hpp, include readerwav.hpp instead"
#endif

/**
 * \file
 *
 * \brief Internal APIs for native RIFFWAV/PCM reader
 */

#ifndef __LIBARCSDEC_READERWAV_DETAILS_HPP__
#define __LIBARCSDEC_READERWAV_DETAILS_HPP__

#include <cstdint>
#include <string>
#include <vector>

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>
#endif

#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"
#endif
#ifndef __LIBARCSDEC_AUDIOBUFFER_HPP__
#include "audiobuffer.hpp" // PCMBlockReader inherits from BlockCreator
#endif

namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{
namespace wave
{


/**
 * \internal
 * \defgroup readerwavInternal Implementation of the WAV reader
 *
 * \ingroup readerwav
 *
 * AudioReader to read RIFF/WAV files containing integer PCM samples.
 *
 * Validation requires CDDA conform samples in PCM format. Additional fields in
 * the format subchunk will cause the validation to fail. Non-standard
 * subchunks are ignored. RIFX containers are currently not supported.
 *
 * \todo This implementation silently relies on a little endian plattform.
 *
 * @{
 */


/**
 * \brief Represents an interface for different reference CDDA representations
 * for the WAV format.
 *
 * For a concrete format like RIFFWAV/PCM, this interface can just be
 * implemented.
 */
class WAV_CDDA_t
{

public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~WAV_CDDA_t() noexcept = default;

	/**
	 * \brief Expected chunk descriptor id, e.g. 0x52494646 for "RIFF" or
	 * 0x52494658 for "RIFX".
	 *
	 * \return Expected chunk descriptor id
	 */
	virtual uint32_t chunk_id() const
	= 0;

	/**
	 * \brief Expected chunk format, e.g. "WAVE".
	 *
	 * \return Expected chunk format
	 */
	virtual uint32_t format() const
	= 0;

	/**
	 * \brief Expected id of the format subchunk, this is "fmt ".
	 *
	 * \return Expected subchunk id of the format subchunk
	 */
	virtual uint32_t fmt_subchunk_id() const
	= 0;

	/**
	 * \brief Expected size of the format subchunk, e.g. 16 for PCM.
	 *
	 * \return Expected byte size of the format subchunk
	 */
	virtual uint32_t fmt_subchunk_size() const
	= 0;

	/**
	 * \brief Expected sample format of the data subchunk, e.g. 0x0001 for PCM.
	 *
	 * \return Expected value of the wFormatTag
	 */
	virtual uint16_t wFormatTag() const
	= 0;

	/**
	 * \brief Expected number of channels.
	 *
	 * \return Expected number of channels
	 */
	virtual uint16_t wChannels() const
	= 0;

	/**
	 * \brief Expected number of samples per second.
	 *
	 * \return Expected value of dwSamplesPerSec
	 */
	virtual uint32_t dwSamplesPerSec() const
	= 0;

	/**
	 * \brief Expected number of bytes per second.
	 *
	 * \return Expected value of dwAvgBytesPerSec
	 */
	virtual uint32_t dwAvgBytesPerSec() const
	= 0;

	/**
	 * \brief Expected sum of bytes for one sample per channel for each channel.
	 *
	 * \return Expected value of wBlockAlign
	 */
	virtual uint16_t wBlockAlign() const
	= 0;

	/**
	 * \brief Expected bits per sample.
	 *
	 * \return Expected value of wBitsPerSample
	 */
	virtual uint16_t wBitsPerSample() const
	= 0;

	/**
	 * \brief Expected id of the data subchunk, this is "data".
	 *
	 * \return Expected subchunk id of the data subchunk
	 */
	virtual uint32_t data_subchunk_id() const
	= 0;
};


/**
 * \brief Implements reference values for CDDA compliant RIFF/WAV PCM.
 */
class RIFFWAV_PCM_CDDA_t final : public WAV_CDDA_t
{
private:

	/**
	 * \brief Constants for accessing first dimension of WAV_CDDA_.
	 */
	enum FIELD : int
	{
		// chunk descriptor
		RIFF                     =  0,  // chunk descriptor id
		FILESIZE                 =  1,  // chunk descriptor id
		WAVE                     =  2,  // chunk descriptor format

		// fmt subchunk
		FMT_SC_NAME              =  3,  // fmt subchunk name
		FMT_SC_SIZE              =  4,  // fmt subchunk size
		FMT_W_FORMAT_TAG         =  5,  // fmt wFormatTag
		FMT_W_CHANNELS           =  6,  // fmt wChannels
		FMT_DW_SAMPLES_PER_SEC   =  7,  // fmt dwSamplesPerSec
		FMT_DW_AVG_BYTES_PER_SEC =  8,  // fmt dwAvgBytesPerSec
		FMT_W_BLOCK_ALIGN        =  9,  // fmt wBlockAlign
		FMT_W_BITS_PER_SAMPLE    = 10,  // fmt wBitsPerSample

		// data subchunk
		DATA_SC_NAME             = 11,  // data subchunk name
		DATA_SC_SIZE             = 12   // data subchunk size
	};

	/**
	 * \brief Number of sections in header.
	 *
	 * This defines an array size and must be equal to the amount of defined
	 * FIELD values.
	 */
	static constexpr int HEADER_FIELD_COUNT_ = 13;

	/**
	 * \brief Offsets and lengths for interpreting a RIFF WAVE header.
	 */
	static constexpr unsigned int BYTES_[HEADER_FIELD_COUNT_][2] =
	{
		{  0, 4}, // Chunk descriptor id 'RIFF'
		{  4, 4}, // Filesize - 8
		{  8, 4}, // Chunk descriptor format 'WAVE'
		{ 12, 4}, // Format: Subchunk name
		{ 16, 4}, // Format: Subchunk size
		{ 20, 2}, // wFormatTag
		{ 22, 2}, // wChannels
		{ 24, 4}, // dwSamplesPerSec
		{ 28, 4}, // dwAvgBytesPerSec
		{ 32, 2}, // wBlockAlign
		{ 34, 2}, // wBitsPerSample
		{ 36, 4}, // Data: Subchunk name
		{ 40, 4}  // Data: Subchunk size
	};

	/**
	 * \brief Encodes access to \c BYTES_[i]
	 */
	enum BYTES : int
	{
		OFFSET = 0,
		LENGTH = 1
	};

	/**
	 * \brief Mark a position as "any byte value accepted here".
	 */
	static constexpr unsigned char any_ = 0xFF;

	/**
	 * \brief Canonical header of a CDDA compliant RIFF WAVE file in PCM format.
	 */
	static const std::array<unsigned char, 44> WAVPCM_HEADER_;

	/**
	 * \brief Returns canonical value of specified header field
	 *
	 * \param[in] field The header field to read
	 *
	 * \return The value of \c field
	 */
	uint32_t header(FIELD field) const;

public:

	/**
	 * \brief Expected chunk descriptor id is "RIFF".
	 *
	 * \return 0x52494646
	 */
	uint32_t chunk_id() const override;

	/**
	 * \brief Expected chunk descriptor format is "WAVE".
	 *
	 * \return 0x57415645
	 */
	uint32_t format() const override;

	/**
	 * \brief Expected RIFF/WAV-compliant format subchunk id is "fmt " (with a
	 * trailing blank).
	 *
	 * \return 0x666d7420
	 */
	uint32_t fmt_subchunk_id() const override;

	/**
	 * \brief Expected format subchunk size is 16 bytes.
	 *
	 * \return 16
	 */
	uint32_t fmt_subchunk_size() const override;

	/**
	 * \brief Expected format identifier is 0x0001, indicating PCM sample
	 * format.
	 *
	 * \return 1
	 */
	uint16_t wFormatTag() const override;

	/**
	 * \brief Expected CDDA-compliant number of channels is 2 (== stereo).
	 *
	 * \return 2
	 */
	uint16_t wChannels() const override;

	/**
	 * \brief Expected CDDA-compliant number of samples per second is 44100.
	 *
	 * \return 44100
	 */
	uint32_t dwSamplesPerSec() const override;

	/**
	 * \brief Expected CDDA-compliant average number of bytes per second is
	 * 176400 (since there are 2 channels multiplied by 2 bytes per sample
	 * multiplied by 44100 samples per second).
	 *
	 * \return 176400
	 */
	uint32_t dwAvgBytesPerSec() const override;

	/**
	 * \brief Expected CDDA-compliant number of bytes per sample block is 4
	 * (since there are 2 channels multiplied by 2 bytes per sample).
	 *
	 * \return 4
	 */
	uint16_t wBlockAlign() const override;

	/**
	 * \brief Expected CDDA-compliant number of bits per sample is 16.
	 *
	 * \return 16
	 */
	uint16_t wBitsPerSample() const override;

	/**
	 * \brief Expected RIFF/WAV-compliant data subchunk id is "data".
	 *
	 * \return 0x64617461
	 */
	uint32_t data_subchunk_id() const override;

	/**
	 * \brief Canoncial RIFF/WAVE header for PCM encoding.
	 */
	static const std::array<unsigned char, 44>& header();

	/**
	 * \brief Compare to bytes that are accepted to have arbitrary values.
	 */
	static const unsigned char& any_byte();
};


/**
 * \brief Represents the parsed chunk descriptor of a WAV file.
 *
 * Since it is a representation generated by a parser, it is readonly.
 */
class WavChunkDescriptor
{

public:

	/**
	 * \brief Constructor.
	 *
	 * \param id        Chunk descriptor id
	 * \param size      Real size in bytes of the chunk descriptor
	 * \param file_size File size in bytes declared by the chunk descriptor
	 * \param format    File format declared by the chunk descriptor
	 */
	WavChunkDescriptor(
			uint32_t id,
			uint32_t size,
			uint32_t file_size,
			uint32_t format
			);

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~WavChunkDescriptor() noexcept;

	/**
	 * \brief ChunkId, either RIFF or RIFX.
	 *
	 * \return ChunkId, either RIFF or RIFX
	 */
	const uint32_t id;

	/**
	 * \brief Size in bytes of the header. This is 12 per RIFF/WAV format
	 * specification.
	 *
	 * \return Byte size of the header, specification requires 12
	 */
	const uint32_t size;

	/**
	 * \brief Declared file size.
	 *
	 * This is physical file size in bytes minus 8. The number 8 represents the
	 * size in bytes of the RIFF declaration and the size in bytes of the file
	 * size declaration.
	 *
	 * \return The file size as declared in the file
	 */
	const uint32_t file_size;

	/**
	 * \brief File format, 'WAVE' expected.
	 *
	 * \return Parsed file format specifier
	 */
	const uint32_t format;
};


/**
 * \brief Represents the parsed subchunk header in a WAV file.
 *
 * Since it is a representation generated by a parser, it is readonly.
 */
class WavSubchunkHeader
{

public:

	/**
	 * \brief Constructor.
	 *
	 * \param id   Id of the subchunk header
	 * \param size Size in bytes of the subchunk
	 */
	WavSubchunkHeader(uint32_t id, uint32_t size);

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~WavSubchunkHeader() noexcept;

	/**
	 * \brief Id of the subchunk.
	 */
	const uint32_t id;

	/**
	 * \brief Size in bytes of the subchunk.
	 */
	const uint32_t size;

	/**
	 * \brief The subchunk id as a human-readable string.
	 *
	 * This is convenient for output in log messages.
	 *
	 * \return Subchunk id as a string
	 */
	std::string name() const;
};


/**
 * \brief Represents the parsed content of a format subchunk.
 *
 * Since it is a representation generated by a parser, it is readonly.
 */
class WavFormatSubchunk
{

public:

	/**
	 * \brief Constructor.
	 */
	WavFormatSubchunk(
			const WavSubchunkHeader &header,
			int wFormatTag,
			int wChannels,
			int64_t dwSamplesPerSec,
			int64_t dwAvgBytesPerSec,
			int wBlockAlign,
			int wBitsPerSample
			);

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~WavFormatSubchunk() noexcept;

	/**
	 * \brief Parsed id of this format subchunk.
	 */
	const uint32_t id;

	/**
	 * \brief Parsed size of this format subchunk.
	 */
	const std::size_t size;

	/**
	 * \brief Parsed format identifier.
	 */
	const int wFormatTag;

	/**
	 * \brief Parsed number of channels.
	 */
	const int wChannels;

	/**
	 * \brief Parsed number of samples per second.
	 */
	const int64_t dwSamplesPerSec;

	/**
	 * \brief Parsed average number of bytes per second.
	 */
	const int64_t dwAvgBytesPerSec;

	/**
	 * \brief Parsed number of bytes per sample block.
	 */
	const int wBlockAlign;

	/**
	 * \brief Parsed number of bits per sample.
	 */
	const int wBitsPerSample;
};


/**
 * \brief Parses byte sequences as syntactic elements of a WAV file, e.g. chunk
 * descriptor, subchunk headers and the entire format subchunk.
 */
class WavPartParser
{

public:

	/**
	 * \brief Default constructor.
	 */
	WavPartParser();

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~WavPartParser() noexcept;

	/**
	 * \brief Specified size in bytes of a compliant RIFF/RIFX header
	 * (4 bytes 'RIFF' + 4 bytes Filesize + 4 bytes 'WAVE').
	 */
	static constexpr uint8_t WAV_BYTES_PER_RIFF_HEADER     = 12;

	/**
	 * \brief Specified ize in bytes of a compliant subchunk header
	 * (4 bytes ChunkID + 4 bytes ChunkSize).
	 */
	static constexpr uint8_t WAV_BYTES_PER_SUBCHUNK_HEADER = 8;

	/**
	 * \brief Expected size in bytes of the fmt subchunk.
	 */
	static constexpr uint8_t WAV_BYTES_IN_FMT_SUBCHUNK     = 16;

	/**
	 * \brief Construct a chunk descriptor from the given byte vector.
	 *
	 * \param[in] bytes The parsed bytes
	 *
	 * \return WavChunkDescriptor representing the parsed bytes
	 */
	WavChunkDescriptor chunk_descriptor(const std::vector<char> &bytes);

	/**
	 * \brief Construct a subchunk header from the given byte vector.
	 *
	 * \param[in] bytes The parsed bytes
	 *
	 * \return WavSubchunkHeader representing the parsed bytes
	 */
	WavSubchunkHeader subchunk_header(const std::vector<char> &bytes);

	/**
	 * \brief Construct a format subchunk from the given byte vector.
	 *
	 * \param[in] header The subchunk header
	 * \param[in] bytes  The parsed bytes
	 *
	 * \return WavFormatSubchunk representing the parsed bytes
	 */
	WavFormatSubchunk format_subchunk(const WavSubchunkHeader &header,
			const std::vector<char> &bytes);


private:

	/**
	 * \brief Internal byte converter
	 */
	ByteConverter convert_;
};


/**
 * \brief Config flags of the audio handler.
 */
enum CONFIG : uint32_t
{
	C_EMPTY_CONFIG      =    0,
	C_RESPECT_HEADER    =    1,
	C_RESPECT_FORMAT    =    2,
	C_RESPECT_DATA      =    4,
	C_RESPECT_TRAILING  =    8
};


/**
 * \brief Event handler for interpreting and validating WAV files.
 *
 * The handler implements the actual behaviour for the data the FileReaderImpl
 * provides while reading.
 *
 * Validation checks for basic WAV format compliance and for presence of CDDA
 * audio format. This handler class uses an instance of WAV_CDDA_t to perform
 * the actual checks against the expected format.
 */
class WavAudioHandler : public ReaderValidatingHandler
{

private:

	/**
	 * \brief State flags of the audio handler. More than one state can be
	 * active.
	 */
	enum STATE : uint32_t
	{
		S_INITIAL           =    0,
		S_STARTED_FORMAT    =    1,
		S_STARTED_DATA      =    2,
		S_VALIDATED_HEADER  =    4,
		S_VALIDATED_FORMAT  =    8,
		S_VALIDATED_DATA    =   16
	};


public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] valid_values An object representing the valid reference values
	 */
	explicit WavAudioHandler(std::unique_ptr<WAV_CDDA_t> valid_values);

	/**
	 * \brief Virtual destructor.
	 */
	virtual ~WavAudioHandler() noexcept;

	/**
	 * \brief Return phyiscal file size.
	 *
	 * \return The physical file size
	 */
	int64_t physical_file_size();

	/**
	 * \brief Handler method: Called by AudioReaderImpl on start of the reading
	 * process.
	 *
	 * \param[in] filename Name of the audio file started to parse
	 * \param[in] phys_file_size Recognized physical file size
	 */
	void start_file(const std::string &filename, const int64_t &phys_file_size);

	/**
	 * \brief Handler method: Called by AudioReaderImpl on EOF.
	 */
	void end_file();

	/**
	 * \brief Handler method: Called by AudioReaderImpl on every start of a
	 * subchunk.
	 *
	 * \param[in] subchunk_header The WavSubchunkHeader as parsed
	 */
	void start_subchunk(const WavSubchunkHeader &subchunk_header);

	/**
	 * \brief Handler method: Called by AudioReaderImpl when chunk descriptor is
	 * encountered.
	 *
	 * \param[in] chunk_descriptor The WavChunkDescriptor as parsed
	 *
	 * \throws InvalidAudioException if audio processing failed
	 */
	void chunk_descriptor(const WavChunkDescriptor &chunk_descriptor);

	/**
	 * \brief Handler method: Called by AudioReaderImpl when format subchunk is
	 * encountered.
	 *
	 * \param[in] format_subchunk The WavFormatSubchunk as parsed
	 *
	 * \throws InvalidAudioException if audio processing failed
	 */
	void subchunk_format(const WavFormatSubchunk &format_subchunk);

	/**
	 * \brief Handler method: Called by AudioReaderImpl when data subchunk is
	 * encountered.
	 *
	 * \param[in] subchunk_size The size of the data subchunk as parsed
	 *
	 * \throws InvalidAudioException if audio processing failed
	 */
	void subchunk_data(const uint32_t &subchunk_size);

	/**
	 * \brief Set a configuration option.
	 *
	 * \param[in] option The option to set
	 */
	void set_config(const CONFIG &option);

	/**
	 * \brief Check for a configuration option.
	 *
	 * \param[in] option The option to check for
	 *
	 * \return TRUE iff the option is set, otherwise false
	 */
	bool has_config(const CONFIG &option);

	/**
	 * \brief Returns TRUE iff WavAudioHandler expects to see also the optional
	 * trailing subchunks after 'data'.
	 *
	 * Technically, this method returns TRUE iff C_RESPECT_TRAILING is set.
	 *
	 * \return TRUE if WavAudioHandler excpects trailing subchunks
	 */
	bool requests_all_subchunks();

	/**
	 * \brief Called when audio processing failed.
	 *
	 * \throws InvalidAudioException if audio processing failed
	 */
	void fail();

	/**
	 * \brief Return the internal validation object.
	 *
	 * \return Internal validation object.
	 */
	const WAV_CDDA_t& validator();


protected:

	/**
	 * \brief Checks for a specific STATE.
	 *
	 * \param[in] state The state to check for
	 *
	 * \return TRUE iff the state is actual, otherwise false
	 */
	bool has_state(const STATE &state);

	/**
	 * \brief Set a specific state.
	 *
	 * \param[in] state The state to set
	 */
	void set_state(const STATE &state);

	/**
	 * \brief Unset a specific state.
	 *
	 * Unsetting a state not set is completely safe and does not alter the
	 * actual state.
	 *
	 * \param[in] state The state to unset
	 */
	void unset_state(const STATE &state);

	/**
	 * \brief Implements chunk_descriptor().
	 *
	 * \param[in] chunk_descriptor The WavChunkDescriptor as parsed
	 */
	virtual void do_chunk_descriptor(
			const WavChunkDescriptor &chunk_descriptor);

	/**
	 * \brief Implements subchunk_format().
	 *
	 * \param[in] format_subchunk The WavFormatSubchunk as parsed
	 */
	virtual void do_subchunk_format(
			const WavFormatSubchunk &format_subchunk);

	/**
	 * \brief Implements subchunk_data().
	 *
	 * \param[in] subchunk_size The size of the data subchunk as parsed
	 */
	virtual void do_subchunk_data(const uint32_t &subchunk_size);

	/**
	 * \brief Implements fail().
	 *
	 * \throws InvalidAudioException if audio processing failed
	 */
	virtual void do_fail();


private:

	/**
	 * \brief Configuration: physical file size in bytes as passed from the
	 * AudioReaderImpl.
	 *
	 * This is required as a reference value for checking the
	 * consistency of the file size declaration in the chunk descriptor.
	 */
	int64_t phys_file_size_;

	/**
	 * \brief Configuration: Reference values for validation.
	 */
	std::unique_ptr<WAV_CDDA_t> valid_;

	/**
	 * \brief Configuration: configuration flags.
	 */
	uint32_t config_;

	/**
	 * \brief State: state flags.
	 */
	uint32_t state_;
};


/**
 * \brief Implements pull reading PCM samples from a std::ifstream.
 *
 * This is the block reading policy for the RIFF/WAV (PCM) format.
 */
class PCMBlockReader : public BlockCreator
{

public:

	/**
	 * \brief Constructs a PCMBlockReader with buffer of size samples_per_block.
	 *
	 * \param[in] samples_per_block Number of 32 bit PCM samples in one block
	 */
	explicit PCMBlockReader(const int32_t samples_per_block);

	// make class non-copyable (1/2)
	PCMBlockReader(const PCMBlockReader &) = delete;

	// TODO Move constructor

	/**
	 * \brief Virtual default destructor.
	 */
	~PCMBlockReader() noexcept override;

	/**
	 * \brief Registers a consuming method for blocks.
	 *
	 * \param[in] func The functor to be registered as block consumer.
	 */
	void register_block_consumer(const std::function<void(
				SampleInputIterator begin, SampleInputIterator end)>
			&func);

	/**
	 * \brief Read blocks from the stream until the end of the stream.
	 *
	 * The number of actual bytes read is returned and will be equal to
	 * total_pcm_bytes on success.
	 *
	 * \param[in] in Stream of bytes to read from
	 * \param[in] total_pcm_bytes Total number of 32 bit PCM samples in the
	 * stream
	 *
	 * \throw FileReadException On any read error
	 *
	 * \return The actual number of bytes read
	 */
	int64_t read_blocks(std::ifstream &in, const int64_t &total_pcm_bytes);

	// make class non-copyable (2/2)
	PCMBlockReader& operator = (const PCMBlockReader &) = delete;

	// TODO Move assignment


private:

	/**
	 * \brief Registered callback method to consume a block.
	 *
	 * Called by block_complete().
	 */
	std::function<void(SampleInputIterator begin, SampleInputIterator end)>
		consume_;
};


/**
 * \brief File reader implementation for files in RIFF/WAVE (PCM) format, i.e.
 * containing 44.100 Hz/16 bit Stereo PCM samples in its data chunk.
 *
 * This class provides the PCM sample data as a succession of blocks
 * of 32 bit PCM samples to its Calculation. The first block starts with the
 * very first PCM sample in the data chunk, i.e. in a compliant CDDA WAV file
 * the 4 bytes following byte 0x2C. The format subchunk is validated to conform
 * to CDDA.
 */
class WavAudioReaderImpl : public BufferedAudioReaderImpl
{

public:

	/**
	 * \brief Standard constructor.
	 */
	WavAudioReaderImpl();

	/**
	 * \brief Virtual destructor.
	 */
	virtual ~WavAudioReaderImpl() noexcept override;

	/**
	 * \brief Register a validation handler.
	 *
	 * \param[in] hndlr The validating handler to set
	 */
	void register_audio_handler(std::unique_ptr<WavAudioHandler> hndlr);


private:

	std::unique_ptr<AudioSize> do_acquire_size(const std::string &filename)
		override;

	void do_process_file(const std::string &filename) override;

	std::unique_ptr<FileReaderDescriptor> do_descriptor() const override;

	/**
	 * \brief Service method: acquire the physical file size in bytes.
	 *
	 * \param[in] filename File to retrieve the physical file size in bytes for
	 *
	 * \return The physical file size in bytes
	 */
	int64_t retrieve_file_size_bytes(const std::string &filename) const;

	/**
	 * \brief Worker method for process_file(): Read WAV file and optionally
	 * validate it for WAV and CDDA compliance.
	 *
	 * \param[in]  in         The ifstream to read from
	 * \param[in]  calculate  Flag to control if actual calculation is performed
	 * \param[out] total_pcm_bytes Number of total bytes representing PCM
	 * samples
	 *
	 * \return Number of actually read bytes
	 *
	 * \throw FileReadException If any problem occurred during reading from in
	 * \throw InvalidAudioException In case of unexpected data
	 */
	int64_t process_file_worker(std::ifstream &in,
			const bool &calculate,
			int64_t &total_pcm_bytes);

	/**
	 * \brief Read the WAV file and optionally validate it. This method provides
	 * the implementation of WavAudioReader::process_file().
	 *
	 * \param[in]  filename   The file to read from
	 * \param[in]  validate   Flag to control if actual validation is performed
	 * \param[in]  calculate  Flag to control if actual calculation is performed
	 * \param[out] total_pcm_bytes Number of total bytes representing PCM
	 * samples
	 *
	 * \return TRUE iff the file could be completely processed, otherwise FALSE
	 *
	 * \throw FileReadException If any problem occurred during reading from in
	 * \throw InvalidAudioException In case of unexpected data
	 */
	void process_file(const std::string &filename,
			const bool &validate,
			const bool &calculate,
			int64_t &total_pcm_bytes);

	/**
	 * \brief Validator handler instance.
	 */
	std::unique_ptr<WavAudioHandler> audio_handler_;
};

/// @}

} // namespace wave
} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

#endif

