file(GLOB_RECURSE hdrs "*.h")
file(GLOB_RECURSE srcs "*.cpp")

function(build_src output filename)
    get_filename_component(file ${filename} NAME)
    set(outfile ${CMAKE_CURRENT_BINARY_DIR}/build/mc_${file})
    set(${output} ${${output}} ${outfile} PARENT_SCOPE)
    if (USE_MATCHCOMPILER STREQUAL "Verify")
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

if (NOT USE_MATCHCOMPILER_OPT STREQUAL "Off")
    foreach(file ${srcs})
        build_src(srcs_build ${file})
    endforeach()

    set(srcs_lib ${srcs_build})
else()
    set(srcs_lib ${srcs})
endif()

if (BUILD_CORE_DLL)
    add_library(cppcheck-core SHARED ${srcs_lib} ${hdrs} $<TARGET_OBJECTS:tinyxml2_objs> $<TARGET_OBJECTS:simplecpp_objs> version.rc)
    target_compile_definitions(cppcheck-core PRIVATE CPPCHECKLIB_EXPORT TINYXML2_EXPORT SIMPLECPP_EXPORT)
else()
    add_library(cppcheck-core OBJECT ${srcs_lib} ${hdrs})
endif()
target_include_directories(cppcheck-core PRIVATE ${PROJECT_SOURCE_DIR}/externals/)
if(USE_BUNDLED_TINYXML2)
    target_include_directories(cppcheck-core PRIVATE ${PROJECT_SOURCE_DIR}/externals/tinyxml2/)
else()
    target_include_directories(cppcheck-core SYSTEM PRIVATE ${tinyxml2_INCLUDE_DIRS})
endif()
target_include_directories(cppcheck-core PRIVATE ${PROJECT_SOURCE_DIR}/externals/picojson/)
target_include_directories(cppcheck-core PRIVATE ${PROJECT_SOURCE_DIR}/externals/simplecpp/)
if (HAVE_RULES)
    target_include_directories(cppcheck-core SYSTEM PRIVATE ${PCRE_INCLUDE})
endif()
if (Boost_FOUND)
    target_include_directories(cppcheck-core SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
endif()

if (NOT CMAKE_DISABLE_PRECOMPILE_HEADERS)
    target_precompile_headers(cppcheck-core PRIVATE precompiled.h)
endif()
