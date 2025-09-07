qt_wrap_cpp(test-resultstree_SRC
        testresultstree.h
)
add_executable(test-resultstree
        ${test-resultstree_SRC}
        testresultstree.cpp
        )
target_link_libraries(test-resultstree ${QT_CORE_LIB} ${QT_GUI_LIB} ${QT_WIDGETS_LIB} ${QT_TEST_LIB})

if (REGISTER_GUI_TESTS)
    add_test(NAME test-resultstree COMMAND $<TARGET_FILE:test-resultstree> -platform offscreen)
endif()

add_dependencies(gui-tests test-resultstree)
