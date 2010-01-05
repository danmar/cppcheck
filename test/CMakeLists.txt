# Minimal CMake build file to build cppcheck test suite

SET(CHECKTEST_HDRS
  testsuite.h
)

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
ADD_EXECUTABLE(test ${CHECKTEST_SRCS} ${CHECKTEST_HDRS})
TARGET_LINK_LIBRARIES(test ${CHECK_LIBS})

# Add custom 'make check' -target
# It compiles and runs tests
add_custom_target(check COMMAND test)
add_dependencies(check test checklib)

