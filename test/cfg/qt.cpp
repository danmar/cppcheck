
// Test library configuration for qt.cfg
//
// Usage:
// $ cppcheck --check-library --enable=information --inconclusive --error-exitcode=1 --suppress=missingIncludeSystem --inline-suppr --library=qt test/cfg/qt.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <QObject>
#include <QString>
#include <QtPlugin>
#include <QFile>
#include <cstdio>
#include <QCoreApplication>


void QString1(QString s)
{
    for (int i = 0; i <= s.size(); ++i) {
        // cppcheck-suppress stlOutOfBounds
        s[i] = 'x';
    }
}

int QString2()
{
    QString s;
    // FIXME cppcheck-suppress reademptycontainer
    return s.size();
}

// Verify that Qt macros do not result in syntax errors, false positives or other issues.
class MacroTest1: public QObject {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.foo.bar" FILE "test.json")

public:
    explicit MacroTest1(QObject *parent = 0);
    ~MacroTest1();
};

class MacroTest2 {
    Q_DECLARE_TR_FUNCTIONS(MacroTest2)

public:
    MacroTest2();
    ~MacroTest2();
};

void MacroTest2_test()
{
    // TODO: remove suppression when #9002 is fixed
    // cppcheck-suppress checkLibraryFunction
    QString str = MacroTest2::tr("hello");
    QByteArray ba = str.toLatin1();
    printf(ba.data());

#ifndef QT_NO_DEPRECATED
    // TODO: remove suppression when #9002 is fixed
    // cppcheck-suppress checkLibraryFunction
    str = MacroTest2::trUtf8("test2");
    ba = str.toLatin1();
    printf(ba.data());
#endif
}

void validCode(int * pIntPtr)
{
    if (QFile::exists("test")) {
    }

    if (pIntPtr != Q_NULLPTR) {
        *pIntPtr = 5;
    }

    if (pIntPtr && *pIntPtr == 1) {
        forever {
        }
    } else if (pIntPtr && *pIntPtr == 2) {
        Q_FOREVER {
        }
    }

    if (Q_LIKELY(pIntPtr)) {}
    if (Q_UNLIKELY(!pIntPtr)) {}

    printf(QT_TR_NOOP("Hi"));
}

void ignoredReturnValue()
{
    // cppcheck-suppress ignoredReturnValue
    QFile::exists("test");
    QFile file1("test");
    // cppcheck-suppress ignoredReturnValue
    file1.exists();
}

void nullPointer(int * pIntPtr)
{
    int * pNullPtr = Q_NULLPTR;
    // cppcheck-suppress nullPointer
    *pNullPtr = 1;

    if (pIntPtr != Q_NULLPTR) {
        *pIntPtr = 2;
    } else {
        // cppcheck-suppress nullPointerRedundantCheck
        *pIntPtr = 3;
    }
}
