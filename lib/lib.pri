# no manual edits - this file is autogenerated by dmake

include($$PWD/pcrerules.pri)
include($$PWD/../externals/externals.pri)
INCLUDEPATH += $$PWD
HEADERS += $${PWD}/addoninfo.h \
           $${PWD}/analyzer.h \
           $${PWD}/analyzerinfo.h \
           $${PWD}/astutils.h \
           $${PWD}/calculate.h \
           $${PWD}/check.h \
           $${PWD}/check64bit.h \
           $${PWD}/checkassert.h \
           $${PWD}/checkautovariables.h \
           $${PWD}/checkbool.h \
           $${PWD}/checkboost.h \
           $${PWD}/checkbufferoverrun.h \
           $${PWD}/checkclass.h \
           $${PWD}/checkcondition.h \
           $${PWD}/checkers.h \
           $${PWD}/checkersreport.h \
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
           $${PWD}/clangimport.h \
           $${PWD}/color.h \
           $${PWD}/config.h \
           $${PWD}/cppcheck.h \
           $${PWD}/ctu.h \
           $${PWD}/errorlogger.h \
           $${PWD}/errortypes.h \
           $${PWD}/filesettings.h \
           $${PWD}/findtoken.h \
           $${PWD}/forwardanalyzer.h \
           $${PWD}/fwdanalysis.h \
           $${PWD}/importproject.h \
           $${PWD}/infer.h \
           $${PWD}/json.h \
           $${PWD}/keywords.h \
           $${PWD}/library.h \
           $${PWD}/mathlib.h \
           $${PWD}/path.h \
           $${PWD}/pathanalysis.h \
           $${PWD}/pathmatch.h \
           $${PWD}/platform.h \
           $${PWD}/precompiled.h \
           $${PWD}/preprocessor.h \
           $${PWD}/programmemory.h \
           $${PWD}/reverseanalyzer.h \
           $${PWD}/settings.h \
           $${PWD}/smallvector.h \
           $${PWD}/standards.h \
           $${PWD}/summaries.h \
           $${PWD}/suppressions.h \
           $${PWD}/symboldatabase.h \
           $${PWD}/templatesimplifier.h \
           $${PWD}/timer.h \
           $${PWD}/token.h \
           $${PWD}/tokenize.h \
           $${PWD}/tokenlist.h \
           $${PWD}/tokenrange.h \
           $${PWD}/utils.h \
           $${PWD}/valueflow.h \
           $${PWD}/valueptr.h \
           $${PWD}/version.h \
           $${PWD}/vf_analyze.h \
           $${PWD}/vf_array.h \
           $${PWD}/vf_common.h \
           $${PWD}/vf_enumvalue.h \
           $${PWD}/vf_globalconstvar.h \
           $${PWD}/vf_globalstaticvar.h \
           $${PWD}/vf_number.h \
           $${PWD}/vf_settokenvalue.h \
           $${PWD}/vf_string.h \
           $${PWD}/vf_unknownfunctionreturn.h \
           $${PWD}/vfvalue.h \
           $${PWD}/xml.h

SOURCES += $${PWD}/valueflow.cpp \
           $${PWD}/tokenize.cpp \
           $${PWD}/symboldatabase.cpp \
           $${PWD}/addoninfo.cpp \
           $${PWD}/analyzerinfo.cpp \
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
           $${PWD}/checkers.cpp \
           $${PWD}/checkersreport.cpp \
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
           $${PWD}/clangimport.cpp \
           $${PWD}/color.cpp \
           $${PWD}/cppcheck.cpp \
           $${PWD}/ctu.cpp \
           $${PWD}/errorlogger.cpp \
           $${PWD}/errortypes.cpp \
           $${PWD}/forwardanalyzer.cpp \
           $${PWD}/fwdanalysis.cpp \
           $${PWD}/importproject.cpp \
           $${PWD}/infer.cpp \
           $${PWD}/keywords.cpp \
           $${PWD}/library.cpp \
           $${PWD}/mathlib.cpp \
           $${PWD}/path.cpp \
           $${PWD}/pathanalysis.cpp \
           $${PWD}/pathmatch.cpp \
           $${PWD}/platform.cpp \
           $${PWD}/preprocessor.cpp \
           $${PWD}/programmemory.cpp \
           $${PWD}/reverseanalyzer.cpp \
           $${PWD}/settings.cpp \
           $${PWD}/summaries.cpp \
           $${PWD}/suppressions.cpp \
           $${PWD}/templatesimplifier.cpp \
           $${PWD}/timer.cpp \
           $${PWD}/token.cpp \
           $${PWD}/tokenlist.cpp \
           $${PWD}/utils.cpp \
           $${PWD}/vf_array.cpp \
           $${PWD}/vf_common.cpp \
           $${PWD}/vf_enumvalue.cpp \
           $${PWD}/vf_globalconstvar.cpp \
           $${PWD}/vf_globalstaticvar.cpp \
           $${PWD}/vf_number.cpp \
           $${PWD}/vf_settokenvalue.cpp \
           $${PWD}/vf_string.cpp \
           $${PWD}/vf_unknownfunctionreturn.cpp \
           $${PWD}/vfvalue.cpp
