/**
 * \file
 *
 * \brief Implements audio reader for RIFF/WAV audio files with PCM.
 */

#ifndef __LIBARCSDEC_READERWAV_HPP__
#include "readerwav.hpp"
#endif

extern "C" {
#include <assert.h>   // for assert
#include <sys/stat.h> // for stat
}

#include <cstdint>
#include <fstream>
#include <functional>
#include <locale>   // for tolower
#include <memory>
#include <sstream>
#include <string>

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>
#endif
#ifndef __LIBARCSTK_SAMPLES_HPP__
#include <arcstk/samples.hpp>
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
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

namespace
{

using arcstk::PCMForwardIterator;
using arcstk::AudioSize;
using arcstk::CDDA;
using arcstk::InvalidAudioException;
using arcstk::SampleSequence;


/**
 * \internal \defgroup readerwavImpl Implementation
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
	virtual ~WAV_CDDA_t() noexcept;

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
class RIFFWAV_PCM_CDDA_t : public WAV_CDDA_t
{

private:

	/**
	 * \brief Expected values including their lengths and byte offsets for a
	 * CDDA compliant RIFF WAV file with PCM samples.
	 */
	static constexpr uint32_t WAV_CDDA_[11][3] =
	{
		//pos len value

		// 0 - 1: RIFF WAVE chunk descriptor (12 bytes)
/*WAV*/	{ 0x00, 4, 0x52494646 },  // file format:      'RIFF'   (big endian)
		//0x04, 4,                // file size in bytes - 8
/*WAV*/	{ 0x08, 4, 0x57415645 },  // content format:   'WAVE'   (big endian)

		// 2 - 9: format subchunk (8 bytes header + 16 bytes data)
/*WAV*/	{ 0x0C, 4, 0x666d7420 },  // subchunk id:      'fmt '   (big endian)
		{ 0x10, 4, 0x00000010 },  // subchunk size:    16       (little endian)
/*PCM*/	{ 0x14, 2, 0x0001     },  // wFormatTag:       1 (==PCM)(little endian)
		{ 0x16, 2, 0x0002     },  // wChannels:        2        (little endian)
		{ 0x18, 4, 0x0000AC44 },  // dwSamplesPerSec:  44100    (little endian)
		{ 0x1C, 4, 0x0002B110 },  // dwAvgBytesPerSec: 176400   (little endian)
		{ 0x20, 2, 0x0004     },  // wBlockAlign:      4 bytes  (little endian)
		{ 0x22, 2, 0x0010     },  // wBitsPerSample:   16       (little endian)

		// 10: start of data subchunk
/*WAV*/ { 0x24, 4, 0x64617461 }   // subchunk id:      'data'   (big endian)
		//0x28, 4,                // subchunk size
	};

	/**
	 * \brief Constants for accessing first dimension of WAV_CDDA_.
	 */
	enum WAV_CDDA_ROW : uint32_t
	{
		// chunk descriptor
		RIFF                     =  0,  // chunk descriptor id
		WAVE                     =  1,  // chunk descriptor format

		// fmt subchunk
		FMT_SC_NAME              =  2,  // fmt subchunk name
		FMT_SC_SIZE              =  3,  // fmt subchunk size
		FMT_W_FORMAT_TAG         =  4,  // fmt wFormatTag
		FMT_W_CHANNELS           =  5,  // fmt wChannels
		FMT_DW_SAMPLES_PER_SEC   =  6,  // fmt dwSamplesPerSec
		FMT_DW_AVG_BYTES_PER_SEC =  7,  // fmt dwAvgBytesPerSec
		FMT_W_BLOCK_ALIGN        =  8,  // fmt wBlockAlign
		FMT_W_BITS_PER_SAMPLE    =  9,  // fmt wBitsPerSample

		// data subchunk
		DATA_SC_NAME             = 10   // data subchunk name
	};

	/**
	 * \brief Constants for accessing second dimension of WAV_CDDA_.
	 */
	enum WAV_CDDA_COL : uint32_t
	{
		POS      = 0, // Position
		LEN      = 1, // Length
		BYTE_VAL = 2  // Value
	};


protected:

	/**
	 * \brief Exclusive accessor method for WAV_CDDA_.
	 *
	 * \param[in] row Row to access
	 * \param[in] col Column to access
	 *
	 * \return Value at specified position
	 */
	uint32_t wav_cdda(const WAV_CDDA_ROW &row, const WAV_CDDA_COL &col) const
	{
		return WAV_CDDA_[row][col];
	}


public:

