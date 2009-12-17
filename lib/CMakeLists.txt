# Minimal CMake build file to build static cppcheck library
# This static library is used to build executables:
# - cli

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

ADD_LIBRARY(checklib STATIC ${CHECKLIB_SRCS})

