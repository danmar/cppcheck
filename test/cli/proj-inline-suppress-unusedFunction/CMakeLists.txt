cmake_minimum_required(VERSION 3.4)

project(proj-inline-suppress-unusedFunction)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}) 
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
