file(GLOB hdrs "*.h")
file(GLOB srcs "*.cpp")

add_library(tinyxml2_objs OBJECT ${srcs} ${hdrs})

# TODO: need to be fixed upstream
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(tinyxml2_objs PRIVATE -Wno-suggest-attribute=format)
endif()
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8.0.0")
        target_compile_options(tinyxml2_objs PRIVATE -Wno-extra-semi-stmt)
    endif()
endif()

