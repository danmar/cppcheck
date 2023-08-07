if (BUILD_TESTS)
    file(GLOB hdrs "*.h")
    file(GLOB srcs "*.cpp")
    list(APPEND testrunner_SOURCES ${hdrs} ${srcs} $<TARGET_OBJECTS:cli_objs>)
    if (NOT BUILD_CORE_DLL)
        list(APPEND testrunner_SOURCES $<TARGET_OBJECTS:cppcheck-core> $<TARGET_OBJECTS:simplecpp_objs>)
        if(USE_BUNDLED_TINYXML2)
            list(APPEND testrunner_SOURCES $<TARGET_OBJECTS:tinyxml2_objs>)
        endif()
    endif()

    add_executable(testrunner ${testrunner_SOURCES})
    target_include_directories(testrunner PRIVATE ${PROJECT_SOURCE_DIR}/lib/ ${PROJECT_SOURCE_DIR}/cli/)
    if(USE_BUNDLED_TINYXML2)
        target_include_directories(testrunner PRIVATE ${PROJECT_SOURCE_DIR}/externals/tinyxml2)
    else()
        target_include_directories(testrunner SYSTEM PRIVATE ${tinyxml2_INCLUDE_DIRS})
    endif()
    target_include_directories(testrunner PRIVATE ${PROJECT_SOURCE_DIR}/externals/simplecpp/)
    if (HAVE_RULES)
        target_link_libraries(testrunner ${PCRE_LIBRARY})
    endif()
    if (WIN32 AND NOT BORLAND)
        if(NOT MINGW)
            target_link_libraries(testrunner Shlwapi.lib)
        else()
            target_link_libraries(testrunner shlwapi)
        endif()
    endif()
    if(tinyxml2_FOUND AND NOT USE_BUNDLED_TINYXML2)
        target_link_libraries(testrunner ${tinyxml2_LIBRARIES})
    endif()
    target_link_libraries(testrunner ${CMAKE_THREAD_LIBS_INIT})
    if (BUILD_CORE_DLL)
        target_compile_definitions(testrunner PRIVATE CPPCHECKLIB_IMPORT SIMPLECPP_IMPORT)
        target_link_libraries(testrunner cppcheck-core)
    endif()
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # $ is used in dinit() dessignated initialization helper
        target_compile_options_safe(testrunner -Wno-dollar-in-identifier-extension)
    endif()

    if (NOT CMAKE_DISABLE_PRECOMPILE_HEADERS)
        target_precompile_headers(testrunner PRIVATE precompiled.h)
    endif()

    add_dependencies(testrunner copy_cfg)
    add_dependencies(testrunner copy_addons)
    add_dependencies(testrunner copy_platforms)
    if (NOT DISABLE_DMAKE)
        add_dependencies(testrunner run-dmake)
    endif()

    if (LIBXML2_XMLLINT_EXECUTABLE)
        # TODO: run the CMake implementation of the tests
        # TODO: get rid of the copy
        add_custom_target(checkcfg ${CMAKE_COMMAND} -E copy $<TARGET_FILE:cppcheck> ${CMAKE_SOURCE_DIR}
                COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/cfg/runtests.sh
                DEPENDS cppcheck validateCFG)
    endif()

    if (REGISTER_TESTS)
        # CMP0064 requires 3.4
        # CMAKE_MATCH_<n> usage for if (MATCHES) requires 3.9
        cmake_minimum_required(VERSION 3.9)
        cmake_policy(SET CMP0064 NEW)
        cmake_policy(SET CMP0057 NEW)

        find_package(Threads REQUIRED)
        include(ProcessorCount)
        ProcessorCount(N)
        set(CTEST_PARALLEL_LEVEL ${N} CACHE STRING "CTest parallel level")
        set(CTEST_TIMEOUT 90 CACHE STRING "CTest timeout")
        add_custom_target(check ${CMAKE_CTEST_COMMAND} --output-on-failure -j ${CTEST_PARALLEL_LEVEL} -C ${CMAKE_CFG_INTDIR} --timeout ${CTEST_TIMEOUT}
                DEPENDS testrunner cppcheck)

        set(SKIP_TESTS "" CACHE STRING "A list of tests to skip")

        function(add_fixture NAME)
            if (${NAME} IN_LIST SKIP_TESTS)
            elseif(TEST ${NAME})
            else()
                add_test(NAME ${NAME} COMMAND testrunner ${NAME} WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
            endif()
        endfunction()

        foreach(SRC ${srcs})
            file(STRINGS ${SRC} FIXTURE_LINE REGEX "^REGISTER_TEST\\([a-zA-z0-9]+\\)$")
            foreach(_fixture_line ${FIXTURE_LINE})
                if(_fixture_line MATCHES "^REGISTER_TEST\\(([a-zA-z0-9]+)\\)$")
                    add_fixture(${CMAKE_MATCH_1})
                endif()
            endforeach()
        endforeach()

        function(add_cfg CFG_TEST)
            set(oneValueArgs PLATFORM NAME)
            set(multiValueArgs ADD_LIBRARY)

            cmake_parse_arguments(PARSE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
            get_filename_component(LIBRARY ${CFG_TEST} NAME_WE)
            # TODO: get rid of this
            if(PARSE_ADD_LIBRARY)
                string(REPLACE ";" "," ADD_LIBRARY "${PARSE_ADD_LIBRARY}")
                set(LIBRARY "${ADD_LIBRARY},${LIBRARY}")
            endif()
            set(PLATFORM unix64)
            if(PARSE_PLATFORM)
                set(PLATFORM ${PARSE_PLATFORM})
            endif()
            if(PARSE_NAME)
                set(TEST_NAME ${PARSE_NAME})
            else()
                string(MAKE_C_IDENTIFIER ${CFG_TEST} TEST_NAME)
            endif()
            if ("cfg-${TEST_NAME}" IN_LIST SKIP_TESTS)
            else()
                # TODO: remove missingInclude disabling when it no longer is implied by --enable=information
                # TODO: add syntax check
                add_test(NAME cfg-${TEST_NAME}
                        COMMAND $<TARGET_FILE:cppcheck>
                        --check-library
                        --platform=${PLATFORM}
                        --library=${LIBRARY}
                        --enable=style,information
                        --inconclusive
                        --force
                        --error-exitcode=1
                        --disable=missingInclude
                        --inline-suppr
                        ${CMAKE_CURRENT_SOURCE_DIR}/cfg/${CFG_TEST}
                )
            endif()
        endfunction()
        # TODO: glob this
        add_cfg(boost.cpp)
        add_cfg(bsd.c)
        add_cfg(cairo.c)
        add_cfg(cppunit.cpp)
        # TODO: posix needs to specified first or it has a different mmap() config
        # TODO: get rid of posix dependency
        add_cfg(gnu.c ADD_LIBRARY posix)
        add_cfg(googletest.cpp)
        add_cfg(gtk.c)
        add_cfg(kde.cpp)
        add_cfg(libcurl.c)
        add_cfg(libsigc++.cpp)
        add_cfg(lua.c)
        add_cfg(opencv2.cpp)
        add_cfg(openmp.c)
        add_cfg(openssl.c)
        add_cfg(posix.c)
        add_cfg(python.c)
        add_cfg(qt.cpp)
        add_cfg(sqlite3.c)
        add_cfg(std.c)
        add_cfg(std.cpp)
        add_cfg(windows.cpp NAME windows32A PLATFORM win32A)
        add_cfg(windows.cpp NAME windows32W PLATFORM win32W)
        add_cfg(windows.cpp NAME windows64 PLATFORM win64)
        add_cfg(wxwidgets.cpp)

        function(fixture_cost NAME COST)
            if(TEST ${NAME})
                set_tests_properties(${NAME} PROPERTIES COST ${COST})
            endif()
        endfunction()

        # Set cost of the more expensive tests to help improve parallel scheduling
        # of tests
        fixture_cost(TestIO 20)
        fixture_cost(cfg-std_c 8)
        fixture_cost(TestThreadExecutor 5)
        fixture_cost(TestLeakAutoVarRecursiveCountLimit 4)
        fixture_cost(TestTokenizer 4)
    endif()
endif()
