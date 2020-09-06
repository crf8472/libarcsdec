/**
 * \file
 *
 * \brief Testable implementation classes for readerflac.
 */

#ifndef __LIBARCSDEC_READERFLAC_HPP__
#error "Do not include readerflac_details.hpp, include readerflac.hpp instead"
#endif

#ifndef __LIBARCSDEC_READERFLAC_DETAILS_HPP__
#define __LIBARCSDEC_READERFLAC_DETAILS_HPP__

#include <FLAC++/decoder.h>
#include <FLAC++/metadata.h>

#include <cstdint>
#include <exception>
#include <memory>
#include <string>
#include <vector>

#ifndef __LIBARCSTK_SAMPLES_HPP__
#include <arcstk/samples.hpp>     // for SampleSequence
#endif

#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"
#endif
#ifndef __LIBARCSDEC_AUDIOBUFFER_HPP__
#include "audiobuffer.hpp"
#endif


namespace arcsdec
{
inline namespace v_1_0_0
{
namespace details
{
namespace flac
{

using arcstk::SampleSequence;

/**
 * \internal \defgroup readerflacImpl Implementation
 *
 * \ingroup readerflac
 *
 * @{
 */

/**
 * \brief Provides an implementation of the FLAC__metadata_callback handler that
 * validates the audio data for conforming to CDDA.
 *
 * Each call of the callback can call one of the handler methods.
 *
 * It is not the responsibility of the FlacMetadataHandler to inform the
 * internal \c Calculation about the total number of samples or bytes. This is
 * done within the implementation of metadata_callback().
 */
class FlacMetadataHandler final : public DefaultValidator
{
public:

	FlacMetadataHandler() = default;

	// class is non-copyable
	FlacMetadataHandler(const FlacMetadataHandler &) = delete;
	FlacMetadataHandler& operator = (const FlacMetadataHandler &) = delete;

	// class is movable
	FlacMetadataHandler(FlacMetadataHandler &&) noexcept = default;
	FlacMetadataHandler& operator = (FlacMetadataHandler &&) noexcept = default;

	/**
	 * \brief To be called manually by the AudioReaderImpl to trigger
	 * validation.
	 *
	 * Validates it for CDDA compliance.
	 *
	 * \param[in] streaminfo The Streaminfo object from FLAC++
	 *
	 * \return TRUE if metadata indicates CDDA conformity, otherwise FALSE
	 */
	bool streaminfo(const FLAC::Metadata::StreamInfo &streaminfo);

private:

	codec_set_type do_codecs() const override;
};


/**
 * \brief File reader implementation for files in CDDA/Flac format, i.e.
 * containing 44.100 Hz/16 bit Stereo PCM data samples.
 *
 * This class provides the PCM sample data as a succession of blocks of 32 bit
 * PCM samples to its \c Calculation. The first block starts with the very
 * first PCM sample in the file. The streaminfo metadata block is validated to
 * conform to CDDA.
 */
class FlacAudioReaderImpl final : public AudioReaderImpl, FLAC::Decoder::File
{
public:

	/**
	 * \brief Default constructor.
	 */
	FlacAudioReaderImpl();

	// class is non-copyable
	FlacAudioReaderImpl(const FlacAudioReaderImpl &) = delete;
	FlacAudioReaderImpl& operator = (const FlacAudioReaderImpl &) = delete;

	// class is movable
	FlacAudioReaderImpl(FlacAudioReaderImpl &&rhs) noexcept = default;
	FlacAudioReaderImpl& operator = (FlacAudioReaderImpl &&rhs) noexcept
		= default;

	/**
	 * \brief Pass frames to internal handler.
	 *
	 * \param[in] frame  The frame describing object as defined by FLAC
	 * \param[in] buffer The sample buffer
	 *
	 * \return Decoder status info
	 */
	::FLAC__StreamDecoderWriteStatus write_callback(
			const ::FLAC__Frame *frame,
			const ::FLAC__int32 *const buffer[]) override;

	/**
	 * \brief Pass metadata by type to internal handler.
	 *
	 * Passes the the total number of samples in the stream to the
	 * sample_count() method since this number is required by process_file() in
	 * any configuration setup.
	 *
	 * \param[in] metadata The StreamMetadata from FLAC
	 */
	void metadata_callback(const ::FLAC__StreamMetadata *metadata) override;

	/**
	 * \brief Log the decoder's error status.
	 *
	 * \param[in] status The StreamDecoderErrorStatus
	 */
	void error_callback(::FLAC__StreamDecoderErrorStatus status) override;

	/**
	 * \brief Register a FLACMetadataHandler to this instance.
	 *
	 * \param[in] hndlr Set the FLACMetadataHandler of this instance
	 */
	void register_validate_handler(std::unique_ptr<FlacMetadataHandler> hndlr);

private:

	std::unique_ptr<AudioSize> do_acquire_size(const std::string &filename)
		override;

	void do_process_file(const std::string &filename) override;

	std::unique_ptr<FileReaderDescriptor> do_descriptor() const override;

	/**
	 * \brief Internal SampleSequence instance.
	 */
	SampleSequence<::FLAC__int32, true> smplseq_;

	/**
	 * \brief Handles each metadata block.
	 */
	std::unique_ptr<FlacMetadataHandler> metadata_handler_;
};

/** @} */

} // namespace flac
} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

#endif

