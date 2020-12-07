if (BUILD_TESTS)
    include_directories(${PROJECT_SOURCE_DIR}/lib/ ${PROJECT_SOURCE_DIR}/cli/)
    if(USE_BUNDLED_TINYXML2)
        include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/externals/tinyxml2)
    endif()
    include_directories(${PROJECT_SOURCE_DIR}/externals/simplecpp/)

    file(GLOB hdrs "*.h")
    file(GLOB srcs "*.cpp")
    list(APPEND testrunner_SOURCES ${hdrs} ${srcs} $<TARGET_OBJECTS:lib_objs> $<TARGET_OBJECTS:cli_objs> $<TARGET_OBJECTS:simplecpp_objs>)
    if(USE_BUNDLED_TINYXML2)
        list(APPEND testrunner_SOURCES $<TARGET_OBJECTS:tinyxml2_objs>)
    endif()

    add_executable(testrunner ${testrunner_SOURCES})
    if (HAVE_RULES)
        target_link_libraries(testrunner ${PCRE_LIBRARY})
    endif()
    if (USE_Z3)
        target_link_libraries(testrunner ${Z3_LIBRARIES})
    endif()
    if (WIN32 AND NOT BORLAND)
        target_link_libraries(testrunner Shlwapi.lib)
    endif()
    if(tinyxml2_FOUND AND NOT USE_BUNDLED_TINYXML2)
        target_link_libraries(testrunner tinyxml2)
    endif()

    if (NOT CMAKE_DISABLE_PRECOMPILE_HEADERS)
        target_precompile_headers(testrunner PRIVATE precompiled.h)
    endif()

    add_custom_target(copy_cfg ALL
        ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/cfg"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/cfg"
        COMMENT "Copying cfg files")
    add_dependencies(testrunner copy_cfg)

    if (LIBXML2_XMLLINT_EXECUTABLE)
        # TODO: get rid of the copy
        add_custom_target(checkcfg ${CMAKE_COMMAND} -E copy $<TARGET_FILE:cppcheck> ${CMAKE_SOURCE_DIR}
                COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/cfg/runtests.sh
                DEPENDS cppcheck validateCFG
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cfg)
    endif()

    if (REGISTER_TESTS)
        cmake_minimum_required(VERSION 3.4) # cmake_policy(SET CMP0064 NEW)
        cmake_policy(SET CMP0064 NEW)
        cmake_policy(SET CMP0057 NEW)

        include(CTest)

        find_package(Threads REQUIRED)
        include(ProcessorCount)
        ProcessorCount(N)
        set(CTEST_PARALLEL_LEVEL ${N} CACHE STRING "CTest parallel level")
        add_custom_target(check ${CMAKE_CTEST_COMMAND} --output-on-failure -j ${CTEST_PARALLEL_LEVEL} -C ${CMAKE_CFG_INTDIR} --timeout 90
                DEPENDS testrunner cppcheck)

        set(SKIP_TESTS "" CACHE STRING "A list of tests to skip")

        function(add_fixture NAME)
            set(options)
            set(oneValueArgs WORKING_DIRECTORY)
            set(multiValueArgs)

            cmake_parse_arguments(PARSE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

            if (${NAME} IN_LIST SKIP_TESTS)
            elseif(TEST ${NAME})
            else()
                set(WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
                if (PARSE_WORKING_DIRECTORY)
                    set(WORKING_DIRECTORY ${PARSE_WORKING_DIRECTORY})
                endif()
                add_test(NAME ${NAME} COMMAND testrunner ${NAME} WORKING_DIRECTORY ${WORKING_DIRECTORY})
            endif()
        endfunction()

        function(fixture_cost NAME COST)
            if(TEST ${NAME})
                set_tests_properties(${NAME} PROPERTIES COST ${COST})
            endif()
        endfunction()

        # TODO: what is this?
        add_fixture(TestSamples WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
        foreach(SRC ${srcs})
            file(STRINGS ${SRC} FIXTURE_LINE REGEX "TestFixture\\(" LIMIT_COUNT 1)
            if(FIXTURE_LINE MATCHES "TestFixture\\(\"([a-zA-z0-9]+)\"\\)")
                add_fixture(${CMAKE_MATCH_1})
            endif()
        endforeach()
        # TODO: parse files for REGISTER_TEST macro
        add_fixture(TestLeakAutoVarStrcpy)
        add_fixture(TestLeakAutoVarWindows)
        add_fixture(TestMemleakInFunction)
        add_fixture(TestMemleakInClass)
        add_fixture(TestMemleakStructMember)
        add_fixture(TestMemleakNoVar)

        function(add_cfg CFG_TEST)
            set(options INCONCLUSIVE)
            set(oneValueArgs PLATFORM NAME)
            set(multiValueArgs LIBRARY)

            cmake_parse_arguments(PARSE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
            if(PARSE_LIBRARY)
                string(REPLACE ";" "," LIBRARY "${PARSE_LIBRARY}")
            else()
                get_filename_component(LIBRARY ${CFG_TEST} NAME_WE)
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
            set(INCONCLUSIVE)
            if(PARSE_INCONCLUSIVE)
                set(INCONCLUSIVE "--inconclusive")
            endif()
            if (${TEST_NAME} IN_LIST SKIP_TESTS)
            else()
                # TODO: add syntax check
                add_test(NAME cfg-${TEST_NAME}
                        COMMAND $<TARGET_FILE:cppcheck>
                        --check-library
                        --platform=${PLATFORM}
                        --library=${LIBRARY}
                        --enable=information
                        --enable=style
                        --error-exitcode=1
                        --suppress=missingIncludeSystem
                        --inline-suppr
                        --template="{file}:{line}:{severity}:{id}:{message}"
                        ${INCONCLUSIVE}
                        ${CMAKE_CURRENT_SOURCE_DIR}/cfg/${CFG_TEST}
                )
            endif()
        endfunction()
        add_cfg(posix.c)
        add_cfg(gnu.c LIBRARY posix;gnu)
        add_cfg(qt.cpp INCONCLUSIVE)
        add_cfg(bsd.c)
        add_cfg(std.c INCONCLUSIVE)
        add_cfg(std.cpp INCONCLUSIVE)
        add_cfg(windows.cpp INCONCLUSIVE NAME windows32A PLATFORM win32A)
        add_cfg(windows.cpp INCONCLUSIVE NAME windows32W PLATFORM win32W)
        add_cfg(windows.cpp INCONCLUSIVE NAME windows64 PLATFORM win64)
        add_cfg(wxwidgets.cpp INCONCLUSIVE)
        add_cfg(gtk.c INCONCLUSIVE)
        add_cfg(boost.cpp INCONCLUSIVE)
        add_cfg(sqlite3.c INCONCLUSIVE)
        add_cfg(openmp.c)
        add_cfg(python.c)
        add_cfg(lua.c)
        add_cfg(libcurl.c)
        add_cfg(cairo.c)
        add_cfg(googletest.cpp INCONCLUSIVE)
        add_cfg(kde.cpp INCONCLUSIVE)
        add_cfg(libsigc++.cpp)
        add_cfg(openssl.c)
        add_cfg(opencv2.cpp)

        # Set cost of the more expensive tests to help improve parallel scheduling
        # of tests
        fixture_cost(TestIO 20)
        fixture_cost(cfg-std_c 8)
        fixture_cost(TestThreadExecutor 5)
        fixture_cost(TestLeakAutoVar 4)
        fixture_cost(TestTokenizer 4)
    endif()
endif()
