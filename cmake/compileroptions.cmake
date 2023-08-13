include(CheckCXXCompilerFlag)

function(add_compile_options_safe FLAG)
    string(MAKE_C_IDENTIFIER "HAS_CXX_FLAG${FLAG}" mangled_flag)
    check_cxx_compiler_flag(${FLAG} ${mangled_flag})
    if (${mangled_flag})
        add_compile_options(${FLAG})
    endif()
endfunction()
    
function(target_compile_options_safe TARGET FLAG)
    string(MAKE_C_IDENTIFIER "HAS_CXX_FLAG${FLAG}" mangled_flag)
    check_cxx_compiler_flag(${FLAG} ${mangled_flag})
    if (${mangled_flag})
        target_compile_options(${TARGET} PRIVATE ${FLAG})
    endif()
endfunction()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Weverything)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        # "Release" uses -O3 by default
        add_compile_options(-O2)
    endif()
    if (WARNINGS_ARE_ERRORS)
        add_compile_options(-Werror)
    endif()
    add_compile_options(-pedantic)
    add_compile_options(-Wall)
    add_compile_options(-Wextra)
    add_compile_options(-Wcast-qual)                # Cast for removing type qualifiers
    add_compile_options(-Wfloat-equal)              # Floating values used in equality comparisons
    add_compile_options(-Wmissing-declarations)     # If a global function is defined without a previous declaration
    add_compile_options(-Wmissing-format-attribute) #
    add_compile_options(-Wno-long-long)
    add_compile_options(-Wpacked)                   #
    add_compile_options(-Wredundant-decls)          # if anything is declared more than once in the same scope
    add_compile_options(-Wundef)
    add_compile_options(-Wno-missing-field-initializers)
    add_compile_options(-Wno-missing-braces)
    add_compile_options(-Wno-sign-compare)
    add_compile_options(-Wno-multichar)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # use pipes instead of temporary files - greatly reduces I/O usage
    add_compile_options(-pipe)

    add_compile_options(-Woverloaded-virtual)       # when a function declaration hides virtual functions from a base class
    add_compile_options(-Wno-maybe-uninitialized)   # there are some false positives
    add_compile_options(-Wsuggest-attribute=noreturn)
    add_compile_options(-Wno-shadow)                # whenever a local variable or type declaration shadows another one
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 14 OR CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 14)
        # TODO: verify this regression still exists in clang-15
        if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
            # work around performance regression - see https://github.com/llvm/llvm-project/issues/53555
            check_cxx_compiler_flag("-mllvm -inline-deferral" _has_mllvm_inline_deferral)
            if (_has_mllvm_inline_deferral)
                add_compile_options(-mllvm -inline-deferral)
            endif()
        endif()

        # use force DWARF 4 debug format since not all tools might be able to handle DWARF 5 yet - e.g. valgrind on ubuntu 20.04
        add_compile_options(-gdwarf-4)
    endif()

    if (USE_LIBCXX)
        add_compile_options(-stdlib=libc++)
        add_link_options(-lc++)
    endif()

   # TODO: fix and enable these warnings - or move to suppression list below
    add_compile_options_safe(-Wno-documentation-unknown-command) # TODO: Clang currently does not support all commands
   add_compile_options_safe(-Wno-inconsistent-missing-destructor-override) # caused by Qt moc code
   add_compile_options_safe(-Wno-unused-exception-parameter)
   add_compile_options_safe(-Wno-old-style-cast)
   add_compile_options_safe(-Wno-sign-conversion)
   add_compile_options_safe(-Wno-shadow-field-in-constructor)
   add_compile_options_safe(-Wno-covered-switch-default)
   add_compile_options_safe(-Wno-shorten-64-to-32)
   add_compile_options_safe(-Wno-zero-as-null-pointer-constant) # TODO: enable when warnings are fixed in in simplecpp and tinyxml2
   add_compile_options_safe(-Wno-implicit-int-conversion)
   add_compile_options_safe(-Wno-double-promotion)
   add_compile_options_safe(-Wno-shadow-field)
   add_compile_options_safe(-Wno-shadow-uncaptured-local)
   add_compile_options_safe(-Wno-implicit-float-conversion)
   add_compile_options_safe(-Wno-switch-enum)
   add_compile_options_safe(-Wno-float-conversion)
   add_compile_options_safe(-Wno-enum-enum-conversion)
   add_compile_options_safe(-Wno-date-time)
   add_compile_options_safe(-Wno-suggest-override) # TODO: enable when warnings are fixed in in tinyxml2
   add_compile_options_safe(-Wno-suggest-destructor-override) # TODO: enable when warnings are fixed in in tinyxml2
   add_compile_options_safe(-Wno-extra-semi-stmt) # TODO: enable when warnings are fixed in in tinyxml2
   add_compile_options(-Wno-disabled-macro-expansion)
   add_compile_options_safe(-Wno-bitwise-instead-of-logical)

   # these cannot be fixed properly without adopting later C++ standards
   add_compile_options_safe(-Wno-unsafe-buffer-usage)
   add_compile_options_safe(-Wno-global-constructors)
   add_compile_options_safe(-Wno-exit-time-destructors)

   # can only be partially addressed
   add_compile_options(-Wno-padded)

   # no need for C++98 compatibility
   add_compile_options(-Wno-c++98-compat)
   add_compile_options(-Wno-c++98-compat-pedantic)

   # only need to be addressed to work around issues in older compilers
   add_compile_options_safe(-Wno-return-std-move-in-c++11)

    # warnings we are currently not interested in
   add_compile_options(-Wno-four-char-constants)
   add_compile_options(-Wno-weak-vtables)

   if(ENABLE_COVERAGE OR ENABLE_COVERAGE_XML)
      message(FATAL_ERROR "Do not use clang to generate code coverage. Use GCC instead.")
   endif()
