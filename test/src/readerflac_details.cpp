#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for readerflac_details.hpp.
 */

#ifndef __LIBARCSDEC_READERFLAC_HPP__
#define __LIBARCSDEC_READERFLAC_HPP__ // allow readerflac_details.hpp
#endif
#ifndef __LIBARCSDEC_READERFLAC_DETAILS_HPP__
#include "readerflac_details.hpp"       // TO BE TESTED
#endif

#ifndef __LIBARCSDEC_READERMOCKS_HPP__
#include "readermocks.hpp"              // for Mock_SampleProcessor
#endif

#include <set>  // for set


TEST_CASE ("FlacMetadataHandler", "[readerflac]" )
{
	using arcsdec::details::flac::FlacMetadataHandler;
	using arcsdec::Codec;

	FlacMetadataHandler h;

	SECTION ("Accepted set of codecs is only FLAC")
	{
		CHECK ( h.codecs() == std::set<Codec>{ Codec::FLAC } );
	}
}


TEST_CASE ("FlacAudioReaderImpl", "[readerflac]" )
{
	using arcsdec::details::flac::FlacAudioReaderImpl;

	FlacAudioReaderImpl r;
	auto proc = Mock_SampleProcessor{};
	r.attach_processor(proc);

	auto d = r.descriptor();

	SECTION ("Parses a syntactically intact input correctly")
	{
		r.process_file("test01.flac");
		// TODO What the mock sees in its callbacks has to be tested
	}
}

