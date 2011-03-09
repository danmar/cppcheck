cmake_minimum_required (VERSION 2.6)

PROJECT(CPPCHECK)

set(CMAKE_MODULE_PATH "${CPPCHECK_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(cli)
ADD_SUBDIRECTORY(gui)
ADD_SUBDIRECTORY(man)

# Exclude tests from command line targets but include them to VS IDE targets.
# There is 'make check' -target for compiling and running tests from
# command line.
IF (MSVC_IDE)
  ADD_SUBDIRECTORY(test)
ELSE (MSVC_IDE)
  ADD_SUBDIRECTORY(test EXCLUDE_FROM_ALL)
ENDIF (MSVC_IDE)
