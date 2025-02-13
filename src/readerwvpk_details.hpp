#ifndef __LIBARCSDEC_READERWVPK_HPP__
#error "Do not include readerwvpk_details.hpp, include readerwvpk.hpp instead"
#endif

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

#ifndef __LIBARCSDEC_READERWVPK_DETAILS_HPP__
#define __LIBARCSDEC_READERWVPK_DETAILS_HPP__

extern "C" {
#include <wavpack/wavpack.h>
}

#include <cstdint>   // for uint8_t, int32_t, int64_t
#include <exception> // for exception
#include <memory>    // for unique_ptr
#include <string>    // for string
#include <vector>    // for vector

#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"  // for AudioReaderImpl
#endif


namespace arcsdec
{
inline namespace v_1_0_0
{

namespace details
{
namespace wavpack
{

/**
 * \internal \defgroup readerwvpkImpl Implementation
 *
 * \ingroup readerwvpk
 *
 * @{
 */


/**
 * \brief Functor for freeing WavpackContext* instances.
 */
struct Free_WavpackContext final
{
	void operator()(::WavpackContext* ctx) const;
};


/**
 * \brief A unique_ptr for WavpackContext using Free_WavpackContext as a
 * custom deleter.
 */
using ContextPtr = std::unique_ptr<::WavpackContext, Free_WavpackContext>;


/**
 * \brief Construction functor for ContextPtr instances.
 */
struct Make_ContextPtr final
{
	ContextPtr operator()(const std::string& filename) const;
};


/**
 * \brief Encapsulates error code from the libwavpack API.
 */
class LibwavpackException final : public std::exception
{
public:

	/**
	 * \brief Construct the message from value and function name.
	 *
	 * \param[in] value Unexpected return value
	 * \param[in] name  Name of function that returned the unexpected value
	 * \param[in] error_msg Error message from libwavpack
	 */
	LibwavpackException(const std::string& value, const std::string& name,
			const std::string& error_msg);

	char const * what() const noexcept override;

private:

	std::string msg_;
};


/**
 * \brief Interface for Wavpack validation values.
 *
 * Can be subclassed to be more permissive. Default implementation allows
 * only WAV format and no floats.
 */
class WAVPACK_CDDA_t
{
public:

	/**
	 * \brief Default destructor.
	 */
	virtual ~WAVPACK_CDDA_t() noexcept;

	/**
	 * \brief Expect lossless compression.
	 *
	 * \return TRUE, since ARCS cannot be computed on lossly compressed files
	 */
	bool lossless() const;

	/**
	 * \brief Declare whether WAV format is required.
	 *
	 * Default implementation returns TRUE.
	 *
	 * \return TRUE iff only WAV format is exclusively required, otherwise FALSE
	 */
	virtual bool wav_format_only() const;

	/**
	 * \brief Declare whether it is required to have samples represented as
	 * integers.
	 *
	 * Default implementation returns FALSE.
	 * In future versions, it may be supported to process float samples.
	 *
	 * \return TRUE iff float samples can be processed, otherwise FALSE
	 */
	virtual bool floats_ok() const;

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
};


/**
 * \brief Represents an opened wavpack audio file.
 */
class WavpackOpenFile final
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
	explicit WavpackOpenFile(const std::string& filename);

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
	 * \brief Returns the total number of 32bit PCM samples this file contains.
	 *
	 * Will return 0 instead of error values or values that are bigger than
	 * 0xFFFFFFFF.
	 *
	 * \return Total number of 32bit PCM samples in this file
	 *
	 * \throw LibwavpackException   If libwavpack does return a negative value
	 * \throw InvalidAudioException If the size exceeds the legal maximum
	 */
	int64_t total_pcm_samples() const;

	/**
	 * \brief Returns TRUE iff the channel ordering is left/right, otherwise
	 * FALSE.
	 *
	 * \return TRUE iff the channel ordering is left/right, otherwise FALSE.
	 *
	 * \throw InvalidAudioException If the channel ordering is unexpected
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
	 *
	 * \throw invalid_argument If buffer.size() < (pcm_samples_to_read * 2)
	 */
	int64_t read_pcm_samples(const int64_t pcm_samples_to_read,
		std::vector<int32_t>& buffer) const;

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
	WavpackOpenFile(const WavpackOpenFile& file) = delete;

	/**
	 * \brief Private copy assignment operator.
	 *
	 * Implemented empty. This class is non-copyable.
	 */
	WavpackOpenFile& operator = (WavpackOpenFile& file) = delete;
};


/**
 * \brief A handler to validate wavpack files for whether they can be processed
 * by this WavpackAudioReaderImpl.
 */
class WavpackValidatingHandler final : public DefaultValidator
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
	~WavpackValidatingHandler() noexcept override;

	/**
	 * \brief Validate the format of the wavpack file.
	 *
	 * \param[in] file The opened file
	 *
	 * \return TRUE if validation succeeded, otherwise FALSE
	 */
	bool validate_format(const WavpackOpenFile& file);

	/**
	 * \brief Validate properties expressed by the mode of the wavpack file.
	 *
	 * \param[in] file The opened file
	 *
	 * \return TRUE if validation succeeded, otherwise FALSE
	 */
	bool validate_mode(const WavpackOpenFile& file);

	/**
	 * \brief Validate audio content of the wavpack file for CDDA conformity.
	 *
	 * \param[in] file The opened file
	 *
	 * \return TRUE if validation succeeded, otherwise FALSE
	 */
	bool validate_cdda(const WavpackOpenFile& file);

	/**
	 * \brief Validate the wavpack version of the wavpack file.
	 *
	 * \param[in] file The opened file
	 *
	 * \return TRUE if validation succeeded, otherwise FALSE
	 */
	bool validate_version(const WavpackOpenFile& file);

private:

	codec_set_type do_codecs() const override;

	/**
	 * \brief Configuration: Reference values for validation.
	 */
	std::unique_ptr<WAVPACK_CDDA_t> valid_;
};


/**
 * \brief Implementation of a AudioReader for the Wavpack format.
 */
class WavpackAudioReaderImpl final : public AudioReaderImpl
{
public:

	/**
	 * \brief Register a validating handler.
	 *
	 * \param[in] v The validating handler to register
	 */
	void register_validate_handler(std::unique_ptr<WavpackValidatingHandler> v);

private:

	std::unique_ptr<AudioSize> do_acquire_size(const std::string& filename)
		final;

	void do_process_file(const std::string& filename) final;

	std::unique_ptr<FileReaderDescriptor> do_descriptor() const final;

	/**
	 * \brief Perform the actual validation process.
	 *
	 * \param[in] file The file to validate
	 *
	 * \return TRUE iff validation succeeded, otherwise FALSE
	 */
	bool perform_validations(const WavpackOpenFile& file);

	/**
	 * \brief Validating handler of this instance.
	 */
	std::unique_ptr<WavpackValidatingHandler> validate_handler_;
};


/** @} */

} // namespace wavpack
} // namespace details
} // namespace v_1_0_0
} // namespace arcsdec

#endif

