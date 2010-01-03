# Minimal CMake build file to build cppcheck command line executable

SET(CHECKCLI_HDRS
  cppcheckexecutor.h
  threadexecutor.h
)

SET(CHECKCLI_SRCS
  cppcheckexecutor.cpp
  threadexecutor.cpp
  main.cpp
)

# Add Windows resource file
if (WIN32)
  SET(CHECKCLI_HDRS
    ${CHECKCLI_HDRS}
    resource.h
  )

  SET(CHECKCLI_SRCS
    ${CHECKCLI_SRCS}
    cppcheck.rc
  )
endif (WIN32)

# Libraries to link
set(CHECK_LIBS
  checklib
)

# Windows needs additional shlwapi library
if (WIN32 AND NOT CYGWIN)
set(CHECK_LIBS
  ${CHECK_LIBS}
  shlwapi
)
endif (WIN32 AND NOT CYGWIN)

if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS
    ${CMAKE_CXX_FLAGS}
    "-Wall -Wextra -pedantic"
  )
endif (CMAKE_COMPILER_IS_GNUCXX)

include_directories (${CPPCHECK_SOURCE_DIR}/lib)
ADD_EXECUTABLE(cppcheck ${CHECKCLI_SRCS} ${CHECKCLI_HDRS})
TARGET_LINK_LIBRARIES(cppcheck ${CHECK_LIBS})

