add_library(cli_objs OBJECT
    cmdlineparser.cpp
    cmdlineparser.h
    cppcheckexecutor.cpp
    cppcheckexecutor.h
    filelister.cpp
    filelister.h
    threadexecutor.cpp
    threadexecutor.h
)
target_include_directories(cli_objs PRIVATE ${PROJECT_SOURCE_DIR}/lib/)
if(USE_BUNDLED_TINYXML2)
    target_include_directories(cli_objs PRIVATE ${PROJECT_SOURCE_DIR}/externals/tinyxml2/)
endif()
target_include_directories(cli_objs PRIVATE ${PROJECT_SOURCE_DIR}/externals/simplecpp/)

list(APPEND cppcheck_SOURCES
    main.cpp
    $<TARGET_OBJECTS:cli_objs>
    $<TARGET_OBJECTS:lib_objs>
    $<TARGET_OBJECTS:simplecpp_objs>
)
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
if (USE_Z3)
    target_link_libraries(cppcheck ${Z3_LIBRARIES})
endif()
if (WIN32 AND NOT BORLAND)
    if(NOT MINGW)
        target_link_libraries(cppcheck Shlwapi.lib)
    else()
        target_link_libraries(cppcheck shlwapi)
    endif()
endif()
if(tinyxml2_FOUND AND NOT USE_BUNDLED_TINYXML2)
    target_link_libraries(cppcheck tinyxml2)
endif()

add_dependencies(cppcheck copy_cfg)
add_dependencies(cppcheck copy_addons)

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

