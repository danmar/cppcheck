qt5_wrap_cpp(test-cppchecklibrarydata_SRC testcppchecklibrarydata.h)
add_executable(test-cppchecklibrarydata
        ${test-cppchecklibrarydata_SRC}
        testcppchecklibrarydata.cpp
        ${CMAKE_SOURCE_DIR}/gui/cppchecklibrarydata.cpp
        )
target_include_directories(test-cppchecklibrarydata PRIVATE ${CMAKE_SOURCE_DIR}/gui)
target_link_libraries(test-cppchecklibrarydata Qt5::Core Qt5::Test)