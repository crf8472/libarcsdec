#ifndef LIBARCSDEC_LIBARCSDEC_HPP_
#define LIBARCSDEC_LIBARCSDEC_HPP_


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

#ifndef LIBARCSDEC_AUDIOREADER_HPP_
#include <arcsdec/audioreader.hpp>
#endif

/* Calculators for Checksums, IDs, ToCs and AudioInfos */

#ifndef LIBARCSDEC_CALCULATORS_HPP_
#include <arcsdec/calculators.hpp>
#endif

/* Format descriptor interface */

#ifndef LIBARCSDEC_DESCRIPTOR_HPP_
#include <arcsdec/descriptor.hpp>
#endif

/* Metaparser interface */

#ifndef LIBARCSDEC_METAPARSER_HPP_
#include <arcsdec/metaparser.hpp>
#endif

/* Sample processing interface */

#ifndef LIBARCSDEC_SAMPLEPROC_HPP_
#include <arcsdec/sampleproc.hpp>
#endif

/* FileReader selection interface */

#ifndef LIBARCSDEC_SELECTION_HPP_
#include <arcsdec/selection.hpp>
#endif

/* Version, Name and Release info */

#ifndef LIBARCSDEC_VERSION_HPP_
#include <arcsdec/version.hpp>
#endif

#endif

