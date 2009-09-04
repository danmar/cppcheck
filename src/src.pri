# Common project include with headers and sources

HEADERS += $$PWD/check.h \
           $$PWD/checkautovariables.h \
           $$PWD/checkbufferoverrun.h \
           $$PWD/checkclass.h \
           $$PWD/checkdangerousfunctions.h \
           $$PWD/checkheaders.h \
           $$PWD/checkmemoryleak.h \
           $$PWD/checkother.h \
           $$PWD/checkstl.h \
           $$PWD/checkunusedfunctions.h \
           $$PWD/classinfo.h \
           $$PWD/cppcheck.h \
           $$PWD/cppcheckexecutor.h \
           $$PWD/errorlogger.h \
           $$PWD/filelister.h \
           $$PWD/mathlib.h \
           $$PWD/preprocessor.h \
           $$PWD/resource.h \
           $$PWD/settings.h \
           $$PWD/threadexecutor.h \
           $$PWD/token.h \
           $$PWD/tokenize.h

# no main.cpp here
SOURCES += $$PWD/checkautovariables.cpp \
           $$PWD/checkbufferoverrun.cpp \
           $$PWD/checkclass.cpp \
           $$PWD/checkdangerousfunctions.cpp \
           $$PWD/checkheaders.cpp \
           $$PWD/checkmemoryleak.cpp \
           $$PWD/checkother.cpp \
           $$PWD/checkstl.cpp \
           $$PWD/checkunusedfunctions.cpp \
           $$PWD/cppcheck.cpp \
           $$PWD/cppcheckexecutor.cpp \
           $$PWD/errorlogger.cpp \
           $$PWD/filelister.cpp \
           $$PWD/mathlib.cpp \
           $$PWD/preprocessor.cpp \
           $$PWD/settings.cpp \
           $$PWD/threadexecutor.cpp \
           $$PWD/token.cpp \
           $$PWD/tokenize.cpp
