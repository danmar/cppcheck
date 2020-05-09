cmake_minimum_required(VERSION 2.8.12)
project(Cppcheck)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)

include(cmake/versions.cmake)
include(cmake/options.cmake)
include(cmake/findDependencies.cmake)
include(cmake/compileroptions.cmake)
include(cmake/compilerDefinitions.cmake)
include(cmake/buildFiles.cmake)
include(cmake/cxx11.cmake)

use_cxx11()

file(GLOB addons "addons/*.py")
file(GLOB cfgs "cfg/*.cfg")
file(GLOB platforms "platforms/*.xml")

if (BUILD_TESTS)
    enable_testing()
endif()

add_subdirectory(externals/tinyxml)
add_subdirectory(externals/simplecpp)
add_subdirectory(lib)   # CppCheck Library 
add_subdirectory(cli)   # Client application
add_subdirectory(test)  # Tests
add_subdirectory(gui)   # Graphical application
add_subdirectory(tools/triage)   # Triage tool
add_subdirectory(oss-fuzz) # OSS-Fuzz clients
add_subdirectory(tools)

include(cmake/printInfo.cmake)
include(cmake/clang_tidy.cmake)
