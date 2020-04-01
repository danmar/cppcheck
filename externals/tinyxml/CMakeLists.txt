file(GLOB hdrs "*.h")
file(GLOB srcs "*.cpp")

add_library(tinyxml_objs OBJECT ${srcs} ${hdrs})
# TODO: needs to be fixed upstream
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(tinyxml_objs PRIVATE -Wno-suggest-attribute=format)
    # TODO: enable this - was added in Clang 8
    #target_compile_options(tinyxml_objs PRIVATE -Wno-extra-semi-stmt)
endif()
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_library(tinyxml_objs_sanitized OBJECT EXCLUDE_FROM_ALL ${srcs} ${hdrs})
    target_compile_options(tinyxml_objs_sanitized PRIVATE -fsanitize=address)
    # TODO: enable this - was added in Clang 8
    #target_compile_options(tinyxml_objs_sanitized PRIVATE -Wno-extra-semi-stmt)
endif()