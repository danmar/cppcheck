include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/externals/tinyxml/)
include_directories(${PROJECT_SOURCE_DIR}/externals/simplecpp/)

file(GLOB_RECURSE hdrs "*.h")
file(GLOB_RECURSE srcs "*.cpp")

function(build_src output filename)
    get_filename_component(file ${filename} NAME)
    set(outfile ${CMAKE_CURRENT_BINARY_DIR}/build/mc_${file})
    set(${output} ${${output}} ${outfile} PARENT_SCOPE)
    if (${USE_MATCHCOMPILER} STREQUAL "Verify")
        set(verify_option "--verify")
    endif()
    add_custom_command(
        OUTPUT ${outfile}
        COMMAND ${PYTHON_EXECUTABLE} "${PROJECT_SOURCE_DIR}/tools/matchcompiler.py" 
                --read-dir="${CMAKE_CURRENT_SOURCE_DIR}" 
                --prefix="mc_" 
                --line 
                ${verify_option} 
                ${file}
        DEPENDS ${file} 
        DEPENDS ${PROJECT_SOURCE_DIR}/tools/matchcompiler.py
    )
    set_source_files_properties(${outfile} PROPERTIES GENERATED TRUE)
endfunction()

foreach(file ${srcs})
    build_src(srcs_build ${file})
endforeach()

if (NOT ${USE_MATCHCOMPILER_OPT} STREQUAL "Off")
    set(srcs_lib ${srcs_build})
else()
    set(srcs_lib ${srcs})
endif()

add_library(lib_objs OBJECT ${srcs_lib} ${hdrs})

