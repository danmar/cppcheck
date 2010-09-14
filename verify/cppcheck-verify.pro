TARGET = cppcheck-verify
TEMPLATE = app
INCLUDEPATH += ../lib
SOURCES += main.cpp \
    mainwindow.cpp \
    ../lib/tokenize.cpp \
    ../lib/token.cpp \
    ../lib/settings.cpp \
    ../lib/preprocessor.cpp \
    ../lib/path.cpp \
    ../lib/mathlib.cpp \
    ../lib/filelister_win32.cpp \
    ../lib/filelister_unix.cpp \
    ../lib/filelister.cpp \
    ../lib/errorlogger.cpp \
    codeeditor.cpp
HEADERS += mainwindow.h \
    ../lib/tokenize.h \
    ../lib/token.h \
    ../lib/settings.h \
    ../lib/preprocessor.h \
    ../lib/path.h \
    ../lib/mathlib.h \
    ../lib/filelister_win32.h \
    ../lib/filelister_unix.h \
    ../lib/filelister.h \
    ../lib/errorlogger.h \
    codeeditor.h
FORMS += mainwindow.ui
