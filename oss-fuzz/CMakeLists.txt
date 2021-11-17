if (ENABLE_OSS_FUZZ AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_executable(fuzz-client EXCLUDE_FROM_ALL
            main.cpp
            type2.cpp
            $<TARGET_OBJECTS:simplecpp_objs>
            $<TARGET_OBJECTS:tinyxml2_objs>
            $<TARGET_OBJECTS:lib_objs>)
    target_include_directories(fuzz-client PRIVATE ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/externals/simplecpp ${CMAKE_SOURCE_DIR}/externals/tinyxml2 ${CMAKE_SOURCE_DIR}/externals)
    target_compile_options(fuzz-client PRIVATE -fsanitize=fuzzer)
    # TODO: target_link_options() requires CMake >= 3.13
    #target_link_options(fuzz-client PRIVATE -fsanitize=fuzzer)
    target_link_libraries(fuzz-client PRIVATE -fsanitize=fuzzer)
    if (HAVE_RULES)
        target_link_libraries(fuzz-client PRIVATE ${PCRE_LIBRARY})
    endif()
    if (USE_Z3)
        target_link_libraries(fuzz-client PRIVATE ${Z3_LIBRARIES})
    endif()

    add_executable(translate EXCLUDE_FROM_ALL
            translate.cpp
            type2.cpp)
endif()
