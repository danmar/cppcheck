#------------------------------------------------------
# Build type
#------------------------------------------------------
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo;MinSizeRel" CACHE STRING "Configs" FORCE)
if(DEFINED CMAKE_BUILD_TYPE)
   SET_PROPERTY(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CMAKE_CONFIGURATION_TYPES})
endif()

if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Debug")
endif()

# ----------------------------------------------------------------------------
#   PROJECT CONFIGURATION
# ----------------------------------------------------------------------------
option(USE_CLANG            "Use Clang compiler"                                            OFF)
option(USE_ANALYZE          "Use Clang compiler with analyze mode"                          OFF)
option(ANALYZE_MEMORY       "Clang dynamic analyzer: detector of uninitialized reads."      OFF)
option(ANALYZE_ADDRESS      "Clang dynamic analyzer: fast memory error detector. "          OFF)
option(ANALYZE_THREAD       "Clang dynamic analyzer: tool that detects data races. "        OFF)
option(ANALYZE_UNDEFINED    "Clang dynamic analyzer: undefined behavior checker. "          OFF)
option(ANALYZE_DATAFLOW     "Clang dynamic analyzer: general dynamic dataflow analysis."    OFF)
option(WARNINGS_ARE_ERRORS  "Treat warnings as errors"                                      OFF)
option(WARNINGS_ANSI_ISO    "Issue all the mandatory diagnostics Listed in C standard"      ON)

set(USE_MATCHCOMPILER "Auto" CACHE STRING "Usage of match compliler")
set_property(CACHE USE_MATCHCOMPILER PROPERTY STRINGS Auto Off On Verify) 
if (USE_MATCHCOMPILER STREQUAL "Auto")
    if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(USE_MATCHCOMPILER_OPT "On")
    else()
        set(USE_MATCHCOMPILER_OPT "Off")
    endif()
else()
    set(USE_MATCHCOMPILER_OPT ${USE_MATCHCOMPILER})
endif()

option(BUILD_TESTS          "Build tests"                                                   OFF)
option(BUILD_GUI            "Build the qt application"                                      OFF)

option(HAVE_RULES           "Usage of rules (needs PCRE library and headers)"               OFF)

set(CMAKE_INCLUDE_DIRS_CONFIGCMAKE ${CMAKE_INSTALL_PREFIX}/include      CACHE PATH "Output directory for headers")
set(CMAKE_LIB_DIRS_CONFIGCMAKE     ${CMAKE_INSTALL_PREFIX}/lib          CACHE PATH "Output directory for libraries")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
