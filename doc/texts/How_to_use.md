# How to Use

NOTE This is currently a stub with a mere TODO list.

## Motivation: Support Many Formats Without Fiddling With Them

  - We are only interested in getting the ARCSs.
  - We therefore need a reader/decoder combination that provides libarcstk with
    the input for calculation.
  - You may use any lossless codec for archiving.
  - You do not have to write your own audio readers since libarcsdec has the
	ones for you.
  - Use libarcsdec for other audio related calculations.

## Use Calculators

  - Calculator reads the input format magically.
  - They just provide the result.

## Query the Capabilities of libarcsdec

  - FileReaders, AudioReaders, Metaparsers
  - FileReaderDescriptor
  - FileReaderRegistry is a queryable static storage of all formats compiled in
  - LibInfo
  - RegisterDescriptor, RegisterFormat

## Request a Descriptor

  - FileReaderSelector
  - DescriptorPreference
  - FileReaderSelection
  - When writing a custom calculator, you need

## Available Descriptors in v0.2

  - parsercue
  - parsertoc
  - parserlibcue
  - readerwav
  - readerflac
  - readerffmpeg
  - readerwvpk
  - readersndfile

## Write a custom Audioreader or Metaparser

  - Use an existing library or implement the format on your own
  - Implement a Matcher and a Descriptor
  - SampleProvider and SampleProcessor

