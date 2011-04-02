# Minimal CMake build file to build static cppcheck library
# This static library is used to build executables:
# - cli
# - tests
# - Qt GUI

if(PCRE_FOUND)
   add_definitions(-DHAVE_RULES)
   include_directories("${PCRE_INCLUDE_DIR}")
endif()

set(CPPCHECK_LIB_DIR "")
include("library_sources.cmake")

if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wshadow -Wno-long-long -Wfloat-equal -Wcast-qual")
endif (CMAKE_COMPILER_IS_GNUCXX)

add_library(checklib STATIC ${CPPCHECK_LIB_SOURCES})

if(PCRE_FOUND)
   target_link_libraries(checklib ${PCRE_LIBRARIES})
endif()
