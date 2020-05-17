if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions(-DDEBUG)
elseif(CMAKE_BUILD_TYPE MATCHES "Release" OR CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    add_definitions(-DNDEBUG)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if (CMAKE_BUILD_TYPE MATCHES "Debug")
        add_definitions(-g -O0)
    elseif(CMAKE_BUILD_TYPE MATCHES "Release" OR CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
        add_definitions(-O2)
    endif()

    if (WARNINGS_ARE_ERRORS)
        add_compile_options(-Werror)
    endif()
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.6)
        message(FATAL_ERROR "${PROJECT_NAME} c++11 support requires g++ 4.6 or greater, but it is ${CMAKE_CXX_COMPILER_VERSION}")
    endif ()

    add_compile_options(-Wcast-qual)                # Cast for removing type qualifiers
    add_compile_options(-Wno-deprecated-declarations)
    add_compile_options(-Wfloat-equal)              # Floating values used in equality comparisons
    add_compile_options(-Wmissing-declarations)     # If a global function is defined without a previous declaration
    add_compile_options(-Wmissing-format-attribute) #
    add_compile_options(-Wno-long-long)
    add_compile_options(-Woverloaded-virtual)       # when a function declaration hides virtual functions from a base class
    add_compile_options(-Wpacked)                   #
    add_compile_options(-Wredundant-decls)          # if anything is declared more than once in the same scope
    add_compile_options(-Wundef)
    add_compile_options(-Wno-shadow)                # whenever a local variable or type declaration shadows another one
    add_compile_options(-Wno-missing-field-initializers)
    add_compile_options(-Wno-missing-braces)
    add_compile_options(-Wno-sign-compare)
    add_compile_options(-Wno-multichar)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")

   add_compile_options(-Wno-deprecated-declarations)
   add_compile_options(-Wno-four-char-constants)
   add_compile_options(-Wno-missing-braces)
   add_compile_options(-Wno-missing-field-initializers)
   add_compile_options(-Wno-multichar)
   add_compile_options(-Wno-sign-compare)
   add_compile_options(-Wno-unused-function)
   if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8.0.0")
      add_compile_options(-Wextra-semi-stmt)
   endif()

   if(ENABLE_COVERAGE OR ENABLE_COVERAGE_XML)
      message(FATAL_ERROR "Not use clang for generate code coverage. Use gcc.")
   endif()
endif()

# TODO: check if this can be enabled again - also done in Makefile
if (CMAKE_SYSTEM_NAME MATCHES "Linux" AND
    CMAKE_CXX_COMPILER_ID MATCHES "Clang")

    add_compile_options(-U_GLIBCXX_DEBUG)
endif()

if (MSVC)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:8000000")
endif()

if (CYGWIN)
    # TODO: this is a linker flag - not a compiler flag
    add_compile_options(-Wl,--stack,8388608)
endif()

include(cmake/dynamic_analyzer_options.cmake)
