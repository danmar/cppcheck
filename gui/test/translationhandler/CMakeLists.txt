qt_wrap_cpp(test-translationhandler_SRC testtranslationhandler.h ${CMAKE_SOURCE_DIR}/gui/translationhandler.h)
add_custom_target(build-translationhandler-deps SOURCES ${test-translationhandler_SRC})
add_dependencies(gui-build-deps build-translationhandler-deps)
add_executable(test-translationhandler
        ${test-translationhandler_SRC}
        testtranslationhandler.cpp
        ${CMAKE_SOURCE_DIR}/gui/common.cpp
        ${CMAKE_SOURCE_DIR}/gui/translationhandler.cpp
        )
target_include_directories(test-translationhandler PRIVATE ${CMAKE_SOURCE_DIR}/gui ${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(test-translationhandler ${QT_CORE_LIB} ${QT_WIDGETS_LIB} ${QT_TEST_LIB})

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Q_UNUSED() in generated code
    target_compile_options_safe(test-translationhandler -Wno-extra-semi-stmt)
endif()

if (REGISTER_GUI_TESTS)
    # TODO: requires X session
    #add_test(NAME test-translationhandler COMMAND $<TARGET_FILE:test-translationhandler>)
endif()