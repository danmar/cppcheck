if(MSVC)
    # add_compile_definitions() requires CMake 3.12

    # Visual Studio only sets _DEBUG
    add_compile_definitions($<$<CONFIG:Debug>:DEBUG>)

    add_definitions(-DWIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DWIN32_LEAN_MEAN)
    add_definitions(-D_WIN64)
endif()

# TODO: this should probably apply to the compiler and not the platform - I think it is only "broken" with MinGW
# TODO: AppleClang only has libc++
# TODO: what about clang-cl and native Win32 clang?
if(CPPCHK_GLIBCXX_DEBUG AND UNIX AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if(USE_LIBCXX)
            if(CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 18 OR CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 18)
                add_definitions(-D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG)
            else()
                add_definitions(-D_LIBCPP_ENABLE_ASSERTIONS=1)
            endif()
            # TODO: also add _LIBCPP_ENABLE_THREAD_SAFETY_ANNOTATIONS?
        endif()
    else()
        # TODO: check if this can be enabled again for Clang - also done in Makefile
        add_definitions(-D_GLIBCXX_DEBUG)
    endif()
endif()

if(HAVE_RULES)
    add_definitions(-DHAVE_RULES)
endif()

if(Boost_FOUND)
    add_definitions(-DHAVE_BOOST)
    if(USE_BOOST_INT128)
        add_definitions(-DHAVE_BOOST_INT128)
    endif()
endif()

if(ENABLE_CHECK_INTERNAL)
    add_definitions(-DCHECK_INTERNAL)
endif()

if(DISALLOW_THREAD_EXECUTOR)
    add_definitions(-DDISALLOW_THREAD_EXECUTOR)
endif()

if(MSVC AND DISABLE_CRTDBG_MAP_ALLOC)
    add_definitions(-DDISABLE_CRTDBG_MAP_ALLOC)
endif()

if(NO_UNIX_SIGNAL_HANDLING)
    add_definitions(-DNO_UNIX_SIGNAL_HANDLING)
endif()

if(NO_UNIX_BACKTRACE_SUPPORT)
    add_definitions(-DNO_UNIX_BACKTRACE_SUPPORT)
endif()

if(NO_WINDOWS_SEH)
    add_definitions(-DNO_WINDOWS_SEH)
endif()

if(FILESDIR_DEF)
    file(TO_CMAKE_PATH "${FILESDIR_DEF}" _filesdir)
    add_definitions(-DFILESDIR="${_filesdir}")
endif()
