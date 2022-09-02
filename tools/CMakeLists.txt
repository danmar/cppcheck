set(srcs_lib pathmatch.cpp path.cpp)
foreach(file ${srcs_lib})
    if (NOT USE_MATCHCOMPILER_OPT STREQUAL "Off")
        set(src "${CMAKE_BINARY_DIR}/lib/build/mc_${file}")
        set_source_files_properties(${src} PROPERTIES GENERATED TRUE)
    else()
        set(src "${PROJECT_SOURCE_DIR}/lib/${file}")
    endif()
    set(srcs_tools ${srcs_tools} ${src})
endforeach()

add_executable(dmake EXCLUDE_FROM_ALL
        dmake.cpp
        ${PROJECT_SOURCE_DIR}/cli/filelister.cpp
        ${srcs_tools}
        ${PROJECT_SOURCE_DIR}/lib/utils.cpp
        $<TARGET_OBJECTS:simplecpp_objs>
)
target_include_directories(dmake PRIVATE ${PROJECT_SOURCE_DIR}/cli ${PROJECT_SOURCE_DIR}/lib ${PROJECT_SOURCE_DIR}/externals/simplecpp)
if (WIN32 AND NOT BORLAND)
    if(NOT MINGW)
        target_link_libraries(dmake Shlwapi.lib)
    else()
        target_link_libraries(dmake shlwapi)
    endif()
endif()

add_custom_target(run-dmake $<TARGET_FILE:dmake>
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        DEPENDS dmake)
