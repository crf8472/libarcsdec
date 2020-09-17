/**
 * \file
 *
 * \brief Implements audio reader for RIFF/WAV audio files with PCM.
 */

#ifndef __LIBARCSDEC_READERWAV_HPP__
#include "readerwav.hpp"
#endif
#ifndef __LIBARCSDEC_READERWAV_DETAILS_HPP__
#include "readerwav_details.hpp"
#endif

extern "C" {
#include <assert.h>   // for assert
#include <sys/stat.h> // for stat
}

#include <cstdint>
#include <fstream>
#include <functional>
#include <iomanip>    // for debug
#include <memory>
#include <sstream>    // for ostringstream
#include <string>
#include <vector>

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"
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
namespace wave
{

using arcstk::SampleInputIterator;
using arcstk::AudioSize;
using arcstk::CDDA;


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
	: id(id_)
	, size(size_)
	, file_size(file_size_)
	, format(format_)
{
	// empty
}


WavChunkDescriptor::~WavChunkDescriptor() noexcept = default;


// WavSubchunkHeader


WavSubchunkHeader::WavSubchunkHeader(uint32_t id_, uint32_t size_)
	: id(id_)
	, size(size_)
{
	// empty
}


WavSubchunkHeader::~WavSubchunkHeader() noexcept = default;


std::string WavSubchunkHeader::name() const
{
	static constexpr uint32_t mask = 0x000000FF;

	std::string name;

	name += static_cast<char>(id >> 24u & mask);
	name += static_cast<char>(id >> 16u & mask);
	name += static_cast<char>(id >>  8u & mask);
	name += static_cast<char>(id        & mask);

	return name;
}


// WavFormatSubchunk


WavFormatSubchunk::WavFormatSubchunk(
			const WavSubchunkHeader &header,
			int wFormatTag_,
			int wChannels_,
			int64_t dwSamplesPerSec_,
			int64_t dwAvgBytesPerSec_,
			int wBlockAlign_,
			int wBitsPerSample_
			)
	: id(header.id)
	, size(header.size)
	, wFormatTag(wFormatTag_)
	, wChannels(wChannels_)
	, dwSamplesPerSec(dwSamplesPerSec_)
	, dwAvgBytesPerSec(dwAvgBytesPerSec_)
	, wBlockAlign(wBlockAlign_)
	, wBitsPerSample(wBitsPerSample_)
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
		BigEndianBytes::to_uint32(bytes[0], bytes[1], bytes[ 2], bytes[ 3]),

		// real header size in bytes
		WAV_BYTES_PER_RIFF_HEADER,

		// parse file size declaration
		LittleEndianBytes::to_uint32(bytes[4], bytes[5], bytes[ 6], bytes[ 7]),

		// parse file format declaration ("WAV")
		BigEndianBytes::to_uint32(bytes[8], bytes[9], bytes[10], bytes[11])
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
		BigEndianBytes::to_uint32(bytes[0], bytes[1], bytes[2], bytes[3]),

		// parse subchunk size
		LittleEndianBytes::to_uint32(bytes[4], bytes[5], bytes[6], bytes[7])
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


// WavAudioHandler


WavAudioHandler::WavAudioHandler(std::unique_ptr<WAV_CDDA_t> valid)
	: DefaultValidator()
	, phys_file_size_(0)
	, valid_(std::move(valid))
	, config_( C_RESPECT_HEADER | C_RESPECT_FORMAT | C_RESPECT_DATA )
	, state_( S_INITIAL )
{
	// empty
}


int64_t WavAudioHandler::physical_file_size()
{
	return phys_file_size_;
}


void WavAudioHandler::start_file(const std::string &filename,
		const int64_t &phys_file_size)
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
	ARCS_LOG(DEBUG1) << "Try to validate RIFF/WAV Header";

	if (not this->has_config(C_RESPECT_HEADER))
	{
		this->set_state(S_VALIDATED_HEADER);
		return;
	}

	if (not this->assert_equals_u("Test: RIFF Header present?",
		descriptor.id, valid_->chunk_id(),
		"Unexpected header start"))
	{
		this->on_failure();
	}

	// RIFFWAV_PCM_CDDA declares file size - 8.
	// The magic value of 8 is the combined length of the 'RIFF' part and the
	// file size declaration itself

