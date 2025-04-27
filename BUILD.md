# How to Build



## Quickstart

We presuppose you have downloaded and unpacked or git-cloned libarcsdec to a
folder named ``libarcsdec``. Thereafter do:

	$ cd libarcsdec       # your libarcsdec root folder where README.md resides
	$ mkdir build && cd build  # create build folder for out-of-source-build
	$ cmake ..           # configure for Release
	$ cmake --build .    # perform the actual build
	$ sudo make install  # install to /usr/local

This will just build and install libarcsdec with all local optimizations and
without debug-symbols, tests and documentation. You will be able to use it in
your project.



## Building libarcsdec on Linux and \*BSD

Libarcsdec >= 0.2 is compiled as C++17. It was developed mainly (but not
exclusively) for Linux. It's runtime dependencies are configurable and may
depend on the requested codec or container format support. With ffmpeg
available, libarcsdec supports virtually every lossless codec and any container
format. It was not tested whether libarcsdec builds out-of-the-box on BSDs but
don't expect major issues.


### Mandatory Buildtime Dependencies

- C++-17-compliant-compiler with C++ standard library
- ``cmake`` >= 3.10
- ``make`` or some other build tool compatible to cmake (the examples suppose
  ``make`` nonetheless)
- [libarcstk][5] -- dependency of libarcsdec


### Optional Buildtime Dependencies

If you intend to run the tests or build the documentation, there are some
more dependencies required.

|Tool                  |Task                   | Description                   |
|----------------------|-----------------------|-------------------------------|
|**Git**               |Testing, Documentation |Clone test framework [Catch2][2] and site generator [m.css][3] |
|**Doxygen**           |Documentation          |Build documentation in HTML (graphviz/dot is not required) |
|**virtualenv**/Python |Documentation          |Build documentation in HTML styled with [m.css][3] |
|**LaTeX**             |Documentation          |Build documentation manual     |

Libarcsdec can use of the following libraries to parse or read input formats:

|Library                 |Version  |Description                                            |
|------------------------|---------|-------------------------------------------------------|
| [libcue][7]            |>= 2.0.0 | parse Cuesheets                                       |
| [ffmpeg][8]            |>= 3.1   | decode virtually any codec in virtually any container |
| flac (with [FLAC++][9])|>= 1.3.1 | decode FLAC audio (in FLAC container files)           |
| [libwavpack][10]       |>= 5     | decode Wavepack audio (in wv container files)         |

