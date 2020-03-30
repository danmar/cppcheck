file(GLOB hdrs "*.h")
file(GLOB srcs "*.cpp")

add_library(tinyxml_objs OBJECT ${srcs} ${hdrs})
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_library(tinyxml_objs_sanitized OBJECT ${srcs} ${hdrs})
    target_compile_options(tinyxml_objs_sanitized PRIVATE -fsanitize=address)
endif()


