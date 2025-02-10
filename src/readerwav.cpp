/**
 * \file
 *
 * \brief Implements audio reader for RIFF/WAV audio files with PCM.
 */

#ifndef __LIBARCSDEC_READERWAV_HPP__
#include "readerwav.hpp"
#endif
#ifndef __LIBARCSDEC_READERWAV_DETAILS_HPP__
#include "readerwav_details.hpp" // for WavAudioHandler, RIFFWAV_PCM_CDDA_t
#endif

extern "C" {
#include <assert.h>   // for assert
#include <sys/stat.h> // for ::stat
}

#include <algorithm>  // for mismatch
#include <array>      // for array
#include <cstdint>    // for uint8_t, uint16_t, uint32_t, int32_t, int64_t
#include <fstream>    // for ifstream
#include <ios>        // for streamsize
#include <limits>     // for numeric_limits
#include <memory>     // for unique_ptr
#include <set>        // for set
#include <sstream>    // for ostringstream
#include <string>     // for string, to_string
#include <utility>    // for make_unique, move
#include <vector>     // for vector

#if __cplusplus >= 201703L
#include <filesystem>
#endif

#ifndef __LIBARCSTK_METADATA_HPP__
#include <arcstk/metadata.hpp>  // for AudioSize, UNIT, CDDA
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>   // for ARCS_LOG, _ERROR, _WARNING, _INFO, _DEBUG
#endif

#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"  // for AudioReaderImpl, *EndianBytes,
#endif
#ifndef __LIBARCSDEC_LIBINSPECT_HPP__
#include "libinspect.hpp"   // for first_libname_match
#endif
#ifndef __LIBARCSDEC_SAMPLEPROC_HPP__
#include "sampleproc.hpp"   // for BLOCKSIZE
#endif
#ifndef __LIBARCSDEC_SELECTION_HPP__
#include "selection.hpp"    // for RegisterDescriptor
#endif
#ifndef __LIBARCSDEC_VERSION_HPP__
#include "version.hpp"      // for LIBARCSDEC_NAME
#endif


namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{

using sample_t = uint32_t;

namespace wave
{

using arcstk::AudioSize;
using arcstk::CDDA;
using arcstk::UNIT;


// RIFFWAV_PCM_CDDA_t

constexpr int           RIFFWAV_PCM_CDDA_t::HEADER_FIELD_COUNT_;

constexpr unsigned int  RIFFWAV_PCM_CDDA_t::BYTES_[13][2];

constexpr unsigned char RIFFWAV_PCM_CDDA_t::any_;

const std::array<unsigned char, 40> RIFFWAV_PCM_CDDA_t::WAVPCM_HEADER_ =
{
	0x52, 0x49, 0x46, 0x46, // BE: 'R','I','F','F'
	any_, any_, any_, any_, // LE: filesize in bytes - 8
	0x57, 0x41, 0x56, 0x45, // BE: 'W','A','V','E'
	0x66, 0x6D, 0x74, 0x20, // BE: Format Subchunk Header: 'f','m','t',' '
	0x10, 0x00, 0x00, 0x00, // LE: Format Subchunk Size: '16'
	0x01, 0x00,             // LE: wFormatTag: 1 (means: PCM),
	0x02, 0x00,             // LE: wChannels: 2 (means: stereo)
	0x44, 0xAC, 0x00, 0x00, // LE: dwSamplesPerSec:  44100
	0x10, 0xB1, 0x02, 0x00, // LE: dwAvgBytesPerSec: 176400
	0x04, 0x00,             // LE: wBlockAlign: 4
	0x10, 0x00,             // LE: wBitsPerSample: 16
	0x64, 0x61, 0x74, 0x61  // BE: Data Subchunk Header: 'd','a','t','a'
                            // LE: Data Subchunk Size
};


uint32_t RIFFWAV_PCM_CDDA_t::header(FIELD field) const
{
	const auto offset = BYTES_[field][OFFSET];
	const auto endpos = BYTES_[field][LENGTH] - 1;

	uint32_t field_val = 0;

	switch (field)
	{
		case RIFF:
		case WAVE:
		case FMT_SC_NAME:
		case DATA_SC_NAME:
			// Big endian decode
			for (unsigned int i = endpos; i < BYTES_[field][LENGTH]; --i)
			{
				field_val |= static_cast<uint32_t>(
						WAVPCM_HEADER_[offset + i] << (endpos - i) * 8);
			}
			break;
		default:
			// Little endian decode
			for (unsigned int i = endpos; i < BYTES_[field][LENGTH]; --i)
			{
				field_val |= static_cast<uint32_t>(
						WAVPCM_HEADER_[offset + i] << i * 8);
			}
	}

	return field_val;
}


uint32_t RIFFWAV_PCM_CDDA_t::chunk_id() const
{
	return header(FIELD::RIFF);
}


uint32_t RIFFWAV_PCM_CDDA_t::format() const
{
	return header(FIELD::WAVE);
}


uint32_t RIFFWAV_PCM_CDDA_t::fmt_subchunk_id() const
{
	return header(FIELD::FMT_SC_NAME);
}


uint32_t RIFFWAV_PCM_CDDA_t::fmt_subchunk_size() const
{
	return header(FIELD::FMT_SC_SIZE);
}


uint16_t RIFFWAV_PCM_CDDA_t::wFormatTag() const
{
	return header(FIELD::FMT_W_FORMAT_TAG);
}


uint16_t RIFFWAV_PCM_CDDA_t::wChannels() const
{
	return header(FIELD::FMT_W_CHANNELS);
}


uint32_t RIFFWAV_PCM_CDDA_t::dwSamplesPerSec() const
{
	return header(FIELD::FMT_DW_SAMPLES_PER_SEC);
}


uint32_t RIFFWAV_PCM_CDDA_t::dwAvgBytesPerSec() const
{
	return header(FIELD::FMT_DW_AVG_BYTES_PER_SEC);
}


uint16_t RIFFWAV_PCM_CDDA_t::wBlockAlign() const
{
	return header(FIELD::FMT_W_BLOCK_ALIGN);
}


uint16_t RIFFWAV_PCM_CDDA_t::wBitsPerSample() const
{
	return header(FIELD::FMT_W_BITS_PER_SAMPLE);
}


uint32_t RIFFWAV_PCM_CDDA_t::data_subchunk_id() const
{
	return header(FIELD::DATA_SC_NAME);
}


const std::array<unsigned char, 40>& RIFFWAV_PCM_CDDA_t::header()
{
	return WAVPCM_HEADER_;
}


const unsigned char& RIFFWAV_PCM_CDDA_t::any_byte()
{
	return any_;
}


// WavChunkDescriptor


WavChunkDescriptor::WavChunkDescriptor(
			uint32_t id_,
			uint32_t size_,
			uint32_t file_size_,
			uint32_t format_
			)
	: id        { id_        }
	, size      { size_      }
	, file_size { file_size_ }
	, format    { format_    }
{
	// empty
}


WavChunkDescriptor::~WavChunkDescriptor() noexcept = default;


// WavSubchunkHeader


WavSubchunkHeader::WavSubchunkHeader(uint32_t id_, int64_t size_)
	: id   { id_   }
	, size { size_ }
{
	// empty
}


WavSubchunkHeader::~WavSubchunkHeader() noexcept = default;


// WavFormatSubchunk


WavFormatSubchunk::WavFormatSubchunk(
			const WavSubchunkHeader& header,
			int     wFormatTag_,
			int     wChannels_,
			int64_t dwSamplesPerSec_,
			int64_t dwAvgBytesPerSec_,
			int     wBlockAlign_,
			int     wBitsPerSample_
			)
	: id               ( header.id         )
	, size             ( header.size       )
	, wFormatTag       ( wFormatTag_       )
	, wChannels        ( wChannels_        )
	, dwSamplesPerSec  ( dwSamplesPerSec_  )
	, dwAvgBytesPerSec ( dwAvgBytesPerSec_ )
	, wBlockAlign      ( wBlockAlign_      )
	, wBitsPerSample   ( wBitsPerSample_   )
{
	// empty
}


WavFormatSubchunk::~WavFormatSubchunk() noexcept = default;


// WavValidator


WavValidator::WavValidator()
	: valid_ { std::make_unique<RIFFWAV_PCM_CDDA_t>() }
{
	// empty
}


void WavValidator::chunk_descriptor(const WavChunkDescriptor& descriptor,
		const int64_t file_size)
{
	ARCS_LOG(DEBUG1) << "Try to validate RIFF/WAV Header";

	fail_if(not assert_equals_u("Test: RIFF Header present?",
		descriptor.id, valid_->chunk_id(),
		"Unexpected RIFF-Header start"));

	// RIFFWAV_PCM_CDDA declares file size - 8.
	// The magic value of 8 is the combined length of the 'RIFF' part and the
	// file size declaration itself

	fail_if(not assert_equals_u(
		"Test: Declared file size conforms to physical file size?",
		descriptor.file_size + 2 * sizeof(uint32_t), file_size,
		"Physical filesize differs from size declaration in RIFF-Header."));

	fail_if(not assert_equals_u("Test: Header declares WAVE format?",
		descriptor.format, valid_->format(),
		"RIFF-Header does not declare WAVE format"));

	ARCS_LOG(DEBUG1) << "RIFF/WAV Header validated";
}


void WavValidator::subchunk_format(const WavFormatSubchunk& fmt)
{
	ARCS_LOG(DEBUG1) << "Try to validate format subchunk";

	using std::to_string;

	fail_if(not assert_equals_u("Test: id is 'fmt '?",
		fmt.id, valid_->fmt_subchunk_id(),
		"Id of subchunk is not 'fmt ' but " + to_string(fmt.id)));

	fail_if(not assert_equals_u("Test: format subchunk size",
		fmt.size, valid_->fmt_subchunk_size(),
		"Unexpected format subchunk size"));

	// by DefaultValidator:
	validate_bits_per_sample(fmt.wBitsPerSample);
	validate_samples_per_second(fmt.dwSamplesPerSec);
	validate_num_channels(fmt.wChannels);

	fail_if(not assert_equals("Test: wFormatTag is PCM",
		fmt.wFormatTag, valid_->wFormatTag(),
		"wFormatTag is not PCM"));

	fail_if(not assert_equals_u("Test: dwAvgBytesPerSec is CDDA",
		fmt.dwAvgBytesPerSec, valid_->dwAvgBytesPerSec(),
		"dwAvgBytesPerSec is not CDDA"));

	fail_if(not assert_equals("Test: wBlockAlign is CDDA",
		fmt.wBlockAlign, valid_->wBlockAlign(),
		"wBlockAlign is not CDDA"));

	ARCS_LOG(DEBUG1) << "Format subchunk validated";
}


void WavValidator::subchunk_data(const uint32_t& subchunk_size)
{
	ARCS_LOG(DEBUG1) << "Try to validate data subchunk";

	//fail_if(not assert_true("Test: format subchunk before data subchunk?",
	//	has_state(S_COMPLETED_FORMAT),
	//	"Did not encounter format subchunk before data subchunk"));
	// If the validation of the format subchunk would have
	// failed we would never have arrived here. It must
	// therefore mean that we did not encounter the format
	// subchunk so far.

	using std::to_string;

	fail_if(not assert_true("Test: regular data subchunk size?",
		static_cast<int>(subchunk_size) % CDDA::BYTES_PER_SAMPLE == 0,
		"Incomplete samples, subchunk size is not a multiple of "
		+ to_string(CDDA::BYTES_PER_SAMPLE)));

	ARCS_LOG(DEBUG1) << "Data subchunk validated";
}


const WAV_CDDA_t* WavValidator::wav_cdda() const
{
	return valid_.get();
}


AudioValidator::codec_set_type WavValidator::do_codecs() const
{
	return {
		Codec::PCM_S16BE,
		Codec::PCM_S16BE_PLANAR,
		Codec::PCM_S16LE,
		Codec::PCM_S16LE_PLANAR,
		Codec::PCM_S32BE,
		Codec::PCM_S32BE_PLANAR,
		Codec::PCM_S32LE,
		Codec::PCM_S32LE_PLANAR
	};
}


// WavAudioHandler


WavAudioHandler::WavAudioHandler()
	: phys_file_size_ { 0 }
	, config_ { C_RESPECT_HEADER | C_RESPECT_FORMAT | C_RESPECT_DATA }
	, state_  { S_INITIAL }
	, validator_ { /* default */ }
{
	// empty
}


int64_t WavAudioHandler::physical_file_size() const
{
	return phys_file_size_;
}


void WavAudioHandler::start_file(const std::string& filename,
		const int64_t& phys_file_size)
{
	ARCS_LOG_DEBUG << "Start reading WAV file: " << filename;

	phys_file_size_ = phys_file_size;
}


void WavAudioHandler::end_file()
{
	ARCS_LOG_DEBUG << "Completed reading of WAV file";
}


void WavAudioHandler::start_subchunk(
		const WavSubchunkHeader& subchunk_header)
{
	if (subchunk_header.id == validator_.wav_cdda()->fmt_subchunk_id())
	{
		set_state(S_STARTED_FORMAT);
	}
	else if (subchunk_header.id == validator_.wav_cdda()->data_subchunk_id())
	{
		set_state(S_STARTED_DATA);
	}
}


void WavAudioHandler::chunk_descriptor(
	const WavChunkDescriptor& descriptor)
{
	if (!has_option(C_RESPECT_HEADER))
	{
		ARCS_LOG(DEBUG1) << "Skip validation of RIFF/WAV Header";

		set_state(S_COMPLETED_HEADER);
		return;
	}

	validator_.chunk_descriptor(descriptor, physical_file_size());

	set_state(S_COMPLETED_HEADER);
}


void WavAudioHandler::subchunk_format(
		const WavFormatSubchunk& fmt)
{
	if (!has_option(C_RESPECT_FORMAT))
	{
		ARCS_LOG(DEBUG1) << "Skip validation of format subchunk";

		set_state(S_COMPLETED_FORMAT);
		return;
	}

	validator_.subchunk_format(fmt);

	unset_state(S_STARTED_FORMAT);
	set_state(S_COMPLETED_FORMAT);
}


void WavAudioHandler::subchunk_data(const uint32_t& subchunk_size)
{
	if (!has_option(C_RESPECT_DATA))
	{
		ARCS_LOG(DEBUG1) << "Skip validation of data subchunk";

		set_state(S_COMPLETED_DATA);
		return;
	}

	validator_.subchunk_data(subchunk_size);

	unset_state(S_STARTED_DATA);
	set_state(S_COMPLETED_DATA);
}


bool WavAudioHandler::has_state(const STATE& state) const
{
	return state_ & state;
}


bool WavAudioHandler::requests_all_subchunks() const
{
	return config_ & C_RESPECT_TRAILING;
}


const WavValidator* WavAudioHandler::validator() const
{
	return &validator_;
}


void WavAudioHandler::set_state(const STATE& state)
{
	state_ |= state;
}


void WavAudioHandler::unset_state(const STATE& state)
{
	state_ &= ~state;
}


uint32_t WavAudioHandler::config() const
{
	return config_;
}


void WavAudioHandler::set_config(const CONFIG& config)
{
	config_ = config;
}


bool WavAudioHandler::has_option(const CONFIG& option) const
{
	return config_ & option;
}


void WavAudioHandler::set_option(const CONFIG& option)
{
	config_ |= option;
}


// WavAudioReaderImpl


WavAudioReaderImpl::WavAudioReaderImpl()
	: WavAudioReaderImpl { nullptr }
{
	// empty
}


WavAudioReaderImpl::WavAudioReaderImpl(std::unique_ptr<WavAudioHandler> hndlr)
	: audio_handler_ { std::move(hndlr) }
{
	// empty
}


WavAudioReaderImpl::~WavAudioReaderImpl() noexcept = default;


std::unique_ptr<AudioSize> WavAudioReaderImpl::do_acquire_size(
	const std::string& audiofilename)
{
	auto total_pcm_bytes = int64_t { 0 };

	try
	{
		// Do not validate, do not calculate, do not emit AudioReader signals

		wav_process_file(audiofilename, samples_per_read(),
				nullptr /* no AudioHandler, no validation */,
				nullptr /* no AudioReader, no signal emission */,
				total_pcm_bytes);
	}
	catch (const std::ifstream::failure& f)
	{
		ARCS_LOG_WARNING << "After byte "
			<< total_pcm_bytes
			<< " an exception occured while reading the audiofile: "
			<< f.what();

		if (total_pcm_bytes == 0)
		{
			throw;
		}
	}

	return std::make_unique<AudioSize>(total_pcm_bytes, UNIT::BYTES);
}


void WavAudioReaderImpl::do_process_file(const std::string& audiofilename)
{
	// Validate And Calculate, emit AudioReader signals

	auto total_pcm_bytes = int64_t { 0 }; /* ignore */
	wav_process_file(audiofilename, samples_per_read(), audio_handler_.get(),
			this, total_pcm_bytes);
}


std::unique_ptr<FileReaderDescriptor> WavAudioReaderImpl::do_descriptor()
	const
{
	return std::make_unique<DescriptorWavPCM>();
}


const WavAudioHandler* WavAudioReaderImpl::audio_handler() const
{
	return audio_handler_.get();
}


void WavAudioReaderImpl::set_audio_handler(
		std::unique_ptr<WavAudioHandler> hndlr)
{
	audio_handler_ = std::move(hndlr);
}


// subchunk_name


std::string subchunk_name(const uint32_t id)
{
	static constexpr uint32_t mask = 0x000000FF;

	std::string name;

	name += static_cast<char>(id >> 24u & mask);
	name += static_cast<char>(id >> 16u & mask);
	name += static_cast<char>(id >>  8u & mask);
	name += static_cast<char>(id        & mask);

	return name;
}


// static definition
constexpr uint8_t WAV::BYTES_PER_RIFF_HEADER;

// static definition
constexpr uint8_t WAV::BYTES_PER_SUBCHUNK_HEADER;

// static definition
constexpr uint8_t WAV::BYTES_IN_FMT_SUBCHUNK;


// wav_parse_chunk_descriptor


WavChunkDescriptor wav_parse_chunk_descriptor(const std::vector<char>& bytes)
{
	if (bytes.size() < WAV::BYTES_PER_RIFF_HEADER)
	{
		return WavChunkDescriptor(0, 0, 0, 0);
	}

	return WavChunkDescriptor(

		// parse RIFF/RIFX header
		BigEndianBytes::to_uint32(bytes[0], bytes[1], bytes[ 2], bytes[ 3]),

		// real header size in bytes
		WAV::BYTES_PER_RIFF_HEADER,

		// parse file size declaration
		LittleEndianBytes::to_uint32(bytes[4], bytes[5], bytes[ 6], bytes[ 7]),

		// parse file format declaration ("WAV")
		BigEndianBytes::to_uint32(bytes[8], bytes[9], bytes[10], bytes[11])
	);
}


// wav_parse_subchunk_header


WavSubchunkHeader wav_parse_subchunk_header(const std::vector<char>& bytes)
{
	if (bytes.size() < WAV::BYTES_PER_SUBCHUNK_HEADER)
	{
		return WavSubchunkHeader(0, 0);
	}

	return WavSubchunkHeader(

		// parse subchunk id
		BigEndianBytes::to_uint32(bytes[0], bytes[1], bytes[2], bytes[3]),

		// parse subchunk size
		static_cast<int64_t>(
			LittleEndianBytes::to_uint32(
				bytes[4], bytes[5], bytes[6], bytes[7]))
	);
}


// wav_parse_format_subchunk


WavFormatSubchunk wav_format_subchunk(const WavSubchunkHeader& header,
		const std::vector<char>& bytes)
{
	if (bytes.size() < WAV::BYTES_IN_FMT_SUBCHUNK)
	{
		return WavFormatSubchunk(WavSubchunkHeader(0, 0), 0, 0, 0, 0, 0, 0);
	}

	return WavFormatSubchunk(

		// subchunk id and size
		header,

		// wFormatTag
		LittleEndianBytes::to_uint16(bytes[ 0], bytes[ 1]),

		// wChannels
		LittleEndianBytes::to_uint16(bytes[ 2], bytes[ 3]),

		// dwSamplesPerSec
		static_cast<int>(
		LittleEndianBytes::to_uint32(bytes[ 4], bytes[ 5], bytes[ 6], bytes[ 7])
		),

		// dwAvgBytesPerSec
		static_cast<int>(
		LittleEndianBytes::to_uint32(bytes[ 8], bytes[ 9], bytes[10], bytes[11])
		),

		// wBlockAlign
		LittleEndianBytes::to_uint16(bytes[12], bytes[13]),

		// wBitsPerSample
		LittleEndianBytes::to_uint16(bytes[14], bytes[15])
	);
}


// wav_read_pcm_data


int64_t wav_read_pcm_data(std::ifstream& in,
		const int64_t    samples_per_read,
		AudioReaderImpl& audio_reader,
		const int64_t&   total_pcm_bytes)
{
	using std::to_string;

	auto samples = std::vector<sample_t>();

	const auto sample_type_size { static_cast<int64_t>(sizeof(samples[0])) };

	const auto bytes_per_block =
		int64_t { samples_per_read * CDDA::BYTES_PER_SAMPLE };

	const int estimated_blocks = total_pcm_bytes / bytes_per_block
				+ (total_pcm_bytes % bytes_per_block ? 1 : 0);

	ARCS_LOG_DEBUG << "START READING " << total_pcm_bytes
		<< " bytes in " << to_string(estimated_blocks) << " blocks with "
		<< bytes_per_block << " bytes per block";

	int32_t samples_todo = total_pcm_bytes / CDDA::BYTES_PER_SAMPLE;

	auto total_bytes_read  = int64_t { 0 };
	auto total_blocks_read = int64_t { 0 };
	auto read_bytes        = int64_t { samples_per_read * sample_type_size };

	using buffersize_t = typename decltype(samples)::size_type;
	samples.resize(static_cast<buffersize_t>(samples_per_read));

	while (total_bytes_read < total_pcm_bytes)
	{
		// Adjust buffer size for last buffer, if necessary

		if (samples_todo < samples_per_read)
		{
			// Avoid trailing zeros in buffer
			samples.resize(static_cast<buffersize_t>(samples_todo));

			read_bytes = samples_todo * sample_type_size;
		}

		// Actually read the bytes

		try
		{
			in.read(reinterpret_cast<char*>(samples.data()), read_bytes);
		}
		catch (const std::ifstream::failure& f)
		{
			total_bytes_read += in.gcount();
			throw FileReadException(f.what(), total_bytes_read + 1);
		}
		total_bytes_read += read_bytes;
		samples_todo     -= static_cast<int32_t>(samples.size());

		// Logging + Statistics

		++total_blocks_read;

		ARCS_LOG_DEBUG << "READ BLOCK " << total_blocks_read
			<< "/" << estimated_blocks;
		ARCS_LOG(DEBUG1) << "Size: " << read_bytes << " bytes";
		ARCS_LOG(DEBUG1) << "      " << samples.size()
				<< " Stereo PCM samples (32 bit)";

		using std::cbegin;
		using std::cend;

		audio_reader.signal_appendsamples(cbegin(samples), cend(samples));
	}

	ARCS_LOG_DEBUG << "END READING after " << total_blocks_read << " blocks";

	ARCS_LOG(DEBUG1) << "Read "
		<< to_string(total_bytes_read / CDDA::BYTES_PER_SAMPLE)
		<< " samples / "
		<< total_bytes_read << " bytes";

	return total_bytes_read;
}


// wav_read_bytes


int64_t wav_read_bytes(std::ifstream& in, const int32_t amount,
			std::vector<char>& bytes, int64_t& byte_count)
{
	ARCS_LOG(DEBUG1) << "Read " << amount << " bytes from wav file";

	try
	{
		in.read(&bytes[0], amount);
	}
	catch (const std::ifstream::failure& f)
	{
		byte_count += in.gcount();
		throw FileReadException(f.what(), byte_count + 1);
	}
	byte_count += amount;
	return in.gcount();
}


// wav_process_file_worker


int64_t wav_process_file_worker(std::ifstream& in,
		const int64_t    samples_per_read,
		WavAudioHandler* audio_handler,
		AudioReaderImpl* audio_reader,
		int64_t&         total_pcm_bytes)
{
	using std::to_string;

	// NOTE: ifstream's exceptions (fail|bad)bit must be activated

	if (audio_reader)
	{
		audio_reader->signal_startinput();
	}

	// byte buffer
	auto bytes { std::vector<char>(WAV::BYTES_PER_RIFF_HEADER) };
	// total number of bytes to read next
	auto bytes_to_read { std::streamsize {
		WAV::BYTES_PER_RIFF_HEADER * sizeof(bytes[0]) }};
	// total number of bytes read so far
	auto total_bytes_read = int64_t { 0 };


	// Parse RIFF chunk descriptor

	wav_read_bytes(in, bytes_to_read, bytes, total_bytes_read);
	if (audio_handler)
	{
		audio_handler->chunk_descriptor(wav_parse_chunk_descriptor(bytes));
	} else
	{
		ARCS_LOG(DEBUG1) << "Skip RIFF/RIFX header";
	}


	// Traverse subchunks
	// Ignore every subchunk except 'fmt' and 'data'

	bytes.resize(WAV::BYTES_PER_SUBCHUNK_HEADER);
	auto subchunk_counter = int     { 0 };
	auto subchunk_size    = int64_t { 0 };

	while (not in.eof()) // traverse subchunks
	{
		// Read subchunk header

		bytes_to_read = WAV::BYTES_PER_SUBCHUNK_HEADER * sizeof(bytes[0]);
		wav_read_bytes(in, bytes_to_read, bytes, total_bytes_read);

		++subchunk_counter;

		const auto subchunk_header { wav_parse_subchunk_header(bytes) };

		ARCS_LOG_DEBUG << "Start subchunk. ID: '"
			<< subchunk_name(subchunk_header.id)
			<< "'  Size: "
			<< to_string(subchunk_header.size)
			<< " bytes";

		if (audio_handler)
		{
			audio_handler->start_subchunk(subchunk_header);
		}

		// Subchunk is 'fmt ' ?

		if (0x666d7420u == subchunk_header.id)
		{
			// Ensure fmt subchunk to be first subchunk after chunk descriptor

			if (subchunk_counter != 1)
			{
				throw InvalidAudioException(
					"Format subchunk does not follow after chunk descriptor.");
			}

			// Read subchunk and pass its content

			ARCS_LOG(DEBUG1) << "Try to read format subchunk";

			std::vector<char> fmt_bytes(
				static_cast<std::size_t>(subchunk_header.size) * sizeof(char));

			bytes_to_read = subchunk_header.size *
				static_cast<int>(sizeof(fmt_bytes[0]));
			// TODO Is sizeof(char) guaranteed to be less or equal to max<int>?

			wav_read_bytes(in, bytes_to_read, fmt_bytes, total_bytes_read);

			ARCS_LOG(DEBUG1) << "Format subchunk read successfully";

			if (audio_handler)
			{
				audio_handler->subchunk_format(
					wav_format_subchunk(subchunk_header, fmt_bytes));
			}

			continue;
		} // fmt

		// Subchunk is 'data' ?

		if (0x64617461u == subchunk_header.id)
		{
			subchunk_size = subchunk_header.size;

			// Pass total_pcm_bytes to Caller and inform Calculation instance
			total_pcm_bytes = subchunk_size; // remind: output parameter
			ARCS_LOG(DEBUG1) << "Total number of audio bytes: "
				<< total_pcm_bytes;

			if (audio_handler)
			{
				audio_handler->subchunk_data(subchunk_size);
			}

			if (audio_reader)
			{
				if (subchunk_size <= std::numeric_limits<int32_t>::max())
				{
					audio_reader->signal_updateaudiosize(
						{ static_cast<int32_t>(subchunk_size), UNIT::BYTES });
				} else
				{
					std::ostringstream msg;
					msg << "Data subchunk declares a size of "
						<< subchunk_size
						<< " bytes which exceeds expected size.";
					throw InvalidAudioException(msg.str());
				}

				// Read audio bytes in blocks and emit AudioReader signals

				const auto block_bytes_read = wav_read_pcm_data(in,
						samples_per_read, *audio_reader,
						total_pcm_bytes);

				total_bytes_read += block_bytes_read;

				if (block_bytes_read != subchunk_size)
				{
					std::ostringstream msg;
					msg << "Expected to read "
						<< total_pcm_bytes
						<< " audio bytes but could only read "
						<< block_bytes_read
						<< " audio bytes.";
					throw FileReadException(msg.str(), total_bytes_read + 1);
				}
			}

			if (!audio_handler || !audio_handler->requests_all_subchunks())
			{
				ARCS_LOG_DEBUG << "Stop reading after data subchunk, ignore "
					<< "all bytes after position " << total_bytes_read;

				if (audio_handler)
				{
					auto remainder {
						audio_handler->physical_file_size() - total_bytes_read };

					if (remainder > 0)
					{
						ARCS_LOG_INFO << "Ignored "
							<< to_string(remainder)
							<< " bytes of unparsed data behind data subchunk "
							<< "(processed: "
							<< to_string(total_bytes_read)
							<< " bytes, file size: "
							<< to_string(audio_handler->physical_file_size())
							<< " bytes)";
					}
				}

				break;
			}

			continue;
		} // data

		// Ignore the content of all Subchunks except of 'fmt ' and 'data'

		in.ignore(subchunk_header.size);

		ARCS_LOG(DEBUG1) << "(Ignore subchunk)";
	} // while

	if (audio_reader) { audio_reader->signal_endinput(); }

	return total_bytes_read;
}


// wav_process_file


void wav_process_file(const std::string& filename,
		const int64_t    samples_per_read,
		WavAudioHandler* audio_handler,
		AudioReaderImpl* audio_reader,
		int64_t&         total_pcm_bytes)
{
	if (filename.empty())
	{
		ARCS_LOG_ERROR << "WAV filename is empty. End.";
		return;
	}

	if (audio_handler)
	{
		audio_handler->start_file(filename,
				retrieve_file_size_bytes(filename));
	}

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
		const auto total_bytes_read { in.gcount() };
		throw FileReadException(f.what(), total_bytes_read + 1);
	}

	ARCS_LOG_DEBUG << "Opened audio file";

	const int64_t bytes_read {
		wav_process_file_worker(in, samples_per_read, audio_handler,
				audio_reader, total_pcm_bytes)};

	ARCS_LOG_DEBUG << "Read " << bytes_read << " bytes from audio file";

	if (audio_handler)
	{
		audio_handler->end_file();
	}

	try
	{
		in.close();
	}
	catch (const std::ifstream::failure& f)
	{
		throw FileReadException(f.what());
	}

	ARCS_LOG(DEBUG1) << "Audio file closed.";
}


// retrieve_file_size_bytes


int64_t retrieve_file_size_bytes(const std::string& filename)
{
#if __cplusplus >= 201703L

	namespace fs = std::filesystem;
	using std::to_string;

	const auto path = fs::path { filename };
	const auto file_size { fs::file_size(path) };

	ARCS_LOG(DEBUG3) << "File size in bytes: " << file_size;

	if (file_size > std::numeric_limits<int64_t>::max())
	{
		throw std::runtime_error("File is too big: " + to_string(file_size));
	}

	return static_cast<int64_t>(file_size);

#else

	struct ::stat stat_buf;
	{
		int rc = ::stat(filename.c_str(), &stat_buf);
		assert (rc != -1);

		// Avoid Warning about Unused Variable rc (ugly, but, well...)
		static_cast<void>(rc);
	}

	ARCS_LOG(DEBUG3) << "File size in bytes: " << stat_buf.st_size;

	return stat_buf.st_size;

#endif
}

} // namespace wave
} // namespace details


