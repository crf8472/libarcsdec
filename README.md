# Audio decoder and TOC parser adapters for libarcstk

[![MIT licensed](https://img.shields.io/badge/license-MIT-blue.svg)](./LICENSE)
[![C++17](https://img.shields.io/badge/C++-17-darkblue.svg)](./DESIGN.md)
[![Release](https://img.shields.io/github/v/release/crf8472/libarcsdec?display_name=tag&include_prereleases)](https://github.com/crf8472/libarcsdec/releases)
[![Build & Test](https://github.com/crf8472/libarcsdec/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/crf8472/libarcsdec/actions/workflows/build-and-test.yml)
[![Sanitizers](https://github.com/crf8472/libarcsdec/actions/workflows/sanitizers.yml/badge.svg)](https://github.com/crf8472/libarcsdec/actions/workflows/sanitizers.yml)
[![clang-tidy](https://github.com/crf8472/libarcsdec/actions/workflows/clang-tidy.yml/badge.svg)](https://github.com/crf8472/libarcsdec/actions/workflows/clang-tidy.yml)


## What libarcsdec does

- Provide a high-level API for sample and metadata input for [libarcstk][1].
- Provide a bunch of decoder and parser adapters to let you calculate ARCSs on
  your already archived lossless audio data.
- Facility to read virtually any lossless codec from virtually any
  container file (by the use of external dependencies).
- Simplified API to recalculate ARCSs of a CD image at any time after ripping.
- You define the task like "Take this audio and this toc and just give me the
  checksums".
- Hides completely the concrete decoding of audio data.
- Hides completely the parsing of metadata files.

The following features are planned, but not yet fully implemented:

- Metadata parser for cdrdao's .toc file format.
- Add OGG support to the audio reader for FLAC/FLAC.


### Current codecs and formats

Libarcsdec contains:

- Generic parser for Cuesheets (builtin).
- Toc/metadata parser for Cuesheets (based on libcue >= 2.0.0).
- Generic audio reader (based on ffmpeg for any lossless codec in any container,
  e.g. alac/M4, alac/CAF, ape/APE, aiff/AIFF, flac/OGG ... you name it).
- Generic audio reader (based on libsndfile).
- Audio reader for flac/FLAC files (based on flac/libFLAC++).
- Audio reader for lossless wavpack/WV files (based on libwavpack).
- Builtin audio reader for RIFFWAV/PCM files.


## What libarcsdec does not

- Libarcsdec does not rip CDs.
- Libarcsdec will not alter your files in any way and cannot be used for tagging
  etc.
- Libarcsdec does not contribute to tasks like verifying/matching, computing of
  the AccurateRip identifier, parsing the AccurateRip response etc. The API for
  those tasks is already provided by [libarcstk][1]. If you need those functions
  in an executable for the command line check whether [arcs-tools][2] fits your
  needs.)
- Libarcsdec offers no network facilities and is not supposed to do so. The
  actual HTTP request for fetching the reference values from AccurateRip is
  better performed by the HTTP networking client of your choice.



## How to Build

### Mandatory buildtime dependencies:

- cmake >= 3.14
- flex >= 2.6.4  (maybe earlier 2.6.x will do)
- bison >= 3.8.2  (maybe earlier 3.8.x will do)

### Mandatory build- and runtime dependencies:

- libarcstk >= 0.4.0-alpha.1

### Optional default build- and runtime dependencies:

- libcue >= 2.0.0
- ffmpeg >= 3.1
- FLAC++ headers >= 1.3.1
- libwavpack >= 5.0.0
- libsndfile >= 1.0.17

If you do not need any of the optional default-on dependencies, you can switch
them off independently from each other:

- build without ffmpeg support by ``-DWITH_FFMPEG=OFF``
- build without FLAC support by ``-DWITH_FLAC=OFF``
- build without WavPack support by ``-DWITH_WAVPACK=OFF``

If you need any of the optional default-off dependencies, you can switch them on
independently from each other:

- build with libcue support by ``-DWITH_LIBCUE=ON``
- build with libsndfile support by ``-DWITH_LIBSNDFILE=ON``

You can switch off or on each of these dependencies thereby leaving libarcstk as
the only mandatory build+runtime dependency. However, this entails that
libarcsdec will only be able to read Cuesheets and WAVE-files with its
respective builtin reading capabilities.

### Configure and start build

Build and install to just use the libarcsdec API:

	$ cd libarcsdec     # your libarcsdec root directory where README.md resides
	$ cmake -B build    # generate directory 'build', configure a release build

If this issues an error that reads

    "Could NOT find libarcstk (missing: LIBARCSTK_VERSION)",

the cause may be that either libarcstk is not installed. Or, cmake failed when
trying to find libarcstk and pkg-config is either not installed or cannot find
the installed .pc-file of libarcstk. A possible cause for the latter could be
that you have installed libarcstk to a directory that cmake does not respect
while searching for files, e.g. ``/usr/local``. This can be fixed by giving
cmake a hint to your install directory like:

	$ cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/usr/local

Proceed:

	$ cmake --build build
	$ sudo cmake --install build   # installs to /usr/local

See a [detailed HowTo](BUILD.md) explaining different build scenarios and all
build switches.



## How to Use

- Consult the [example applications](./examples/README.md). They illustrate how
  to calculate checksums without being affected by the details of input formats.
- [Build the API documentation](BUILD.md#building-the-api-documentation) and
  view it in a browser


## Current Limitations

- No production release yet - will be 1.0.0.
- API is not considered stable before 1.0.0 (may change any time in any way
  until then).
- The readers try to guess the channel ordering to recognize swapped channels.
  The default channel assignment left/right (with left = channel 0, right =
  channel 1) works well but scenarios with different channel assignment
  are not well tested.
- Some readers may be more robust than others, especially the very fast RIFF/WAV
  PCM reader should be considered a last resort (it is valuable for debugging
  and testing).
- Handling of data tracks is not implemented and data tracks are just processed
  like audio tracks. What happens is completely untested.
- Untested on big endian plattforms.


## Bugs

- No sufficient unit tests yet


[1]: https://github.com/crf8472/libarcstk
[2]: https://github.com/crf8472/arcs-tools

