lessThan(QT_MAJOR_VERSION, 5): error(requires >= Qt 5 (You used: $$QT_VERSION))

QT += core gui widgets

TARGET = triage
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11
INCLUDEPATH += ../../gui

MOC_DIR = temp
OBJECTS_DIR = temp
UI_DIR = temp

SOURCES += main.cpp\
        mainwindow.cpp \
        ../../gui/codeeditorstyle.cpp \
        ../../gui/codeeditor.cpp

HEADERS  += mainwindow.h \
        ../../gui/codeeditorstyle.h \
        ../../gui/codeeditor.h

FORMS    += mainwindow.ui
