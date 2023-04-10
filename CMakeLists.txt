cmake_minimum_required(VERSION 2.8.12)
project(Cppcheck)

include(cmake/cxx11.cmake)
use_cxx11()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)

include(cmake/ccache.cmake)
include(cmake/compilerCheck.cmake)
include(cmake/versions.cmake)
include(cmake/options.cmake)
include(cmake/findDependencies.cmake)
include(cmake/compileroptions.cmake)
include(cmake/compilerDefinitions.cmake)
include(cmake/buildFiles.cmake)
include(cmake/printInfo.cmake)
if(BUILD_GUI)
    include(cmake/qtCompat.cmake)
endif()


file(GLOB addons "addons/*.py")
file(GLOB cfgs "cfg/*.cfg")
file(GLOB platforms "platforms/*.xml")

if(LIBXML2_XMLLINT_EXECUTABLE)
    add_custom_target(validateCFG DEPENDS validateCFG-cmd)
    add_custom_command(OUTPUT validateCFG-cmd
            COMMAND ${LIBXML2_XMLLINT_EXECUTABLE} --noout ${CMAKE_SOURCE_DIR}/cfg/cppcheck-cfg.rng)
    foreach(cfg ${cfgs})
        add_custom_command(OUTPUT validateCFG-cmd APPEND
                COMMAND ${LIBXML2_XMLLINT_EXECUTABLE} --noout --relaxng ${CMAKE_SOURCE_DIR}/cfg/cppcheck-cfg.rng ${cfg})
    endforeach()
    # this is a symbolic name for a build rule and not an output file
    set_source_files_properties(validateCFG-cmd PROPERTIES SYMBOLIC "true")

    add_custom_target(validatePlatforms ${LIBXML2_XMLLINT_EXECUTABLE} --noout ${CMAKE_SOURCE_DIR}/platforms/cppcheck-platforms.rng)
    foreach(platform ${platforms})
        get_filename_component(platformname ${platform} NAME_WE)
        add_custom_target(validatePlatforms-${platformname} ${LIBXML2_XMLLINT_EXECUTABLE} --noout --relaxng ${CMAKE_SOURCE_DIR}/platforms/cppcheck-platforms.rng ${platform})
        add_dependencies(validatePlatforms validatePlatforms-${platformname})
    endforeach()

    add_custom_target(errorlist-xml $<TARGET_FILE:cppcheck> --errorlist > ${CMAKE_BINARY_DIR}/errorlist.xml
            DEPENDS cppcheck)

    add_custom_target(example-xml $<TARGET_FILE:cppcheck> --xml --enable=all --inconclusive --max-configs=1 ${CMAKE_SOURCE_DIR}/samples 2> ${CMAKE_BINARY_DIR}/example.xml
            DEPENDS cppcheck)

    add_custom_target(createXMLExamples DEPENDS errorlist-xml example-xml)

    if(PYTHON_EXECUTABLE)
        add_custom_target(checkCWEEntries ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/listErrorsWithoutCWE.py -F ${CMAKE_BINARY_DIR}/errorlist.xml
                DEPENDS errorlist-xml)
    endif()

    add_custom_target(validateXML ${LIBXML2_XMLLINT_EXECUTABLE} --noout ${CMAKE_SOURCE_DIR}/cppcheck-errors.rng
            COMMAND ${LIBXML2_XMLLINT_EXECUTABLE} --noout --relaxng ${CMAKE_SOURCE_DIR}/cppcheck-errors.rng ${CMAKE_BINARY_DIR}/errorlist.xml
            COMMAND ${LIBXML2_XMLLINT_EXECUTABLE} --noout --relaxng ${CMAKE_SOURCE_DIR}/cppcheck-errors.rng ${CMAKE_BINARY_DIR}/example.xml
            DEPENDS createXMLExamples
            )

    add_custom_target(validateRules ${LIBXML2_XMLLINT_EXECUTABLE} --noout ${CMAKE_SOURCE_DIR}/rules/*.xml)
endif()

if(BUILD_TESTS)
    enable_testing()
endif()

add_custom_target(copy_cfg ALL
    ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/cfg"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/cfg"
    COMMENT "Copying cfg files to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}")

add_custom_target(copy_addons ALL
    ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/addons"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/addons"
    COMMENT "Copying addons files to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}")

add_custom_target(copy_platforms ALL
    ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/platforms"
    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/platforms"
    COMMENT "Copying platforms files to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}")

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
add_subdirectory(tools)

include(cmake/clang_tidy.cmake)
