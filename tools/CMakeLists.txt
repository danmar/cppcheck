# TODO: when using ccache and matchcompiler this will accessed before the file was generated and thus the build fails
set(srcs_lib pathmatch.cpp path.cpp)
foreach(file ${srcs_lib})
    if (NOT USE_MATCHCOMPILER_OPT STREQUAL "Off")
        set(src "${CMAKE_BINARY_DIR}/lib/build/mc_${file}")
        set_source_files_properties(${src} PROPERTIES GENERATED TRUE)
    else()
        set(src "${CMAKE_SOURCE_DIR}/lib/${file}")
    endif()
    set(srcs_tools ${srcs_tools} ${src})
endforeach()

add_executable(dmake EXCLUDE_FROM_ALL
        dmake.cpp
        ${CMAKE_SOURCE_DIR}/cli/filelister.cpp
        ${srcs_tools}
        ${CMAKE_SOURCE_DIR}/lib/utils.cpp
        $<TARGET_OBJECTS:simplecpp_objs>
)
target_include_directories(dmake PRIVATE ${CMAKE_SOURCE_DIR}/cli ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/externals/simplecpp)
if (WIN32 AND NOT BORLAND)
    if(NOT MINGW)
        target_link_libraries(dmake Shlwapi.lib)
    else()
        target_link_libraries(dmake shlwapi)
    endif()
endif()

add_custom_target(run-dmake $<TARGET_FILE:dmake>
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        DEPENDS dmake)
