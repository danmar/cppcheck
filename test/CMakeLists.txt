# Minimal CMake build file to build cppcheck test suite
find_package(PCRE REQUIRED)

SET(CPPCHECK_TEST_SOURCES
  "${CPPCHECK_SOURCE_DIR}/cli/filelister.cpp"
  testmemleak.cpp
  testother.cpp
  testpreprocessor.cpp
  testautovariables.cpp
  testredundantif.cpp
  testbufferoverrun.cpp
  testrunner.cpp
  testcharvar.cpp
  testsimplifytokens.cpp
  testclass.cpp
  teststl.cpp
  testconstructors.cpp
  testsuite.cpp
  testcppcheck.cpp
  testdangerousfunctions.cpp
  testtoken.cpp
  testdivision.cpp
  testtokenize.cpp
  testexceptionsafety.cpp
  testunusedfunctions.cpp
  testfilelister.cpp
  testunusedprivfunc.cpp
  testincompletestatement.cpp
  testunusedvar.cpp
  testmathlib.cpp
)

set(CPPCHECK_LIB_DIR "${CPPCHECK_SOURCE_DIR}/lib/")
include("${CPPCHECK_LIB_DIR}library_sources.cmake")

if(WIN32)
   set(CPPCHECK_TEST_SOURCES ${CPPCHECK_TEST_SOURCES}
       "${CPPCHECK_SOURCE_DIR}/cli/filelister_win32.cpp")

   if(NOT CYGWIN)
      # Windows needs additional shlwapi library.
      set(CPPCHECK_TEST_LINK_LIBRARIES ${CPPCHECK_TEST_LINK_LIBRARIES} shlwapi)
   endif()
else()
   set(CPPCHECK_TEST_SOURCES ${CPPCHECK_TEST_SOURCES}
       "${CPPCHECK_SOURCE_DIR}/cli/filelister_unix.cpp")
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wshadow -Wno-long-long -Wfloat-equal -Wcast-qual")
endif (CMAKE_COMPILER_IS_GNUCXX)

include_directories("${CPPCHECK_SOURCE_DIR}/lib" "${PCRE_INCLUDE_DIR}")
add_executable(test ${CPPCHECK_TEST_SOURCES} ${CPPCHECK_LIB_SOURCES})
TARGET_LINK_LIBRARIES(test ${CPPCHECK_TEST_LINK_LIBRARIES} ${PCRE_LIBRARIES})

# Add custom 'make check' -target
# It compiles and runs tests
add_custom_target(check COMMAND test)
add_dependencies(check test)
