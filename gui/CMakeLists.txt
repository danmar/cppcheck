if (BUILD_GUI)

    if (CMAKE_BUILD_TYPE MATCHES "Release")
        add_definitions(-DQT_NO_DEBUG)
        add_definitions(-DQT_NO_DEBUG_OUTPUT)
        add_definitions(-DQT_NO_WARNING_OUTPUT)
    else()
        add_definitions(-DQT_DEBUG)
    endif()

    list(APPEND cppcheck-gui_HEADERS
        aboutdialog.h
        application.h
        applicationdialog.h
        applicationlist.h
        checkstatistics.h
        checkthread.h
        codeeditor.h
        codeeditorstyle.h
        codeeditstylecontrols.h
        codeeditstyledialog.h
        common.h
        cppchecklibrarydata.h
        csvreport.h
        erroritem.h
        filelist.h
        fileviewdialog.h
        functioncontractdialog.h
        helpdialog.h
        libraryaddfunctiondialog.h
        librarydialog.h
        libraryeditargdialog.h
        mainwindow.h
        newsuppressiondialog.h
        platforms.h
        precompiled_qmake.h
        printablereport.h
        projectfile.h
        projectfiledialog.h
        report.h
        resultstree.h
        resultsview.h
        scratchpad.h
        settingsdialog.h
        showtypes.h
        statsdialog.h
        threadhandler.h
        threadresult.h
        translationhandler.h
        txtreport.h
        variablecontractsdialog.h
        xmlreport.h
        xmlreportv2.h
    )
    QT5_WRAP_UI(uis_hdrs
        about.ui
        application.ui
        file.ui
        functioncontractdialog.ui
        helpdialog.ui
        librarydialog.ui
        libraryaddfunctiondialog.ui
        libraryeditargdialog.ui
        mainwindow.ui
        newsuppressiondialog.ui
        projectfiledialog.ui
        resultsview.ui
        scratchpad.ui
        settings.ui
        stats.ui
        variablecontractsdialog.ui
    )
    QT5_ADD_RESOURCES(resources "gui.qrc")
    QT5_ADD_TRANSLATION(qms
        cppcheck_de.ts
        cppcheck_es.ts
        cppcheck_fi.ts
        cppcheck_fr.ts
        cppcheck_it.ts
        cppcheck_ja.ts
        cppcheck_ko.ts
        cppcheck_nl.ts
        cppcheck_ru.ts
        cppcheck_sr.ts
        cppcheck_sv.ts
        cppcheck_zh_CN.ts
    )
    list(APPEND cppcheck-gui-deps ${cppcheck-gui_HEADERS} ${uis_hdrs} ${resources} ${qms})
    add_custom_target(gui-build-deps SOURCES ${cppcheck-gui-deps})

    list(APPEND cppcheck-gui_SOURCES
        aboutdialog.cpp
        application.cpp
        applicationdialog.cpp
        applicationlist.cpp
        checkstatistics.cpp
        checkthread.cpp
        codeeditor.cpp
        codeeditorstyle.cpp
        codeeditstylecontrols.cpp
        codeeditstyledialog.cpp
        common.cpp
        cppchecklibrarydata.cpp
        csvreport.cpp
        erroritem.cpp
        filelist.cpp
        fileviewdialog.cpp
        functioncontractdialog.cpp
        libraryeditargdialog.cpp
        libraryaddfunctiondialog.cpp
        librarydialog.cpp
        helpdialog.cpp
        main.cpp
        mainwindow.cpp
        newsuppressiondialog.cpp
        platforms.cpp
        printablereport.cpp
        projectfile.cpp
        projectfiledialog.cpp
        report.cpp
        resultstree.cpp
        resultsview.cpp
        scratchpad.cpp
        settingsdialog.cpp
        showtypes.cpp
        statsdialog.cpp
        threadhandler.cpp
        threadresult.cpp
        translationhandler.cpp
        txtreport.cpp
        variablecontractsdialog.cpp
        xmlreport.cpp
        xmlreportv2.cpp
        $<TARGET_OBJECTS:lib_objs>
        $<TARGET_OBJECTS:simplecpp_objs>)
    if(USE_BUNDLED_TINYXML2)
        list(APPEND cppcheck-gui_SOURCES $<TARGET_OBJECTS:tinyxml2_objs>)
    endif()

    add_executable(cppcheck-gui ${cppcheck-gui-deps} ${cppcheck-gui_SOURCES})
    set_target_properties(cppcheck-gui PROPERTIES AUTOMOC ON)
    target_include_directories(cppcheck-gui PRIVATE ${PROJECT_SOURCE_DIR}/lib/)
    if(USE_BUNDLED_TINYXML2)
        target_include_directories(cppcheck-gui PRIVATE ${PROJECT_SOURCE_DIR}/externals/tinyxml2/)
    endif()
    if (HAVE_RULES)
        target_link_libraries(cppcheck-gui ${PCRE_LIBRARY})
    endif()
    if (USE_Z3)
        target_link_libraries(cppcheck-gui ${Z3_LIBRARIES})
    endif()
    if(tinyxml2_FOUND AND NOT USE_BUNDLED_TINYXML2)
        target_link_libraries(cppcheck-gui tinyxml2)
    endif()
    target_link_libraries(cppcheck-gui Qt5::Core Qt5::Gui Qt5::Widgets Qt5::PrintSupport Qt5::Help)
    if(WITH_QCHART)
        target_compile_definitions (cppcheck-gui PRIVATE HAVE_QCHART)
        target_link_libraries(cppcheck-gui Qt5::Charts)
    endif()
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # Q_UNUSED() in generated code
        target_compile_options_safe(cppcheck-gui -Wno-extra-semi-stmt)
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
