file(GLOB hdrs "*.h")
file(GLOB srcs "*.cpp")
file(GLOB mainfile "main.cpp")
list(REMOVE_ITEM srcs ${mainfile})

add_library(cli_objs OBJECT ${hdrs} ${srcs})
target_include_directories(cli_objs PRIVATE ${PROJECT_SOURCE_DIR}/lib/)
if(USE_BUNDLED_TINYXML2)
    target_include_directories(cli_objs PRIVATE ${PROJECT_SOURCE_DIR}/externals/tinyxml2/)
else()
    target_include_directories(cli_objs SYSTEM PRIVATE ${tinyxml2_INCLUDE_DIRS})
endif()
target_include_directories(cli_objs PRIVATE ${PROJECT_SOURCE_DIR}/externals/simplecpp/)
if (NOT CMAKE_DISABLE_PRECOMPILE_HEADERS)
    target_precompile_headers(cli_objs PRIVATE precompiled.h)
endif()
if (BUILD_CORE_DLL)
    target_compile_definitions(cli_objs PRIVATE CPPCHECKLIB_IMPORT TINYXML2_IMPORT)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14)
    # false positive warning in up to Clang 13 - caused by FD_ZERO macro
    set_source_files_properties(processexecutor.cpp PROPERTIES COMPILE_FLAGS -Wno-reserved-identifier)
endif()

list(APPEND cppcheck_SOURCES ${hdrs} ${mainfile} $<TARGET_OBJECTS:cli_objs>)
if (NOT BUILD_CORE_DLL)
    list(APPEND cppcheck_SOURCES $<TARGET_OBJECTS:cppcheck-core>)
    list(APPEND cppcheck_SOURCES $<TARGET_OBJECTS:simplecpp_objs>)
    if(USE_BUNDLED_TINYXML2)
        list(APPEND cppcheck_SOURCES $<TARGET_OBJECTS:tinyxml2_objs>)
    endif()
endif()
if (WIN32)
    list(APPEND cppcheck_SOURCES version.rc)
endif()

add_executable(cppcheck ${cppcheck_SOURCES})
target_include_directories(cppcheck PRIVATE ${PROJECT_SOURCE_DIR}/lib/)
if(USE_BUNDLED_TINYXML2)
    target_include_directories(cppcheck PRIVATE ${PROJECT_SOURCE_DIR}/externals/tinyxml2/)
else()
    target_include_directories(cppcheck SYSTEM PRIVATE ${tinyxml2_INCLUDE_DIRS})
endif()
target_include_directories(cppcheck PRIVATE ${PROJECT_SOURCE_DIR}/externals/simplecpp/)
if (HAVE_RULES)
    target_link_libraries(cppcheck ${PCRE_LIBRARY})
endif()
if (WIN32 AND NOT BORLAND)
    if(NOT MINGW)
        target_link_libraries(cppcheck Shlwapi.lib)
    else()
        target_link_libraries(cppcheck shlwapi)
    endif()
endif()
if(tinyxml2_FOUND AND NOT USE_BUNDLED_TINYXML2)
    target_link_libraries(cppcheck ${tinyxml2_LIBRARIES})
endif()
target_link_libraries(cppcheck ${CMAKE_THREAD_LIBS_INIT})
if (BUILD_CORE_DLL)
    target_link_libraries(cppcheck cppcheck-core)
endif()

add_dependencies(cppcheck copy_cfg)
add_dependencies(cppcheck copy_addons)
add_dependencies(cppcheck copy_platforms)
if (NOT DISABLE_DMAKE)
    add_dependencies(cppcheck run-dmake)
endif()

install(TARGETS cppcheck
    RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
    COMPONENT applications)

install(FILES ${addons}
   DESTINATION ${FILESDIR}/addons
   COMPONENT headers)

install(FILES ${cfgs}
   DESTINATION ${FILESDIR}/cfg
   COMPONENT headers)

install(FILES ${platforms}
   DESTINATION ${FILESDIR}/platforms
   COMPONENT headers)

