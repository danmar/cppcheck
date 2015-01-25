if (BUILD_TESTS)

    include_directories(
        ${PROJECT_SOURCE_DIR}/lib/
        ${PROJECT_SOURCE_DIR}/cli/
        ${PROJECT_SOURCE_DIR}/externals/tinyxml
    )

    file(GLOB hdrs "*.h")
    file(GLOB srcs "*.cpp")

    link_libraries(${PROJECT_NAME} tinyxml)

    add_executable(testrunner ${hdrs} ${srcs} 
        $<TARGET_OBJECTS:cli_lib>
    )

endif()
