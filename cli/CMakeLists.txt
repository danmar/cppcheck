# Minimal CMake build file to build cppcheck command line executable
find_package(PCRE REQUIRED)

set(TINYXML_INCLUDE_DIR "${CPPCHECK_SOURCE_DIR}/externals/tinyxml/")

SET(CPPCHECK_CLI_SOURCES
  cmdlineparser.cpp
  cppcheckexecutor.cpp
  filelister.cpp
  main.cpp
  threadexecutor.cpp
  "${TINYXML_INCLUDE_DIR}tinystr.cpp"
  "${TINYXML_INCLUDE_DIR}tinyxml.cpp"
  "${TINYXML_INCLUDE_DIR}tinyxmlerror.cpp"
  "${TINYXML_INCLUDE_DIR}tinyxmlparser.cpp")

set(CPPCHECK_LIB_DIR "${CPPCHECK_SOURCE_DIR}/lib/")
include("${CPPCHECK_LIB_DIR}library_sources.cmake")

if(WIN32)
   # Add Windows resource file
   set(CPPCHECK_CLI_SOURCES ${CPPCHECK_CLI_SOURCES} filelister_win32.cpp
       cppcheck.rc)

   if(NOT CYGWIN)
      # Windows needs additional shlwapi library.
      set(CPPCHECK_CLI_LINK_LIBRARIES ${CPPCHECK_CLI_LINK_LIBRARIES} shlwapi)
   endif()
else()
   set(CPPCHECK_CLI_SOURCES ${CPPCHECK_CLI_SOURCES} filelister_unix.cpp)
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wshadow -Wno-long-long -Wfloat-equal -Wcast-qual")
endif (CMAKE_COMPILER_IS_GNUCXX)

include_directories("${CPPCHECK_SOURCE_DIR}/lib"
                    "${PCRE_INCLUDE_DIR}"
                    "${TINYXML_INCLUDE_DIR}")
add_executable(cppcheck ${CPPCHECK_CLI_SOURCES} ${CPPCHECK_LIB_SOURCES})
target_link_libraries(cppcheck ${CPPCHECK_CLI_LINK_LIBRARIES} ${PCRE_LIBRARIES})
