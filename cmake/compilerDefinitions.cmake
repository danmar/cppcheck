if (MSVC)
    # Visual Studio only sets _DEBUG
    if (CMAKE_BUILD_TYPE MATCHES "Debug")
        add_definitions(-DDEBUG)
    endif()

    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if (UNIX)
    # TODO: check if this can be enabled again for Clang - also done in Makefile
    if (CMAKE_BUILD_TYPE MATCHES "Debug" AND NOT (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
        add_definitions(-D_GLIBCXX_DEBUG)
    endif()    
endif()

if (HAVE_RULES)
    add_definitions(-DHAVE_RULES -DTIXML_USE_STL)
endif()

if (USE_Z3)
    add_definitions(-DUSE_Z3)
endif()

if (ENABLE_CHECK_INTERNAL)
    add_definitions(-DCHECK_INTERNAL)
endif()

file(TO_CMAKE_PATH ${FILESDIR} _filesdir)
add_definitions(-DFILESDIR="${_filesdir}")
