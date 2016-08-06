include_directories(${PROJECT_SOURCE_DIR}/lib/)
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/externals/tinyxml/)
include_directories(${PROJECT_SOURCE_DIR}/externals/simplecpp/)

file(GLOB hdrs "*.h")
file(GLOB srcs "*.cpp")
file(GLOB mainfile "main.cpp")
list(REMOVE_ITEM srcs ${mainfile})

add_library(cli_objs OBJECT ${hdrs} ${srcs}) 
add_executable(cppcheck ${hdrs} ${mainfile} $<TARGET_OBJECTS:cli_objs> $<TARGET_OBJECTS:lib_objs> $<TARGET_OBJECTS:tinyxml_objs> $<TARGET_OBJECTS:simplecpp_objs>)
if (HAVE_RULES)
    target_link_libraries(cppcheck pcre)
endif()

install(TARGETS cppcheck
    RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
    COMPONENT applications)

install(FILES ${cfgs}
   DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/${PROJECT_NAME}/
   COMPONENT headers)
