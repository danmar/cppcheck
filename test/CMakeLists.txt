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

include_directories (${CPPCHECK_SOURCE_DIR}/lib)
ADD_EXECUTABLE(test ${CHECKTEST_SRCS})
TARGET_LINK_LIBRARIES(test checklib)

