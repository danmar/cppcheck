project(CppCheck)
cmake_minimum_required(VERSION 2.8)

include(cmake/versions.cmake REQUIRED)
include(cmake/options.cmake REQUIRED)
include(cmake/findDependencies.cmake REQUIRED)
include(cmake/compileroptions.cmake REQUIRED)
include(cmake/compilerDefinitions.cmake REQUIRED)
include(cmake/buildFiles.cmake REQUIRED)

file(GLOB cfgs "cfg/*.cfg")

add_subdirectory(externals/tinyxml)
add_subdirectory(lib)   # CppCheck Library 
add_subdirectory(cli)   # Client application
add_subdirectory(test)  # Tests
ADD_SUBDIRECTORY(gui)   # Graphical application

include(cmake/printInfo.cmake REQUIRED)
