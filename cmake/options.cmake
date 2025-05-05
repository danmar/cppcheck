# Build type
#------------------------------------------------------
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo;MinSizeRel" CACHE STRING "Configs" FORCE)
if(DEFINED CMAKE_BUILD_TYPE)
  SET_PROPERTY(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CMAKE_CONFIGURATION_TYPES})
endif()

if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type set - defaulting to Debug")
  set(CMAKE_BUILD_TYPE "Debug")
endif()

# ----------------------------------------------------------------------------
#   PROJECT CONFIGURATION
# ----------------------------------------------------------------------------
option(ANALYZE_MEMORY       "Build with MemorySanitizer to detect usage of uninitialized memory" OFF)
if(ANALYZE_MEMORY)
    message(STATUS "MemorySanitzer requires an instrumented libc++")
endif()
option(ANALYZE_ADDRESS      "Build with AddressSanitzer to detect memory error"             OFF)
option(ANALYZE_THREAD       "Build with ThreadSanitizer to detect data races"               OFF)
option(ANALYZE_UNDEFINED    "Build with UndefinedBehaviorSanitizer to detect undefined behavior" OFF)
option(ANALYZE_TYPE         "Build with TypeSanitizer to detect aliasing issues"            OFF)

option(WARNINGS_ARE_ERRORS  "Treat warnings as errors"                                      OFF)
if(WARNINGS_ARE_ERRORS)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24")
        message(WARNING "WARNINGS_ARE_ERRORS is deprecated - please use CMAKE_COMPILE_WARNING_AS_ERROR instead")
    endif()
    set(CMAKE_COMPILE_WARNING_AS_ERROR On)
endif()
option(EXTERNALS_AS_SYSTEM  "Treat externals as system includes"                            OFF)

set(USE_MATCHCOMPILER "Auto" CACHE STRING "Usage of match compiler")
set_property(CACHE USE_MATCHCOMPILER PROPERTY STRINGS Auto Off On Verify)
if(USE_MATCHCOMPILER)
    if(USE_MATCHCOMPILER STREQUAL "Auto")
        if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
            message(STATUS "Non-debug build detected - enabling matchcompiler")
            set(USE_MATCHCOMPILER_OPT "On")
        else()
            message(STATUS "Debug build detected - disabling matchcompiler")
            set(USE_MATCHCOMPILER_OPT "Off")
        endif()
    elseif(USE_MATCHCOMPILER STREQUAL "Verify")
        set(USE_MATCHCOMPILER_OPT "Verify")
    else()
        set(USE_MATCHCOMPILER_OPT "On")
    endif()
else()
    set(USE_MATCHCOMPILER_OPT "Off")
endif()

option(BUILD_CORE_DLL       "Build lib as cppcheck-core.dll with Visual Studio"             OFF)
if(BUILD_CORE_DLL AND NOT MSVC)
    message(FATAL_ERROR "Building of lib as DLL is only supported with Visual Studio")
endif()
option(BUILD_TESTS          "Build tests"                                                   OFF)
option(REGISTER_TESTS       "Register tests in CTest"                                       ON)
option(ENABLE_CHECK_INTERNAL "Enable internal checks"                                       OFF)
option(DISABLE_DMAKE        "Disable run-dmake dependencies"                                OFF)
option(BUILD_MANPAGE        "Enable man target to build manpage"                            OFF)

option(BUILD_CLI            "Build the cli application"                                     ON)

option(BUILD_GUI            "Build the qt application"                                      OFF)
option(WITH_QCHART          "Enable QtCharts usage in the GUI"                              OFF)
option(USE_QT6              "Prefer Qt6 when available"                                     OFF)
option(REGISTER_GUI_TESTS   "Register GUI tests in CTest"                                   ON)
option(BUILD_ONLINE_HELP    "Build online help"                                             OFF)
option(BUILD_TRIAGE         "Build triage UI tool"                                          OFF)

if(NOT BUILD_GUI)
    if(BUILD_ONLINE_HELP)
        message(FATAL_ERROR "BUILD_ONLINE_HELP requires BUILD_GUI to be enabled")
    endif()
    if(BUILD_TRIAGE)
        message(FATAL_ERROR "BUILD_TRIAGE requires BUILD_GUI to be enabled")
    endif()
endif()

option(HAVE_RULES           "Usage of rules (needs PCRE library and headers)"               OFF)
option(USE_BUNDLED_TINYXML2 "Usage of bundled TinyXML2 library"                             ON)
if(BUILD_CORE_DLL AND NOT USE_BUNDLED_TINYXML2)
    message(FATAL_ERROR "Cannot use external TinyXML2 library when building lib as DLL")
endif()
option(CPPCHK_GLIBCXX_DEBUG "Usage of STL debug checks in Debug build"                      ON)
option(DISALLOW_THREAD_EXECUTOR "Disallow usage of ThreadExecutor for -j"                   OFF)
if(DISALLOW_THREAD_EXECUTOR AND WIN32)
    message(FATAL_ERROR "Cannot disable usage of ThreadExecutor on Windows as no other executor implementation is currently available")
endif()
option(USE_BOOST            "Force usage of Boost"                                          OFF)
option(USE_BOOST_INT128     "Usage of Boost.Multiprecision 128-bit integer for Mathlib"     OFF)
if (NOT USE_BOOST AND USE_BOOST_INT128)
    message(FATAL_ERROR "USE_BOOST_INT128 requires USE_BOOST to be enabled")
endif()
option(USE_LIBCXX           "Use libc++ instead of libstdc++"                               OFF)

option(DISABLE_CRTDBG_MAP_ALLOC "Disable usage of Visual Studio C++ memory leak detection in Debug build" OFF)
option(NO_UNIX_SIGNAL_HANDLING "Disable usage of Unix Signal Handling"                      OFF)
option(NO_UNIX_BACKTRACE_SUPPORT "Disable usage of Unix Backtrace support"                  OFF)
option(NO_WINDOWS_SEH       "Disable usage of Windows SEH"                                  OFF)

option(ENABLE_CSA_ALPHA     "Enable Clang Static Analyzer alpha checkers for run-clang-tidy-csa target"  OFF)

# TODO: disable by default like make build?
option(FILESDIR "Hard-coded directory for files to load from"                               OFF)

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
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

string(LENGTH "${FILESDIR}" _filesdir_len)
# override FILESDIR if it is set or empty
if(FILESDIR OR ${_filesdir_len} EQUAL 0)
# TODO: verify that it is an abolute path?
    set(FILESDIR_DEF                   ${FILESDIR})
else()
    set(FILESDIR_DEF                   ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME} CACHE STRING "Cppcheck files directory")
endif()
