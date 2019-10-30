#include "catch2/catch.hpp"

#ifndef __LIBARCSDEC_DESCRIPTORS_HPP__
#include "descriptors.hpp"
#endif
#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include "audioreader.hpp"
#endif

/**
 * \file descriptors.cpp Tests for all API classes exported by descriptors.hpp
 */

TEST_CASE ( "List descriptors", "[audioreaderselection]" )
{
	arcsdec::AudioReaderSelection selection;
}

TEST_CASE ( "Test stub (does nothing)", "[filereaderselection]" )
{
}

