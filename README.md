# Audio decoder and TOC parser adapters for libarcstk

[![MIT licensed](https://img.shields.io/badge/license-MIT-blue.svg)](./LICENSE)
[![C++14](https://img.shields.io/badge/C++-14-darkred.svg)](./API.md)
[![Latest](https://img.shields.io/badge/dynamic/json.svg?url=https://codeberg.org/api/v1/repos/tristero/libarcsdec/releases&label=latest&query=$.0.name&color=yellowgreen)](https://codeberg.org/tristero/libarcsdec/releases)



## What libarcsdec is

- A high-level API for sample and metadata input for [libarcstk][1]
- A bunch of decoder and parser adapters to let you calculate ARCSs on your
  already archived lossless audio data
- Facility to read virtually any lossless codec from virtually any
  container file (by the use of external dependencies)
- Lets you recalculate ARCSs of a CD image at any time after ripping
- You define the task like "Take this audio and this TOC and just give me the
  checksums"
- Hides completely the concrete decoding of audio data
- Hides completely the parsing of metadata files

The following features are planned, but not yet implemented:

- Metadata parser for compact discs (based on libcdio)
- Metadata parser for cdrdao's TOC format
- Generic audio reader based on libsndfile (just as an alternative to ffmpeg)
- Add OGG support to the audio reader for FLAC/FLAC
- Add support for embedded CUE sheets to the audio reader for FLAC/FLAC


## Current codecs and formats

Libarcsdec contains:

- TOC/Metadata parser for CUE sheets (based on libcue >= 2.0.0)
- Generic audio reader (based on ffmpeg >= 3.1, for any lossless codec in any
  container, e.g. ALAC/M4, ALAC/CAF, APE/APE, AIFF/AIFF, FLAC/OGG ... you name
  it)
- Audio reader for FLAC/FLAC files (based on flac/libFLAC++)
- Audio reader for lossless Wavpack/WV files (based on libwavpack)
- Native audio reader for RIFFWAV/PCM files


## What libarcsdec does not

- Libarcsdec does not alter your files in any way and cannot be used for tagging
  etc.
- Libarcsdec does not contribute to tasks like verifying/matching, computing of
  the AccurateRip identifier, parsing the AccurateRip response etc. The API for
  those things is already provided by [libarcstk][1]
- Libarcsdec does not rip CDs
- Libarcsdec offers no network facilities and is not supposed to do so. The
  actual HTTP request for fetching the reference values from AccurateRip is
  better performed by the HTTP networking client of your choice.


## How to Build

### Mandatory buildtime dependencies:

- cmake >= 3.9.6

### Mandatory build- and runtime dependencies:

- libarcstk >= 0.1.0-alpha.1
- libcue >= 2.0.0

### Optional default build- and runtime dependencies:

- FLAC++ headers >= 1.3.1
- libwavpack >= 5.0.0
- ffmpeg >= 3.1

If you do not need any of the optional default dependencies, you can switch them
off independently from each other:

- build without FLAC support by ``-DWITH_FLAC=OFF``
- build without WavPack support by ``-DWITH_WVPK=OFF``
- build without ffmpeg support by ``-DWITH_FFMPEG=OFF``

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
installed .pc-file of libarcstk, e.g. because cmake does not respect /usr/local
while searching for files. The latter could be fixed by giving cmake a hint
like:

	$ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/usr/local ..

Proceed:

	$ cmake --build .
	$ sudo make install # installs to /usr/local

See a [detailed HowTo](BUILD.md) explaining different build scenarios and all
build switches.


## How to Use

- Consult the [example application](./examples). It illustrates the supported
  usecases.
- [Build the API documentation](BUILD.md#building-the-api-documentation) and
  view it in a browser


## Current Limitations

- Consider this pre-alpha quality
- No production release yet - will be 1.0.0
- API is not considered stable before 1.0.0 (may change any time in any way
  until then)
- The strategy to select a reader for a given input is rudimentary: just the
  first reader that passes the format tests is selected. So the reader appearing
  accepting codec/format X occurring first in the list "shadows" all subsequent
  readers that would also be able to read the input.
- The readers try to guess the channel ordering to recognize swapped channels.
  The default channel assignment left/right (with left = channel 0, right =
  channel 1) works well but scenarios with different channel assignment
  are not well tested
- Some readers may be more robust than others, especially the very fast RIFF/WAV
  PCM reader should be considered a last resort (it is valuable for debugging
  and testing)
- Handling of data tracks is not implemented and data tracks are just processed
  like audio tracks. What happens is completely untested.
- Untested on big endian plattforms
- Never built, installed or tested on Windows or Mac OS X


## Bugs

- No sufficient unit tests yet


[1]: https://codeberg.org/tristero/libarcstk

