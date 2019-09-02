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
    add_dependencies(check testrunner)

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

    # Set cost of the more expensive tests to help improve parallel scheduling
    # of tests
    fixture_cost(TestIO 20)
    fixture_cost(TestThreadExecutor 5)
    fixture_cost(TestLeakAutoVar 4)
    fixture_cost(TestTokenizer 4)

endif()
