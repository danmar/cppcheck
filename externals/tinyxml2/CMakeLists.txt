add_library(tinyxml2_objs OBJECT tinyxml2.cpp tinyxml2.h)

# TODO: needs to be fixed upstream
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(tinyxml2_objs PRIVATE -Wno-suggest-attribute=format)
endif()
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options_safe(tinyxml2_objs -Wno-extra-semi-stmt)
    target_compile_options_safe(tinyxml2_objs -Wno-implicit-fallthrough)
endif()

