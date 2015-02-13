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

    add_custom_target(copy_cfg ALL
        COMMENT "Copying cfg files")
    add_custom_command(
        TARGET copy_cfg
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/cfg"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/cfg")

    add_dependencies(testrunner copy_cfg)

endif()
