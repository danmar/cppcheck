SET(EXTRA_C_FLAGS "")
SET(EXTRA_C_FLAGS_RELEASE "-DNDEBUG")
SET(EXTRA_C_FLAGS_DEBUG "-DDEBUG -O0")

IF (USE_CLANG)
   SET (CMAKE_C_COMPILER_ID            "Clang")
   SET (CMAKE_CXX_COMPILER_ID          "Clang")
   SET (CMAKE_C_COMPILER               "/usr/bin/clang")
   SET (CMAKE_CXX_COMPILER             "/usr/bin/clang++")

   SET (CMAKE_C_FLAGS                  "-Wall -std=c99")
   SET (CMAKE_C_FLAGS_DEBUG            "-g")
   SET (CMAKE_C_FLAGS_RELEASE          "-O2")

   SET (CMAKE_CXX_FLAGS                "-Wall")
   SET (CMAKE_CXX_FLAGS_DEBUG          "-g")
   SET (CMAKE_CXX_FLAGS_RELEASE        "-O2")
ENDIF()

IF (USE_ANALYZE)
   SET (CMAKE_C_COMPILER_ID            "ccc-analyzer")
   SET (CMAKE_CXX_COMPILER_ID          "c++-analyzer")
   SET (CMAKE_C_COMPILER               "/usr/share/clang/scan-build/ccc-analyzer")
   SET (CMAKE_CXX_COMPILER             "/usr/share/clang/scan-build/c++-analyzer")

   SET (CMAKE_C_FLAGS                  "-Wall -std=c99")
   SET (CMAKE_C_FLAGS_DEBUG            "-g")
   SET (CMAKE_C_FLAGS_RELEASE          "-O2")

   SET (CMAKE_CXX_FLAGS                "-Wall")
   SET (CMAKE_CXX_FLAGS_DEBUG          "-g")
   SET (CMAKE_CXX_FLAGS_RELEASE        "-O2")
ENDIF()

IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
   execute_process(
   COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
   if (NOT (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7))
      message(FATAL_ERROR "${PROJECT_NAME} c++11 support requires g++ 4.7 or greater.")
   endif ()

   # long-long always necessary for Qt
   # -Wno-maybe-uninitialized #Simulator Antonio
   #add_extra_compiler_option(-Wno-variadic-macros) #To avoid errors in gstreamer
   SET(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wall -Werror=return-type -Wno-long-long -Wno-maybe-uninitialized")

   IF(WARNINGS_ANSI_ISO)
      # Warnings with c++11 and Release mode
      SET(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wstrict-aliasing=3")
   ELSE()
      SET(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wno-narrowing -Wno-delete-non-virtual-dtor -Wno-unnamed-type-template-args")
   ENDIF()

ELSEIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

   IF(NOT EXISTS ${CMAKE_CXX_COMPILER})
      MESSAGE( FATAL_ERROR "Clang++ not found. " )
   ENDIF()

   IF(ENABLE_COVERAGE OR ENABLE_COVERAGE_XML)
      MESSAGE(FATAL_ERROR "Not use clang for generate code coverage. Use gcc. ")
   ENDIF()

ELSEIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "c++-analyzer")

   IF(NOT EXISTS ${CMAKE_CXX_COMPILER})
      MESSAGE( FATAL_ERROR "c++-analyzer not found. " )
   ENDIF()

   IF(ENABLE_COVERAGE OR ENABLE_COVERAGE_XML)
      MESSAGE(FATAL_ERROR "Not use c++-analyzer for generate code coverage. Use gcc. ")
   ENDIF()

ENDIF()

IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "c++-analyzer" )

   IF(WARNINGS_ANSI_ISO)
      SET(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -pedantic -Wcast-align -Wextra")
   ENDIF()

   IF(WARNINGS_ARE_ERRORS)
      SET(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Werror")
   ENDIF()

   SET(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -std=c++11")

ENDIF()

INCLUDE(cmake/dynamic_analyzer_options.cmake    REQUIRED)

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
