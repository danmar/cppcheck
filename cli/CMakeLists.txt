# Minimal CMake build file to build cppcheck command line executable

SET(CHECKCLI_SRCS
  cppcheckexecutor.cpp
  threadexecutor.cpp
  main.cpp
)

# Windows needs additional shlwapi library
if (WIN32 AND NOT CYGWIN)
set(CHECK_LIBS
  checklib
  shlwapi
)
else (WIN32 AND NOT CYGWIN)
set(CHECK_LIBS
  checklib
)
endif (WIN32 AND NOT CYGWIN)

include_directories (${CPPCHECK_SOURCE_DIR}/lib)
ADD_EXECUTABLE(cppcheck ${CHECKCLI_SRCS})
TARGET_LINK_LIBRARIES(cppcheck ${CHECK_LIBS})

