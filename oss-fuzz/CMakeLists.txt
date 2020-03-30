if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_executable(fuzz-client
            main.cpp
            type2.cpp)
    target_include_directories(fuzz-client PRIVATE ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/externals/simplecpp ${CMAKE_SOURCE_DIR}/externals/tinyxml ${CMAKE_SOURCE_DIR}/externals)
    target_compile_options(fuzz-client PRIVATE -fsanitize=fuzzer)
    target_link_libraries(fuzz-client PRIVATE simplecpp_objs_sanitized tinyxml_objs_sanitized lib_objs_sanitized)
    target_link_options(fuzz-client PRIVATE -fsanitize=address -fsanitize=fuzzer)

    add_executable(translate
            translate.cpp
            type2.cpp)
endif()