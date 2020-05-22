if (NOT USE_MATCHCOMPILER_OPT MATCHES "Off")
    set(lib_src "${CMAKE_BINARY_DIR}/lib/build/mc_")
else()
    set(lib_src "${CMAKE_SOURCE_DIR}/lib/")
endif()

add_executable(dmake EXCLUDE_FROM_ALL
        dmake.cpp
        ${CMAKE_SOURCE_DIR}/cli/filelister.cpp
        ${lib_src}pathmatch.cpp
        ${lib_src}path.cpp
        ${CMAKE_SOURCE_DIR}/lib/utils.cpp
        ${CMAKE_SOURCE_DIR}/externals/simplecpp/simplecpp
)
target_include_directories(dmake PRIVATE ${CMAKE_SOURCE_DIR}/cli ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/externals/simplecpp)
if (WIN32 AND NOT BORLAND)
    target_link_libraries(dmake Shlwapi.lib)
endif()

add_custom_target(run-dmake $<TARGET_FILE:dmake>
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        DEPENDS dmake)