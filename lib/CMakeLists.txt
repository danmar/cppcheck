set(hdrs
    analyzer.h
    analyzerinfo.h
    astutils.h
    bughuntingchecks.h
    calculate.h
    check.h
    check64bit.h
    checkassert.h
    checkautovariables.h
    checkbool.h
    checkboost.h
    checkbufferoverrun.h
    checkclass.h
    checkcondition.h
    checkexceptionsafety.h
    checkfunctions.h
    checkinternal.h
    checkio.h
    checkleakautovar.h
    checkmemoryleak.h
    checknullpointer.h
    checkother.h
    checkpostfixoperator.h
    checksizeof.h
    checkstl.h
    checkstring.h
    checktype.h
    checkuninitvar.h
    checkunusedfunctions.h
    checkunusedvar.h
    checkvaarg.h
    clangimport.h
    color.h
    config.h
    cppcheck.h
    ctu.h
    errorlogger.h
    errortypes.h
    exprengine.h
    forwardanalyzer.h
    importproject.h
    infer.h
    library.h
    matchcompiler.h
    mathlib.h
    path.h
    pathanalysis.h
    pathmatch.h
    platform.h
    precompiled.h
    preprocessor.h
    programmemory.h
    reverseanalyzer.h
    settings.h
    standards.h
    summaries.h
    suppressions.h
    symboldatabase.h
    templatesimplifier.h
    timer.h
    token.h
    tokenize.h
    tokenlist.h
    tokenrange.h
    utils.h
    valueflow.h
    valueptr.h
    version.h
)
set(srcs
    astutils.cpp
    analyzerinfo.cpp
    bughuntingchecks.cpp
    check.cpp
    check64bit.cpp
    checkassert.cpp
    checkautovariables.cpp
    checkbool.cpp
    checkboost.cpp
    checkbufferoverrun.cpp
    checkclass.cpp
    checkcondition.cpp
    checkexceptionsafety.cpp
    checkfunctions.cpp
    checkinternal.cpp
    checkio.cpp
    checkleakautovar.cpp
    checkmemoryleak.cpp
    checknullpointer.cpp
    checkother.cpp
    checkpostfixoperator.cpp
    checksizeof.cpp
    checkstl.cpp
    checkstring.cpp
    checktype.cpp
    checkuninitvar.cpp
    checkunusedfunctions.cpp
    checkunusedvar.cpp
    checkvaarg.cpp
    clangimport.cpp
    color.cpp
    cppcheck.cpp
    ctu.cpp
    errorlogger.cpp
    errortypes.cpp
    exprengine.cpp
    forwardanalyzer.cpp
    importproject.cpp
    infer.cpp
    library.cpp
    mathlib.cpp
    path.cpp
    pathanalysis.cpp
    pathmatch.cpp
    platform.cpp
    preprocessor.cpp
    programmemory.cpp
    reverseanalyzer.cpp
    settings.cpp
    summaries.cpp
    suppressions.cpp
    symboldatabase.cpp
    templatesimplifier.cpp
    timer.cpp
    token.cpp
    tokenize.cpp
    tokenlist.cpp
    utils.cpp
    valueflow.cpp
)

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
target_include_directories(lib_objs PRIVATE ${PROJECT_SOURCE_DIR}/externals/)
if(USE_BUNDLED_TINYXML2)
    target_include_directories(lib_objs PRIVATE ${PROJECT_SOURCE_DIR}/externals/tinyxml2/)
endif()
target_include_directories(lib_objs PRIVATE ${PROJECT_SOURCE_DIR}/externals/picojson/)
target_include_directories(lib_objs PRIVATE ${PROJECT_SOURCE_DIR}/externals/simplecpp/)
if (HAVE_RULES)
    target_include_directories(lib_objs SYSTEM PRIVATE ${PCRE_INCLUDE})
endif()
if (USE_Z3)
    target_include_directories(lib_objs SYSTEM PRIVATE ${Z3_CXX_INCLUDE_DIRS})
endif()

if (NOT CMAKE_DISABLE_PRECOMPILE_HEADERS)
    target_precompile_headers(lib_objs PRIVATE precompiled.h)
endif()
