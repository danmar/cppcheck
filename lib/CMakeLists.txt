include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/externals/tinyxml/)

file(GLOB_RECURSE hdrs "*.h")
file(GLOB_RECURSE srcs "*.cpp")

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
