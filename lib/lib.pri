# no manual edits - this file is autogenerated by dmake

include($$PWD/pcrerules.pri)
include($$PWD/../externals/externals.pri)
INCLUDEPATH += $$PWD
HEADERS += $${PWD}/analyzerinfo.h \
           $${PWD}/astutils.h \
           $${PWD}/check.h \
           $${PWD}/check64bit.h \
           $${PWD}/checkassert.h \
           $${PWD}/checkautovariables.h \
           $${PWD}/checkbool.h \
           $${PWD}/checkboost.h \
           $${PWD}/checkbufferoverrun.h \
           $${PWD}/checkclass.h \
           $${PWD}/checkcondition.h \
           $${PWD}/checkexceptionsafety.h \
           $${PWD}/checkfunctions.h \
           $${PWD}/checkinternal.h \
           $${PWD}/checkio.h \
           $${PWD}/checkleakautovar.h \
           $${PWD}/checkmemoryleak.h \
           $${PWD}/checknullpointer.h \
           $${PWD}/checkother.h \
           $${PWD}/checkpostfixoperator.h \
           $${PWD}/checksizeof.h \
           $${PWD}/checkstl.h \
           $${PWD}/checkstring.h \
           $${PWD}/checktype.h \
           $${PWD}/checkuninitvar.h \
           $${PWD}/checkunusedfunctions.h \
           $${PWD}/checkunusedvar.h \
           $${PWD}/checkvaarg.h \
           $${PWD}/clangastdump.h \
           $${PWD}/cppcheck.h \
           $${PWD}/ctu.h \
           $${PWD}/errorlogger.h \
           $${PWD}/exprengine.h \
           $${PWD}/importproject.h \
           $${PWD}/library.h \
           $${PWD}/mathlib.h \
           $${PWD}/path.h \
           $${PWD}/pathanalysis.h \
           $${PWD}/pathmatch.h \
           $${PWD}/platform.h \
           $${PWD}/preprocessor.h \
           $${PWD}/programmemory.h \
           $${PWD}/settings.h \
           $${PWD}/suppressions.h \
           $${PWD}/symboldatabase.h \
           $${PWD}/templatesimplifier.h \
           $${PWD}/timer.h \
           $${PWD}/token.h \
           $${PWD}/tokenize.h \
           $${PWD}/tokenlist.h \
           $${PWD}/valueflow.h

SOURCES += $${PWD}/analyzerinfo.cpp \
           $${PWD}/astutils.cpp \
           $${PWD}/check.cpp \
           $${PWD}/check64bit.cpp \
           $${PWD}/checkassert.cpp \
           $${PWD}/checkautovariables.cpp \
           $${PWD}/checkbool.cpp \
           $${PWD}/checkboost.cpp \
           $${PWD}/checkbufferoverrun.cpp \
           $${PWD}/checkclass.cpp \
           $${PWD}/checkcondition.cpp \
           $${PWD}/checkexceptionsafety.cpp \
           $${PWD}/checkfunctions.cpp \
           $${PWD}/checkinternal.cpp \
           $${PWD}/checkio.cpp \
           $${PWD}/checkleakautovar.cpp \
           $${PWD}/checkmemoryleak.cpp \
           $${PWD}/checknullpointer.cpp \
           $${PWD}/checkother.cpp \
           $${PWD}/checkpostfixoperator.cpp \
           $${PWD}/checksizeof.cpp \
           $${PWD}/checkstl.cpp \
           $${PWD}/checkstring.cpp \
           $${PWD}/checktype.cpp \
           $${PWD}/checkuninitvar.cpp \
           $${PWD}/checkunusedfunctions.cpp \
           $${PWD}/checkunusedvar.cpp \
           $${PWD}/checkvaarg.cpp \
           $${PWD}/clangastdump.cpp \
           $${PWD}/cppcheck.cpp \
           $${PWD}/ctu.cpp \
           $${PWD}/errorlogger.cpp \
           $${PWD}/exprengine.cpp \
           $${PWD}/importproject.cpp \
           $${PWD}/library.cpp \
           $${PWD}/mathlib.cpp \
           $${PWD}/path.cpp \
           $${PWD}/pathanalysis.cpp \
           $${PWD}/pathmatch.cpp \
           $${PWD}/platform.cpp \
           $${PWD}/preprocessor.cpp \
           $${PWD}/programmemory.cpp \
           $${PWD}/settings.cpp \
           $${PWD}/suppressions.cpp \
           $${PWD}/symboldatabase.cpp \
           $${PWD}/templatesimplifier.cpp \
           $${PWD}/timer.cpp \
           $${PWD}/token.cpp \
           $${PWD}/tokenize.cpp \
           $${PWD}/tokenlist.cpp \
           $${PWD}/valueflow.cpp