	/**
	 * \brief Virtual default destructor.
	 */
	~RIFFWAV_PCM_CDDA_t() noexcept override;

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
			const uint32_t &id,
			const uint32_t &size,
			const uint32_t &file_size,
			const uint32_t &format
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
	WavSubchunkHeader(const uint32_t &id, const uint32_t &size);

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
			const uint16_t &wFormatTag,
			const uint16_t &wChannels,
			const uint32_t &dwSamplesPerSec,
			const uint32_t &dwAvgBytesPerSec,
			const uint16_t &wBlockAlign,
			const uint16_t &wBitsPerSample
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
	const uint32_t size;

	/**
	 * \brief Parsed format identifier.
	 */
	const uint16_t wFormatTag;

	/**
	 * \brief Parsed number of channels.
	 */
	const uint16_t wChannels;

	/**
	 * \brief Parsed number of samples per second.
	 */
	const uint32_t dwSamplesPerSec;

	/**
	 * \brief Parsed average number of bytes per second.
	 */
	const uint32_t dwAvgBytesPerSec;

	/**
	 * \brief Parsed number of bytes per sample block.
	 */
	const uint16_t wBlockAlign;

	/**
	 * \brief Parsed number of bits per sample.
	 */
	const uint16_t wBitsPerSample;
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
	uint64_t physical_file_size();

	/**
	 * \brief Handler method: Called by AudioReaderImpl on start of the reading
	 * process.
	 *
	 * \param[in] filename Name of the audio file started to parse
	 * \param[in] phys_file_size Recognized physical file size
	 */
	void start_file(const std::string &filename,
			const uint64_t &phys_file_size);

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
	uint64_t phys_file_size_;

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
	explicit PCMBlockReader(const uint32_t &samples_per_block);

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
				PCMForwardIterator begin, PCMForwardIterator end)>
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
	uint64_t read_blocks(std::ifstream &in, const uint64_t &total_pcm_bytes);

	// make class non-copyable (2/2)
	PCMBlockReader& operator = (const PCMBlockReader &) = delete;

	// TODO Move assignment


private:

	/**
	 * \brief Registered callback method to consume a block.
	 *
	 * Called by block_complete().
	 */
	std::function<void(PCMForwardIterator begin, PCMForwardIterator end)>
		consume_;
};


/**
 * \brief File reader implementation for files in RIFF/WAVE (PCM) format, i.e.
 * containing 44.100 Hz/16 bit Stereo PCM samples in its data chunk.
 *
 * This class provides the PCM sample data as a succession of blocks
 * of 32 bit PCM samples to its \ref Calculation. The first block starts with the very
 * first PCM sample in the data chunk, i.e. in a compliant CDDA WAV file the 4
 * bytes following byte 0x2C. The format subchunk is validated to conform to
 * CDDA.
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
	virtual ~WavAudioReaderImpl() noexcept;

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

	/**
	 * \brief Service method: acquire the physical file size in bytes.
	 *
	 * \param[in] filename File to retrieve the physical file size in bytes for
	 *
	 * \return The physical file size in bytes
	 */
	uint64_t retrieve_file_size_bytes(const std::string &filename) const;

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
	uint64_t process_file_worker(std::ifstream &in,
			const bool &calculate,
			uint64_t &total_pcm_bytes);

	/**
	 * \brief Read the WAV file and optionally validate it. This method provides
	 * the implementation of WavAudioReader::process_file().
	 *
	 * \param[in]  in         The ifstream to read from
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
			uint64_t &total_pcm_bytes);

	/**
	 * \brief Validator handler instance.
	 */
	std::unique_ptr<WavAudioHandler> audio_handler_;
};


/// \cond UNDOC_FUNCTION_BODIES


// WAV_CDDA_t


WAV_CDDA_t::~WAV_CDDA_t() noexcept = default;


// RIFFWAV_PCM_CDDA_t


constexpr uint32_t RIFFWAV_PCM_CDDA_t::WAV_CDDA_[11][3];


RIFFWAV_PCM_CDDA_t::~RIFFWAV_PCM_CDDA_t() noexcept = default;


uint32_t RIFFWAV_PCM_CDDA_t::chunk_id() const
{
	return wav_cdda(RIFF, BYTE_VAL);
}


uint32_t RIFFWAV_PCM_CDDA_t::format() const
{
	return wav_cdda(WAVE, BYTE_VAL);
}


uint32_t RIFFWAV_PCM_CDDA_t::fmt_subchunk_id() const
{
	return wav_cdda(FMT_SC_NAME, BYTE_VAL);
}


uint32_t RIFFWAV_PCM_CDDA_t::fmt_subchunk_size() const
{
	return wav_cdda(FMT_SC_SIZE, BYTE_VAL);
}


uint16_t RIFFWAV_PCM_CDDA_t::wFormatTag() const
{
	return wav_cdda(FMT_W_FORMAT_TAG, BYTE_VAL);
}


uint16_t RIFFWAV_PCM_CDDA_t::wChannels() const
{
	return wav_cdda(FMT_W_CHANNELS, BYTE_VAL);
}


