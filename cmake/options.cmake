#------------------------------------------------------
# Build type
#------------------------------------------------------
SET(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configs" FORCE)
IF(DEFINED CMAKE_BUILD_TYPE)
   SET_PROPERTY(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CMAKE_CONFIGURATION_TYPES})
ENDIF()

IF(NOT CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE "Debug")
ENDIF()

# ----------------------------------------------------------------------------
#   PROJECT CONFIGURATION
# ----------------------------------------------------------------------------
OPTION(USE_CLANG            "Use Clang compiler"                                            OFF)
OPTION(USE_ANALYZE          "Use Clang compiler with analyze mode"                          OFF)
OPTION(ANALYZE_MEMORY       "Clang dynamic analyzer: is a detector of uninitialized reads." OFF)
OPTION(ANALYZE_ADDRESS      "Clang dynamic analyzer: is a fast memory error detector. "     OFF)
OPTION(ANALYZE_THREAD       "Clang dynamic analyzer: is a tool that detects data races. "   OFF)
OPTION(ANALYZE_UNDEFINED    "Clang dynamic analyzer: undefined behavior checker. "          OFF)
OPTION(ANALYZE_DATAFLOW     "Clang dynamic analyzer: is a general dynamic dataflow analysis." OFF)
OPTION(WARNINGS_ARE_ERRORS  "Treat warnings as errors"                                      ON)
OPTION(WARNINGS_ANSI_ISO    "Issue all the mandatory diagnostics Listed in C standard"      ON)

OPTION(BUILD_SHARED_LIBS    "Build shared libraries"                                        ON)
OPTION(BUILD_TESTS          "Build tests (unitary, integration)"                            OFF)
OPTION(BUILD_QT_APP         "Build the qt application"                                      OFF)
OPTION(BUILD_UTILS_TESTS    "Build applications tests using the different modules"          OFF)

SET(CMAKE_INCLUDE_DIRS_CONFIGCMAKE ${CMAKE_INSTALL_PREFIX}/include      CACHE PATH "Output directory for headers")
SET(CMAKE_LIB_DIRS_CONFIGCMAKE     ${CMAKE_INSTALL_PREFIX}/lib          CACHE PATH "Output directory for libraries")
SET(RUNTIME_OUTPUT_DIRECTORY       ${PROJECT_BINARY_DIR}/bin)
SET(ARCHIVE_OUTPUT_DIRECTORY       ${PROJECT_BINARY_DIR}/lib)
SET(LIBRARY_OUTPUT_DIRECTORY       ${PROJECT_BINARY_DIR}/lib)