endif()

if (MSVC)
    # General
    add_compile_options(/W4) # Warning Level
    add_compile_options(/Zi) # Debug Information Format - Program Database
    if (WARNINGS_ARE_ERRORS)
        add_compile_options(/WX) # Treat Warning As Errors
    endif()
    add_compile_options(/MP) # Multi-processor Compilation

    # Advanced
    # Character Set - Use Unicode Character Set
    # No Whole Program Optimization

    # C/C++ - Optimization
    add_compile_options($<$<NOT:$<CONFIG:Debug>>:/O2>) # Optimization - Maximum Optimization (Favor Speed)
    add_compile_options($<$<NOT:$<CONFIG:Debug>>:/Ob2>) # Inline Function Expansion - Any Suitable
    add_compile_options($<$<NOT:$<CONFIG:Debug>>:/Oi>) # Enable Intrinsic Functions
    add_compile_options($<$<NOT:$<CONFIG:Debug>>:/Ot>) # Favor fast code
    add_compile_options($<$<NOT:$<CONFIG:Debug>>:/Oy>) # Omit Frame Pointers
    add_compile_options($<$<CONFIG:Debug>:/Od>) # Optimization - Disabled

    # C/C++ - Code Generation
    add_compile_options($<$<NOT:$<CONFIG:Debug>>:/GF>) # Enable String Pooling
    add_compile_options($<$<NOT:$<CONFIG:Debug>>:/MD>) # Runtime Library - Multi-threaded DLL
    add_compile_options($<$<NOT:$<CONFIG:Debug>>:/GS->) # Disable Security Check
    add_compile_options($<$<NOT:$<CONFIG:Debug>>:/Gy>) # Enable Function-Level Linking
    add_compile_options($<$<CONFIG:Debug>:/MDd>) # Runtime Library - Multi-threaded Debug DLL
    add_compile_options($<$<CONFIG:Debug>:/GS>) # Enable Security Check

    # C/C++ - Language
    add_compile_options(/Zc:rvalueCast) # Enforce type conversion rules
    #add_compile_options(/std:c++14) # C++ Language Standard - ISO C++14 Standard

    # C/C++ - Browse Information
    # Enable Browse Information - No

    # C/C++ - Advanced
    add_compile_options(/wd4018) # warning C4018: '>': signed/unsigned mismatch
    add_compile_options(/wd4127) # warning C4127: conditional expression is constant
    add_compile_options(/wd4146) # warning C4146: unary minus operator applied to unsigned type, result still unsigned
    add_compile_options(/wd4244) # warning C4244: 'initializing': conversion from 'int' to 'char', possible loss of data
    add_compile_options(/wd4251)
    # Clang: -Wshorten-64-to-32 -Wimplicit-int-conversion
    add_compile_options(/wd4267) # warning C4267: 'return': conversion from 'size_t' to 'int', possible loss of data
    add_compile_options(/wd4389) # warning C4389: '==': signed/unsigned mismatch
    add_compile_options(/wd4482)
    add_compile_options(/wd4512)
    add_compile_options(/wd4701) # warning C4701: potentially uninitialized local variable 'err' used
    add_compile_options(/wd4706) # warning C4706: assignment within conditional expression
    add_compile_options(/wd4800) # warning C4800: 'const SymbolDatabase *' : forcing value to bool 'true' or 'false' (performance warning)
    add_compile_options(/wd4805) # warning C4805: '==' : unsafe mix of type 'bool' and type 'long long' in operation

    # C/C++ - All Options
    add_compile_options(/Zc:throwingNew /Zc:__cplusplus) # Additional Options

    # Linker - General
    add_link_options($<$<CONFIG:Debug>:/INCREMENTAL>) # Enable Incremental Linking - Yes

    add_link_options(/NOLOGO) # SUppress Startup Banner - Yes
    # Ignore Import Library - Yes

    # Linker - Debugging
    add_link_options(/DEBUG) # Generate Debug Information

    # Linker - System
    # Stack Reserve Size - 8000000
    # Stack Commit Size - 8000000
    add_link_options(/LARGEADDRESSAWARE) # Enable Large Addresses - Yes

    # Linker - Optimization
    add_link_options(/OPT:REF) # References - Yes
    add_link_options(/OPT:ICF) # Enable COMDAT Folding - Yes

    # Linker - Advanced
    add_link_options($<$<NOT:$<CONFIG:Debug>>:/RELEASE>) # Set Checksum - Yes
endif()

# TODO: check if this can be enabled again - also done in Makefile
if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND
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
