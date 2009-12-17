# Minimal CMake build file to build cppcheck test suite

SET(CHECKTEST_SRCS
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
ADD_EXECUTABLE(test ${CHECKTEST_SRCS})
TARGET_LINK_LIBRARIES(test ${CHECK_LIBS})

