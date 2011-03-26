# Minimal CMake build file to build cppcheck test suite
find_package(PCRE REQUIRED)

set(TINYXML_INCLUDE_DIR "${CPPCHECK_SOURCE_DIR}/externals/tinyxml/")

SET(CHECKTEST_SRCS
  "${CPPCHECK_SOURCE_DIR}/cli/cmdlineparser.cpp"
  "${CPPCHECK_SOURCE_DIR}/cli/cppcheckexecutor.cpp"
  "${CPPCHECK_SOURCE_DIR}/cli/filelister.cpp"
  "${CPPCHECK_SOURCE_DIR}/cli/threadexecutor.cpp"
  "${CPPCHECK_SOURCE_DIR}/cli/pathmatch.cpp"
  options.cpp
  testautovariables.cpp
  testbufferoverrun.cpp
  testcharvar.cpp
  testclass.cpp
  testcmdlineparser.cpp
  testconstructors.cpp
  testcppcheck.cpp
  testdivision.cpp
  testerrorlogger.cpp
  testexceptionsafety.cpp
  testincompletestatement.cpp
  testmathlib.cpp
  testmemleak.cpp
  testnullpointer.cpp
  testobsoletefunctions.cpp
  testoptions.cpp
  testother.cpp
  testpath.cpp
  testpostfixoperator.cpp
  testpreprocessor.cpp
  testrunner.cpp
  testsettings.cpp
  testsimplifytokens.cpp
  teststl.cpp
  testsuite.cpp
  testsymboldatabase.cpp
  testthreadexecutor.cpp
  testtoken.cpp
  testtokenize.cpp
  testuninitvar.cpp
  testunusedfunctions.cpp
  testunusedprivfunc.cpp
  testunusedvar.cpp
  "${TINYXML_INCLUDE_DIR}tinystr.cpp"
  "${TINYXML_INCLUDE_DIR}tinyxml.cpp"
  "${TINYXML_INCLUDE_DIR}tinyxmlerror.cpp"
  "${TINYXML_INCLUDE_DIR}tinyxmlparser.cpp")

set(CPPCHECK_LIB_DIR "${CPPCHECK_SOURCE_DIR}/lib/")
include("${CPPCHECK_LIB_DIR}library_sources.cmake")

if(WIN32)
   if(NOT CYGWIN)
      # Windows needs additional shlwapi library.
      set(CHECK_LIBS ${CHECK_LIBS} shlwapi)
   endif()
else()
   set(CHECKTEST_SRCS ${CHECKTEST_SRCS} testfilelister_unix.cpp)
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wshadow -Wno-long-long -Wfloat-equal -Wcast-qual")
endif (CMAKE_COMPILER_IS_GNUCXX)

include_directories("${CPPCHECK_SOURCE_DIR}/lib"
                    "${CPPCHECK_SOURCE_DIR}/cli"
                    "${PCRE_INCLUDE_DIR}"
                    "${TINYXML_INCLUDE_DIR}")
add_executable(test ${CHECKTEST_SRCS} ${CPPCHECK_LIB_SOURCES})
TARGET_LINK_LIBRARIES(test ${CHECK_LIBS} ${PCRE_LIBRARIES})

# Add custom 'make check' -target
# It compiles and runs tests
add_custom_target(check COMMAND test)
add_dependencies(check test)