uint32_t RIFFWAV_PCM_CDDA_t::dwSamplesPerSec() const
{
	return wav_cdda(FMT_DW_SAMPLES_PER_SEC, BYTE_VAL);
}


uint32_t RIFFWAV_PCM_CDDA_t::dwAvgBytesPerSec() const
{
	return wav_cdda(FMT_DW_AVG_BYTES_PER_SEC, BYTE_VAL);
}


uint16_t RIFFWAV_PCM_CDDA_t::wBlockAlign() const
{
	return wav_cdda(FMT_W_BLOCK_ALIGN, BYTE_VAL);
}


uint16_t RIFFWAV_PCM_CDDA_t::wBitsPerSample() const
{
	return wav_cdda(FMT_W_BITS_PER_SAMPLE, BYTE_VAL);
}


uint32_t RIFFWAV_PCM_CDDA_t::data_subchunk_id() const
{
	return wav_cdda(DATA_SC_NAME, BYTE_VAL);
}


// WavChunkDescriptor


WavChunkDescriptor::WavChunkDescriptor(
			const uint32_t &id,
			const uint32_t &size,
			const uint32_t &file_size,
			const uint32_t &format
			)
	: id(id)
	, size(size)
	, file_size(file_size)
	, format(format)
{
	// empty
}


WavChunkDescriptor::~WavChunkDescriptor() noexcept = default;


// WavSubchunkHeader


WavSubchunkHeader::WavSubchunkHeader(const uint32_t &id, const uint32_t &size)
	: id(id)
	, size(size)
{
	// empty
}


WavSubchunkHeader::~WavSubchunkHeader() noexcept = default;


std::string WavSubchunkHeader::name() const
{
	static constexpr uint32_t mask = 0x000000FF;

	std::string name;

	name += id >> 24 & mask;
	name += id >> 16 & mask;
	name += id >>  8 & mask;
	name += id       & mask;

	return name;
}


// WavFormatSubchunk


WavFormatSubchunk::WavFormatSubchunk(
			const WavSubchunkHeader &header,
			const uint16_t &wFormatTag,
			const uint16_t &wChannels,
			const uint32_t &dwSamplesPerSec,
			const uint32_t &dwAvgBytesPerSec,
			const uint16_t &wBlockAlign,
			const uint16_t &wBitsPerSample
			)
	: id(header.id)
	, size(header.size)
	, wFormatTag(wFormatTag)
	, wChannels(wChannels)
	, dwSamplesPerSec(dwSamplesPerSec)
	, dwAvgBytesPerSec(dwAvgBytesPerSec)
	, wBlockAlign(wBlockAlign)
	, wBitsPerSample(wBitsPerSample)
{
	// empty
}


WavFormatSubchunk::~WavFormatSubchunk() noexcept = default;


// WavPartParser


// static definition
constexpr uint8_t WavPartParser::WAV_BYTES_PER_RIFF_HEADER;

// static definition
constexpr uint8_t WavPartParser::WAV_BYTES_PER_SUBCHUNK_HEADER;

// static definition
constexpr uint8_t WavPartParser::WAV_BYTES_IN_FMT_SUBCHUNK;


WavPartParser::WavPartParser()
	: convert_()
{
	// empty
}


WavPartParser::~WavPartParser() noexcept = default;


WavChunkDescriptor WavPartParser::chunk_descriptor(
	const std::vector<char> &bytes)
{
	if (bytes.size() < WAV_BYTES_PER_RIFF_HEADER)
	{
		return WavChunkDescriptor(0, 0, 0, 0);
	}

	return WavChunkDescriptor(

		// parse RIFF/RIFX header
		convert_.be_bytes_to_uint32(bytes[0], bytes[1], bytes[ 2], bytes[ 3]),

		// real header size in bytes
		WAV_BYTES_PER_RIFF_HEADER,

		// parse file size declaration
		convert_.le_bytes_to_int32 (bytes[4], bytes[5], bytes[ 6], bytes[ 7]),

		// parse file format declaration ("WAVE")
		convert_.be_bytes_to_uint32(bytes[8], bytes[9], bytes[10], bytes[11])
	);
}


WavSubchunkHeader WavPartParser::subchunk_header(const std::vector<char> &bytes)
{
	if (bytes.size() < WAV_BYTES_PER_SUBCHUNK_HEADER)
	{
		return WavSubchunkHeader(0, 0);
	}

	return WavSubchunkHeader(

		// parse subchunk id
		convert_.be_bytes_to_uint32(bytes[0], bytes[1], bytes[2], bytes[3]),

		// parse subchunk size
		convert_.le_bytes_to_uint32(bytes[4], bytes[5], bytes[6], bytes[7])
	);
}


