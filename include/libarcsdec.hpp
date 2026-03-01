#ifndef __LIBARCSDEC_LIBARCSDEC_HPP__
#define __LIBARCSDEC_LIBARCSDEC_HPP__


/**
 * \file
 *
 * \brief libarcsdec API.
 *
 * \details
 *
 * Include the entire API of libarcsdec.
 */

/* Audioreader interface */

#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include <arcsdec/audioreader.hpp>
#endif

/* Calculators for Checksums, IDs, ToCs and AudioInfos */

#ifndef __LIBARCSDEC_CALCULATORS_HPP__
#include <arcsdec/calculators.hpp>
#endif

/* Format descriptor interface */

#ifndef __LIBARCSDEC_DESCRIPTOR_HPP__
#include <arcsdec/descriptor.hpp>
#endif

/* Metaparser interface */

#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include <arcsdec/metaparser.hpp>
#endif

/* Sample processing interface */

#ifndef __LIBARCSDEC_SAMPLEPROC_HPP__
#include <arcsdec/sampleproc.hpp>
#endif

/* FileReader selection interface */

#ifndef __LIBARCSDEC_SELECTION_HPP__
#include <arcsdec/selection.hpp>
#endif

/* Version, Name and Release info */

#ifndef __LIBARCSDEC_VERSION_HPP__
#include <arcsdec/version.hpp>
#endif

#endif

