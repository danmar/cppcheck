if (ENABLE_OSS_FUZZ AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_executable(fuzz-client EXCLUDE_FROM_ALL
            main.cpp
            type2.cpp
            $<TARGET_OBJECTS:simplecpp_objs_sanitized>
            $<TARGET_OBJECTS:tinyxml_objs_sanitized>
            $<TARGET_OBJECTS:lib_objs_sanitized>)
    target_include_directories(fuzz-client PRIVATE ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/externals/simplecpp ${CMAKE_SOURCE_DIR}/externals/tinyxml ${CMAKE_SOURCE_DIR}/externals)
    target_compile_options(fuzz-client PRIVATE -fsanitize=fuzzer)
    # requires CMake >= 3.13
    #target_link_options(fuzz-client PRIVATE -fsanitize=address -fsanitize=fuzzer)
    target_link_libraries(fuzz-client PRIVATE -fsanitize=address -fsanitize=fuzzer)

    add_executable(translate EXCLUDE_FROM_ALL
            translate.cpp
            type2.cpp)
endif()
