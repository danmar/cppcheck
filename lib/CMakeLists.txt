file(GLOB_RECURSE hdrs "*.h")
file(GLOB_RECURSE srcs "*.cpp")

function(build_src output filename)
    get_filename_component(file ${filename} NAME)
    set(outfile ${CMAKE_CURRENT_BINARY_DIR}/build/mc_${file})
    set(${output} ${${output}} ${outfile} PARENT_SCOPE)
    if (USE_MATCHCOMPILER MATCHES "Verify")
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

if (NOT USE_MATCHCOMPILER_OPT MATCHES "Off")
    set(srcs_lib ${srcs_build})
else()
    set(srcs_lib ${srcs})
endif()

add_library(lib_objs OBJECT ${srcs_lib} ${hdrs})
target_include_directories(lib_objs SYSTEM PRIVATE ${PROJECT_SOURCE_DIR}/externals/)
if(USE_BUNDLED_TINYXML2)
    target_include_directories(lib_objs SYSTEM PRIVATE ${PROJECT_SOURCE_DIR}/externals/tinyxml2/)
endif()
target_include_directories(lib_objs SYSTEM PRIVATE ${PROJECT_SOURCE_DIR}/externals/picojson/)
target_include_directories(lib_objs SYSTEM PRIVATE ${PROJECT_SOURCE_DIR}/externals/simplecpp/)
if (HAVE_RULES)
    target_include_directories(lib_objs SYSTEM PRIVATE ${PCRE_INCLUDE})
endif()
if (Boost_FOUND)
    target_include_directories(lib_objs SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
endif()

if (NOT CMAKE_DISABLE_PRECOMPILE_HEADERS)
    target_precompile_headers(lib_objs PRIVATE precompiled.h)
endif()
