include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/externals/tinyxml/)
include_directories(${PROJECT_SOURCE_DIR}/lib/)

file(GLOB hdrs "*.h")
file(GLOB srcs "*.cpp")

link_libraries(${PROJECT_NAME} tinyxml)

add_executable(cppcheck ${srcs} ${hdrs})
