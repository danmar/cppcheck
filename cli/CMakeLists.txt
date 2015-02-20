include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/externals/tinyxml/)
include_directories(${PROJECT_SOURCE_DIR}/lib/)

file(GLOB hdrs "*.h")
file(GLOB srcs "*.cpp")
file(GLOB mainfile "main.cpp")
list(REMOVE_ITEM srcs ${mainfile})

link_libraries(CppCheckLib tinyxml)

# TODO - Probably we should move this library to other place since it's used by the tests and the
# application
add_library(cli_lib OBJECT ${hdrs} ${srcs})

add_executable(cppcheck main.cpp 
    $<TARGET_OBJECTS:cli_lib>
)

if (WIN32)
        target_link_libraries(cppcheck Shlwapi) 
endif()

install(TARGETS cppcheck
    RUNTIME DESTINATION bin
    COMPONENT applications)

install(FILES ${cfgs}
   DESTINATION share/cppcheck/
   COMPONENT headers)
