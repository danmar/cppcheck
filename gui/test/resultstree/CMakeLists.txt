qt_wrap_cpp(test-resultstree_SRC testresultstree.h ${CMAKE_SOURCE_DIR}/gui/resultstree.h)
add_custom_target(build-resultstree-deps SOURCES ${test-resultstree_SRC})
add_dependencies(gui-build-deps build-resultstree-deps)
add_executable(test-resultstree
        ${test-resultstree_SRC}
        testresultstree.cpp
        ${CMAKE_SOURCE_DIR}/gui/resultstree.cpp
        ${CMAKE_SOURCE_DIR}/gui/erroritem.cpp
        ${CMAKE_SOURCE_DIR}/gui/showtypes.cpp
        ${CMAKE_SOURCE_DIR}/gui/report.cpp
        ${CMAKE_SOURCE_DIR}/gui/xmlreportv2.cpp
        )
target_include_directories(test-resultstree PRIVATE ${CMAKE_SOURCE_DIR}/gui ${CMAKE_SOURCE_DIR}/lib)
target_compile_definitions(test-resultstree PRIVATE SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(test-resultstree ${QT_CORE_LIB} ${QT_GUI_LIB} ${QT_WIDGETS_LIB} ${QT_TEST_LIB})

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Q_UNUSED() in generated code
    target_compile_options_safe(test-resultstree -Wno-extra-semi-stmt)
endif()

if (REGISTER_GUI_TESTS)
    add_test(NAME test-resultstree COMMAND $<TARGET_FILE:test-resultstree>)
endif()
