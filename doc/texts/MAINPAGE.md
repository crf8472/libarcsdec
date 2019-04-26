\mainpage libarcsdec - A library with audio decoder and TOC parser adapters for libarcstk


Home: https://codeberg.org/tristero/libarcsdec


\section features Features

- A high-level API for sample and metadata input for [libarcstk][1]
- A bunch of decoder and parser adapters to let you calculate ARCSs on your
  already archived lossless audio data
- You define the task like "Take this audio and this TOC and just give me the
  checksums"
- Lets you recalculate ARCSs of a CD image at any time after ripping
- Facility to read virtually any lossless codec from virtually any
  container file (by the use of external dependencies)
- Hides completely the concrete decoding of audio data
- Hides completely the parsing of metadata files



\section nonfeatures What libarcsdec does not

- Libarcsdec does not alter your files in any way and cannot be used for tagging
  etc.
- Libarcsdec does not contribute to tasks like verifying/matching, computing of
  the AccurateRip identifier, parsing the AccurateRip response etc. The API for
  those things is already provided by [libarcstk][1]
- Libarcsdec does not rip CDs
- Libarcsdec offers no network facilities and is not supposed to do so. The
  actual HTTP request for fetching the reference values from AccurateRip is
  better performed by the HTTP networking client of your choice.



\section howtobuild How to Build

Build and install to just use the API:

	$ cd libarcsdec     # your libarcsdec root directory where README.md resides
	$ mkdir build && cd build
	$ cmake -DCMAKE_BUILD_TYPE=Release ..
	$ cmake --build .
	$ sudo make install # installs to /usr/local

A detailed HowTo explaining different build scenarios and all build switches see
BUILD.md.


[1]: https://codeberg.org/tristero/libarcstk
