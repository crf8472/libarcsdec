# Changelog

All notable changes to this project will be documented in this file.

## [0.3.0]

### Added
  - API: provide include-all header libarcsdec.hpp

### Changed
  - API: Organize everything in 3 namespaces: read, select, calc
  - API: ARCSCalculator::calculate() overloads provide
  - API: ARIdCalculator::calculate() overloads provide
  - API: ToCParser::parse() provides a ToC instead of std::unique_ptr<ToC>
  - API: AudioInfo provides an AudioSize instead of
    std::unique_ptr<AudioSize>
  - readerffmpeg: remove deprecated symbols from FF_API_SUBFRAMES
    (deprecated since 2023-05-15 by
	commit c29a1cbd03d5dd6b3161e1acf9cb31111511ac0a)
  - Build: Reorganize CMake structure, make top-level CMakeLists.txt smaller
  - Build: Remove deprecated build switch USE_MCSS
  - Build: Add build switch WITH_EXAMPLES
  - Build: Reorganize tests

### Fixed
  - No more breaking tests if -DWITH_FLAC=OFF
  - readersndfile emits start_input and end_input


## [0.2.0-alpha.3] - 2026-01-03

### Changed
  - API: Remove output parameter 'leadout' from ARCSCalculator::calculate()
  - API: ARCSCalculator::calculate() returns std::pair instead of Checksums


## [0.2.0-alpha.2] - 2025-12-31

### Added
  - Start of experimental support for cdrtoc meta format (currently broken yet)
  - Improve handling of log messages from ffmpeg
  - Documentation: Start to write some HowTos (work-in-progress)

### Changed
  - API: remove ARId from return tuple of ARCSCalculator::calculate()
  - Build: Deprecate configure switches -DUSE_ in favour of -DUSE_DOC_TOOL=
  - Build: Default-deactivate LaTeX on target 'doc'
  - Build: Improve doxygen postprocessing, fix some annoyances in output
  - Build: Add experimental support for doxygen-awesome-css
  - Build: Add GraphViz options for debugging cmake targets
  - Documentation: Add BUILD.md to doxygen documentation
  - Minor cleaning in code

### Fixed
  - Build: Some fixes in build management


## [0.2.0-alpha.1] - 2025-03-02

### Changed
  - Compileable as C++17. Support for C++14 is dropped in main branch. (Use
    branch 0.1 if you require C++14.)
  - Client code using version 0.1* will not be compileable anymore with
    version 0.2 and above.
  - Requires libarcstk v0.3
  - Use std::filesystem for file system tasks

### Fixed
  - Many small fixes, like using 'final' more consequently


## [0.1.1-alpha.1] - 2024-04-04

### Added
  - API: dependency-free parser for cuesheets

### Changed
  - API: libcue-based parser changed id to 'libcue' (from: 'cuesheet')
  - API: id 'cuesheet' does now point to newly added stock parser
  - API: the libcue-based parser has been given a new id: libcue
  - libcue is not anymore a mandatory dependency but optional with default ON
  - Update libarcstk submodule to 0.2.0-alpha.1

### Fixed
  - Many little fixes


## [0.1.0-alpha.6] - 2023-08-01

### Changed
  - Determine CMake minimum version to 3.6
  - Update libarcstk submodule to 0.1.1-beta.2
  - Update Catch2 to 3.4.0

### Fixed
  - Test only compiled readers/parsers


## [0.1.0-alpha.5] - 2023-03-12

### Added
  - FileReaders can now be selected by their id
  - Adapt new channel API of ffmpeg 5.1
  - Experimental support for libsndfile
  - Build config to be cloned as a submodule

### Changed
  - API: Refactor interfaces of calculators
  - API: Complete redo of the selection mechanism

### Fixed
  - No more compile warnings with ffmpeg >= 6.0
  - Fix compile bugs with ffmpeg < 5.0
  - Compatibility to libarcstk 0.1.1-beta.1


## [0.1.0-alpha.4] - 2022-10-02

### Changed
  - Adjust warnings flags for sources and tests
  - Upgrade to Catch2 v3.1.0

### Fixed
  - Some compile warnings


## [0.1.0-alpha.3] - 2021-10-18

### Changed
  - Complete refactoring of ffmpeg based reader (several fixes)
  - Update build-process for documentation

### Removed
  - audiobuffer.cpp


## [0.1.0-alpha.2] - 2021-03-11

### Changed
  - Remove unused generic buffering (BlockAccumulator and SampleBuffer)
  - Add signals for 'startinput' and 'endinput' to all *readers
  - Rewrite most of the ffmpeg-based reader, thereby using modern C++ smart
    pointers
  - Improve exceptions
  - Loads of refactorings, improved some class interfaces
  - Remove some doxygen warnings
  - Remove loads of clang-warnings (e.g. added missing 'override' declarations)
  - Fix #1
  - Add a lot of unit tests


## [0.1.0-alpha.1] - 2019-06-02

### Added
  - Initial pre-release

