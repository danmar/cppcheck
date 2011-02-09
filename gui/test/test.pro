TEMPLATE = app
TARGET = test
CONFIG += qtestlib
DEPENDPATH += . ..
INCLUDEPATH += . ..

# tests
SOURCES += main.cpp \
    testtranslationhandler.cpp \
    testxmlreport.cpp \
    testxmlreportv1.cpp

HEADERS += testtranslationhandler.h \
    testxmlreport.h \
    testxmlreportv1.h

# GUI
SOURCES += ../erroritem.cpp \
    ../report.cpp \
    ../translationhandler.cpp \
    ../xmlreport.cpp \
    ../xmlreportv1.cpp
    
HEADERS += ../erroritem.h \
    ../report.h \
    ../translationhandler.h \
    ../xmlreport.h \
    ../xmlreportv1.h
