if (ENABLE_OSS_FUZZ AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(fuzz-client_SRC
            main.cpp
            type2.cpp
    )
    if(USE_BUNDLED_TINYXML2)
        list(APPEND fuzz-client_SRC $<TARGET_OBJECTS:tinyxml2_objs>)
    endif()
    add_executable(fuzz-client EXCLUDE_FROM_ALL
            ${fuzz-client_SRC}
            $<TARGET_OBJECTS:simplecpp_objs>
            $<TARGET_OBJECTS:lib_objs>)
    target_include_directories(fuzz-client PRIVATE ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/externals/simplecpp ${CMAKE_SOURCE_DIR}/externals)
    if(USE_BUNDLED_TINYXML2)
        target_include_directories(fuzz-client PRIVATE ${CMAKE_SOURCE_DIR}/externals/tinyxml2/)
    endif()
    target_compile_options(fuzz-client PRIVATE -fsanitize=fuzzer)
    # TODO: target_link_options() requires CMake >= 3.13
    #target_link_options(fuzz-client PRIVATE -fsanitize=fuzzer)
    target_link_libraries(fuzz-client PRIVATE -fsanitize=fuzzer)
    if (HAVE_RULES)
        target_link_libraries(fuzz-client PRIVATE ${PCRE_LIBRARY})
    endif()
    if(tinyxml2_FOUND AND NOT USE_BUNDLED_TINYXML2)
        target_link_libraries(fuzz-client PRIVATE ${tinyxml2_LIBRARIES})
    endif()

    add_executable(translate EXCLUDE_FROM_ALL
            translate.cpp
            type2.cpp)
endif()
