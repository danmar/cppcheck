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
option(ANALYZE_MEMORY       "Clang dynamic analyzer: detector of uninitialized reads."      OFF)
option(ANALYZE_ADDRESS      "Clang dynamic analyzer: fast memory error detector. "          OFF)
option(ANALYZE_THREAD       "Clang dynamic analyzer: tool that detects data races. "        OFF)
option(ANALYZE_UNDEFINED    "Clang dynamic analyzer: undefined behavior checker. "          OFF)
option(ANALYZE_DATAFLOW     "Clang dynamic analyzer: general dynamic dataflow analysis."    OFF)
option(WARNINGS_ARE_ERRORS  "Treat warnings as errors"                                      OFF)

set(USE_MATCHCOMPILER "Auto" CACHE STRING "Usage of match compiler")
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

option(BUILD_CORE_DLL       "Build lib as cppcheck-core.dll with Visual Studio"             OFF)
if (NOT MSVC)
    set(BUILD_CORE_DLL OFF)
endif()
option(BUILD_TESTS          "Build tests"                                                   OFF)
option(REGISTER_TESTS       "Register tests in CTest"                                       ON)
option(ENABLE_CHECK_INTERNAL "Enable internal checks"                                       OFF)
option(DISABLE_DMAKE        "Disable run-dmake dependencies"                                OFF)

option(BUILD_GUI            "Build the qt application"                                      OFF)
option(WITH_QCHART          "Enable QtCharts usage in the GUI"                              OFF)
option(USE_QT6              "Prefer Qt6 when available"                                     OFF)
option(REGISTER_GUI_TESTS   "Register GUI tests in CTest"                                   ON)

option(HAVE_RULES           "Usage of rules (needs PCRE library and headers)"               OFF)
option(USE_BUNDLED_TINYXML2 "Usage of bundled tinyxml2 library"                             ON)
if (BUILD_CORE_DLL)
    set(USE_BUNDLED_TINYXML2 ON)
endif()
option(CPPCHK_GLIBCXX_DEBUG "Usage of STL debug checks in Debug build"                      ON)
option(USE_THREADS          "Usage of threads instead of fork() for -j"                     OFF)
option(USE_BOOST            "Usage of Boost"                                                OFF)
option(USE_LIBCXX           "Use libc++ instead of libstdc++"                               OFF)

option(DISABLE_CRTDBG_MAP_ALLOC "Disable usage of Visual Studio C++ memory leak detection in Debug build" OFF)
option(NO_UNIX_SIGNAL_HANDLING "Disable usage of Unix Signal Handling"                      OFF)
option(NO_UNIX_BACKTRACE_SUPPORT "Disable usage of Unix Backtrace support"                  OFF)
option(NO_WINDOWS_SEH       "Disable usage of Windows SEH"                                  OFF)

if (CMAKE_VERSION VERSION_EQUAL "3.16" OR CMAKE_VERSION VERSION_GREATER "3.16")
    set(CMAKE_DISABLE_PRECOMPILE_HEADERS Off CACHE BOOL "Disable precompiled headers")
    # need to disable the prologue or it will be treated like a system header and not emit any warnings
    # see https://gitlab.kitware.com/cmake/cmake/-/issues/21219
    set(CMAKE_PCH_PROLOGUE "")
else()
    set(CMAKE_DISABLE_PRECOMPILE_HEADERS On CACHE BOOL "Disable precompiled headers")
endif()

set(CMAKE_INCLUDE_DIRS_CONFIGCMAKE ${CMAKE_INSTALL_PREFIX}/include      CACHE PATH "Output directory for headers")
set(CMAKE_LIB_DIRS_CONFIGCMAKE     ${CMAKE_INSTALL_PREFIX}/lib          CACHE PATH "Output directory for libraries")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

set(FILESDIR                       ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME} CACHE STRING "Cppcheck files directory")

