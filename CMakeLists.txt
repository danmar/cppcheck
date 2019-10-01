project(Cppcheck)
cmake_minimum_required(VERSION 2.8.11)

include(GNUInstallDirs)

include(cmake/versions.cmake REQUIRED)
include(cmake/options.cmake REQUIRED)
include(cmake/findDependencies.cmake REQUIRED)
include(cmake/compileroptions.cmake REQUIRED)
include(cmake/compilerDefinitions.cmake REQUIRED)
include(cmake/buildFiles.cmake REQUIRED)
include(cmake/cxx11.cmake REQUIRED)

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
ADD_SUBDIRECTORY(gui)   # Graphical application
ADD_SUBDIRECTORY(tools/triage)   # Triage tool

include(cmake/printInfo.cmake REQUIRED)

if (ENABLE_PACKAGING)
include(cmake/PkgInfo.cmake )
set(CPACK_PACKAGE_CONTACT "${PKG_USERNAME}")
set(CPACK_PACKAGE_VERSION "${VERSION}")

include(CPack)
cpack_add_component(cppcheck)
endif()