// DescriptorWavPCM


DescriptorWavPCM::~DescriptorWavPCM() noexcept = default;


std::string DescriptorWavPCM::do_id() const
{
	return "wavpcm";
}


std::string DescriptorWavPCM::do_name() const
{
	return "RIFF/WAV(PCM)";
}


std::set<Format> DescriptorWavPCM::define_formats() const
{
	return { Format::WAV };
}


std::set<Codec> DescriptorWavPCM::define_codecs() const
{
	return {
		Codec::PCM_S16BE,
		Codec::PCM_S16BE_PLANAR,
		Codec::PCM_S16LE,
		Codec::PCM_S16LE_PLANAR,
		Codec::PCM_S32BE,
		Codec::PCM_S32BE_PLANAR,
		Codec::PCM_S32LE,
		Codec::PCM_S32LE_PLANAR
	};
}


LibInfo DescriptorWavPCM::do_libraries() const
{
	return { { "-genuine-",
		details::first_libname_match(details::runtime_deps(""), LIBARCSDEC_NAME)
	} };
}


std::unique_ptr<FileReader> DescriptorWavPCM::do_create_reader() const
{
	using details::wave::WavAudioHandler;
	using details::wave::WavAudioReaderImpl;

	auto handler = std::make_unique<WavAudioHandler>();
	auto reader  = std::make_unique<WavAudioReaderImpl>(std::move(handler));

	return std::make_unique<AudioReader>(std::move(reader));
}


std::unique_ptr<FileReaderDescriptor> DescriptorWavPCM::do_clone() const
{
	return std::make_unique<DescriptorWavPCM>();
}


// Add this descriptor to the audio descriptor registry

namespace {

const auto d = RegisterDescriptor<DescriptorWavPCM>();

} // namespace

} // namespace v_1_0_0
} // namespace arcsdec

