# Minimal CMake build file
# Builds:
# - static library from lib directory
# - commandline executable
# - test suite

# To build with CMake:
# - install CMake 2.6 or later
# - $ cmake .
# - $ make

cmake_minimum_required (VERSION 2.6)

PROJECT(CPPCHECK)

ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(cli)
ADD_SUBDIRECTORY(test)
ADD_SUBDIRECTORY(gui)

