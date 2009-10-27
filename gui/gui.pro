TEMPLATE = app
TARGET = 
QT += xml
DEPENDPATH += .
INCLUDEPATH += .
RCC_DIR = temp
MOC_DIR = temp
OBJECTS_DIR = temp
UI_DIR = temp
CONFIG += warn_on
RESOURCES = gui.qrc
FORMS = main.ui \
	resultsview.ui \
	application.ui \
	settings.ui \
	file.ui	\
	about.ui
	
TRANSLATIONS =  cppcheck_fi.ts \
                cppcheck_nl.ts \
		cppcheck_en.ts \
		cppcheck_se.ts \
		cppcheck_de.ts \
		cppcheck_pl.ts \
		cppcheck_ru.ts 

# Windows-specific options
CONFIG += embed_manifest_exe

include($$PWD/../lib/lib.pri)
HEADERS += mainwindow.h \
           checkthread.h \
           resultsview.h \
           resultstree.h \
           settingsdialog.h \
           threadresult.h \
           threadhandler.h \
           applicationlist.h \
           applicationdialog.h \
           aboutdialog.h \
           common.h \
           fileviewdialog.h \
           projectfile.h \
           report.h \
           txtreport.h \
           xmlreport.h \
           translationhandler.h \
           csvreport.h
SOURCES += main.cpp \
           mainwindow.cpp\
           checkthread.cpp \
           resultsview.cpp \
           resultstree.cpp \
           threadresult.cpp \
           threadhandler.cpp \
           settingsdialog.cpp \
           applicationlist.cpp \
           applicationdialog.cpp \
           aboutdialog.cpp \
           fileviewdialog.cpp \
           projectfile.cpp \
           report.cpp \
           txtreport.cpp \
           xmlreport.cpp \
           translationhandler.cpp \
           csvreport.cpp

win32 {
	RC_FILE = cppcheck-gui.rc
	HEADERS += ../lib/resource.h
	LIBS += -lshlwapi
}

# run lrelease before build
lrelease.commands = lrelease gui.pro
QMAKE_EXTRA_TARGETS += lrelease
PRE_TARGETDEPS += lrelease
