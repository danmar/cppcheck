file(GLOB hdrs "*.h")
file(GLOB srcs "*.cpp")

add_library(simplecpp_objs OBJECT ${srcs} ${hdrs})
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options_safe(simplecpp_objs -Wno-zero-as-null-pointer-constant)
endif()