Each dependency to a library in this list can be activated or de-activated for
building by using its corresponding [configure switch](#configure-switches).

Some of these dependencies are activated for building by default (see
[Configure switches](#configure-switches)). They can be deactivated safely if
not required.

Of course, it is possible to deactivate all dependencies. Deactivating all
dependencies renders libarcsdec incapable of reading most input formats.
Libarcsdec without any dependencies is only capable of reading the following
input formats:

  - WAV audio files encoding PCM-16bit stereo, 44100 Hz (== [CDDA][6])
  - Cuesheets
  - CDRDAO's toc files


NOTE on ffmpeg:

Libarcsdec uses the decoding API introduced with libavcodec 57.37.100 on 21,
April 2016, hence the requirements are:

|ffmpeg-lib  |minimal version |
|------------|----------------|
|libavformat |57.33.100       |
|libavcodec  |57.37.100       |
|libavutil   |55.22.100       |

which determines ffmpeg 3.1 as the earliest possible version. Libarcsdec can not
be compiled against earlier versions of ffmpeg.


### Installed files

The following 13 files will be installed to your system:

- The shared object libarcsdec.so.x.y.z (along with a symbolic link
  ``libarcsdec.so``) in the standard library location (e.g. /usr/local/lib)
- The 7 public header files ``audioreader.hpp``, ``calculators.hpp``,
  ``descriptor.hpp``, ``metaparser.hpp``, ``sampleproc.hpp``, ``selection.hpp``
  and ``version.hpp`` in the default include location (e.g.
  ``/usr/local/include``).
- The 4 cmake packaging files ``libarcsdec-config.cmake``,
  ``libarcsdec-config-version.cmake``, ``libarcsdec-targets.cmake`` and
  ``libarcsdec-targets-release.cmake`` in directory ``libarcsdec`` beneath the
  default cmake location (e.g. ``/usr/local/lib/cmake``). Those files allow
  other projects to simply import libarcsdec's exported cmake targets.
- The pkg-config configuration file ``libarcsdec.pc`` in the default pkgconfig
  location (e.g. ``/usr/local/lib/pkgconfig``).

The default installation prefix can be changed by passing the actual prefix to
cmake. This is achieved by using the switch
``-DCMAKE_INSTALL_PREFIX=/path/to/install/dir`` in the configure step. See
[Configure switches](#configure-switches) for more configuration options.

We describe the build configuration for the following profiles:
- [User](#users) (read: a developer who uses libarcsdec in her project)
- [Contributing developer](#contributors) (who intends to debug and test
  libarcsdec and maybe contribute to the documentation)
- [Package maintainer](#package-maintainers) (who intends to package libarcsdec
  for some target system).


### Users

You intend to install libarcsdec on your system, say, as a dependency for your
own project. You just need libarcsdec to be available along with its headers and
not getting in your way:

	$ cmake -DCMAKE_BUILD_TYPE=Release ..
	$ cmake --build .
	$ sudo make install

Note that this presupposes the three optional buildtime dependencies that are
activated in the default configuration:

- ffmpeg
- FLAC++
- libwavpack

Any of the [configure switches](#configure-switches) can be used to deactivate
optional dependencies which are activated by default.

For example to compile without libwavpack, do:

	$ cmake -DCMAKE_BUILD_TYPE=Release -DWITH_LIBWAVPACK=OFF ..
	$ cmake --build .
	$ sudo make install


### Contributors

You want to debug into the libarcsdec code, hence you need to build libarcsdec
*with* debugging symbols and *without* aggressive optimization:

	$ cmake -DCMAKE_BUILD_TYPE=Debug ..

For also building and running the tests, just use the corresponding switch:

	$ cmake -DCMAKE_BUILD_TYPE=Debug -DWITH_TESTS=ON ..

Thereafter just start the build and run the tests:

	$ cmake --build .
	$ ctest

Note: This build will take *significantly longer* than the build without
tests.


### Package maintainers

You want to build libarcsdec with a release profile but without any architecture
specific optimization (e.g. without ``-march=native`` and ``-mtune=generic`` for
g++ or clang++).

Furthermore, you would like to adjust the install prefix path such that
libarcsdec is configured for being installed in the real system prefix (such as
``/usr``) instead of some default prefix (such as ``/usr/local``).

You may also want to specify a staging directory as an intermediate install
target.

When using clang++ or g++, all of these can be achieved as follows:

	$ cmake -DCMAKE_BUILD_TYPE=Release -DWITH_NATIVE=OFF -DCMAKE_INSTALL_PREFIX=/usr ..
	$ cmake --build .
	$ make DESTDIR=/my/staging/dir install

**Note** that ``-DWITH_NATIVE=OFF`` currently only works for clang++ and g++.
The build process is *untested* and *broken* on other compilers.

If you use another compiler than clang++ or g++, CMake will not apply any
project specific modifications to the compiler default settings. Therefore, you
have to carefully inspect the build process (e.g. by using ``$ make VERBOSE=1``
instead of ``cmake --build .``) to verify which compiler settings are actually
used.


### Configure switches

|Switch              |Description                                     |Default|
|--------------------|------------------------------------------------|-------|
|CMAKE_BUILD_TYPE    |Build type for release or debug             |``Release``|
|CMAKE_INSTALL_PREFIX|Top-level install location prefix   |*plattform defined*|
|CMAKE_EXPORT_COMPILE_COMMANDS|Rebuild a [compilation database](#deep-language-support-in-your-editor) when configuring |OFF    |
|USE_DOC_TOOL        |Set 'MCSS' to use [m.css](#doxygen-by-m-css-with-html5-and-css3-tested-but-still-experimental) to build the documentation. Set 'LUALATEX' to build the manual. | *none* |
|WITH_DOCS           |Configure for [documentation](#building-the-api-documentation)                                     |OFF    |
|WITH_NATIVE         |Use platform [specific optimization](#turn-optimizing-on-off) on compiling                         |       |
|                    |CMAKE_BUILD_TYPE=Debug                                                                             |OFF    |
|                    |CMAKE_BUILD_TYPE=Release                                                                           |ON     |
|WITH_TESTS          |Compile [tests](#run-unit-tests) (but don't run them)                                              |OFF    |
|WITH_LIBCUE         |Build with libcue support                                                                          |OFF    |
|WITH_FFMPEG         |Build with ffmpeg support                                                                          |ON     |
|WITH_FLAC           |Build with FLAC support by libflac                                                                 |ON     |
|WITH_WAVPACK        |Build with Wavpack support by libwavpack                                                           |ON     |
|WITH_LIBSNDFILE     |Build with libsndfile support                                                                      |OFF    |
|WITH_SUBMODULES     |Build with libarcstk as a submodule                                                                |OFF    |

Note that ``USE_DOC_TOOL`` can be passed multiple values. For example, building
the HTML version as well as the manual in one build run is achieved by:

	$ cmake -DUSE_DOC_TOOL=MCSS\;LUALATEX ..


### Switch between clang++ and g++

Libarcsdec is tested to compile with clang++ as well as with g++.

If you want to switch the compiler, you should just hint CMake what compiler to
use. On unixoid systems you can usually do this via the environment variables
``CC`` and ``CXX``.

If your current compiler is not clang++ and you want to use your installed
clang++:

	$ export CC=$(type -p clang)
	$ export CXX=$(type -p clang++)

If your current compiler is not g++ and you want to use your installed g++:

	$ export CC=$(type -p gcc)
	$ export CXX=$(type -p g++)

Delete your directory ``build`` since it contains metadata from the previous
compiler. Start off cleanly.

	$ cd ..
	$ rm -rf build

CMake-reconfigure the project to have the change take effect:

	$ mkdir build && cd build
	$ cmake ..

To check whether your setting took effect, observe the CMake output. During the
configure step, CMake informs about the actual C++-compiler like:

	-- The CXX compiler identification is Clang 19.1.7
	...
	-- Check for working CXX compiler: /usr/bin/clang++ - works


### Turn optimizing on/off

You may or may not want the ``-march=native`` and ``-mtune=generic`` switches on
compilation. For Debug-builds, they are ``OFF`` by default, but can be added by
using ``-DWITH_NATIVE=ON``. For now, this switch has only influence when using
g++ or clang++. For other compilers, default settings apply.


### Run unit tests

Note that ``-DWITH_TESTS=ON`` will try to git-clone the testing framework
[Catch2][2] within your ``build`` directory and fail if this does not work.

Running the unit tests is *not* part of the build process. To run the tests,
invoke ``ctest`` manually in the ``build`` directory after ``cmake --build .``
is completed.

Note that ctest will write report files in the ``build`` folder, their name
pattern is ``report.<testcase>.xml`` where ``<testcase>`` corresponds to a
``.cpp``-file in ``test/src``.


### Build with libarcstk as a submodule

Having installed the dependencies system-wide is considered the standard setup.
For some development tasks, this may nonetheless not be convenient. If
installing libarcstk is for any reason not applicable, libarcstk can
alternatively be build as a submodule of libarcsdec. For setting up a fresh
local libarcsdec repo with libarcstk as a submodule of libarcsdec, do the
following:

	$ git clone --recurse-submodules https://github.com/crf8472/libarcsdec
	$ cd libarcsdec      # your libarcsdec root folder where README.md resides
	$ mkdir build && cd build  # create build folder for out-of-source-build
	$ cmake -DWITH_SUBMODULES=ON .. # add any switches required
	$ cmake --build .    # perform the actual build (including libarcstk)

Alternatively, if you already have cloned libarcsdec without using the
``--recurse-submodules`` switch, you can alternatively setup the submodule
thereafter by applying:

	$ cd libarcsdec      # your libarcsdec root folder where README.md resides
	$ git submodule init
	$ git submodule update # clones libarcstk as a submodule
	$ mkdir build && cd build  # create build folder for out-of-source-build
	$ cmake -DWITH_SUBMODULES=ON ..

Note that if libarcsdec was configured with ``-DWITH_SUBMODULES=ON`` any
switches will be applied to libarcstk as well! If you use ``-DWITH_TESTS`` or
``WITH_DOCS`` when configuring libarcsdec those options will be applied
recursively to libarcstk. Equivalently, using ``make install`` will install
libarcsdec as well as libarcstk!

*Note* that this setup is only intended to make handling synchronous development
tasks (such as debugging) on both libraries libarcsdec and libarcstk easier. It
is not intended to support packaging or installation in productional
environments.


### Cleaning the project

Clean only the shared library binaries (when in directory ``build``):

	$ cmake -P CMakeFiles/libarcsdec.dir/cmake_clean.cmake

Clean the project entirely:

	$ cmake --build . --target clean

Note that this forces to recompile everything including Catch2 if
``-DWITH_TESTS`` is configured.

Completely wipe everything configured and built locally (when in top-level
directory):

	$ rm -rf build



## Building the API documentation

When you configure the project, switch ``-DWITH_DOCS=ON`` is required to prepare
building the documentation. Only this configuration option will create the
target ``doc`` that can build the documentation.

Doxygen is required for building the documentation in either case.

The documentation can be build as a set of static HTML pages (recommended) or as
a PDF manual using LaTeX (experimental, very alpha).

When building HTML, you may choose either the stock HTML output of doxygen or
the HTML output styled by m.css. Doxygen's stock HTML output is stable but looks
outdated. The m.css-styled seems by far user-friendlier, cleaner and more
adapted for documentation of modern C++. On the other hand it is more cutting
edge and therefore not as stable as doxygen's stock HTML output. Credits for
m.css go to [mozra][3].


### Website: Doxygen Stock HTML

The generation of the documentation sources must be requested at configuration
stage. The documentation sources will not be generated automatically during
build. It is required to call target ``doc`` manually.

	$ cd build
	$ cmake -DWITH_DOCS=ON ..
	$ cmake --build . --target doc

This will build the documentation sources for HTML in subdirectories of
``build/generated-docs/doxygen``. Open the file
``build/generated-docs/doxygen/html/index.html`` in your browser to see the
entry page.


### Website: m.css with HTML5 and CSS3 via doxygen's XML

Accompanying [m.css][3] comes a doxygen style. It takes the doxygen XML output
and generates a static site in plain HTML5 and CSS3 from it (nearly without
JavaScript).

The [public APIdoc of libarcsdec is build with m.css][4].

This APIdoc can be built locally by the following steps:

	$ cd build
	$ cmake -DWITH_DOCS=ON -DUSE_DOC_TOOL=MCSS ..
	$ cmake --build . --target doc

CMake then creates a local python sandbox in directory ``build`` using
``virtualenv``, installs jinja2 and Pygments in it, then clones [m.css][3], and
then runs m.css which internally runs doxygen. Maybe this process needs
finetuning for some environments. (It is completely untested on Windows and will
not work.)

Documentation is generated in ``build/generated-docs/mcss`` and you can
load ``build/generated-docs/mcss/html/index.html`` in your browser.


### Manual: PDF by LaTeX (smoke-tested, more or less)

Libarcsdec provides also support for a PDF manual using LaTeX. An actual LaTeX
installation (containing ``lualatex`` and ``epstopdf``) is required for creating
the manual.

Building the PDF manual can be requested by using ``-DUSE_DOC_TOOL=LUALATEX``.
It will therefore be typeset by building the ``doc`` target.

The entire process:

	$ cd build
	$ cmake -DWITH_DOCS=ON -DUSE_DOC_TOOL=LUALATEX ..
	$ cmake --build . --target doc

This will create the manual ``refman.pdf`` in folder
``build/generated-docs/doxygen/lualatex`` (while issuing loads of warnings,
which is perfectly normal).

Note that I did never give any love to the manual. It will build. Not more.
However, it will not be convenient to read or look good at its current stage.


## Using a compilation database

A compilation database provides the dependencies and paths used for building the
project. CDBs are used for deep language support in the ``$EDITOR`` or IDE.

If you intend to use an LSP server (e.g. the one from clang++), the use of the
CDB is encouraged since otherwise the LSP server may not find required paths and
augment your display with artifacts that suggest errors which in fact don't
exist.

You may have noticed that libarcsdec comes with a top-level ``.clang`` file that
already points to ``compile_commands.json`` in the same directory. This prepares
the support for clang-based DLS for libarcsdec. However, the compilation
database is OFF in the default configuration and must be re-built locally for
the local compiler and the local settings:

	$ cd build
	$ cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
	$ cd ..
	$ ln -s build/compile_commands.json . # May or may not be required

The compilation database is now recreated whenever configuring the project.
Its recreation can be forced by just doing:

	$ cd build
	$ cmake ..

If you do not intend to use an LSP server, it is completely safe to skip this
paragraph, ignore the ``.clang`` file and just feel good. It will not get in
your way.


## Build on Windows ... duh!

No Windows port yet :-(

In fact, as a lack of requirement, libarcsdec has not yet even been tried to be
built on Windows.

To avoid any show-stoppers for porting libarcsdec to Windows or other platforms,
libarcsdec tries to avoid Linux-specific calls almost completely.

Of course it cannot be guaranteed that any dependency of libarcsdec is available
for your platform. The Fauxdacious project has documented [how to get each of
the dependencies to work (also including pkg-config)][11] on Windows using
MinGW.


[1]: https://include-what-you-use.org/
[2]: https://github.com/catchorg/Catch2
[3]: https://mcss.mosra.cz/doxygen/
[4]: https://crf8472.github.io/crf8472/libarcsdec/current/
[5]: https://github.com/crf8472/libarcstk
[6]: https://en.wikipedia.org/wiki/Compact_Disc_Digital_Audio
[7]: https://github.com/libnitsk/libcue
[8]: http://ffmpeg.org/
[9]: https://xiph.org/flac/api/group__flacpp.html
[10]: https://www.wavpack.com/
[11]: https://github.com/wildstar84/fauxdacious/blob/master/contrib/win32/fauxdacious_buildnotes.htm

