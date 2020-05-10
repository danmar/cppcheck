file(GLOB hdrs "*.h")
file(GLOB srcs "*.cpp")

add_library(tinyxml_objs OBJECT ${srcs} ${hdrs})

# TODO: need to be fixed upstream
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(tinyxml_objs PRIVATE -Wno-suggest-attribute=format)
endif()
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8.0.0")
        target_compile_options(tinyxml_objs PRIVATE -Wno-extra-semi-stmt)
    endif()
endif()

# TODO: conflicts with ANALYZE_* options
if (ENABLE_OSS_FUZZ AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_library(tinyxml_objs_sanitized OBJECT EXCLUDE_FROM_ALL ${srcs} ${hdrs})
    target_compile_options(tinyxml_objs_sanitized PRIVATE -fsanitize=address)
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8.0.0")
        target_compile_options(tinyxml_objs_sanitized PRIVATE -Wno-extra-semi-stmt)
    endif()
endif()