WavFormatSubchunk WavPartParser::format_subchunk(
	const WavSubchunkHeader &header, const std::vector<char> &bytes)
{
	if (bytes.size() < WAV_BYTES_IN_FMT_SUBCHUNK)
	{
		return WavFormatSubchunk(WavSubchunkHeader(0, 0), 0, 0, 0, 0, 0, 0);
	}

	return WavFormatSubchunk(

		// subchunk id and size
		header,

		// wFormatTag
		convert_.le_bytes_to_uint16(bytes[ 0], bytes[ 1]),

		// wChannels
		convert_.le_bytes_to_uint16(bytes[ 2], bytes[ 3]),

		// dwSamplesPerSec
		convert_.le_bytes_to_uint32(bytes[ 4], bytes[ 5], bytes[ 6], bytes[ 7]),

		// dwAvgBytesPerSec
		convert_.le_bytes_to_uint32(bytes[ 8], bytes[ 9], bytes[10], bytes[11]),

		// wBlockAlign
		convert_.le_bytes_to_uint16(bytes[12], bytes[13]),

		// wBitsPerSample
		convert_.le_bytes_to_uint16(bytes[14], bytes[15])
	);
}


// WavAudioHandler


WavAudioHandler::WavAudioHandler(std::unique_ptr<WAV_CDDA_t> valid)
	: ReaderValidatingHandler()
	, phys_file_size_(0)
	, valid_(std::move(valid))
	, config_( C_RESPECT_HEADER | C_RESPECT_FORMAT | C_RESPECT_DATA )
	, state_( S_INITIAL )
{
	// empty
}


WavAudioHandler::~WavAudioHandler() noexcept = default;


uint64_t WavAudioHandler::physical_file_size()
{
	return phys_file_size_;
}


void WavAudioHandler::start_file(const std::string &filename,
		const uint64_t &phys_file_size)
{
	ARCS_LOG_DEBUG << "Start validating WAV file: " << filename;

	phys_file_size_ = phys_file_size;
}


void WavAudioHandler::end_file()
{
	ARCS_LOG_DEBUG << "Completed validation of WAV file";
}


void WavAudioHandler::start_subchunk(
		const WavSubchunkHeader &subchunk_header)
{
	if (subchunk_header.id == valid_->fmt_subchunk_id())
	{
		this->set_state(S_STARTED_FORMAT);
	}
	else if (subchunk_header.id == valid_->data_subchunk_id())
	{
		this->set_state(S_STARTED_DATA);
	}
}


void WavAudioHandler::chunk_descriptor(
	const WavChunkDescriptor &descriptor)
{
	this->do_chunk_descriptor(descriptor);

	if (this->has_errors())
	{
		this->fail(); // throws exception
	}
}


void WavAudioHandler::subchunk_format(
		const WavFormatSubchunk &format_subchunk)
{
	this->do_subchunk_format(format_subchunk);

	if (this->has_errors())
	{
		this->fail(); // throws exception
	}
}


void WavAudioHandler::subchunk_data(const uint32_t &subchunk_size)
{
	this->do_subchunk_data(subchunk_size);

	if (this->has_errors())
	{
		this->fail(); // throws exception
	}
}


void WavAudioHandler::do_chunk_descriptor(
	const WavChunkDescriptor &descriptor)
{
	ARCS_LOG(DEBUG1) << "Try to validate RIFF/WAV Header";

	if (not this->has_config(C_RESPECT_HEADER))
	{
		this->set_state(S_VALIDATED_HEADER);
		return;
	}

	if (not this->assert_equals("Test: RIFF Header present?",
		descriptor.id, valid_->chunk_id(),
		"Unexpected header start."))
	{
		ARCS_LOG_ERROR << this->last_error();
		return;
	}

	// RIFFWAV_PCM_CDDA declares file size - 8.
	// The magic value of 8 is the combined length of the 'RIFF' part and the
	// file size declaration itself

	if (not this->assert_equals(
		"Test: Declared file size conforms to physical file size?",
		descriptor.file_size + 2 * sizeof(uint32_t),
		this->physical_file_size(),
		"Unexpected header start."))
	{
		ARCS_LOG_ERROR << this->last_error();
		return;
	}

	if (not this->assert_equals("Test: Header declares WAVE format?",
		descriptor.format, valid_->format(),
		"Header does not declare WAVE format."))
	{
		ARCS_LOG_ERROR << this->last_error();
		return;
	}

	ARCS_LOG(DEBUG1) << "RIFF/WAV Header validated";

	this->set_state(S_VALIDATED_HEADER);
}


