if (BUILD_GUI)

    set(CMAKE_AUTOMOC ON)
    if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
        add_definitions(-DQT_NO_DEBUG)
        add_definitions(-DQT_NO_DEBUG_OUTPUT)
        add_definitions(-DQT_NO_WARNING_OUTPUT)
    else()
        add_definitions(-DQT_DEBUG)
    endif()

    include_directories(${PROJECT_SOURCE_DIR}/lib/)
    include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/externals/tinyxml/)

    file(GLOB hdrs "*.h")
    file(GLOB srcs "*.cpp")
    file(GLOB uis  "*.ui")
    file(GLOB tss  "*.ts")
    QT5_WRAP_UI(uis_hdrs ${uis})
    QT5_ADD_RESOURCES(resources "gui.qrc")
    QT5_ADD_TRANSLATION(qms ${tss})

    add_executable(cppcheck-gui ${hdrs} ${srcs} ${uis_hdrs} ${resources} ${qms} $<TARGET_OBJECTS:lib_objs> $<TARGET_OBJECTS:tinyxml_objs> $<TARGET_OBJECTS:simplecpp_objs>)
    if (HAVE_RULES)
        target_link_libraries(cppcheck-gui pcre)
    endif()
    target_link_libraries(cppcheck-gui Qt5::Core Qt5::Gui Qt5::Widgets Qt5::PrintSupport)

    install(TARGETS cppcheck-gui RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR} COMPONENT applications)
    install(FILES ${qms} DESTINATION ${CMAKE_INSTALL_FULL_BINDIR} COMPONENT applications)

    install(FILES cppcheck-gui.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)

    # icons
    install(FILES cppcheck-gui.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
    install(FILES cppcheck-gui.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/64x64/apps)

    set(CMAKE_AUTOMOC OFF)

endif()
