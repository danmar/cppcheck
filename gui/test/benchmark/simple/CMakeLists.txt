qt5_wrap_cpp(test-benchmark-simple_SRC benchmarksimple.h)
add_executable(benchmark-simple
        ${test-benchmark-simple_SRC}
        benchmarksimple.cpp
        $<TARGET_OBJECTS:lib_objs>
        $<TARGET_OBJECTS:tinyxml2_objs>
        $<TARGET_OBJECTS:simplecpp_objs>
        )
target_include_directories(benchmark-simple PRIVATE ${CMAKE_SOURCE_DIR}/lib)
target_compile_definitions(benchmark-simple PRIVATE SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(benchmark-simple Qt5::Core Qt5::Test)
if (HAVE_RULES)
    target_link_libraries(benchmark-simple ${PCRE_LIBRARY})
endif()
if (USE_Z3)
    target_link_libraries(benchmark-simple ${Z3_LIBRARIES})
endif()