qt5_wrap_cpp(test-projectfile_SRC testprojectfile.h ${CMAKE_SOURCE_DIR}/gui/projectfile.h)
add_executable(test-projectfile
        ${test-projectfile_SRC}
        testprojectfile.cpp
        ${CMAKE_SOURCE_DIR}/gui/projectfile.cpp
        )
target_include_directories(test-projectfile PRIVATE ${CMAKE_SOURCE_DIR}/gui ${CMAKE_SOURCE_DIR}/lib)
target_compile_definitions(test-projectfile PRIVATE SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(test-projectfile Qt5::Core Qt5::Test)