	if (not this->assert_equals_u(
		"Test: Declared file size conforms to physical file size?",
		descriptor.file_size + 2 * sizeof(uint32_t),
		this->physical_file_size(),
		"Unexpected header start"))
	{
		this->on_failure();
	}

	if (not this->assert_equals_u("Test: Header declares WAVE format?",
		descriptor.format, valid_->format(),
		"Header does not declare WAVE format"))
	{
		this->on_failure();
	}

	ARCS_LOG(DEBUG1) << "RIFF/WAV Header validated";

	this->set_state(S_VALIDATED_HEADER);
}


void WavAudioHandler::subchunk_format(
		const WavFormatSubchunk &format_subchunk)
{
	ARCS_LOG(DEBUG1) << "Try to validate format subchunk";

	if (not this->assert_equals_u("Test: id is 'fmt '?",
		format_subchunk.id, valid_->fmt_subchunk_id(),
		"Id of subchunk is not 'fmt ' but "
		+ std::to_string(format_subchunk.id)))
	{
		this->on_failure();
	}

	if (not this->assert_equals_u("Test: format subchunk size",
		format_subchunk.size,
		valid_->fmt_subchunk_size(),
		"Unexpected format subchunk size"))
	{
		this->on_failure();
	}

	this->bits_per_sample(format_subchunk.wBitsPerSample);
	this->samples_per_second(format_subchunk.dwSamplesPerSec);
	this->num_channels(format_subchunk.wChannels);

	if (not this->assert_equals("Test: wFormatTag is PCM",
		format_subchunk.wFormatTag, valid_->wFormatTag(),
		"wFormatTag is not PCM"))
	{
		this->on_failure();
	}

	if (not this->assert_equals_u("Test: dwAvgBytesPerSec is CDDA",
		format_subchunk.dwAvgBytesPerSec,
		valid_->dwAvgBytesPerSec(),
		"dwAvgBytesPerSec is not CDDA"))
	{
		this->on_failure();
	}

	if (not this->assert_equals("Test: wBlockAlign is CDDA",
		format_subchunk.wBlockAlign,
		valid_->wBlockAlign(),
		"wBlockAlign is not CDDA"))
	{
		this->on_failure();
	}

	this->unset_state(S_STARTED_FORMAT);
	this->set_state(S_VALIDATED_FORMAT);

	ARCS_LOG(DEBUG1) << "Format subchunk validated";
}


