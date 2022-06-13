file(GLOB hdrs "*.h")
file(GLOB srcs "*.cpp")
file(GLOB mainfile "main.cpp")
list(REMOVE_ITEM srcs ${mainfile})

add_library(cli_objs OBJECT ${hdrs} ${srcs})
target_include_directories(cli_objs PRIVATE ${PROJECT_SOURCE_DIR}/lib/)
if(USE_BUNDLED_TINYXML2)
    target_include_directories(cli_objs PRIVATE ${PROJECT_SOURCE_DIR}/externals/tinyxml2/)
endif()
target_include_directories(cli_objs PRIVATE ${PROJECT_SOURCE_DIR}/externals/simplecpp/)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14)
    # false positive warning in up to Clang 13 - caused by FD_ZERO macro
    set_source_files_properties(threadexecutor.cpp PROPERTIES COMPILE_FLAGS -Wno-reserved-identifier)
endif()

list(APPEND cppcheck_SOURCES ${hdrs} ${mainfile} $<TARGET_OBJECTS:cli_objs> $<TARGET_OBJECTS:lib_objs> $<TARGET_OBJECTS:simplecpp_objs>)
if(USE_BUNDLED_TINYXML2)
    list(APPEND cppcheck_SOURCES $<TARGET_OBJECTS:tinyxml2_objs>)
endif()

add_executable(cppcheck ${cppcheck_SOURCES})
target_include_directories(cppcheck PRIVATE ${PROJECT_SOURCE_DIR}/lib/)
if(USE_BUNDLED_TINYXML2)
    target_include_directories(cppcheck PRIVATE ${PROJECT_SOURCE_DIR}/externals/tinyxml2/)
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
if (USE_THREADS)
    target_link_libraries(cppcheck ${CMAKE_THREAD_LIBS_INIT})
endif()

add_dependencies(cppcheck copy_cfg)
add_dependencies(cppcheck copy_addons)
add_dependencies(cppcheck run-dmake)

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

