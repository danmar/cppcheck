include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/externals/tinyxml/)

if (${CMAKE_BUILD_TYPE} STREQUAL Release)
    # Process input files with the matchcompiler tool
    execute_process(
        COMMAND python tools/matchcompiler.py --build-directory ${PROJECT_BINARY_DIR}/libMatchCompiler
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )
    SET(SOURCES_PATH ${PROJECT_BINARY_DIR}/libMatchCompiler/)
else()
    SET(SOURCES_PATH ${CMAKE_CURRENT_SOURCE_DIR})
endif()

file(GLOB_RECURSE hdrs "${SOURCES_PATH}/*.h")
file(GLOB_RECURSE srcs "${SOURCES_PATH}/*.cpp")

add_library(${PROJECT_NAME} ${srcs} ${hdrs})

if (HAVE_RULES)
    target_link_libraries(${PROJECT_NAME} pcre)
endif()

install(TARGETS ${PROJECT_NAME}
   RUNTIME DESTINATION bin COMPONENT libraries
   LIBRARY DESTINATION lib PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE COMPONENT libraries
   ARCHIVE DESTINATION lib COMPONENT libraries)

install(FILES ${hdrs}
   DESTINATION include/${PROJECT_NAME}/
   COMPONENT headers)
