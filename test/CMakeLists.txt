if (BUILD_TESTS)

    cmake_policy(SET CMP0064 NEW)
    cmake_policy(SET CMP0057 NEW)

    include(CTest)

    find_package(Threads REQUIRED)
    include(ProcessorCount)
    ProcessorCount(N)
    set(CTEST_PARALLEL_LEVEL ${N} CACHE STRING "CTest parallel level")
    add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -j ${CTEST_PARALLEL_LEVEL} -C ${CMAKE_CFG_INTDIR} --timeout 90)

    include_directories(${PROJECT_SOURCE_DIR}/lib/ ${PROJECT_SOURCE_DIR}/cli/)
    include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/externals/tinyxml)
    include_directories(${PROJECT_SOURCE_DIR}/externals/simplecpp/)

    file(GLOB hdrs "*.h")
    file(GLOB srcs "*.cpp")

    add_executable(testrunner ${hdrs} ${srcs} $<TARGET_OBJECTS:lib_objs> $<TARGET_OBJECTS:cli_objs> $<TARGET_OBJECTS:tinyxml_objs> $<TARGET_OBJECTS:simplecpp_objs>)
    if (HAVE_RULES)
        target_link_libraries(testrunner pcre)
    endif()

    add_custom_target(copy_cfg ALL
        COMMENT "Copying cfg files")
    add_custom_command(
        TARGET copy_cfg
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/cfg"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/cfg")

    if (WIN32 AND NOT BORLAND)
        target_link_libraries(testrunner Shlwapi.lib)
    endif()

    add_dependencies(testrunner copy_cfg)
    add_dependencies(check testrunner cppcheck)

    set(SKIP_TESTS "" CACHE STRING "A list of tests to skip")

    function(add_fixture NAME)
        if (${NAME} IN_LIST SKIP_TESTS)
        else()
            add_test(NAME ${NAME} COMMAND testrunner ${NAME} WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
        endif()
    endfunction()

    function(fixture_cost NAME COST)
        if(TEST ${NAME})
            set_tests_properties(${NAME} PROPERTIES COST ${COST})
        endif()
    endfunction()

    foreach(SRC ${srcs})
        file(STRINGS ${SRC} FIXTURE_LINE REGEX "TestFixture\\(" LIMIT_COUNT 1)
        if(FIXTURE_LINE MATCHES "TestFixture\\(\"([a-zA-z0-9]+)\"\\)")
            add_fixture(${CMAKE_MATCH_1})
        endif()
    endforeach()

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
                    ${INCONCLUSIVE}
                    ${CMAKE_CURRENT_SOURCE_DIR}/cfg/${CFG_TEST}
            )
        endif()
    endfunction()
    add_cfg(boost.cpp INCONCLUSIVE)
    add_cfg(bsd.c)
    add_cfg(gnu.c LIBRARY posix gnu)
    add_cfg(googletest.cpp)
    add_cfg(gtk.c)
    add_cfg(libcurl.c)
    add_cfg(lua.c)
    add_cfg(openmp.c)
    add_cfg(posix.c)
    add_cfg(python.c)
    add_cfg(qt.cpp)
    add_cfg(sqlite3.c INCONCLUSIVE)
    add_cfg(std.c INCONCLUSIVE)
    add_cfg(std.cpp INCONCLUSIVE)
    add_cfg(windows.cpp INCONCLUSIVE NAME windows32A PLATFORM win32A)
    add_cfg(windows.cpp INCONCLUSIVE NAME windows32W PLATFORM win32W)
    add_cfg(windows.cpp INCONCLUSIVE NAME windows64 PLATFORM win64)
    add_cfg(wxwidgets.cpp INCONCLUSIVE)

    # Set cost of the more expensive tests to help improve parallel scheduling
    # of tests
    fixture_cost(TestIO 20)
    fixture_cost(cfg-std_c 8)
    fixture_cost(TestThreadExecutor 5)
    fixture_cost(TestLeakAutoVar 4)
    fixture_cost(TestTokenizer 4)
endif()
