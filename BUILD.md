# How to Build



## Building libarcsdec on Linux and \*BSD

Libarcsdec is written in C++14. It was developed mainly (but not exclusively)
for Linux. It's runtime dependencies depend on the required codec or container
format support. With ffmpeg available, libarcsdec supports virtually every
lossless codec and any container format. It was not tested whether libarcsdec
builds out-of-the-box on BSDs but don't expect major issues.


## Buildtime dependencies


### Mandatory Buildtime dependencies

- C++-14-compliant-compiler with C++ standard library
- ``cmake`` >= 3.9.6
- ``make`` or some other build tool compatible to cmake (the examples suppose
  ``make`` nonetheless)
- [libarcs][6] -- dependency of libarcsdec
- [libcue][5] >= 2.0.0 -- parse CUE sheets


### Optional Buildtime dependencies

- flac (with [FLAC++][7]) >= 1.3.1 -- decode FLAC audio (in FLAC container files)
- [libwavpack][8] >= 5 -- decode WavePack audio (in wv container files)
- [ffmpeg][9] >= 3.1 -- decode virtually any codec in virtually any container
- git - for testing: to clone test framework [Catch2][2] as an external project
  when running the unit tests. You also need git if you want to build the
  documentation with m.css (instead of stock doxygen).
- Doxygen - for documentation: to build the API documentation in HTML or LaTeX
- graphviz - for documentation: to build the API documentation in HTML or LaTeX
- LaTeX (TeXLive for instance) - for documentation: to build the documentation
  in LaTeX
- [include-what-you-use][1] - for development: to control ``include``-relationships

NOTE on ffmpeg:

Libarcs uses the decoding API introduced with libavcodec 57.37.100 on 21, April
2016, hence the requirements are:

|ffmpeg-lib  |minimal version |
|------------|----------------|
|libavformat |57.33.100       |
|libavcodec  |57.37.100       |
|libavutil   |55.22.100       |

which determines ffmpeg 3.1 as the earliest possible version. Libarcs can not be
compiled against earlier versions of ffmpeg.


## Building the library

We presuppose you have downloaded and unpacked or git-cloned libarcsdec to a
folder named ``libarcsdec``. Then do:

	$ cd libarcsdec      # your libarcsdec root folder where README.md resides
	$ mkdir build && cd build  # create build folder for out-of-source-build
	$ cmake -DCMAKE_BUILD_TYPE=Release ..   # configure build type
	$ cmake --build .    # perform the actual build
	$ sudo make install  # installs to /usr/local

This will just install libarcsdec with all optimizations and without
debug-symbols and tests.

We describe the build configuration for the following profiles:
- User (read: a developer who uses libarcsdec in her project)
- Contributing developer (who wants to debug and test)
- Package maintainer (who intends to package libarcsdec for some target system).



### Users

You intend to install libarcsdec on your system, say, as a dependency for your
own project. You just want libarcsdec to be available along with its headers and
not getting in your way:

	$ cmake -DCMAKE_BUILD_TYPE=Release ..
	$ cmake --build .
	$ sudo make install

This presupposes the three optional buildtime dependencies
- FLAC++,
- libwavpack and
- ffmpeg.

