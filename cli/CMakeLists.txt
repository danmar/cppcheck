# Minimal CMake build file to build cppcheck command line executable

SET(CHECKCLI_SRCS
  cppcheckexecutor.cpp
  threadexecutor.cpp
  main.cpp
)

include_directories (${CPPCHECK_SOURCE_DIR}/lib)
ADD_EXECUTABLE(cppcheck ${CHECKCLI_SRCS})
TARGET_LINK_LIBRARIES(cppcheck checklib)

