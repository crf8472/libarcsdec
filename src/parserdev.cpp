/**
 * \file parsercue.cpp Implements parser for CUESheets
 *
 */


#ifndef __LIBARCSDEC_PARSERDEV_HPP__
#include "parserdev.hpp"
#endif

extern "C" {
#include <cdio/cdio.h>
#include <cdio/mmc.h>  // for cdio_audio_get_msf_seconds
#include <cdio/util.h> // for CDIO_CD_SECS_PER_MIN, cdio_from_bcd8
}

#include <memory>
#include <string>    // from .h
#include <vector>    // from .h

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include "metaparser.hpp"
#endif


namespace arcsdec
{

inline namespace v_1_0_0
{

namespace
{

using arcs::InvalidMetadataException;
using arcs::TOC;
using arcs::make_toc;


/**
 * \cond IMPL_ONLY
 *
 * \internal \defgroup parserDevImpl Implementation details of CUESheet parsing
 *
 * \ingroup parserdev
 *
 * @{
 */


/**
 * Reads TOC from a CD.
 */
class DevParserImpl : public MetadataParserImpl
{

public:

	/**
	 * Default constructor
	 */
	DevParserImpl();

	/**
	 * Default destructor
	 */
	virtual ~DevParserImpl() noexcept;


private:

	std::unique_ptr<TOC> do_parse(const std::string &filename) override;

	/**
	 * Name of the last parsed CUE file
	 */
	std::string filename_;
};


/// @}
/// \endcond IMPL_ONLY


// DevParserImpl


DevParserImpl::DevParserImpl()
	:filename_()
{
	//empty
}


DevParserImpl::~DevParserImpl() noexcept = default;


std::unique_ptr<TOC> DevParserImpl::do_parse(const std::string &/*device*/)
{
	// Open device and print some info about the driver

    ::CdIo_t* p_cdio = ::cdio_open(NULL, DRIVER_DEVICE);

    if (NULL != p_cdio)
    {
		std::stringstream ss;

		ss << "Driver selected by libcdio is: "
			<< ::cdio_get_driver_name(p_cdio);

		ss << "Default device for this driver is: "
			<< ::cdio_get_default_device(p_cdio);

		ARCS_LOG_INFO << ss.str();

    } else
    {
		ARCS_LOG_ERROR << "Could not open device: no driver found";
		return nullptr;
    }

	const ::driver_id_t* driver_id_p;
    for (driver_id_p = ::cdio_drivers; *driver_id_p != ::DRIVER_UNKNOWN;
		driver_id_p++)
    {
		if (::cdio_have_driver(*driver_id_p))
		{
			ARCS_LOG_DEBUG << "We have: "
				<< ::cdio_driver_describe(*driver_id_p);
		} else
		{
			ARCS_LOG_DEBUG << "We don't have: "
				<< ::cdio_driver_describe(*driver_id_p);
		}
    }

    // Figure out what kind of CD (image) we've got

	::track_t cdio_first_track_num = ::cdio_get_first_track_num(p_cdio);
	::track_t cdio_last_track_num = ::cdio_get_last_track_num(p_cdio);

	if ( ::CDIO_INVALID_TRACK == cdio_first_track_num ||
		 ::CDIO_INVALID_TRACK == cdio_last_track_num)
	{
		ARCS_LOG_ERROR << "Error while reading TOC";

		::cdio_destroy(p_cdio);
		return nullptr;
	}

	::track_t cdio_track_count = ::cdio_get_num_tracks(p_cdio);

	int first_audio_trk = -1;
	int first_data_trk = -1;
	int num_audio_tracks = 0;
	int num_data_tracks = 0;
	std::vector<::lsn_t> track_lsn(cdio_track_count+1);
	::msf_t msf;
	::lsn_t lsn;
	//unsigned int sectors;

	int frames;
	int frames_prev = 0;
	int frames_curr = 0;

	// Count number of data and audio tracks
	for (uint16_t i = cdio_first_track_num; i <= cdio_track_count; ++i)
	{
		if (::TRACK_FORMAT_AUDIO == ::cdio_get_track_format(p_cdio, i))
		{
			++num_audio_tracks;

			if (-1 == first_audio_trk)
			{
				first_audio_trk = i;
			}

			if (::cdio_get_track_msf(p_cdio, i, &msf))
			{
				frames = (::cdio_from_bcd8(msf.m) * 60
						+ ::cdio_from_bcd8(msf.s)-2) * 75
						+ ::cdio_from_bcd8(msf.f);

				frames_curr = frames - frames_prev;

				frames_prev = frames;

				ARCS_LOG_INFO << "(MSF) Track " << i << ": " << frames_curr
					<< "  "
					<< static_cast<uint32_t>(::cdio_from_bcd8(msf.m)) << ":"
					<< static_cast<uint32_t>(::cdio_from_bcd8(msf.s)-2) << "."
					<< static_cast<uint32_t>(::cdio_from_bcd8(msf.f));

				/*
				if (i != cdio_first_track_num)
				{
					secs = ::cdio_audio_get_msf_seconds(&track_msf[i])
						- ::cdio_audio_get_msf_seconds(&track_msf[i-1]);

					ARCS_LOG_INFO << "Track " << i << ": "
						<< secs << " secs"
						<< " == (" << secs * 75 << " frames)"
						<< " == (" << frames_curr << " frames)"
						;
						//<< secs / CDIO_CD_SECS_PER_MIN
						//<< ":"
						//<< secs % CDIO_CD_SECS_PER_MIN;
				}
				*/
			} else
			{
				ARCS_LOG_ERROR << "Error reading track " << i;
			}

			lsn = ::cdio_get_track_lsn(p_cdio, i);
			track_lsn[i] = CDIO_INVALID_LSN != lsn ? lsn : -1;

		} else
		{
			++num_data_tracks;
			if (-1 == first_data_trk) first_data_trk = i;
		}
	}

	std::vector<int32_t> offsets(cdio_track_count);

	uint16_t k = 0;
	for (uint16_t j = cdio_first_track_num; j <= cdio_track_count; ++j, ++k)
	{
		ARCS_LOG_INFO << "Track " << j << " offset: " << track_lsn[j];
		offsets[k] = track_lsn[j];
	}

	// Get sample count

	lsn_t last_lsn = ::cdio_get_track_last_lsn(p_cdio, cdio_track_count);

	if (CDIO_INVALID_LSN == last_lsn)
	{
		::cdio_destroy(p_cdio);
		throw InvalidMetadataException("Could not read leadout");
	}

	ARCS_LOG_INFO << "Leadout frame: " << last_lsn;

	if (0 == num_data_tracks)
	{
		ARCS_LOG_INFO << "Audio CD";
	} else
	{
		ARCS_LOG_INFO << "CD has " << num_data_tracks
			<< " data tracks and is not audio";

		::cdio_destroy(p_cdio);
		return nullptr;
	}

	// Finish the device and build TOC

	::cdio_destroy(p_cdio);

	uint32_t track_count = cdio_track_count;

	return make_toc(track_count, offsets, last_lsn);
}


} // namespace


// DescriptorCdio


DescriptorCdio::~DescriptorCdio() noexcept = default;


std::string DescriptorCdio::do_name() const
{
	return "physical device";
}


bool DescriptorCdio::do_accepts_bytes(const std::vector<char> & /* bytes */,
		const uint64_t & /* offset */) const
{
	return false;
}


bool DescriptorCdio::do_accepts_suffix(const std::string &suffix) const
{
	// NOTE: We know that device names usually do not contain ".". If this is
	// the case, get_suffix() will return the entire name, so we presuppose
	// having the entire name instead of the suffix and test whether this name
	// starts with a device path.

	// TODO Determine device name adequately instead of this instant hack

	std::string path("/dev/sr");

	if (suffix.length() < path.length())
	{
		return false;
    }

    return (0 == suffix.compare(0, path.length(), path));

	// Commented out: check whether suffix ends with path
    //return (0 == suffix.compare (
	//	suffix.length() - path.length(), path.length(),
	//	path)
	//);
}


std::unique_ptr<FileReader> DescriptorCdio::do_create_reader() const
{
	auto impl = std::make_unique<DevParserImpl>();

	return std::make_unique<MetadataParser>(std::move(impl));
}


std::unique_ptr<FileReaderDescriptor> DescriptorCdio::do_clone() const
{
	return std::make_unique<DescriptorCdio>();
}

} // namespace v_1_0_0

} // namespace arcsdec

