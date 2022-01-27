if (BUILD_GUI AND BUILD_TESTS)
    if (CMAKE_BUILD_TYPE MATCHES "Release")
        add_definitions(-DQT_NO_DEBUG)
        add_definitions(-DQT_NO_DEBUG_OUTPUT)
        add_definitions(-DQT_NO_WARNING_OUTPUT)
    else()
        add_definitions(-DQT_DEBUG)
    endif()

    qt5_wrap_ui(uis_hdrs
        mainwindow.ui
    )

    add_custom_target(triage-build-ui-deps SOURCES
        ${uis_hdrs}
    )
    add_executable(triage
        main.cpp
        mainwindow.cpp
        mainwindow.h
        ${uis_hdrs}
        ${PROJECT_SOURCE_DIR}/gui/codeeditorstyle.cpp
        ${PROJECT_SOURCE_DIR}/gui/codeeditor.cpp)
    set_target_properties(triage PROPERTIES AUTOMOC ON)
    target_include_directories(triage PRIVATE ${PROJECT_SOURCE_DIR}/lib/ ${PROJECT_SOURCE_DIR}/gui/)
    target_link_libraries(triage Qt5::Core Qt5::Gui Qt5::Widgets)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # Q_UNUSED() in generated code
        target_compile_options_safe(triage -Wno-extra-semi-stmt)
    endif()
endif()
