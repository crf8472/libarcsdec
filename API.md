# API

Note that every symbol of libarcsdec resides in namespace ``arcs``.



## Public API: [calculators.hpp](./src/calculators.hpp)

A high level API for reading TOC files and calculating ARCSs. You only
need to throw in the TOC and the audiofile without any care about the
container format or codec - as long as it is losslessly encoded. You can
calculate the actual ARCSs of the tracks as well as the AccurateRip id of the
original CD. There is also a format indifferent TOCParser for TOC metadata. You
can just provide your files and the API will determine audio format and codec as
well as metadata format.



# Internal APIs

Of course, there are four internal APIs that are not exported to the public
scope, but may be interesting for either understanding the project structure or
for people who would like to contribute:

- [audioreader.hpp](./src/audioreader.hpp) - Write a new or extend an existing
  audio reader.
- [metaparser.hpp](./src/metaparser.hpp) - Write a new or extend an existing
  parser for TOC informations.
- [fileformats.hpp](./src/fileformats.hpp) - Tune how readers and parsers are
  selected for a given input
- [audiobuffer.hpp](./src/audiobuffer.hpp) - Understand or modify the way how
  audio samples are optionally buffered for calculation.

The parser\*.hpp and reader\*.hpp headers provide interfaces to concrete
metadata parsers and audio readers. They are not informative.

