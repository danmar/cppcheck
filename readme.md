# Cppcheck [![Build Status](https://travis-ci.org/danmar/cppcheck.png?branch=master)](https://travis-ci.org/danmar/cppcheck)

## Donations

### Donations

If you find Cppcheck useful for you, feel free to make a donation.

[![Donate](http://pledgie.com/campaigns/4127.png)](http://pledgie.com/campaigns/4127)

## About the name

The original name of this program was "C++check", but it was later changed to "Cppcheck".

Despite the name, Cppcheck is designed for both C and C++.

## Manual

A manual is available [online](http://cppcheck.sourceforge.net/manual.pdf).

## Compiling

Any C++ compiler should work.

To build the GUI, you need Qt.

When building the command line tool, [PCRE](http://www.pcre.org/) is normally used.
PCRE is optional.

There are multiple compilation choices:
* qmake - cross platform build tool
* Windows: Visual Studio or Qt Creator or MinGW
* gnu make
* g++

### qmake

You can use the gui/gui.pro file to build the GUI.

```shell
cd gui
qmake
make
```

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

### gnu make

To build Cppcheck with rules (PCRE dependency):

```shell
make HAVE_RULES=yes
```

To build Cppcheck without rules (no dependencies):

```shell
make
```

### g++ (for experts)

If you just want to build Cppcheck without dependencies then you can use this command:

```shell
g++ -o cppcheck -Ilib cli/*.cpp lib/*.cpp
```

If you want to use `--rule` and `--rule-file` then dependencies are needed:

```shell
g++ -o cppcheck -lpcre -DHAVE_RULES -Ilib -Iexternals cli/*.cpp lib/*.cpp externals/tinyxml/*.cpp
```

### MinGW

```shell
make LDFLAGS=-lshlwapi
```

### Cross compiling Win32 (CLI) version of Cppcheck in Linux

```shell
sudo apt-get install mingw32
make CXX=i586-mingw32msvc-g++ LDFLAGS="-lshlwapi"
mv cppcheck cppcheck.exe
```

## Webpage

http://cppcheck.sourceforge.net/
