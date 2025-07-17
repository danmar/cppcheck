qt_wrap_cpp(test-filelist_SRC testfilelist.h)
add_custom_target(build-testfilelist-deps SOURCES ${test-filelist_SRC})
add_dependencies(gui-build-deps build-testfilelist-deps)
add_executable(test-filelist
        ${test-filelist_SRC}
        testfilelist.cpp
        ${CMAKE_SOURCE_DIR}/gui/filelist.cpp
        ${CMAKE_SOURCE_DIR}/lib/pathmatch.cpp
        ${CMAKE_SOURCE_DIR}/lib/path.cpp
        ${CMAKE_SOURCE_DIR}/lib/utils.cpp
        $<TARGET_OBJECTS:simplecpp_objs>
        )
target_include_directories(test-filelist PRIVATE ${CMAKE_SOURCE_DIR}/gui ${CMAKE_SOURCE_DIR}/lib)
target_externals_include_directories(test-filelist PRIVATE ${CMAKE_SOURCE_DIR}/externals/simplecpp)
target_compile_definitions(test-filelist PRIVATE SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(test-filelist ${QT_CORE_LIB} ${QT_TEST_LIB})

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(QT_VERSION VERSION_LESS "6.0.0")
        # Q_UNUSED() in generated code - see https://bugreports.qt.io/browse/QTBUG-82978
        target_compile_options_safe(test-filelist -Wno-extra-semi-stmt)
    endif()
    if(QT_VERSION VERSION_GREATER_EQUAL "6.9.0")
        # caused by Qt generated moc code starting with 6.9.0 - see https://bugreports.qt.io/browse/QTBUG-135638
        target_compile_options_safe(test-filelist -Wno-ctad-maybe-unsupported)
    endif()
endif()

if (REGISTER_GUI_TESTS)
    add_test(NAME test-filelist COMMAND $<TARGET_FILE:test-filelist>)
endif()

add_dependencies(gui-tests test-filelist)