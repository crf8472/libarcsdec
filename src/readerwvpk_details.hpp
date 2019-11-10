/**
 * \file
 *
 * \brief Testable implementation classes for readerwvpk.
 *
 * The Wavpack AudioReader will only read Wavpack files containing losslessly
 * compressed integer samples. Validation requires CDDA conform samples in PCM
 * format. Float samples are not supported. Original file formats other than
 * WAV are not supported.
 */

#ifndef __LIBARCSDEC_READERWVPK_HPP__
#error "Do not include readerwvpk_details.hpp, include readerwvpk.hpp instead"
#endif

#ifndef __LIBARCSDEC_READERWVPK_DETAILS_HPP__
#define __LIBARCSDEC_READERWVPK_DETAILS_HPP__

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

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

/**
 * \internal \defgroup readerwvpkImpl Implementation
 *
 * \ingroup readerwvpk
 *
 * @{
 */


/**
 * \brief Represents an interface for different reference CDDA representations
 * for the WAV format.
 *
 * For a concrete format like RIFFWAV/PCM, this interface can just
 * be implemented.
 */
class WAVPACK_CDDA_t
{

public:


	/**
	 * \brief Default destructor.
	 */
	virtual ~WAVPACK_CDDA_t() noexcept;

	/**
	 * \brief Declare whether WAV format is required.
	 *
	 * \return TRUE iff only WAV format is exclusively required, otherwise FALSE
	 */
	virtual bool wav_format_only() const = 0;

	/**
	 * \brief Expect lossless compression.
	 *
	 * This method is non-virtual because it
	 * makes no sense to ever change this requirement.
	 *
	 * \return TRUE, since ARCS cannot be computed on lossly compressed files
	 */
	bool lossless() const;

	/**
	 * \brief Declare whether it is required to have samples represented as
	 * integers.
	 *
	 * In future versions, it may be supported to process float samples.
	 *
	 * \return TRUE iff float samples can be processed, otherwise FALSE
	 */
	virtual bool floats_ok() const = 0;

	/**
	 * \brief Declare the least version of wavpack that is supported.
	 *
	 * Files with a lower version cannot be processed.
	 *
	 * \return The least version of wavpack supported
	 */
	virtual int at_least_version() const;

	/**
	 * \brief Declare the highest version of wavpack that is supported.
	 *
	 * Files with a higher version cannot be processed.
	 *
	 * \return The highest version of wavpack supported
	 */
	virtual int at_most_version() const;

	/**
	 * \brief Declare the required number of bytes per sample and channel.
	 *
	 * (For CDDA, this is 2 for 16 bit.)
	 *
	 * \return Require 2 bytes (for 16 bit).
	 */
	virtual int bytes_per_sample() const;
};


/**
 * \brief Reference values for CDDA conforming Wavepack with integer PCM.
 */
class WAVPACK_WAV_PCM_CDDA_t : public WAVPACK_CDDA_t
{

public:

	/**
	 * \brief Default destructor.
	 */
	virtual ~WAVPACK_WAV_PCM_CDDA_t() noexcept override;

	/**
	 * \brief Specifies WAV format as exclusively required.
	 *
	 * \return TRUE
	 */
	bool wav_format_only() const override;

	/**
	 * \brief Specifies float samples as not supported.
	 *
	 * (This in fact, restricts the support to integer samples.)
	 *
	 * \return FALSE
	 */
	virtual bool floats_ok() const override;
};


/**
 * \brief Represents an opened wavpack audio file.
 */
class WavpackOpenFile
{

public:

	/**
	 * \brief Open wavpack file with the given name.
	 *
	 * This class does RAII and does not require you to manually close the
	 * instance.
	 *
	 * \param[in] filename The file to open
	 *
	 * \throw FileReadException If file could not be opened
	 */
	explicit WavpackOpenFile(const std::string &filename);

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~WavpackOpenFile() noexcept;

	/**
	 * \brief Returns TRUE if file is lossless.
	 *
	 * \return TRUE iff file contains losslessly compressed data, otherwise
	 * FALSE
	 */
	bool is_lossless() const;

	/**
	 * \brief Returns TRUE if original file was WAV.
	 *
	 * \return TRUE iff the original file had WAV format, otherwise FALSE.
	 */
	bool has_wav_format() const;

	/**
	 * \brief Returns TRUE if original file had float samples.
	 *
	 * \return TRUE iff the original file contained float samples
	 */
	bool has_float_samples() const;

	/**
	 * \brief Returns the Wavpack version that built this file.
	 *
	 * \return Number of the Wavpack version used for encoding this file
	 */
	uint8_t version() const;

	/**
	 * \brief Returns the bits per sample.
	 *
	 * \return Number of bits per sample
	 */
	int bits_per_sample() const;

	/**
	 * \brief Returns the number of channels.
	 *
	 * \return Number of channels
	 */
	int num_channels() const;

	/**
	 * \brief Returns the sampling rate as number of samples per second.
	 *
	 * \return Number of samples per second
	 */
	int64_t samples_per_second() const;

