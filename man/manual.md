

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