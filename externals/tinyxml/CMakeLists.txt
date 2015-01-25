file(GLOB hdrs "*.h")
file(GLOB srcs "*.cpp")

add_library(tinyxml ${srcs} ${hdrs})

install(TARGETS tinyxml
   RUNTIME DESTINATION bin COMPONENT libraries
   LIBRARY DESTINATION lib PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE COMPONENT libraries
   ARCHIVE DESTINATION lib COMPONENT libraries)

install(FILES ${hdrs}
   DESTINATION include/${PROJECT_NAME}/
   COMPONENT headers)
