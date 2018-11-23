set(EXTRA_C_FLAGS "")
set(EXTRA_C_FLAGS_RELEASE "-DNDEBUG")
set(EXTRA_C_FLAGS_DEBUG "-DDEBUG")

if (USE_CLANG)
    set (CMAKE_C_COMPILER_ID            "Clang")
    set (CMAKE_CXX_COMPILER_ID          "Clang")
    set (CMAKE_C_COMPILER               "/usr/bin/clang")
    set (CMAKE_CXX_COMPILER             "/usr/bin/clang++")

    set (CMAKE_C_FLAGS                  "-std=c99")
    set (CMAKE_C_FLAGS_DEBUG            "-g")
    set (CMAKE_C_FLAGS_RELEASE          "-O2")

    set (CMAKE_CXX_FLAGS                "")
    set (CMAKE_CXX_FLAGS_DEBUG          "-g")
    set (CMAKE_CXX_FLAGS_RELEASE        "-O2")
endif()

if (USE_ANALYZE)
    set (CMAKE_C_COMPILER_ID            "ccc-analyzer")
    set (CMAKE_CXX_COMPILER_ID          "c++-analyzer")
    set (CMAKE_C_COMPILER               "/usr/share/clang/scan-build/ccc-analyzer")
    set (CMAKE_CXX_COMPILER             "/usr/share/clang/scan-build/c++-analyzer")

    set (CMAKE_C_FLAGS                  "-Wall -std=c99")
    set (CMAKE_C_FLAGS_DEBUG            "-g")
    set (CMAKE_C_FLAGS_RELEASE          "-O2")

    set (CMAKE_CXX_FLAGS                "-Wall")
    set (CMAKE_CXX_FLAGS_DEBUG          "-g")
    set (CMAKE_CXX_FLAGS_RELEASE        "-O2")
endif()

set(CMAKE_CXX_FLAGS_ASAN "-g -fsanitize=address,undefined -fno-sanitize-recover=all"
    CACHE STRING "Compiler flags in asan build"
    FORCE)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
    if (NOT (GCC_VERSION VERSION_GREATER 4.6 OR GCC_VERSION VERSION_EQUAL 4.6))
        message(FATAL_ERROR "${PROJECT_NAME} c++11 support requires g++ 4.6 or greater, but it is ${GCC_VERSION}")
    endif ()

    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wabi")
    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wcast-qual")                # Cast for removing type qualifiers
    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wconversion")               # Implicit conversions that may alter a value
    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wfloat-equal")              # Floating values used in equality comparisons
    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Winline")                   # If a inline declared function couldn't be inlined
    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wmissing-declarations")     # If a global function is defined without a previous declaration
    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wmissing-format-attribute") # 
    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Woverloaded-virtual")       # when a function declaration hides virtual functions from a base class
    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wpacked")                   # 
    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wredundant-decls")          # if anything is declared more than once in the same scope
    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wno-shadow")                # whenever a local variable or type declaration shadows another one
    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wsign-promo")               # 
    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wno-missing-field-initializers")
    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wno-missing-braces")
    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wno-sign-compare")

    if(WARNINGS_ANSI_ISO)
#        set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Werror=return-type")        # 
#        set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wstrict-aliasing=3")
    else()
        set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wno-narrowing")
        set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wno-delete-non-virtual-dtor")
        set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wno-unnamed-type-template-args")
    endif()

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

   if(NOT EXISTS ${CMAKE_CXX_COMPILER})
      MESSAGE( FATAL_ERROR "Clang++ not found. " )
   endif()

   set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wno-four-char-constants")
   set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wno-missing-braces")
   set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wno-missing-field-initializers")
   set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wno-multichar")
   set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wno-sign-compare")
   set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wno-unused-function")

   if(ENABLE_COVERAGE OR ENABLE_COVERAGE_XML)
      MESSAGE(FATAL_ERROR "Not use clang for generate code coverage. Use gcc. ")
   endif()

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "c++-analyzer")

   if(NOT EXISTS ${CMAKE_CXX_COMPILER})
      MESSAGE( FATAL_ERROR "c++-analyzer not found. " )
   endif()

   if(ENABLE_COVERAGE)
      MESSAGE(FATAL_ERROR "Not use c++-analyzer for generate code coverage. Use gcc. ")
   endif()

endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "c++-analyzer" )

   if(WARNINGS_ANSI_ISO)
           set(EXTRA_C_FLAGS "-Wextra -pedantic ${EXTRA_C_FLAGS}")
#           set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wlogical-op")
           set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wno-long-long") # Don't warn about long long usage.
   endif()

   if(WARNINGS_ARE_ERRORS)
      set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Werror")
   endif()

   set(EXTRA_C_FLAGS "-Wall -std=c++0x ${EXTRA_C_FLAGS}")
   
   set(EXTRA_C_FLAGS_DEBUG "${EXTRA_C_FLAGS_DEBUG} -O0")

endif()

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux" AND
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -U_GLIBCXX_DEBUG")
endif()

include(cmake/dynamic_analyzer_options.cmake    REQUIRED)

# Add user supplied extra options (optimization, etc...)
# ==========================================================
set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS}" CACHE INTERNAL "Extra compiler options")
set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE}" CACHE INTERNAL "Extra compiler options for Release build")
set(EXTRA_C_FLAGS_DEBUG "${EXTRA_C_FLAGS_DEBUG}" CACHE INTERNAL "Extra compiler options for Debug build")

#combine all "extra" options
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_C_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${EXTRA_C_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${EXTRA_C_FLAGS_RELEASE}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_C_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}  ${EXTRA_C_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${EXTRA_C_FLAGS_DEBUG}")
