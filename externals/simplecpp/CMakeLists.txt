file(GLOB hdrs "*.h")
file(GLOB srcs "*.cpp")

add_library(simplecpp_objs OBJECT ${srcs} ${hdrs})
# TODO: conflicts with ANALYZE_* options
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_library(simplecpp_objs_sanitized OBJECT EXCLUDE_FROM_ALL ${srcs} ${hdrs})
    target_compile_options(simplecpp_objs_sanitized PRIVATE -fsanitize=address)
endif()


