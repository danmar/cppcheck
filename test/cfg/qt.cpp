
// Test library configuration for qt.cfg
//
// Usage:
// $ cppcheck --check-library --enable=information --inconclusive --error-exitcode=1 --suppress=missingIncludeSystem --inline-suppr --library=qt test/cfg/qt.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <QObject>
#include <QString>
#include <QVector>
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

void QList1(QList<int> intListArg)
{
    for (int i = 0; i <= intListArg.size(); ++i) {
        // cppcheck-suppress stlOutOfBounds
        intListArg[i] = 1;
    }
    // cppcheck-suppress containerOutOfBoundsIndexExpression
    intListArg[intListArg.length()] = 5;
    // cppcheck-suppress containerOutOfBoundsIndexExpression
    intListArg[intListArg.count()] = 10;
    // cppcheck-suppress containerOutOfBoundsIndexExpression
    printf("val: %d\n", intListArg[intListArg.size()]);

    QList<QString> qstringList1{"one", "two"};
    (void)qstringList1[1];

    QList<QString> qstringList2 = {"one", "two"};
    (void)qstringList2[1];
    qstringList2.clear();
    // TODO: cppcheck-suppress containerOutOfBounds #9243
    (void)qstringList2[1];

    QList<QString> qstringList3;
    qstringList3 << "one" << "two";
    //(void)qstringList3[1]; // TODO: no containerOutOfBounds error should be shown #9242
    // cppcheck-suppress ignoredReturnValue
    qstringList3.startsWith("one");
    // cppcheck-suppress ignoredReturnValue
    qstringList3.endsWith("one");
    // cppcheck-suppress ignoredReturnValue
    qstringList3.count();
    // cppcheck-suppress ignoredReturnValue
    qstringList3.length();
    // cppcheck-suppress ignoredReturnValue
    qstringList3.size();
    // cppcheck-suppress ignoredReturnValue
    qstringList3.at(5);
    // cppcheck-suppress invalidFunctionArg
    (void)qstringList3.at(-5);

    QList<QString> qstringList4;
    // cppcheck-suppress containerOutOfBounds
    (void)qstringList4[0];
    qstringList4.append("a");
    (void)qstringList4[0];
    qstringList4.clear();
    // TODO: cppcheck-suppress containerOutOfBounds #9243
    (void)qstringList4[0];
}

void QVector1(QVector<int> intVectorArg)
{
    for (int i = 0; i <= intVectorArg.size(); ++i) {
        // cppcheck-suppress stlOutOfBounds
        intVectorArg[i] = 1;
    }
    // cppcheck-suppress containerOutOfBoundsIndexExpression
    intVectorArg[intVectorArg.length()] = 5;
    // cppcheck-suppress containerOutOfBoundsIndexExpression
    intVectorArg[intVectorArg.count()] = 10;
    // cppcheck-suppress containerOutOfBoundsIndexExpression
    printf("val: %d\n", intVectorArg[intVectorArg.size()]);

    QVector<QString> qstringVector1{"one", "two"};
    (void)qstringVector1[1];

    QVector<QString> qstringVector2 = {"one", "two"};
    (void)qstringVector2[1];

    QVector<QString> qstringVector3;
    qstringVector3 << "one" << "two";
    //(void)qstringVector3[1]; // TODO: no containerOutOfBounds error should be shown #9242
    // cppcheck-suppress ignoredReturnValue
    qstringVector3.startsWith("one");
    // cppcheck-suppress ignoredReturnValue
    qstringVector3.endsWith("one");
    // cppcheck-suppress ignoredReturnValue
    qstringVector3.count();
    // cppcheck-suppress ignoredReturnValue
    qstringVector3.length();
    // cppcheck-suppress ignoredReturnValue
    qstringVector3.size();
    // cppcheck-suppress ignoredReturnValue
    qstringVector3.at(5);
    // cppcheck-suppress invalidFunctionArg
    (void)qstringVector3.at(-5);
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
