qt_wrap_cpp(test-cppchecklibrarydata_SRC testcppchecklibrarydata.h)
QT_ADD_RESOURCES(test-cppchecklibrarydata_resources "resources.qrc")
add_custom_target(build-cppchecklibrarydata-deps SOURCES ${test-cppchecklibrarydata_SRC} ${test-cppchecklibrarydata_resources})
add_dependencies(gui-build-deps build-cppchecklibrarydata-deps)
add_executable(test-cppchecklibrarydata
        ${test-cppchecklibrarydata_SRC}
        ${test-cppchecklibrarydata_resources}
        testcppchecklibrarydata.cpp
        ${CMAKE_SOURCE_DIR}/gui/cppchecklibrarydata.cpp
        )
target_include_directories(test-cppchecklibrarydata PRIVATE ${CMAKE_SOURCE_DIR}/gui)
target_compile_definitions(test-cppchecklibrarydata PRIVATE SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(test-cppchecklibrarydata ${QT_CORE_LIB} ${QT_TEST_LIB})

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Q_UNUSED() in generated code
    target_compile_options_safe(test-cppchecklibrarydata -Wno-extra-semi-stmt)
endif()

if (REGISTER_GUI_TESTS)
    add_test(NAME test-cppchecklibrarydata COMMAND $<TARGET_FILE:test-cppchecklibrarydata>)
endif()