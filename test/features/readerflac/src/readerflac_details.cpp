#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for readerflac_details.hpp.
 */

#ifndef LIBARCSDEC_READERFLAC_HPP_
#define LIBARCSDEC_READERFLAC_HPP_     // allow readerflac_details.hpp
#endif
#ifndef LIBARCSDEC_READERFLAC_DETAILS_HPP_
#include "readerflac_details.hpp"       // TO BE TESTED
#endif

#include <set>                          // for set

#ifndef LIBARCSDEC_READERMOCKS_HPP_
#include "readermocks.hpp"              // for Mock_SampleProcessor
#endif


TEST_CASE ("FlacDefaultMetadataHandler", "[readerflac]" )
{
	using arcsdec::read::details::flac::FlacDefaultMetadataHandler;
	using arcsdec::read::Codec;

	FlacDefaultMetadataHandler h;

	SECTION ("Accepted set of codecs is only FLAC")
	{
		CHECK ( h.codecs() == std::set<Codec>{ Codec::FLAC } );
	}
}


TEST_CASE ("FlacAudioReaderImpl", "[readerflac]" )
{
	using arcsdec::read::details::flac::FlacAudioReaderImpl;
	using arcsdec::read::details::flac::FlacDefaultMetadataHandler;
	using arcsdec::read::details::flac::FlacDefaultErrorHandler;

	FlacAudioReaderImpl r;
	r.register_metadata_handler(std::make_unique<FlacDefaultMetadataHandler>());
	r.register_error_handler(std::make_unique<FlacDefaultErrorHandler>());

	auto proc = Mock_SampleProcessor{};
	r.set_handler(&proc);

	auto d = r.descriptor();

	SECTION ("Parses a syntactically intact input correctly")
	{
		r.process_file("test01.flac");
		// TODO What the mock sees in its callbacks has to be tested
	}
}

