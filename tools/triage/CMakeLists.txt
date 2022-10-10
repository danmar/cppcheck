if (BUILD_GUI AND BUILD_TESTS)
    # disable all clang-tidy checks for Qt generated files
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/.clang-tidy"
"---
Checks:          '-*,misc-definitions-in-headers'
WarningsAsErrors: '*'
CheckOptions:
  - { key: HeaderFileExtensions,          value: 'x' }
")

    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        add_definitions(-DQT_NO_DEBUG)
        add_definitions(-DQT_NO_DEBUG_OUTPUT)
        add_definitions(-DQT_NO_WARNING_OUTPUT)
    else()
        add_definitions(-DQT_DEBUG)
    endif()

    file(GLOB hdrs "*.h")
    file(GLOB srcs "*.cpp")
    file(GLOB uis  "*.ui")
    qt_wrap_ui(uis_hdrs ${uis})

    add_custom_target(triage-build-ui-deps SOURCES ${hdrs} ${uis_hdrs})
    add_executable(
        triage
        ${hdrs}
        ${srcs}
        ${uis_hdrs}
        ${PROJECT_SOURCE_DIR}/gui/codeeditorstyle.cpp
        ${PROJECT_SOURCE_DIR}/gui/codeeditor.cpp)
    set_target_properties(triage PROPERTIES AUTOMOC ON)
    target_include_directories(triage PRIVATE ${PROJECT_SOURCE_DIR}/lib/ ${PROJECT_SOURCE_DIR}/gui/)
    target_link_libraries(triage ${QT_CORE_LIB} ${QT_GUI_LIB} ${QT_WIDGETS_LIB})
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # Q_UNUSED() in generated code
        target_compile_options_safe(triage -Wno-extra-semi-stmt)
        # caused by Qt generated moc code
        target_compile_options_safe(triage -Wno-redundant-parens)
    endif()
endif()
