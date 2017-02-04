find_package(Qt4 4.6.1 COMPONENTS QtCore QtGui QtXML)
if (HAVE_RULES)
    find_library(PCRE pcre)
    if (NOT PCRE)
        message(FATAL_ERROR "pcre dependency for RULES has not been found")
    endif()
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC OFF)

if (NOT ${USE_MATCHCOMPILER_OPT} STREQUAL "Off")
    find_package(PythonInterp)
    if (NOT ${PYTHONINTERP_FOUND})
        message(WARNING "No python interpreter found. Therefore, the match compiler is switched off.")
        set(USE_MATCHCOMPILER_OPT "Off")
    endif()
endif()

