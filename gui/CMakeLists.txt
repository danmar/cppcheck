# Minimal CMake build file to build cppcheck Qt GUI
find_package(PCRE REQUIRED)

# find and setup Qt4 for this project
find_package(Qt4)

IF(QT4_FOUND)

# Add needed Qt modules
set(QT_USE_QTHELP TRUE)
set(QT_USE_QTMAIN TRUE)
set(QT_USE_QTXML TRUE)
include(${QT_USE_FILE})

include_directories("${CPPCHECK_SOURCE_DIR}/lib"
# Generated files (in build directory) need to know gui directory
                    "${CPPCHECK_SOURCE_DIR}/gui"
# Include binary directory where code from UI files gets created
                    "${CMAKE_CURRENT_BINARY_DIR}"
                    "${PCRE_INCLUDE_DIR}")

# Header files - listed for mocking
SET(CPPCHECK_GUI_HEADERS
  aboutdialog.h
  applicationdialog.h
  applicationlist.h
  checkstatistics.h
  checkthread.h
  common.h
  csvreport.h
  erroritem.h
  filelist.h
  fileviewdialog.h
  helpwindow.h
  mainwindow.h
  logview.h
  project.h
  projectfile.h
  projectfiledialog.h
  report.h
  resultstree.h
  resultsview.h
  settingsdialog.h
  statsdialog.h
  threadhandler.h
  threadresult.h
  translationhandler.h
  txtreport.h
  xmlreport.h
)

# Source files
SET(CPPCHECK_GUI_SOURCES
  aboutdialog.cpp
  applicationdialog.cpp
  applicationlist.cpp
  checkstatistics.cpp
  checkthread.cpp
  csvreport.cpp
  erroritem.cpp
  filelist.cpp
  fileviewdialog.cpp
  helpwindow.cpp
  logview.cpp
  main.cpp
  mainwindow.cpp
  project.cpp
  projectfile.cpp
  projectfiledialog.cpp
  report.cpp
  resultstree.cpp
  resultsview.cpp
  settingsdialog.cpp
  statsdialog.cpp
  threadhandler.cpp
  threadresult.cpp
  translationhandler.cpp
  txtreport.cpp
  xmlreport.cpp
)

# UI files
SET(CPPCHECK_UIS
  about.ui
  application.ui
  file.ui
  helpwindow.ui
  logview.ui
  main.ui
  projectfile.ui
  resultsview.ui
  settings.ui
  stats.ui
)

# Translation files
SET(CPPCHECK_GUI_TRANS
  cppcheck_de.ts
  cppcheck_en.ts
  cppcheck_fi.ts
  cppcheck_fr.ts
  cppcheck_ja.ts
  cppcheck_nl.ts
  cppcheck_pl.ts
  cppcheck_ru.ts
  cppcheck_se.ts
  cppcheck_sr.ts
)

set(CPPCHECK_LIB_DIR "${CPPCHECK_SOURCE_DIR}/lib/")
include("${CPPCHECK_LIB_DIR}library_sources.cmake")

if(WIN32)
   # Add Windows resource file
   set(CPPCHECK_GUI_SOURCES ${CPPCHECK_GUI_SOURCES} cppcheck-gui.rc)

   if(NOT CYGWIN)
      # Windows needs additional shlwapi library.
      set(CPPCHECK_GUI_LINK_LIBRARIES ${CPPCHECK_GUI_LINK_LIBRARIES} shlwapi htmlhelp)
   endif()
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif (CMAKE_COMPILER_IS_GNUCXX)

# Generate rules for building source files from the resources
QT4_ADD_RESOURCES(CPPCHECK_RCC_SOURCES gui.qrc)

# Process UI files
QT4_WRAP_UI(CPPCHECK_FORMS_HEADERS ${CPPCHECK_UIS})

# Mock header files
QT4_WRAP_CPP(CPPCHECK_MOC_SOURCES ${CPPCHECK_GUI_HEADERS})

# add translations ...
QT4_ADD_TRANSLATION(CPPCHECK_QM ${CPPCHECK_GUI_TRANS})

# Create folders for Visual Studio IDE
SOURCE_GROUP("Header Files" FILES ${CPPCHECK_GUI_HEADERS})
SOURCE_GROUP("Ui Files" ".ui$")
SOURCE_GROUP("Moc Files" "moc_.*cxx$")

ADD_EXECUTABLE(gui WIN32 ${CPPCHECK_GUI_SOURCES} ${CPPCHECK_MOC_SOURCES}
               ${CPPCHECK_FORMS_HEADERS} ${CPPCHECK_RCC_SOURCES} ${CPPCHECK_LIB_SOURCES})
TARGET_LINK_LIBRARIES(gui ${CPPCHECK_GUI_LINK_LIBRARIES} ${PCRE_LIBRARIES} ${QT_LIBRARIES})

ELSE(QT4_FOUND)
message("GUI not built since QT4 not found.")
ENDIF(QT4_FOUND)
