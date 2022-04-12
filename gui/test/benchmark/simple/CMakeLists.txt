qt_wrap_cpp(test-benchmark-simple_SRC benchmarksimple.h)
add_custom_target(build-testbenchmark-simple-deps SOURCES ${test-benchmark-simple_SRC})
add_dependencies(gui-build-deps build-testbenchmark-simple-deps)
if(USE_BUNDLED_TINYXML2)
    list(APPEND test-benchmark-simple_SRC $<TARGET_OBJECTS:tinyxml2_objs>)
endif()
add_executable(benchmark-simple
        ${test-benchmark-simple_SRC}
        benchmarksimple.cpp
        $<TARGET_OBJECTS:lib_objs>
        $<TARGET_OBJECTS:simplecpp_objs>
        )
target_include_directories(benchmark-simple PRIVATE ${CMAKE_SOURCE_DIR}/lib)
target_compile_definitions(benchmark-simple PRIVATE SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(benchmark-simple ${QT_CORE_LIB} ${QT_TEST_LIB})
if (HAVE_RULES)
    target_link_libraries(benchmark-simple ${PCRE_LIBRARY})
endif()
if(tinyxml2_FOUND AND NOT USE_BUNDLED_TINYXML2)
    target_link_libraries(benchmark-simple ${tinyxml2_LIBRARIES})
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14)
        # false positive in up to CLang 13 - caused by QBENCHMARK macro
        set_source_files_properties(benchmarksimple.cpp PROPERTIES COMPILE_FLAGS -Wno-reserved-identifier)
    endif()
    # caused by Q_UNUSED macro
    set_source_files_properties(moc_benchmarksimple.cpp PROPERTIES COMPILE_FLAGS -Wno-extra-semi-stmt)
endif()
