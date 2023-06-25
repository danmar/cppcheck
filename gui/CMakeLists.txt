if (BUILD_GUI)

    # disable all clang-tidy checks for Qt generated files
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/.clang-tidy"
"---
Checks:          '-*,misc-definitions-in-headers'
WarningsAsErrors: '*'
CheckOptions:
  - { key: HeaderFileExtensions,          value: 'x' }
")

    add_compile_definitions($<$<NOT:$<CONFIG:Debug>>:QT_NO_DEBUG>)
    add_compile_definitions($<$<NOT:$<CONFIG:Debug>>:QT_NO_DEBUG_OUTPUT>)
    add_compile_definitions($<$<NOT:$<CONFIG:Debug>>:QT_NO_WARNING_OUTPUT>)
    add_compile_definitions($<$<CONFIG:Debug>:QT_DEBUG>)

    file(GLOB hdrs "*.h")
    file(GLOB srcs "*.cpp")
    file(GLOB uis  "*.ui")
    file(GLOB tss  "*.ts")
    QT_WRAP_UI(uis_hdrs ${uis})
    QT_ADD_RESOURCES(resources "gui.qrc")
    # TODO: passing "-no-obsolete" here breaks the translations
    QT_CREATE_TRANSLATION(qms ${CMAKE_CURRENT_SOURCE_DIR} ${tss})
    list(APPEND cppcheck-gui-deps ${hdrs} ${uis_hdrs} ${resources} ${qms})
    add_custom_target(gui-build-deps SOURCES ${cppcheck-gui-deps})

    list(APPEND cppcheck-gui_SOURCES ${srcs})
    if (NOT BUILD_CORE_DLL)
        list(APPEND cppcheck-gui_SOURCES $<TARGET_OBJECTS:cppcheck-core> $<TARGET_OBJECTS:simplecpp_objs>)
        if(USE_BUNDLED_TINYXML2)
            list(APPEND cppcheck-gui_SOURCES $<TARGET_OBJECTS:tinyxml2_objs>)
        endif()
    endif()

    add_executable(cppcheck-gui ${cppcheck-gui-deps} ${cppcheck-gui_SOURCES})
    set_target_properties(cppcheck-gui PROPERTIES AUTOMOC ON)
    target_include_directories(cppcheck-gui PRIVATE ${PROJECT_SOURCE_DIR}/lib/)
    if(USE_BUNDLED_TINYXML2)
        target_include_directories(cppcheck-gui PRIVATE ${PROJECT_SOURCE_DIR}/externals/tinyxml2/)
    else()
        target_include_directories(cppcheck-gui SYSTEM PRIVATE ${tinyxml2_INCLUDE_DIRS})
    endif()
    if (NOT CMAKE_DISABLE_PRECOMPILE_HEADERS)
        target_precompile_headers(cppcheck-gui PRIVATE precompiled.h)
    endif()
    if (HAVE_RULES)
        target_link_libraries(cppcheck-gui ${PCRE_LIBRARY})
    endif()
    if(tinyxml2_FOUND AND NOT USE_BUNDLED_TINYXML2)
        target_link_libraries(cppcheck-gui ${tinyxml2_LIBRARIES})
    endif()
    target_link_libraries(cppcheck-gui ${QT_CORE_LIB} ${QT_GUI_LIB} ${QT_WIDGETS_LIB} ${QT_PRINTSUPPORT_LIB} ${QT_HELP_LIB} ${QT_NETWORK_LIB})
    if(WITH_QCHART)
        target_link_libraries(cppcheck-gui ${QT_CHARTS_LIB})
    endif()
    if (BUILD_CORE_DLL)
        target_compile_definitions(cppcheck-gui PRIVATE CPPCHECKLIB_IMPORT TINYXML2_IMPORT)
        target_link_libraries(cppcheck-gui cppcheck-core)
    endif()
    if(MSVC)
        # compilation will fail as e.g. QList::realloc would be replaced by MSVC's macro definition
        target_compile_definitions(cppcheck-gui PRIVATE $<$<CONFIG:Debug>:DISABLE_CRTDBG_MAP_ALLOC>)
    endif()
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # Q_UNUSED() in generated code
        target_compile_options_safe(cppcheck-gui -Wno-extra-semi-stmt)
        # caused by Qt generated moc code
        target_compile_options_safe(cppcheck-gui -Wno-redundant-parens)
    endif()

    install(TARGETS cppcheck-gui RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR} COMPONENT applications)
    install(FILES ${qms} DESTINATION ${CMAKE_INSTALL_FULL_BINDIR} COMPONENT applications)

    install(FILES cppcheck-gui.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)

    # icons
    install(FILES cppcheck-gui.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
    install(FILES cppcheck-gui.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/64x64/apps)

    if (BUILD_TESTS)
        add_subdirectory(test)
    endif()
endif()
