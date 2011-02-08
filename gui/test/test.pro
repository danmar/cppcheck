TEMPLATE = app
TARGET = test
CONFIG += qtestlib
DEPENDPATH += . ..
INCLUDEPATH += . ..

# tests
SOURCES += main.cpp \
    testtranslationhandler.cpp \
    testxmlreport.cpp

HEADERS += testtranslationhandler.h \
    testxmlreport.h

# GUI
SOURCES += report.cpp \
    ../translationhandler.cpp \
    ../xmlreport.cpp
    
HEADERS += report.h \
    ../translationhandler.h \
    ../xmlreport.h
