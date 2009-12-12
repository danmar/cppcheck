# Minimal CMake build file to build static cppcheck library
# This static library is used to build executables:
# - cli

SET(CHECKLIB_SRCS
  checkautovariables.cpp
  checkmemoryleak.cpp
  filelister.cpp
  checkbufferoverrun.cpp
  checkother.cpp
  mathlib.cpp
  checkclass.cpp
  checkstl.cpp
  preprocessor.cpp
  checkdangerousfunctions.cpp
  checkunusedfunctions.cpp
  settings.cpp
  checkexceptionsafety.cpp
  cppcheck.cpp
  token.cpp
  checkheaders.cpp
  errorlogger.cpp
  tokenize.cpp
)

ADD_LIBRARY(checklib STATIC ${CHECKLIB_SRCS})

