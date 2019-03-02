

# Cppcheck 1.88 dev

## Introduction

Cppcheck is an analysis tool for C/C++ code. Unlike C/C++ compilers and many other analysis tools, it doesn't detect
syntax errors. Instead, Cppcheck detects the types of bugs that the compilers normally fail to detect. The goal is no
false positives.

Supported code and platforms:

  - You can check non-standard code that contains various compiler extensions, inline assembly code, etc.
  - Cppcheck should be compilable by any C++ compiler that handles the latest C++ standard.
  - Cppcheck should work on any platform that has sufficient CPU and memory.

Please understand that there are limits of Cppcheck. Cppcheck is rarely wrong about reported errors. But there are
many bugs that it doesn't detect.

You will find more bugs in your software by testing your software carefully, than by using Cppcheck. You will find
more bugs in your software by instrumenting your software, than by using Cppcheck. But Cppcheck can still detect some
of the bugs that you miss when testing and instrumenting your software.

## Getting started

### GUI

It is not required but creating a new project file is a good first step. There are a few options you can tweak to get
good results.

In the project settings dialog, the first option is "Import project". It is recommended that you use this feature if
you can. Cppcheck can import:
 - Visual studio solution / project
 - Compile database (can be generated for instance by cmake)
 - Borland C++ Builder 6

When you have filled out the project settings and click on OK; the Cppcheck analysis will start.

### Command line

A good first command to try is either...

If you have a Visual studio solution / compile database (cmake/qbs/etc) / C++ Builder 6 project:

    cppcheck --enable=warning --project=<path of solution / project / compile database>

Or:

    cppcheck --enable=warning <folder where your source code is>

You can extend and adjust the analysis in many ways later.

