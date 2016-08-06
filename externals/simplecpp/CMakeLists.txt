file(GLOB hdrs "*.h")
file(GLOB srcs "*.cpp")

add_library(simplecpp_objs OBJECT ${srcs} ${hdrs})


