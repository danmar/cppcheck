# Minimal CMake build file to build cppcheck command line executable
find_package(PCRE REQUIRED)

SET(CHECKCLI_SRCS
  cppcheckexecutor.cpp
  filelister.cpp
  threadexecutor.cpp
  main.cpp
)

set(CPPCHECK_LIB_DIR "${CPPCHECK_SOURCE_DIR}/lib/")
include("${CPPCHECK_LIB_DIR}library_sources.cmake")

if(WIN32)
   # Add Windows resource file
   set(CHECKCLI_SRCS ${CHECKCLI_SRCS} filelister_win32.cpp
       cppcheck.rc)

   if(NOT CYGWIN)
      # Windows needs additional shlwapi library.
      set(CHECK_LIBS ${CHECK_LIBS} shlwapi)
   endif()
else()
   set(CHECKCLI_SRCS ${CHECKCLI_SRCS} filelister_unix.cpp)
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS
    ${CMAKE_CXX_FLAGS}
    "-Wall -Wextra -pedantic -Wshadow -Wno-long-long -Wfloat-equal -Wcast-qual"
  )
endif (CMAKE_COMPILER_IS_GNUCXX)

include_directories("${CPPCHECK_SOURCE_DIR}/lib"
                    "${PCRE_INCLUDE_DIR}")
add_executable(cppcheck ${CHECKCLI_SRCS} ${CPPCHECK_LIB_SOURCES})
TARGET_LINK_LIBRARIES(cppcheck ${CHECK_LIBS} ${PCRE_LIBRARIES})
