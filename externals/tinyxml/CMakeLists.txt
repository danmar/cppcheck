file(GLOB hdrs "*.h")
file(GLOB srcs "*.cpp")

add_library(tinyxml_objs OBJECT ${srcs} ${hdrs})


