# Minimal CMake build file
# Builds:
# - static library from lib directory
# - commandline executable
# - test suite
# - Qt GUI

# To build with CMake:
# - install CMake 2.6 or later
# - $ cmake .
# - $ make

cmake_minimum_required (VERSION 2.6)

PROJECT(CPPCHECK)

ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(cli)
ADD_SUBDIRECTORY(test)

# Don't build GUI for Cygwin (most installations don't have QT)
IF (NOT CYGWIN)
  ADD_SUBDIRECTORY(gui)
endif (NOT CYGWIN)
