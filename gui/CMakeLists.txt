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

# UI files
SET(CHECK_UIS
  about.ui
  application.ui
  file.ui
  main.ui
  resultsview.ui
  settings.ui
)

SET(CHECK_RCCS gui.qrc)

# Generate rules for building source files from the resources
QT4_ADD_RESOURCES(CHECK_RCC_SRCS ${CHECK_RCCS})

# Process UI files
QT4_WRAP_UI(CHECK_UIS_H ${CHECK_UIS})

# Mock header files
QT4_WRAP_CPP(CHECK_MOC_SRCS ${CHECK_HEADERS})

include_directories (${CPPCHECK_SOURCE_DIR}/lib)

# Include binary directory where code from UI files gets created
include_directories(${CMAKE_CURRENT_BINARY_DIR})

ADD_EXECUTABLE(gui ${CHECKGUI_SRCS} ${CHECK_MOC_SRCS} ${CHECK_UIS_H} ${CHECK_RCC_SRCS})
TARGET_LINK_LIBRARIES(gui checklib ${QT_LIBRARIES})

