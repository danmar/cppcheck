#------------------------------------------------------
# Build type
#------------------------------------------------------
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configs" FORCE)
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
option(WARNINGS_ARE_ERRORS  "Treat warnings as errors"                                      OFF)
option(WARNINGS_ANSI_ISO    "Issue all the mandatory diagnostics Listed in C standard"      ON)

option(BUILD_SHARED_LIBS    "Build shared libraries"                                        ON)
option(BUILD_TESTS          "Build tests"                                                   OFF)
option(BUILD_GUI            "Build the qt application"                                      OFF)

option(HAVE_RULES           "Usage of rules (needs PCRE library and headers)"               OFF)

set(CMAKE_INCLUDE_DIRS_CONFIGCMAKE ${CMAKE_INSTALL_PREFIX}/include      CACHE PATH "Output directory for headers")
set(CMAKE_LIB_DIRS_CONFIGCMAKE     ${CMAKE_INSTALL_PREFIX}/lib          CACHE PATH "Output directory for libraries")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
