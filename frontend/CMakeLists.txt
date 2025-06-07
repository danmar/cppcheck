file(GLOB hdrs "*.h")
file(GLOB srcs "*.cpp")

add_library(frontend_objs OBJECT ${hdrs} ${srcs})
target_include_directories(frontend_objs PRIVATE ${PROJECT_SOURCE_DIR}/lib)