void WavAudioHandler::do_subchunk_format(
		const WavFormatSubchunk &format_subchunk)
{
	ARCS_LOG(DEBUG1) << "Try to validate format subchunk";

	if (not this->assert_equals("Test: id is 'fmt '?",
		format_subchunk.id, valid_->fmt_subchunk_id(),
		"Id of subchunk is not 'fmt ' but "
		+ std::to_string(format_subchunk.id)))
	{
		ARCS_LOG_ERROR << this->last_error();
		return;
	}

	if (not this->assert_equals("Test: format subchunk size",
		format_subchunk.size, valid_->fmt_subchunk_size(),
		"Unexpected format subchunk size."))
	{
		ARCS_LOG_ERROR << this->last_error();
		return;
	}

	{
		CDDAValidator validate;

		if (not this->assert_true("Test (CDDA): Channels [wChannels]",
			validate.num_channels(format_subchunk.wChannels),
			"Number of channels does not conform to CDDA"))
		{
			ARCS_LOG_ERROR << this->last_error();
			return;
		}

		if (not this->assert_true(
			"Test (CDDA): Samples per second [dwSamplesPerSec]",
			validate.samples_per_second(format_subchunk.dwSamplesPerSec),
			"Number of channels does not conform to CDDA"))
		{
			ARCS_LOG_ERROR << this->last_error();
			return;
		}

		if (not this->assert_true(
			"Test (CDDA): Bits per sample [wBitsPerSample]",
			validate.bits_per_sample(format_subchunk.wBitsPerSample),
			"Number of bits per sample does not conform to CDDA"))
		{
			ARCS_LOG_ERROR << this->last_error();
			return;
		}
	}

	if (not this->assert_equals("Test: wFormatTag is PCM",
		format_subchunk.wFormatTag, valid_->wFormatTag(),
		"wFormatTag is not PCM."))
	{
		ARCS_LOG_ERROR << this->last_error();
		return;
	}

	if (not this->assert_equals("Test: dwAvgBytesPerSec is CDDA",
		format_subchunk.dwAvgBytesPerSec, valid_->dwAvgBytesPerSec(),
		"dwAvgBytesPerSec is not CDDA."))
	{
		ARCS_LOG_ERROR << this->last_error();
		return;
	}

	if (not this->assert_equals("Test: wBlockAlign is CDDA",
		format_subchunk.wBlockAlign, valid_->wBlockAlign(),
		"wBlockAlign is not CDDA."))
	{
		ARCS_LOG_ERROR << this->last_error();
		return;
	}

	this->unset_state(S_STARTED_FORMAT);
	this->set_state(S_VALIDATED_FORMAT);

	ARCS_LOG(DEBUG1) << "Format subchunk validated";
}


void WavAudioHandler::do_subchunk_data(const uint32_t &subchunk_size)
{
	ARCS_LOG(DEBUG1) << "Try to validate data subchunk";

	if (not this->assert_true("Test: format subchunk before data subchunk?",
		this->has_state(S_VALIDATED_FORMAT),
		"Did not encounter format subchunk before data subchunk"))
	{
		// If the validation of the format subchunk would have
		// failed we would never have arrived here. It must
		// therefore mean that we did not encounter the format
		// subchunk so far.

		ARCS_LOG_ERROR << this->last_error();
		return;
	}

	if (not this->assert_true("Test: regular data subchunk size?",
		subchunk_size % CDDA.BYTES_PER_SAMPLE == 0,
		"Incomplete samples, subchunk size is not a multiple of "
		+ std::to_string(CDDA.BYTES_PER_SAMPLE)))
	{
		ARCS_LOG_ERROR << this->last_error();
		return;
	}

	this->unset_state(S_STARTED_DATA);
	this->set_state(S_VALIDATED_DATA);

	ARCS_LOG(DEBUG1) << "Data subchunk validated";
}


bool WavAudioHandler::has_state(const STATE &state)
{
	return state_ & state;
}


bool WavAudioHandler::requests_all_subchunks()
{
	return config_ & C_RESPECT_TRAILING;
}


void WavAudioHandler::fail()
{
	this->do_fail();
}


const WAV_CDDA_t& WavAudioHandler::validator()
{
	return *valid_;
}


void WavAudioHandler::set_state(const STATE &state)
{
	state_ |= state;
}


void WavAudioHandler::unset_state(const STATE &state)
{
	state_ &= ~state;
}


bool WavAudioHandler::has_config(const CONFIG &option)
{
	return config_ & option;
}


void WavAudioHandler::set_config(const CONFIG &option)
{
	config_ |= option;
}


void WavAudioHandler::do_fail()
{
	auto errors = this->get_errors();

	for (const auto& msg : errors)
	{
		ARCS_LOG_ERROR << msg;
	}

	ARCS_LOG_ERROR << "Validation failed";

	throw InvalidAudioException("Validation failed");
}


// PCMBlockReader


PCMBlockReader::PCMBlockReader(const uint32_t &samples_per_block)
	: BlockCreator(samples_per_block)
	, consume_()
{
	// empty
}


PCMBlockReader::~PCMBlockReader() noexcept = default;


