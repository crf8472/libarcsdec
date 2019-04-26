# Example application: albumcalc

This example application demonstrates how libarcsdec is used to calculate
AccurateRip checksums for each track of an album. The album is provided as a
single audio file accompanied by a CUESheet.

This example implements the exact same usecase as [the example of the same name
in libarcstk](https://codeberg.org/tristero/libarcstk/examples/albumcalc), but
using libarcsdec. If you compare the code of this example to the code of
libarcstk's albumcalc example you get an illustration what libarcsdec adds to
libarcstk.


## Requirements

Dependencies for building and running:

- libarcsdec
- libarcstk


## Build

Build application with just

	$ make

For removing all compiled and temporary files, just use

	$ make clean


## Usage

albumcalc expects two filenames as parameters, the first being a CUE sheet and
the second an audio file in a lossless audio format. (Due to a current
limitation of libarcsdec, WMA lossless can actually not be decoded.)

	$ ./albumcalc <name_of_chuesheet.cue> <name_of_audio_file>

For more information, read the comments in [albumcalc.cpp](./albumcalc.cpp).