You can use any of the [build switches](#build-switches) to deactivate optional
dependencies which are activated by default.

Regardless of whether you used any switches, this will install the following
files to your system:

- the shared object libarcsdec.so.x.y.z (along with a symbolic link
  ``libarcsdec.so``) in the standard library location (e.g. /usr/local/lib)
- the public header file calculators.hpp in the standard include location
  (e.g. /usr/local/include).
- the pkg-config configuration file libarcsdec.pc

You can change the install prefix by calling cmake with the
``-DCMAKE_INSTALL_PREFIX=/path/to/install/dir`` switch.


### Contributors

You want to debug into the libarcsdec code, hence you need to build libarcsdec
with debugging symbols and without aggressive optimization:

	$ cmake -DCMAKE_BUILD_TYPE=Debug ..
	$ cmake --build .

For also building and running the tests, just use the corresponding switch:

	$ cmake -DCMAKE_BUILD_TYPE=Debug -DWITH_TESTS=ON ..
	$ cmake --build .
	$ ctest

This will take significantly longer than building without tests.

Note that ``-DWITH_TESTS=ON`` will try to git-clone the testing framework
[Catch2][2] within your directory ``build`` and fail if this does not work.

Running the unit tests is *not* part of the build process. The tests have to be
executed manually after the build is completed, just by invoking ``ctest``.

Note that ctest will write report files in the ``build`` folder, their name
pattern is ``report.<testcase>.xml``.

You may or may not want the ``-march=native`` and ``-mtune=generic`` switches on
compilation. For Debug-builds, they are not used by default, but can be added by
using ``-DWITH_NATIVE=ON``. For now, this switch has only influence when using
gcc or clang. For other compilers, default settings apply.

During early development stage, I tended to mess up the includes. So for mere
personal use, I configured a workflow for reviewing the includes. CMake brings
some support for Google's tool [include-what-you-use][1]. If you have installed
this tool, you can instruct CMake to use it:

	$ cmake -DCMAKE_BUILD_TYPE=Debug -DIWYU=ON ..
	$ cmake --build . 2>iwuy.txt

This runs every source file through inlcude-what-you-use instead of the actual
compiler and writes the resulting analysis to file ``iwyu.txt``. If you want to
use your real compiler again, you have to reconfigure the project.


### Package maintainers

You want to build libarcsdec with a release profile but without any architecture
specific optimization (e.g. without ``-march=native`` and ``-mtune=generic`` for
gcc or clang).

Furthermore, you would like to adjust the install prefix path such that
libarcsdec is configured for being installed in the real system prefix (such as
``/usr``) instead of some default prefix (such as ``/usr/local``). You will also
choose a staging directory as an intermediate install target.

When using clang or gcc, this can be achieved as follows:

	$ cmake -DCMAKE_BUILD_TYPE=Release -DWITH_NATIVE=OFF -DCMAKE_INSTALL_PREFIX=/usr ..
	$ cmake --build .
	$ make DESTDIR=/my/staging/dir install

*Note* that ``-DWITH_NATIVE=OFF`` currently only works for clang and gcc.

If you use another compiler than clang or gcc, CMake will not apply any project
specific modifications to the compiler default settings. Therefore, you have to
carefully inspect the build process (e.g. by using ``$ make VERBOSE=1``) to
verify which compiler settings are actually used.

Use ``-DAS_STATIC=ON`` if you like to compile libarcsdec as a static library.

By default, the release-build of libarcsdec uses -O3. If you intend to change
that, locate [CMakeLists.txt](./CMakeLists.txt) in the root directory and adjust
it to your needs.


### Build switches

Whether or not libarcsdec shall use (some of) those optional runtime dependencies
is configured via CMake. The desired configuration may differ for users,
developers and packagers of libarcsdec, which is pointed out below.

Libarcs' CMake configuration knows the following switches:

|Switch         |Description                                     |Default|
|---------------|------------------------------------------------|-------|
|WITH_IWYU      |Use include-what-you-use on compiling           |OFF    |
|WITH_DOCS      |Configure documentation (but don't build it)    |OFF    |
|WITH_NATIVE    |Use platform specific optimization on compiling |OFF for CMAKE_BUILD_TYPE=Debug, ON for CMAKE_BUILD_TYPE=Release|
|WITH_TESTS     |Compile tests for runing ctest                  |OFF    |
|USE_MCSS       |Use [m.css][3] when building the documentation  |OFF    |
|WITH_FLAC      |Build with FLAC support by libflac              |ON     |
|WITH_WAVPACK   |Build with Wavpack support by libwavpack        |ON     |
|WITH_FFMPEG    |Build with ffmpeg support                       |ON     |

Note that there is no option to deactivate libcue since libcue is currently the
only way to pass TOC data to the library. Deactiving it would remove the
capability to process TOC data entirely.



## Building the API documentation

When you configure the project, switch ``-DWITH_DOCS=ON`` is required to prepare
building the documentation.

Doxygen and the tool ``dot`` from graphviz are required for building the
documentation. The documentation can be build as a set of static HTML pages
or as a PDF manual using LaTeX.


### Quickstart: Doxygen Stock HTML

The generation of the documentation sources must be requested at configuration
stage. The documentation sources will not be generated automatically during
build. It is required to call target ``doc`` manually. This target requires
doxygen and graphviz.

	$ cd build
	$ cmake -DWITH_DOCS=ON ..
	$ cmake --build . --target doc

This will build the documentation sources for HTML as well as LaTeX in
subdirectories of ``build/doc/``. Open the file ``build/doc/html/index.html`` in
your browser to see the entry page.


### Doxygen with HTML5 and CSS3 (experimental)

Accompanying [m.css][3] comes a doxygen style. It takes the doxygen XML output
and generates a static site in plain HTML5 and CSS3 from it (without
JavaScript). The resulting site presents the content much cleaner and (at least
in my opinion) more to the point regarding its structure than the stock doxygen
HTML output.

Build the m.css based documentation by the following steps:

	$ cd build
	$ cmake -DUSE_MCSS=ON ..   # Implies -DWITH_DOCS=ON
	$ cmake --build . --target doc

This generates the documentation in ``build/doc-mcss/``, you can load
``build/doc-mcss/html/index.html`` in your browser.

CMake builds a local python sandbox with ``virtualenv``, installs jinja2 and
Pygments in it, then clones m.css, and then runs doxygen by m.css's
``dox2html.py`` in the process. This is a bit of a machinery and maybe it needs
finetuning for some environments I did not foresee. Help on improvements is
welcome.


### Manual: PDF by LaTeX

Libarcsdec provides also support for a PDF manual using LaTeX. An actual LaTeX
installation (along with ``pdflatex``) is required for creating the manual.

Building the PDF manual is only available without ``-DUSE_MCSS=ON``. Using
``-DUSE_MCSS=ON`` will effectively turn off LaTeX source generation!

Building the ``doc`` target like in the examples above will create the LaTeX
sources for the PDF manual but will not automatically typeset the actual PDF
document. If you intend to build the PDF manual, do:

	$ cd build
	$ cmake -DWITH_DOCS=ON ..  # Do not use -DUSE_MCSS=ON!
	$ cmake --build . --target doc
	$ cd doc/latex
	$ make

This will create the manual ``refman.pdf`` in folder ``build/doc/latex`` (while
issueing loads of ``Underfull \hbox`` warnings, which is perfectly normal).


## Libarcsdec code in my ``$EDITOR``

If you used to work with some features of what is called "deep language support"
for C++, you may notice that libarcsdec comes with a top-level ``.clang`` file
that points to a file ``compile_commands.json`` in the same directory that will
only exist if you generate it. Generate it by:

	$ cd build
	$ cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
	$ ln -s compile_commands.json ..  # May or may not be required

This generated file is a compilation database. If your ``$EDITOR`` is configured
accordingly, the compilation database may (or may not) help your ``$EDITOR`` to
provide some enhanced features for navigating the code (i.e.
``goto-definition``) or autocompletion. If this sounds odd for you, it is
completely safe to skip this paragraph, ignore the ``.clang`` file and just feel
good.

I used to work with autocompletion for C++ in neovim. With the project, I share
the ``.clang`` file as a configuration stub for this. If you ignore it, it will
not get in your way.


## Build on Windows ... duh!

No Windows port yet :-(

In fact, as a lack of requirement, libarcsdec has not yet even been tried to be
built on Windows.

To avoid any show-stoppers for porting libarcsdec to Windows or other platforms,
libarcsdec tries to avoid POSIX or Linux-specific calls almost completely. (In
[readerwav.cpp](./src/readerwav.cpp), POSIX' ``stat()`` is used to retrieve the
file size.)

Of course it cannot be guaranteed that any dependency of libarcsdec is available
for your platform. The Fauxdacious project has documented [how to get each of
the dependencies to work (also including pkg-config)][4] on Windows using MinGW.


[1]: https://include-what-you-use.org/
[2]: https://github.com/catchorg/Catch2
[3]: https://mcss.mosra.cz/doxygen/
[4]: http://phoenixcomm.net/~jturner/fauxdacious_buildnotes.htm
[5]: https://github.com/libnitsk/libcue
[6]: https://codeberg.org/tristero/libarcs
[7]: https://xiph.org/flac/api/group__flacpp.html
[8]: http://www.wavpack.com/
[9]: http://ffmpeg.org/
