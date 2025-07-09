file(GLOB hdrs "*.h")
file(GLOB srcs "*.cpp")

add_library(simplecpp ${srcs} ${hdrs})
target_dll_compile_definitions(simplecpp SIMPLECPP_EXPORT SIMPLECPP_IMPORT)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options_safe(simplecpp -Wno-zero-as-null-pointer-constant)
endif()

target_include_directories(simplecpp PUBLIC .)
