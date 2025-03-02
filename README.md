# Audio decoder and TOC parser adapters for libarcstk

[![MIT licensed](https://img.shields.io/badge/license-MIT-blue.svg)](./LICENSE)
[![C++17](https://img.shields.io/badge/C++-17-darkblue.svg)](./API.md)
[![Release](https://img.shields.io/github/v/release/crf8472/libarcsdec?display_name=tag&include_prereleases)](https://github.com/crf8472/libarcsdec/releases)

## What libarcsdec does

- Provide a high-level API for sample and metadata input for [libarcstk][1].
- A bunch of decoder and parser adapters to let you calculate ARCSs on your
  already archived lossless audio data.
- Facility to read virtually any lossless codec from virtually any
  container file (by the use of external dependencies).
- Simplified API to recalculate ARCSs of a CD image at any time after ripping.
- You define the task like "Take this audio and this TOC and just give me the
  checksums".
- Hides completely the concrete decoding of audio data.
- Hides completely the parsing of metadata files.

The following features are planned, but not yet implemented:

- Metadata parser for compact discs (based on libcdio).
- Metadata parser for cdrdao's TOC format.
- Add OGG support to the audio reader for FLAC/FLAC.
- Add support for embedded CUE sheets to the audio reader for FLAC/FLAC.


## Current codecs and formats

Libarcsdec contains:

- TOC/Metadata parser for CUE sheets (based on libcue >= 2.0.0).
- Generic audio reader (based on ffmpeg >= 3.1, for any lossless codec in any
  container, e.g. ALAC/M4, ALAC/CAF, APE/APE, AIFF/AIFF, FLAC/OGG ... you name
  it).
- Audio reader for FLAC/FLAC files (based on flac/libFLAC++).
- Audio reader for lossless Wavpack/WV files (based on libwavpack).
- Native audio reader for RIFFWAV/PCM files.


## What libarcsdec does not

- Libarcsdec does not alter your files in any way and cannot be used for tagging
  etc.
- Libarcsdec does not contribute to tasks like verifying/matching, computing of
  the AccurateRip identifier, parsing the AccurateRip response etc. The API for
  those things is already provided by [libarcstk][1].
- Libarcsdec does not rip CDs.
- Libarcsdec offers no network facilities and is not supposed to do so. The
  actual HTTP request for fetching the reference values from AccurateRip is
  better performed by the HTTP networking client of your choice.


## How to Build

### Mandatory buildtime dependencies:

- cmake >= 3.14
- flex >= 2.6.4  (maybe earlier 2.6.x will do)
- bison >= 3.8.2  (maybe earlier 3.8.x will do)

### Mandatory build- and runtime dependencies:

- libarcstk >= 0.2.0-alpha.1

### Optional default build- and runtime dependencies:

- libcue >= 2.0.0
- FLAC++ headers >= 1.3.1
- libwavpack >= 5.0.0
- ffmpeg >= 3.1
- libsndfile >= 1.0.17

If you do not need any of the optional default dependencies, you can switch them
off independently from each other:

- build without libcue support by ``-DWITH_LIBCUE=OFF``
- build without FLAC support by ``-DWITH_FLAC=OFF``
- build without WavPack support by ``-DWITH_WVPK=OFF``
- build without ffmpeg support by ``-DWITH_FFMPEG=OFF``
- build without libsndfile support by ``-DWITH_LIBSNDFILE=OFF``

You cannot switch off libcue since this would leave libarcsdec unable to parse
any TOC data, rendering it effectively useless.

### Configure and start build

Build and install to just use the libarcsdec API:

	$ cd libarcsdec     # your libarcsdec root directory where README.md resides
	$ mkdir build && cd build
	$ cmake -DCMAKE_BUILD_TYPE=Release ..  # use any build switches you need

If this issues an error that reads

    "Could NOT find libarcstk (missing: LIBARCSTK_VERSION)",

the cause may be that pkg-config is either not installed or cannot find the
installed .pc-file of libarcstk. A possible cause for the latter could be that
you have installed libarcstk to a directory that cmake does not respect while
searching for files, e.g. ``/usr/local``. This can be fixed by giving
cmake a hint to your install directory like:

	$ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/usr/local ..

Proceed:

	$ cmake --build .
	$ sudo make install # installs to /usr/local

See a [detailed HowTo](BUILD.md) explaining different build scenarios and all
build switches.



## How to Use

- Consult the [example application](./examples/albumcalc). It illustrates how to
  calculate checksums without caring about the details of input formats.
- [Build the API documentation](BUILD.md#building-the-api-documentation) and
  view it in a browser


## Current Limitations

- No production release yet - will be 1.0.0.
- API is not considered stable before 1.0.0 (may change any time in any way
  until then).
- The strategy to select a reader for a given input is rudimentary: just the
  first reader that passes the format tests is selected. So the reader appearing
  accepting codec/format X occurring first in the list "shadows" all subsequent
  readers that would also be able to read the input.
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
- Never built, installed or tested on Windows or Mac OS X.


## Bugs

- No sufficient unit tests yet


[1]: https://github.com/crf8472/libarcstk

