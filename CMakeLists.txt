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

find_program(LIBXML2_XMLLINT_EXECUTABLE xmllint)
if (LIBXML2_XMLLINT_EXECUTABLE)
    add_custom_target(validateCFG ${LIBXML2_XMLLINT_EXECUTABLE} --noout ${CMAKE_SOURCE_DIR}/cfg/cppcheck-cfg.rng)
    foreach(cfg ${cfgs})
        get_filename_component(cfgname ${cfg} NAME_WE)
        add_custom_target(validateCFG-${cfgname} ${LIBXML2_XMLLINT_EXECUTABLE} --noout --relaxng ${CMAKE_SOURCE_DIR}/cfg/cppcheck-cfg.rng ${cfg})
        add_dependencies(validateCFG validateCFG-${cfgname})
    endforeach()
endif()

if (BUILD_TESTS)
    enable_testing()
endif()

if(USE_BUNDLED_TINYXML2)
    message(STATUS "Using bundled version of tinyxml2")
    add_subdirectory(externals/tinyxml2)
endif()
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