void PCMBlockReader::register_block_consumer(const std::function<void(
			PCMForwardIterator begin, PCMForwardIterator end
		)> &consume)
{
	consume_ = consume;
}


uint64_t PCMBlockReader::read_blocks(std::ifstream &in,
		const uint64_t &total_pcm_bytes)
{
	std::vector<uint32_t> samples(this->samples_per_block());

	uint32_t bytes_per_block =
			this->samples_per_block() * CDDA.BYTES_PER_SAMPLE;

	uint32_t estimated_blocks = total_pcm_bytes / bytes_per_block
				+ (total_pcm_bytes % bytes_per_block ? 1 : 0);

	ARCS_LOG_DEBUG << "START READING " << total_pcm_bytes
		<< " bytes in " << std::to_string(estimated_blocks) << " blocks with "
		<< bytes_per_block << " bytes per block";

	uint32_t samples_todo = total_pcm_bytes / CDDA.BYTES_PER_SAMPLE;
	uint32_t total_bytes_read   = 0;
	uint32_t total_blocks_read  = 0;

	uint32_t read_bytes = this->samples_per_block() * sizeof(uint32_t);
	// FIXME Use sample type!

	while (total_bytes_read < total_pcm_bytes)
	{
		// Adjust buffer size for last buffer, if necessary

		if (samples_todo < this->samples_per_block())
		{
			// Avoid trailing zeros in buffer
			samples.resize(samples_todo);

			//read_bytes = samples_todo * sizeof(samples.front());
			read_bytes = samples_todo * sizeof(uint32_t); // FIXME Use sample type!
		}

		// Actually read the bytes

		try
		{
			in.read(reinterpret_cast<char*>(samples.data()), read_bytes);
		}
		catch (const std::ifstream::failure& f)
		{
			total_bytes_read += in.gcount();

			ARCS_LOG_ERROR << "Failed to read from file: " << f.what();

			throw FileReadException(f.what(), total_bytes_read + 1);
		}
		total_bytes_read += read_bytes;
		samples_todo     -= samples.size();

		// Logging + Statistics

		++total_blocks_read;

		ARCS_LOG_DEBUG << "READ BLOCK " << total_blocks_read
			<< "/" << estimated_blocks;
		ARCS_LOG(DEBUG1) << "Size: " << read_bytes << " bytes";
		ARCS_LOG(DEBUG1) << "      " << samples.size()
				<< " Stereo PCM samples (32 bit)";

		if (not consume_)
		{
			ARCS_LOG_ERROR << "No block consumer registered.";
			continue;
		}
		this->consume_(samples.begin(), samples.end());
	}

	ARCS_LOG_DEBUG << "END READING after " << total_blocks_read << " blocks";

	ARCS_LOG(DEBUG1) << "Read "
		<< std::to_string(total_bytes_read / CDDA.BYTES_PER_SAMPLE)
		<< " samples / "
		<< total_bytes_read << " bytes";

	return total_bytes_read;
}


// WavAudioReaderImpl


WavAudioReaderImpl::WavAudioReaderImpl()
	: audio_handler_()
{
	// empty
}


WavAudioReaderImpl::~WavAudioReaderImpl() noexcept = default;


uint64_t WavAudioReaderImpl::retrieve_file_size_bytes(
		const std::string &filename) const
{
	// TODO C-style stuff: Use C++17's std::filesystem in the future
	struct stat stat_buf; // needs include <sys/stat.h>
	{
		int rc = stat(filename.c_str(), &stat_buf);
		assert (rc != -1); // needs include <assert.h>

		// Avoid Warning about Unused Variable rc (ugly, but, well...)
		static_cast<void>(rc);
	}

	return stat_buf.st_size;
}


