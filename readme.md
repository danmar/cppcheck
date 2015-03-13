# Cppcheck [![Build Status](https://travis-ci.org/danmar/cppcheck.svg?branch=master)](https://travis-ci.org/danmar/cppcheck) [![Coverity Scan Build Status](https://scan.coverity.com/projects/512/badge.svg)](https://scan.coverity.com/projects/512) [![Build status AppVeyor](https://ci.appveyor.com/api/projects/status/y242bl0lc5chu6j1?svg=true)](https://ci.appveyor.com/project/danmar/cppcheck)

## Donations

If you find Cppcheck useful for you, feel free to make a donation.

[![Donate](http://pledgie.com/campaigns/4127.png)](http://pledgie.com/campaigns/4127)

## About the name

The original name of this program was "C++check", but it was later changed to "Cppcheck".

Despite the name, Cppcheck is designed for both C and C++.

## Manual

A manual is available [online](http://cppcheck.sourceforge.net/manual.pdf).

## Compiling

Any C++11 compiler should work. For compilers with partial C++11 support it may work. If your compiler has the C++11 features that are available in Visual Studio 2010 then it will work. If nullptr is not supported by your compiler then this can be emulated using the header lib/cxx11emu.h.

To build the GUI, you need Qt.

When building the command line tool, [PCRE](http://www.pcre.org/) is optional. It is used if you build with rules.

There are multiple compilation choices:
* qmake - cross platform build tool provided by Qt
* CMake - cross platform build tool
* Windows: Visual Studio (VS 2010 and above) or Qt Creator or MinGW
* gnu make
* g++ 4.4 (and above)
* clang++ 2.9 (and above)

### qmake

You can use the gui/gui.pro file to build the GUI.

```shell
cd gui
qmake
make
```

### CMake

The normal procedure for configuring a project with CMake (both in Linux and Windows) is:
mkdir build && cd build
cmake ..

Depending on the platform and the CMake generator used (it can be specified with the -G option) it
will generate a Makefile, Visual Studio Solution, Ninja file, or other project files for other IDEs.
By default it is only enabled the compilation of the internal libraries and the console application.
If you want to build the tests, the GUI application or use more advanced options you should enabled
the specific CMake variables provided. For that purpose, the cmake GUI application (cmake-gui) could
be very useful.

This CMake configuration provides a install target that will install the cppcheck libraries and
applications in the installation path (specified by the CMake variable CMAKE_PREFIX_INSTALL).

### Visual Studio

Use the cppcheck.sln file. The rules are normally enabled.

To compile with rules (PCRE dependency):
* the PCRE dll is needed. It can be downloaded from [here](http://cppcheck.sourceforge.net/pcre-8.10-vs.zip).

To compile without rules (no dependencies):
* remove the preprocessor define `HAVE_RULES` from the project
* remove the pcre.lib from the project

### Qt Creator + MinGW

The PCRE dll is needed to build the CLI. It can be downloaded here:
http://software-download.name/pcre-library-windows/

### GNU make

Simple build (no dependencies):

```shell
make
```

The recommended release build is:

```shell
make SRCDIR=build CFGDIR=cfg HAVE_RULES=yes
```

Flags:

1.  `SRCDIR=build`  
    Python is used to optimise cppcheck

2.  `CFGDIR=cfg`  
    Specify folder where .cfg files are found

3.  `HAVE_RULES=yes`  
    Enable rules (PCRE is required if this is used)

### g++ (for experts)

If you just want to build Cppcheck without dependencies then you can use this command:

```shell
g++ -o cppcheck -std=c++0x -include lib/cxx11emu.h -Iexternals/tinyxml -Ilib cli/*.cpp lib/*.cpp externals/tinyxml/*.cpp
```

If you want to use `--rule` and `--rule-file` then dependencies are needed:

```shell
g++ -o cppcheck -std=c++0x -include lib/cxx11emu.h -lpcre -DHAVE_RULES -Ilib -Iexternals/tinyxml cli/*.cpp lib/*.cpp externals/tinyxml/*.cpp
```

### MinGW

```shell
make LDFLAGS=-lshlwapi
```

### Other Compiler/IDE

1. Create a empty project file / makefile.
2. Add all cpp files in the cppcheck cli and lib folders to the project file / makefile.
3. Compile.

### Cross compiling Win32 (CLI) version of Cppcheck in Linux

```shell
sudo apt-get install mingw32
make CXX=i586-mingw32msvc-g++ LDFLAGS="-lshlwapi" RDYNAMIC=""
mv cppcheck cppcheck.exe
```

## Webpage

http://cppcheck.sourceforge.net/
