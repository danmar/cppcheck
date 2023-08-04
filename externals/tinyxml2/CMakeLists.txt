file(GLOB hdrs "*.h")
file(GLOB srcs "*.cpp")

add_library(tinyxml2_objs OBJECT ${srcs} ${hdrs})
if (BUILD_CORE_DLL)
    target_compile_definitions(tinyxml2_objs PRIVATE TINYXML2_EXPORT)
endif()

# TODO: needs to be fixed upstream
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(tinyxml2_objs PRIVATE -Wno-suggest-attribute=format)
endif()
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options_safe(tinyxml2_objs -Wno-extra-semi-stmt)
    target_compile_options_safe(tinyxml2_objs -Wno-implicit-fallthrough)
    target_compile_options_safe(tinyxml2_objs -Wno-suggest-override)
    target_compile_options_safe(tinyxml2_objs -Wno-suggest-destructor-override)
    target_compile_options_safe(tinyxml2_objs -Wno-zero-as-null-pointer-constant)
    target_compile_options_safe(tinyxml2_objs -Wno-format-nonliteral)
    target_compile_options_safe(tinyxml2_objs -Wno-old-style-cast)
endif()