void WavAudioHandler::subchunk_data(const uint32_t &subchunk_size)
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

		this->on_failure();
	}

	if (not this->assert_true("Test: regular data subchunk size?",
		static_cast<int>(subchunk_size) % CDDA.BYTES_PER_SAMPLE == 0,
		"Incomplete samples, subchunk size is not a multiple of "
		+ std::to_string(CDDA.BYTES_PER_SAMPLE)))
	{
		this->on_failure();
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


AudioValidator::codec_set_type WavAudioHandler::do_codecs() const
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


// PCMBlockReader


PCMBlockReader::PCMBlockReader(const int32_t samples_per_block)
	: BlockCreator(samples_per_block)
	, consume_()
{
	// empty
}


PCMBlockReader::~PCMBlockReader() noexcept = default;


void PCMBlockReader::register_block_consumer(const std::function<void(
			SampleInputIterator begin, SampleInputIterator end
		)> &consume)
{
	consume_ = consume;
}


int64_t PCMBlockReader::read_blocks(std::ifstream &in,
		const int64_t &total_pcm_bytes)
{
	std::vector<uint32_t> samples; // FIXME Use symbolized sample type

	const int64_t sample_type_size = static_cast<int64_t>(sizeof(samples[0]));

	const int64_t bytes_per_block =
		static_cast<int64_t>(this->samples_per_block()) * CDDA.BYTES_PER_SAMPLE;

	const int estimated_blocks = total_pcm_bytes / bytes_per_block
				+ (total_pcm_bytes % bytes_per_block ? 1 : 0);

	ARCS_LOG_DEBUG << "START READING " << total_pcm_bytes
		<< " bytes in " << std::to_string(estimated_blocks) << " blocks with "
		<< bytes_per_block << " bytes per block";

	int32_t samples_todo = total_pcm_bytes / CDDA.BYTES_PER_SAMPLE;
	int64_t total_bytes_read   = 0;
	int64_t total_blocks_read  = 0;

	int64_t read_bytes = this->samples_per_block() * sample_type_size;

	using buffersize_t = typename decltype(samples)::size_type;
	samples.resize(static_cast<buffersize_t>(this->samples_per_block()));

	while (total_bytes_read < total_pcm_bytes)
	{
		// Adjust buffer size for last buffer, if necessary

		if (samples_todo < this->samples_per_block())
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

		if (consume_)
		{
			this->consume_(samples.begin(), samples.end());
		} else
		{
			ARCS_LOG_ERROR << "No block consumer registered.";
		}
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


int64_t WavAudioReaderImpl::retrieve_file_size_bytes(
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


int64_t WavAudioReaderImpl::process_file_worker(std::ifstream &in,
		const bool &calculate,
		int64_t &total_pcm_bytes)
{
	this->signal_startinput();

	int64_t total_bytes_read = 0;

	// Read the first bytes and parse them as chunk descriptor.
	// NOTE: ifstream's exceptions (fail|bad)bit must be activated

	std::vector<char> bytes(WavPartParser::WAV_BYTES_PER_RIFF_HEADER);

	std::streamsize bytes_to_read =
		WavPartParser::WAV_BYTES_PER_RIFF_HEADER * sizeof(bytes[0]);
	try
	{
		in.read(&bytes[0], bytes_to_read);
	}
	catch (const std::ifstream::failure& f)
	{
		total_bytes_read += in.gcount();
		throw FileReadException(f.what(), total_bytes_read + 1);
	}
	total_bytes_read += bytes_to_read;


	// Traverse subchunks, looking for 'fmt ' and 'data' thereafter

	WavPartParser parser;
	audio_handler_->chunk_descriptor(parser.chunk_descriptor(bytes));

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

		const auto& valid = audio_handler_->validator();

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
			// Pass total_pcm_bytes to Caller and inform Calculation instance

			total_pcm_bytes = subchunk_header.size; // remind: output parameter

			audio_handler_->subchunk_data(subchunk_header.size);

			if (calculate)
			{
				AudioSize audiosize;
				audiosize.set_pcm_byte_count(subchunk_header.size);
				this->signal_updateaudiosize(audiosize);

				// Read audio bytes in blocks

				PCMBlockReader reader(this->samples_per_read());

				reader.register_block_consumer(
					std::bind(&WavAudioReaderImpl::signal_appendsamples, this,
						std::placeholders::_1, std::placeholders::_2)
					);

				auto block_bytes_read = reader.read_blocks(in, total_pcm_bytes);
				total_bytes_read += block_bytes_read;

				if (block_bytes_read != total_pcm_bytes)
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

			if (not audio_handler_->requests_all_subchunks())
			{
				ARCS_LOG_DEBUG << "Stop reading after data subchunk";

				auto remainder =
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

	this->signal_endinput();

	return total_bytes_read;
}


void WavAudioReaderImpl::process_file(const std::string &filename,
		const bool &validate,
		const bool &calculate,
		int64_t &total_pcm_bytes)
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
		const int64_t total_bytes_read = in.gcount();
		throw FileReadException(f.what(), total_bytes_read + 1);
	}

	const int64_t bytes_read =
		this->process_file_worker(in, calculate, total_pcm_bytes);

	ARCS_LOG_DEBUG << "Read " << bytes_read << " bytes from audio file";

	audio_handler_->end_file();

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


std::unique_ptr<AudioSize> WavAudioReaderImpl::do_acquire_size(
	const std::string &audiofilename)
{
	int64_t total_pcm_bytes = 0;

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
	audiosize->set_pcm_byte_count(static_cast<uint64_t>(total_pcm_bytes));
	return audiosize;
}


void WavAudioReaderImpl::do_process_file(const std::string &audiofilename)
{
	int64_t total_pcm_bytes = 0;

	// Validate And Calculate

	this->process_file(audiofilename, true, true, total_pcm_bytes);
}


std::unique_ptr<FileReaderDescriptor> WavAudioReaderImpl::do_descriptor()
	const
{
	return std::make_unique<DescriptorWavPCM>();
}


void WavAudioReaderImpl::register_audio_handler(
		std::unique_ptr<WavAudioHandler> hndlr)
{
	audio_handler_ = std::move(hndlr);
}

} // namespace wave
} // namespace details


// DescriptorWavPCM


DescriptorWavPCM::~DescriptorWavPCM() noexcept = default;


std::string DescriptorWavPCM::do_name() const
{
	return "RIFF/WAV(PCM)";
}


LibInfo DescriptorWavPCM::do_libraries() const
{
	return { { "-genuine-",
		details::find_lib(details::runtime_deps(""), LIBARCSDEC_NAME) } };
}


bool DescriptorWavPCM::do_accepts_bytes(const std::vector<unsigned char> &bytes,
		const uint64_t &offset) const
{
	using details::wave::RIFFWAV_PCM_CDDA_t;

	// No test bytes? => fail
	if (bytes.empty())
	{
		ARCS_LOG(DEBUG1) << "Test bytes empty, no match";
		return false;
	}

	// Test Bytes Beyond Canonical Part? => accept
	if (offset >= RIFFWAV_PCM_CDDA_t::header().size())
	{
		ARCS_LOG(DEBUG1) <<
			"Test bytes accepted since they are beyond WAV header";
		return true;
	}

	// determine start pointers

	auto in_current  = bytes.begin();
	auto ref_current = RIFFWAV_PCM_CDDA_t::header().begin() + offset;

	// determine end pointers

	const auto ref_bytes    = RIFFWAV_PCM_CDDA_t::header().size() - offset;
	const bool longer_input = bytes.size() > ref_bytes;

	const auto in_stop  = longer_input
		? bytes.begin() + static_cast<int>(ref_bytes) + 1 /* past-the-end */
		: bytes.end();

	const auto ref_stop = longer_input
		? RIFFWAV_PCM_CDDA_t::header().end()
		: ref_current + bytes.size() + 1 /* past-the-end */;

	do
	{
		const auto m = std::mismatch(in_current, in_stop, ref_current);

		// Reached the end? => Success
		if (m.first == in_stop or m.second == ref_stop)
		{
			break;
		}

		// Is it an actual mismatch (i.e. on a non-wildcard byte)?
		if (m.second != ref_stop
				and *m.second != RIFFWAV_PCM_CDDA_t::any_byte())
		{
			return false;
		}

		// Mismatch was on a "wildcard byte", so skip all following bytes until
		// the wildcard sequence ends.

		in_current  = m.first;
		ref_current = m.second;

		// Skip all input bytes referring to 'any_byte' in the reference bytes
		while (in_current != in_stop and ref_current != ref_stop
				and *ref_current == RIFFWAV_PCM_CDDA_t::any_byte())
		{
			++in_current;
			++ref_current;
		}

	} while (in_current != in_stop and ref_current != ref_stop);

	return true;
}


std::unique_ptr<FileReader> DescriptorWavPCM::do_create_reader() const
{
	using details::wave::WAV_CDDA_t;
	using details::wave::RIFFWAV_PCM_CDDA_t;
	using details::wave::WavAudioReaderImpl;
	using details::wave::WavAudioHandler;

	std::unique_ptr<WAV_CDDA_t> valid = std::make_unique<RIFFWAV_PCM_CDDA_t>();
	auto handler = std::make_unique<WavAudioHandler>(std::move(valid));

	auto impl = std::make_unique<WavAudioReaderImpl>();
	impl->register_audio_handler(std::move(handler));

	return std::make_unique<AudioReader>(std::move(impl));
}


bool DescriptorWavPCM::do_accepts(Codec codec) const
{
	return codecs().find(codec) != codecs().end();
}


std::set<Codec> DescriptorWavPCM::do_codecs() const
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


bool DescriptorWavPCM::do_accepts(Format format) const
{
	return format == Format::WAV;
}


std::set<Format> DescriptorWavPCM::do_formats() const
{
	return { Format::WAV };
}


std::unique_ptr<FileReaderDescriptor> DescriptorWavPCM::do_clone() const
{
	return std::make_unique<DescriptorWavPCM>();
}


// Add this descriptor to the audio descriptor registry

namespace {

const auto d = RegisterAudioDescriptor<DescriptorWavPCM>();

} // namespace

} // namespace v_1_0_0

} // namespace arcsdec