uint64_t WavAudioReaderImpl::process_file_worker(std::ifstream &in,
		const bool &calculate,
		uint64_t &total_pcm_bytes)
{
	uint64_t total_bytes_read = 0;

	// Read the first bytes and parse them as chunk descriptor.
	// NOTE: ifstream's exceptions (fail|bad)bit must be activated

	std::vector<char> bytes(WavPartParser::WAV_BYTES_PER_RIFF_HEADER);

	uint64_t bytes_to_read =
		WavPartParser::WAV_BYTES_PER_RIFF_HEADER * sizeof(bytes[0]);
	try
	{
		in.read(&bytes[0], bytes_to_read);
	}
	catch (const std::ifstream::failure& f)
	{
		total_bytes_read += in.gcount();

		ARCS_LOG_ERROR << "Failed to read chunk descriptor from file: "
				<< f.what();

		throw FileReadException(f.what(), total_bytes_read + 1);
	}
	total_bytes_read += bytes_to_read;

	WavPartParser parser;

	audio_handler_->chunk_descriptor(parser.chunk_descriptor(bytes));


	// Traverse subchunks, looking for 'fmt ' and 'data' thereafter

	bytes.resize(WavPartParser::WAV_BYTES_PER_SUBCHUNK_HEADER);

	int subchunk_counter = 0;

	while (not in.eof())
	{
		// Read subchunk header

		bytes_to_read =
			WavPartParser::WAV_BYTES_PER_SUBCHUNK_HEADER * sizeof(bytes[0]);
		try
		{
			in.read(&bytes[0], bytes_to_read);
		}
		catch (const std::ifstream::failure& f)
		{
			total_bytes_read += in.gcount();

			ARCS_LOG_ERROR << "Failed to read subchunk header from file: "
					<< f.what();

			throw FileReadException(f.what(), total_bytes_read + 1);
		}
		total_bytes_read += bytes_to_read;
		++subchunk_counter;

		WavSubchunkHeader subchunk_header = parser.subchunk_header(bytes);

		ARCS_LOG_DEBUG << "Start subchunk. ID: '"
			<< subchunk_header.name()
			<< "'  Size: "
			<< std::to_string(subchunk_header.size)
			<< " bytes";

		audio_handler_->start_subchunk(subchunk_header);

		const WAV_CDDA_t& valid = audio_handler_->validator();

		// Subchunk: 'fmt '

		if (subchunk_header.id == valid.fmt_subchunk_id())
		{
			// Ensure fmt subchunk to be first subchunk after chunk descriptor

			if (subchunk_counter != 1)
			{
				throw InvalidAudioException(
					"Format subchunk does not follow after chunk descriptor.");
			}

			// Read subchunk and pass its content

			ARCS_LOG(DEBUG1) << "Try to read format subchunk";

			std::vector<char> fmt_bytes(subchunk_header.size * sizeof(char));

			bytes_to_read = subchunk_header.size * sizeof(fmt_bytes[0]);
			try
			{
				in.read(&fmt_bytes[0], bytes_to_read);
			}
			catch (const std::ifstream::failure& f)
			{
				total_bytes_read += in.gcount();

				ARCS_LOG_ERROR << "Failed to read format subchunk from file: "
					<< f.what();

				throw FileReadException(f.what(), total_bytes_read + 1);
			}
			total_bytes_read += bytes_to_read;

			ARCS_LOG(DEBUG1) << "Format subchunk read successfully";

			audio_handler_->subchunk_format(
				parser.format_subchunk(subchunk_header, fmt_bytes));

			continue;
		} // fmt

		// Subchunk: 'data'

		if (subchunk_header.id == valid.data_subchunk_id())
		{
			// Pass total_pcm_bytes to Caller and inform \ref Calculation instance

			total_pcm_bytes = subchunk_header.size; // return parameter

			ARCS_LOG_INFO << "Total samples: " <<
				(total_pcm_bytes / CDDA.BYTES_PER_SAMPLE);

			audio_handler_->subchunk_data(subchunk_header.size);

			if (calculate)
			{
				AudioSize audiosize;
				audiosize.set_pcm_byte_count(subchunk_header.size);
				this->process_audiosize(audiosize);

				// Read audio bytes in blocks

				PCMBlockReader block_reader(this->samples_per_read());

				block_reader.register_block_consumer(
					std::bind(&WavAudioReaderImpl::process_samples, this,
						std::placeholders::_1, std::placeholders::_2)
					);

				uint64_t block_bytes_read =
					block_reader.read_blocks(in, total_pcm_bytes);

				total_bytes_read += block_bytes_read;

				if (block_bytes_read != total_pcm_bytes)
				{
					std::stringstream ss;
					ss << "Expected to read "
						<< total_pcm_bytes
						<< " audio bytes but could only read "
						<< block_bytes_read
						<< " audio bytes.";

					ARCS_LOG_ERROR << ss.str();

					throw FileReadException(ss.str(), total_bytes_read + 1);
				}
			}

			if (not audio_handler_->requests_all_subchunks())
			{
				ARCS_LOG_DEBUG << "Stop reading after data subchunk";

				uint64_t remainder =
					audio_handler_->physical_file_size() - total_bytes_read;

				if (remainder > 0)
				{
					ARCS_LOG_INFO << "Ignored "
						<< std::to_string(remainder)
						<< " bytes of unparsed data behind data subchunk."
						<< "(processed: "
						<< std::to_string(total_bytes_read)
						<< " bytes, file size: "
						<< std::to_string(audio_handler_->physical_file_size())
						<< " bytes)";
				}

				break;
			}

			continue;
		} // data

		// Ignore the content of all Subchunks except of 'fmt ' and 'data'

		in.ignore(subchunk_header.size);

		ARCS_LOG(DEBUG1) << "(Ignore subchunk)";
	} // while

	return total_bytes_read;
}


