# Example application: albumcalc

This example application demonstrates how libarcsdec is used to calculate
AccurateRip checksums for each track of an album. The album is provided as a
single audio file accompanied by a Cuesheet.

This example implements the exact same usecase as
[the example of the same name in libarcstk](
https://github.com/crf8472/libarcstk/tree/main/examples/albumcalc/),
but using the libarcsdec API instead of using libarcstk directly. If you compare
the code of this example to the code of libarcstk's albumcalc example you get an
illustration what libarcsdec adds to libarcstk.


## Requirements

Dependencies for building and running:

- libarcsdec.so in the ''build'' directory
- libarcstk.so in the ''build/libs/libarcstk'' directory or installed in system


## Build

If you have initialized and updated the submodule and configured libarcsdec
with the switch ''-DWITH_SUBMODULE=ON'', the dependency libarcstk will be
available in the build tree.

In this case, build the application with just

	$ make

otherwise use

	$ make LIBARCSTK=-larcstk

to use the system-installed version, but make sure it's up-to-date.

For removing all compiled and temporary files, just use

	$ make clean


## Usage

albumcalc expects two filenames as parameters, the first being a Cuesheet and
the second an audio file in a lossless audio format. (Due to a current
limitation of libarcsdec, WMA lossless can actually not be decoded.)

	$ ./albumcalc <name_of_chuesheet.cue> <name_of_audio_file>

For more information, read the comments in [albumcalc.cpp](./albumcalc.cpp).

