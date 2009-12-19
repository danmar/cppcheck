# Minimal CMake build file to build cppcheck Qt GUI

# find and setup Qt4 for this project
find_package(Qt4 REQUIRED)

# Add needed Qt modules
set(QT_USE_QTMAIN TRUE)
set(QT_USE_QTXML TRUE)
include(${QT_USE_FILE})

# Header files - listed for mocking
SET(CHECK_HEADERS
  aboutdialog.h
  applicationdialog.h
  applicationlist.h
  checkthread.h
  common.h
  csvreport.h
  fileviewdialog.h
  mainwindow.h
  projectfile.h
  report.h
  resultstree.h
  resultsview.h
  settingsdialog.h
  threadhandler.h
  threadresult.h
  translationhandler.h
  txtreport.h
  xmlreport.h
)

# Source files
SET(CHECKGUI_SRCS
  aboutdialog.cpp
  applicationdialog.cpp 
  applicationlist.cpp
  checkthread.cpp
  csvreport.cpp
  fileviewdialog.cpp
  main.cpp
  mainwindow.cpp
  projectfile.cpp
  report.cpp
  resultstree.cpp
  resultsview.cpp
  settingsdialog.cpp
  threadhandler.cpp
  threadresult.cpp
  translationhandler.cpp
  txtreport.cpp
  xmlreport.cpp
)

# Add Windows resource file
if (WIN32)
  SET(CHECKGUI_SRCS
    ${CHECKGUI_SRCS}
    cppcheck-gui.rc
  )
endif (WIN32)

# UI files
SET(CHECK_UIS
  about.ui
  application.ui
  file.ui
  main.ui
  resultsview.ui
  settings.ui
)

# Translation files
SET(CHECK_TRANS
  cppcheck_de.ts
  cppcheck_fi.ts
  cppcheck_pl.ts
  cppcheck_se.ts
  cppcheck_en.ts
  cppcheck_nl.ts
  cppcheck_ru.ts
)

SET(CHECK_RCCS gui.qrc)

# Libraries to link
set(CHECK_LIBS
  checklib
)

# Windows needs additional shlwapi library
if (WIN32 AND NOT CYGWIN)
set(CHECK_LIBS
  ${CHECK_LIBS}
  shlwapi
)
endif (WIN32 AND NOT CYGWIN)

# Generate rules for building source files from the resources
QT4_ADD_RESOURCES(CHECK_RCC_SRCS ${CHECK_RCCS})

# Process UI files
QT4_WRAP_UI(CHECK_UIS_H ${CHECK_UIS})

# Mock header files
QT4_WRAP_CPP(CHECK_MOC_SRCS ${CHECK_HEADERS})

# add translations ...
QT4_ADD_TRANSLATION(CHECK_QM ${CHECK_TRANS})

include_directories (${CPPCHECK_SOURCE_DIR}/lib)

# Include binary directory where code from UI files gets created
include_directories(${CMAKE_CURRENT_BINARY_DIR})

ADD_EXECUTABLE(gui ${CHECKGUI_SRCS} ${CHECK_MOC_SRCS} ${CHECK_UIS_H}
  ${CHECK_RCC_SRCS} ${CHECK_QM})
TARGET_LINK_LIBRARIES(gui ${CHECK_LIBS} ${QT_LIBRARIES})