void WavAudioReaderImpl::process_file(const std::string &filename,
		const bool &validate,
		const bool &calculate,
		uint64_t &total_pcm_bytes)
{
	if (filename.empty())
	{
		ARCS_LOG_ERROR << "WAV filename is empty. End.";
		return;
	}

	if (not audio_handler_)
	{
		ARCS_LOG_ERROR << "No audio handler configured, return";
		return;
	}

	if (not validate)
	{
		// If no Validation is Requested, Turn off all Tests for any
		// Canonical File Part

		audio_handler_->set_config(C_EMPTY_CONFIG);
	}

	audio_handler_->start_file(filename, retrieve_file_size_bytes(filename));

	std::ifstream in;

	// Methods process_file_worker() and PCMBlockReader::read_blocks() rely on
	// the failbit and badbit exceptions being activated

	in.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		in.open(filename, std::ifstream::in | std::ifstream::binary);
	}
	catch (const std::ifstream::failure& f)
	{
		int64_t total_bytes_read = in.gcount();

		ARCS_LOG_ERROR << "Failed to open audio file: " << f.what();

		throw FileReadException(f.what(), total_bytes_read + 1);
	}

	uint64_t bytes_read =
		this->process_file_worker(in, calculate, total_pcm_bytes);

	ARCS_LOG_DEBUG << "Read " << bytes_read << " bytes from audio file";

	audio_handler_->end_file();

	try
	{
		in.close();
	}
	catch (const std::ifstream::failure& f)
	{
		ARCS_LOG_ERROR << "Failed to close audio file: " << f.what();
		throw FileReadException(f.what());
	}

	ARCS_LOG(DEBUG1) << "Audio file closed.";
}


std::unique_ptr<AudioSize> WavAudioReaderImpl::do_acquire_size(
	const std::string &audiofilename)
{
	uint64_t total_pcm_bytes = 0;

	try
	{
		// Neither Validate Nor Calculate

		this->process_file(audiofilename, false, false, total_pcm_bytes);
	}
	catch (const std::ifstream::failure& f)
	{
		if (total_pcm_bytes == 0)
		{
			throw;
		}

		ARCS_LOG_WARNING << "Acquired context but there was an exception"
			<< " while reading the audiofile: " << f.what();
	}

	std::unique_ptr<AudioSize> audiosize = std::make_unique<AudioSize>();
	audiosize->set_pcm_byte_count(total_pcm_bytes);
	return audiosize;
}


void WavAudioReaderImpl::do_process_file(const std::string &audiofilename)
{
	uint64_t total_pcm_bytes = 0;

	// Validate And Calculate

	this->process_file(audiofilename, true, true, total_pcm_bytes);
}


void WavAudioReaderImpl::register_audio_handler(
		std::unique_ptr<WavAudioHandler> hndlr)
{
	audio_handler_ = std::move(hndlr);
}

/// \endcond

/// @}

} // namespace

/// \cond UNDOC_FUNCTION_BODIES

// DescriptorWavPCM


DescriptorWavPCM::~DescriptorWavPCM() noexcept = default;


std::string DescriptorWavPCM::do_name() const
{
	return "RIFF/WAV (PCM)";
}


bool DescriptorWavPCM::do_accepts_bytes(const std::vector<char> &bytes,
		const uint64_t &offset) const
{
	if (not offset == 0 or bytes.size() < 22)
	{
		return false;
	}

	RIFFWAV_PCM_CDDA_t correct;
	ByteConverter actual;

	return
		correct.chunk_id() == // RIFF
		actual.be_bytes_to_uint32(bytes[0], bytes[1], bytes[2], bytes[3])
		and

		// bytes 4 - 6 are irrelevant for recognition

		correct.format() == // WAVE
		actual.be_bytes_to_uint32(bytes[8], bytes[9], bytes[10], bytes[11])
		and

		// bytes 12 - 19 are part of fmt subchunk (pointless for recognition)

		correct.wFormatTag() == // PCM
		actual.le_bytes_to_uint16(bytes[20], bytes[21]);
}


bool DescriptorWavPCM::do_accepts_suffix(const std::string &suffix) const
{
	std::locale locale;
	return std::tolower(suffix, locale) == "wav";
}


std::unique_ptr<FileReader> DescriptorWavPCM::do_create_reader() const
{
	auto impl = std::make_unique<WavAudioReaderImpl>();

	std::unique_ptr<WAV_CDDA_t> valid = std::make_unique<RIFFWAV_PCM_CDDA_t>();
	auto handler = std::make_unique<WavAudioHandler>(std::move(valid));
	impl->register_audio_handler(std::move(handler));

	return std::make_unique<AudioReader>(std::move(impl));
}


std::unique_ptr<FileReaderDescriptor> DescriptorWavPCM::do_clone() const
{
	return std::make_unique<DescriptorWavPCM>();
}

/// \endcond

} // namespace v_1_0_0

} // namespace arcsdec

