# Minimal CMake build file to build static cppcheck library
# This static library is used to build executables:
# - cli
# - tests
# - Qt GUI
find_package(PCRE REQUIRED)
include_directories("${PCRE_INCLUDE_DIR}")

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
  filelister_unix.cpp
  filelister_win32.cpp
  mathlib.cpp
  preprocessor.cpp
  settings.cpp
  token.cpp
  tokenize.cpp
)

if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wshadow -Wno-long-long -Wfloat-equal -Wcast-qual")
endif (CMAKE_COMPILER_IS_GNUCXX)

ADD_LIBRARY(checklib STATIC ${CHECKLIB_SRCS})
target_link_libraries(checklib ${PCRE_LIBRARIES})
