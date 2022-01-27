cmake_minimum_required(VERSION 2.8.12)
project(Cppcheck)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)

include(cmake/versions.cmake)
include(cmake/options.cmake)
include(cmake/findDependencies.cmake)
include(cmake/compileroptions.cmake)
include(cmake/compilerDefinitions.cmake)
include(cmake/buildFiles.cmake)
include(cmake/cxx11.cmake)

use_cxx11()

set(addons
    __init__.py
    cert.py
    cppcheck.py
    cppcheckdata.py
    findcasts.py
    misc.py
    misra.py
    misra_9.py
    naming.py
    namingng.py
    runaddon.py
    threadsafety.py
    y2038.py
)
set(cfgs
    avr.cfg
    bento4.cfg
    boost.cfg
    bsd.cfg
    cairo.cfg
    cppcheck-cfg.rng
    cppcheck-lib.cfg
    cppunit.cfg
    daca.cfg
    dpdk.cfg
    embedded_sql.cfg
    emscripten.cfg
    ginac.cfg
    gnu.cfg
    googletest.cfg
    gtk.cfg
    icu.cfg
    kde.cfg
    libcerror.cfg
    libcurl.cfg
    libsigc++.cfg
    lua.cfg
    mfc.cfg
    microsoft_atl.cfg
    microsoft_sal.cfg
    microsoft_unittest.cfg
    motif.cfg
    nspr.cfg
    ntl.cfg
    opencv2.cfg
    opengl.cfg
    openmp.cfg
    openssl.cfg
    pcre.cfg
    posix.cfg
    python.cfg
    qt.cfg
    ruby.cfg
    sdl.cfg
    sfml.cfg
    sqlite3.cfg
    std.cfg
    tinyxml2.cfg
    vcl.cfg
    windows.cfg
    wxsqlite3.cfg
    wxsvg.cfg
    wxwidgets.cfg
    zlib.cfg
)
set(platforms
    aix_ppc64.xml
    arm32-wchar_t2.xml
    arm32-wchar_t4.xml
    arm64-wchar_t2.xml
    arm64-wchar_t4.xml
    avr8.xml
    cppcheck-platforms.rng
    cray_sv1.xml
    elbrus-e1cp.xml
    mips32.xml
    msp430_eabi_large_datamodel.xml
    pic8.xml
    pic8-enhanced.xml
    pic16.xml
    unix32-unsigned.xml
    unix64-unsigned.xml
)

find_program(LIBXML2_XMLLINT_EXECUTABLE xmllint)
if (LIBXML2_XMLLINT_EXECUTABLE)
    add_custom_target(validateCFG ${LIBXML2_XMLLINT_EXECUTABLE} --noout ${CMAKE_SOURCE_DIR}/cfg/cppcheck-cfg.rng)
    foreach(cfg ${cfgs})
        get_filename_component(cfgname ${cfg} NAME_WE)
        add_custom_target(validateCFG-${cfgname} ${LIBXML2_XMLLINT_EXECUTABLE} --noout --relaxng ${CMAKE_SOURCE_DIR}/cfg/cppcheck-cfg.rng ${cfg})
        add_dependencies(validateCFG validateCFG-${cfgname})
    endforeach()
endif()

if (BUILD_TESTS)
    enable_testing()
endif()

add_custom_target(copy_cfg ALL
    ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/cfg"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/cfg"
    COMMENT "Copying cfg files to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}")

add_custom_target(copy_addons ALL
    ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/addons"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/addons"
    COMMENT "Copying addons files to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}")

if(USE_BUNDLED_TINYXML2)
    message(STATUS "Using bundled version of tinyxml2")
    add_subdirectory(externals/tinyxml2)
endif()
add_subdirectory(externals/simplecpp)
add_subdirectory(lib)   # CppCheck Library 
add_subdirectory(cli)   # Client application
add_subdirectory(test)  # Tests
add_subdirectory(gui)   # Graphical application
add_subdirectory(tools/triage)   # Triage tool
add_subdirectory(oss-fuzz) # OSS-Fuzz clients
add_subdirectory(tools)

include(cmake/printInfo.cmake)
include(cmake/clang_tidy.cmake)
