# Minimal CMake build file to build static cppcheck library
# This static library is used to build executables:
# - cli
# - tests
# - Qt GUI

SET(CHECKLIB_SRCS
  checkautovariables.cpp
  checkbufferoverrun.cpp
  checkclass.cpp
  checkdangerousfunctions.cpp
  checkexceptionsafety.cpp
  checkheaders.cpp
  checkmemoryleak.cpp
  checkother.cpp
  checkstl.cpp
  checkunusedfunctions.cpp
  cppcheck.cpp
  errorlogger.cpp
  executionpath.cpp
  filelister.cpp
  mathlib.cpp
  preprocessor.cpp
  settings.cpp
  token.cpp
  tokenize.cpp
)

if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS
    ${CMAKE_CXX_FLAGS}
    "-Wall -Wextra -pedantic"
  )
endif (CMAKE_COMPILER_IS_GNUCXX)

ADD_LIBRARY(checklib STATIC ${CHECKLIB_SRCS})

