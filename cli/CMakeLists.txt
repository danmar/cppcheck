# Minimal CMake build file to build cppcheck command line executable
find_package(PCRE REQUIRED)

SET(CPPCHECK_CLI_SOURCES
  cppcheckexecutor.cpp
  threadexecutor.cpp
  main.cpp
)

# Libraries to link
#set(CHECK_LIBS
#  checklib
#)

aux_source_directory(${CPPCHECK_SOURCE_DIR}/lib LIB_SRCS)

if(WIN32)
   # Add Windows resource file
   set(CPPCHECK_CLI_SOURCES ${CPPCHECK_CLI_SOURCES} cppcheck.rc)

   if(NOT CYGWIN)
      # Windows needs additional shlwapi library.
      set(CPPCHECK_CLI_LINK_LIBRARIES ${CPPCHECK_CLI_LINK_LIBRARIES} shlwapi)
   endif()
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wshadow -Wno-long-long -Wfloat-equal -Wcast-qual")
endif (CMAKE_COMPILER_IS_GNUCXX)

include_directories (${CPPCHECK_SOURCE_DIR}/lib)
ADD_EXECUTABLE(cppcheck ${CPPCHECK_CLI_SOURCES} ${LIB_SRCS})
TARGET_LINK_LIBRARIES(cppcheck ${CPPCHECK_CLI_LINK_LIBRARIES})
