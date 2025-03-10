# Example applications

libarcsdec comes with 2 mini examples for what can be done with libarcstk when
libarcsdec is present and how this is to be done.

Building the examples requires libarcsdec to have been successfully build (e.g.
the .so-file residing in ``build/``.). If you do not intend to use g++, adjust
the Makefiles to your compiler before building the examples.

The example applications are compiled against the libarcsdec.so object in the
build tree! Use each example application exclusively in the directory it was
compiled since it is only guaranteed to work from there.

The aim of the examples is demonstration, not productive use. Backup any file
you intend to use as input for the example applications!

Note: The example applications do exclusively target situations where an entire
album as a single audio file along with its metadata (as a Cuesheet) is to be
processed. This restriction keeps the examples reasonably small. Of course,
libarcsdec can also process single audio files representing specific tracks or a
set of audio files that represent an album or parts of an album. For this use
cases, there is currently no example provided but given the examples and the API
documentation it should not be that hard to figure out how to do it.

Each of the example applications lets the user input her own data and is
intended to provide useful results for a specific task.

- [albumid](./albumid/README.md) - Demonstrates how to derive the
  AccurateRip-ID of a ripped album. With this id, libarcsdec also provides the
  Query-Request-URL specific to that album as well as the canonical filename for
  the response file. This part of the API can be used to send a query request to
  AccurateRip and receive the checksums for the actual album ripped locally.
- [albumcalc](./albumcalc/README.md) - Demonstrates how AccurateRip checksums
  can be calculated locally on a ripped audio image and a Cuesheet.

## Disclaimer

Note that despite the example applications may provide some useful functions,
they are rudimentary implemented and in no way hardened for production use. They
are mere local demonstrations. Do not use them as tools!