	/**
	 * \brief Returns the bytes per sample (and channel, hence it is 2 for
	 * CDDA).
	 *
	 * \return Number of bytes per sample
	 */
	int bytes_per_sample() const;

	/**
	 * \brief Returns the total number of 32bit PCM samples this file contains.
	 *
	 * \return Total number of 32bit PCM samples in this file
	 */
	int64_t total_pcm_samples() const;

	/**
	 * \brief Returns TRUE iff the channel ordering is left/right, otherwise
	 * FALSE.
	 *
	 * \return TRUE iff the channel ordering is left/right, otherwise FALSE.
	 */
	bool channel_order() const;

	/**
	 * \brief Returns channel mask (expect 3 to indicate stereo for CDDA).
	 *
	 * \return The channel mask of the wavepack file
	 */
	int channel_mask() const;

	/**
	 * \brief Indicates whether the core audio file needs channel reorder.
	 *
	 * The core audio file may have a different channel ordering than the
	 * Wavepack container.
	 *
	 * \return TRUE if the channels have to be reordered, otherwise FALSE
	 */
	bool needs_channel_reorder() const;

	/**
	 * \brief Read the specified number of 32 bit PCM samples.
	 *
	 * Note that each
	 * 16 bit sample will be stored as a single int32, so buffer will require
	 * a size of pcm_samples_to_read * number_of_channels to store the requested
	 * number of samples.
	 *
	 * \param[in] pcm_samples_to_read Number of 32 bit samples to read
	 * \param[in,out] buffer The buffer to read the samples to
	 *
	 * \return Number of 32 bit PCM samples actually read
	 */
	int64_t read_pcm_samples(const int64_t &pcm_samples_to_read,
		std::vector<int32_t> *buffer) const;


private:

	// forward declaration for private implementation
	class Impl;

	/**
	 * \brief Private implementation
	 */
	std::unique_ptr<Impl> impl_;

	/**
	 * \brief Private copy constructor.
	 *
	 * Implemented empty. This class is non-copyable.
	 */
	WavpackOpenFile(const WavpackOpenFile &file) = delete;

	/**
	 * \brief Private copy assignment operator.
	 *
	 * Implemented empty. This class is non-copyable.
	 */
	WavpackOpenFile& operator = (WavpackOpenFile &file) = delete;
};


/**
 * \brief A handler to validate wavpack files for whether they can be processed
 * by this WavpackAudioReaderImpl.
 */
class WavpackValidatingHandler : public ReaderValidatingHandler
{

public:

	/**
	 * \brief Constructor with reference values.
	 */
	explicit WavpackValidatingHandler(
			std::unique_ptr<WAVPACK_CDDA_t> valid_values);

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~WavpackValidatingHandler() noexcept;

	/**
	 * \brief Validate the format of the wavpack file.
	 *
	 * \param[in] file The opened file
	 *
	 * \return TRUE if validation succeeded, otherwise FALSE
	 */
	bool validate_format(const WavpackOpenFile &file);

	/**
	 * \brief Validate properties expressed by the mode of the wavpack file.
	 *
	 * \param[in] file The opened file
	 *
	 * \return TRUE if validation succeeded, otherwise FALSE
	 */
	bool validate_mode(const WavpackOpenFile &file);

	/**
	 * \brief Validate audio content of the wavpack file for CDDA conformity.
	 *
	 * \param[in] file The opened file
	 *
	 * \return TRUE if validation succeeded, otherwise FALSE
	 */
	bool validate_cdda(const WavpackOpenFile &file);

	/**
	 * \brief Validate the wavpack version of the wavpack file.
	 *
	 * \param[in] file The opened file
	 *
	 * \return TRUE if validation succeeded, otherwise FALSE
	 */
	bool validate_version(const WavpackOpenFile &file);


private:

	/**
	 * \brief Configuration: Reference values for validation.
	 */
	std::unique_ptr<WAVPACK_CDDA_t> valid_;
};


/**
 * \brief Implementation of a AudioReader for the Wavpack format.
 */
class WavpackAudioReaderImpl : public BufferedAudioReaderImpl
{

public:

	/**
	 * \brief Default constructor.
	 */
	WavpackAudioReaderImpl();

	/**
	 * \brief Default destructor.
	 */
	virtual ~WavpackAudioReaderImpl() noexcept override;

	/**
	 * \brief Register a validating handler.
	 *
	 * \param[in] v The validating handler to register
	 */
	void register_validate_handler(std::unique_ptr<WavpackValidatingHandler> v);


private:

	std::unique_ptr<AudioSize> do_acquire_size(const std::string &filename)
		override;

	void do_process_file(const std::string &filename) override;

	std::unique_ptr<FileReaderDescriptor> do_descriptor() const override;

	/**
	 * \brief Perform the actual validation process.
	 *
	 * \param[in] file The file to validate
	 *
	 * \return TRUE iff validation succeeded, otherwise FALSE
	 */
	bool perform_validations(const WavpackOpenFile &file);

	/**
	 * \brief Validating handler of this instance.
	 */
	std::unique_ptr<WavpackValidatingHandler> validate_handler_;
};


/** @} */

} // namespace v_1_0_0

} // namespace arcsdec

#endif

