include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/externals/tinyxml/)
include_directories(${PROJECT_SOURCE_DIR}/lib/)

file(GLOB hdrs "*.h")
file(GLOB srcs "*.cpp")
file(GLOB mainfile "main.cpp")
list(REMOVE_ITEM srcs ${mainfile})

link_libraries(${PROJECT_NAME} tinyxml)

add_library(cli_lib OBJECT ${hdrs} ${srcs})

add_executable(cppcheck main.cpp 
    $<TARGET_OBJECTS:cli_lib>
)

install(TARGETS cppcheck
    RUNTIME DESTINATION bin
    COMPONENT applications)

install(FILES ${cfgs}
   DESTINATION share/${PROJECT_NAME}/
   COMPONENT headers)
