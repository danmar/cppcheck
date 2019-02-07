
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

void validCode(int * pIntPtr)
{
    if (QFile::exists("test")) {
    }

    if (pIntPtr != Q_NULLPTR) {
        *pIntPtr = 5;
    }
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
