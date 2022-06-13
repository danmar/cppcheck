qt_wrap_cpp(test-cppchecklibrarydata_SRC testcppchecklibrarydata.h)
add_custom_target(build-cppchecklibrarydata-deps SOURCES ${test-cppchecklibrarydata_SRC})
add_dependencies(gui-build-deps build-cppchecklibrarydata-deps)
add_executable(test-cppchecklibrarydata
        ${test-cppchecklibrarydata_SRC}
        testcppchecklibrarydata.cpp
        ${CMAKE_SOURCE_DIR}/gui/cppchecklibrarydata.cpp
        )
target_include_directories(test-cppchecklibrarydata PRIVATE ${CMAKE_SOURCE_DIR}/gui)
target_link_libraries(test-cppchecklibrarydata ${QT_CORE_LIB} ${QT_TEST_LIB})

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Q_UNUSED() in generated code
    target_compile_options_safe(test-cppchecklibrarydata -Wno-extra-semi-stmt)
endif()