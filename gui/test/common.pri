CONFIG += qtestlib
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets # In Qt 5 widgets are in separate module
    QT += printsupport # In Qt 5 QPrinter/QPrintDialog are in separate module
}
INCLUDEPATH += $${PWD}/..

LIBS += -L$$PWD/../../externals
INCLUDEPATH += $${PWD}/../../externals

include($${PWD}/../../lib/lib.pri)

# GUI
SOURCES += $${PWD}/../erroritem.cpp \
    $${PWD}/../filelist.cpp \
    $${PWD}/../projectfile.cpp \
    $${PWD}/../report.cpp \
    $${PWD}/../translationhandler.cpp \
    $${PWD}/../xmlreport.cpp \
    $${PWD}/../xmlreportv1.cpp \
    $${PWD}/../xmlreportv2.cpp

HEADERS += $${PWD}/../erroritem.h \
    $${PWD}/../filelist.h \
    $${PWD}/../projectfile.h \
    $${PWD}/../report.h \
    $${PWD}/../translationhandler.h \
    $${PWD}/../xmlreport.h \
    $${PWD}/../xmlreportv1.h \
    $${PWD}/../xmlreportv2.h

contains(QMAKE_CC, gcc) {
    QMAKE_CXXFLAGS += -std=c++11
}

contains(QMAKE_CXX, clang++) {
    QMAKE_CXXFLAGS += -std=c++11
}
