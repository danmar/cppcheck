
// Test library configuration for qt.cfg
//
// Usage:
// $ cppcheck --check-library --library=qt --enable=style,information --inconclusive --error-exitcode=1 --disable=missingInclude --inline-suppr test/cfg/qt.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <QObject>
#include <QString>
#include <QVector>
#include <QStack>
#include <QByteArray>
#include <QList>
#include <QLinkedList>
#include <QtPlugin>
#include <QFile>
#include <cstdio>
#include <QCoreApplication>
#include <QLoggingCategory>
#include <QTest>


void QString1(QString s)
{
    for (int i = 0; i <= s.size(); ++i) {
        // cppcheck-suppress stlOutOfBounds
        s[i] = 'x';
    }
}

bool QString2()
{
    QString s;
    // cppcheck-suppress knownConditionTrueFalse
    return s.size();
}

QString::iterator QString3()
{
    QString qstring1;
    QString qstring2;
    // cppcheck-suppress mismatchingContainers
    for (QString::iterator it = qstring1.begin(); it != qstring2.end(); ++it)
    {}

    QString::iterator it = qstring1.begin();
    // cppcheck-suppress returnDanglingLifetime
    return it;
}

void QString4()
{
    // cppcheck-suppress unusedVariable
    QString qs;
}

// cppcheck-suppress passedByValue
bool QString5(QString s) { // #10710
    return s.isEmpty();
}

// cppcheck-suppress passedByValue
QStringList QString6(QString s) {
    return QStringList{ "*" + s + "*" };
}

// cppcheck-suppress passedByValue
bool QString7(QString s, const QString& l) {
    return l.startsWith(s);
}

void QByteArray1(QByteArray byteArrayArg)
{
    for (int i = 0; i <= byteArrayArg.size(); ++i) {
        // cppcheck-suppress stlOutOfBounds
        byteArrayArg[i] = 'x';
    }

    // cppcheck-suppress containerOutOfBoundsIndexExpression
    byteArrayArg[byteArrayArg.length()] = 'a';
    // cppcheck-suppress containerOutOfBoundsIndexExpression
    byteArrayArg[byteArrayArg.count()] = 'b';
    // cppcheck-suppress containerOutOfBoundsIndexExpression
    printf("val: %c\n", byteArrayArg[byteArrayArg.size()]);

    QByteArray byteArray1{'a', 'b'};
    (void)byteArray1[1];
    // cppcheck-suppress ignoredReturnValue
    byteArray1.at(1);
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
    // cppcheck-suppress containerOutOfBounds
    (void)qstringList2[1];

    QList<QString> qstringList3;
    qstringList3 << "one" << "two";
    (void)qstringList3[1];
    // FIXME: The following should have a containerOutOfBounds suppression #9242
    (void)qstringList3[3];
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
    // cppcheck-suppress containerOutOfBounds
    (void)qstringList4[0];
}

QList<int> QList2() { // #10556
    QList<int> v;

    for (int i = 0; i < 4; ++i)
    {
        v.append(i);
        (void)v.at(i);
    }
    return v;
}

QList<int>::iterator QList3()
{
    QList<int> qlist1;
    QList<int> qlist2;
    // cppcheck-suppress mismatchingContainers
    for (QList<int>::iterator it = qlist1.begin(); it != qlist2.end(); ++it)
    {}

    QList<int>::iterator it = qlist1.begin();
    // cppcheck-suppress returnDanglingLifetime
    return it;
}

void QLinkedList1()
{
    // cppcheck-suppress unreadVariable
    QLinkedList<QString> qstringLinkedList1{"one", "two"};

    QLinkedList<QString> qstringLinkedList2 = {"one", "two"};
    qstringLinkedList2.clear();

    QLinkedList<QString> qstringLinkedList3;
    qstringLinkedList3 << "one" << "two";
    // cppcheck-suppress ignoredReturnValue
    qstringLinkedList3.startsWith("one");
    // cppcheck-suppress ignoredReturnValue
    qstringLinkedList3.endsWith("one");
    // cppcheck-suppress ignoredReturnValue
    qstringLinkedList3.count();
    // cppcheck-suppress ignoredReturnValue
    qstringLinkedList3.size();

    QLinkedList<QString> qstringLinkedList4;
    qstringLinkedList4.append("a");
    qstringLinkedList4.clear();
}

QLinkedList<int>::iterator QLinkedList3()
{
    QLinkedList<int> intQLinkedList1;
    QLinkedList<int> intQLinkedList2;
    // cppcheck-suppress mismatchingContainers
    for (QLinkedList<int>::iterator it = intQLinkedList1.begin(); it != intQLinkedList2.end(); ++it)
    {}

    QLinkedList<int>::iterator it = intQLinkedList1.begin();
    // cppcheck-suppress returnDanglingLifetime
    return it;
}

void QStringList1(QStringList stringlistArg)
{
    for (int i = 0; i <= stringlistArg.size(); ++i) {
        // cppcheck-suppress stlOutOfBounds
        stringlistArg[i] = "abc";
    }
    // cppcheck-suppress containerOutOfBoundsIndexExpression
    stringlistArg[stringlistArg.length()] = "ab";
    stringlistArg[stringlistArg.length() - 1] = "ab"; // could be valid
    // cppcheck-suppress containerOutOfBoundsIndexExpression
    stringlistArg[stringlistArg.count()] = "12";
    stringlistArg[stringlistArg.count() - 1] = "12"; // could be valid
    // cppcheck-suppress containerOutOfBoundsIndexExpression
    (void)stringlistArg[stringlistArg.size()];
    (void)stringlistArg[stringlistArg.size() - 1]; // could be valid

    QStringList qstringlist1{"one", "two"};
    (void)qstringlist1[1];

    QStringList qstringlist2 = {"one", "two"};
    (void)qstringlist2[1];

    QStringList qstringlist3;
    qstringlist3 << "one" << "two";
    (void)qstringlist3[1];
    // FIXME: The following should have a containerOutOfBounds suppression #9242
    (void)qstringlist3[3];
    // cppcheck-suppress ignoredReturnValue
    qstringlist3.startsWith("one");
    // cppcheck-suppress ignoredReturnValue
    qstringlist3.endsWith("one");
    // cppcheck-suppress ignoredReturnValue
    qstringlist3.count();
    // cppcheck-suppress ignoredReturnValue
    qstringlist3.length();
    // cppcheck-suppress ignoredReturnValue
    qstringlist3.size();
    // cppcheck-suppress ignoredReturnValue
    qstringlist3.at(5);
    // cppcheck-suppress invalidFunctionArg
    (void)qstringlist3.at(-5);
}

QStringList::iterator QStringList2()
{
    QStringList qstringlist1;
    QStringList qstringlist2;
    // cppcheck-suppress mismatchingContainers
    for (QStringList::iterator it = qstringlist1.begin(); it != qstringlist2.end(); ++it)
    {}

    QStringList::iterator it = qstringlist1.begin();
    // cppcheck-suppress returnDanglingLifetime
    return it;
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
    (void)qstringVector3[1];
    // FIXME: The following should have a containerOutOfBounds suppression #9242
    (void)qstringVector3[3];
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

QVector<int>::iterator QVector2()
{
    QVector<int> qvector1;
    QVector<int> qvector2;
    // cppcheck-suppress mismatchingContainers
    for (QVector<int>::iterator it = qvector1.begin(); it != qvector2.end(); ++it)
    {}

    QVector<int>::iterator it = qvector1.begin();
    // cppcheck-suppress returnDanglingLifetime
    return it;
}

// cppcheck-suppress passedByValue
void duplicateExpression_QString_Compare(QString style) //#8723
{
    // cppcheck-suppress duplicateExpression
    if (style.compare( "x", Qt::CaseInsensitive ) == 0 || style.compare( "x", Qt::CaseInsensitive ) == 0)
    {}
}

void QStack1(QStack<int> intStackArg)
{
    for (int i = 0; i <= intStackArg.size(); ++i) {
        // cppcheck-suppress stlOutOfBounds
        intStackArg[i] = 1;
    }
    // cppcheck-suppress containerOutOfBoundsIndexExpression
    intStackArg[intStackArg.length()] = 5;
    // cppcheck-suppress containerOutOfBoundsIndexExpression
    intStackArg[intStackArg.count()] = 10;
    // cppcheck-suppress containerOutOfBoundsIndexExpression
    printf("val: %d\n", intStackArg[intStackArg.size()]);

    QStack<QString> qstringStack1;
    qstringStack1.push("one");
    qstringStack1.push("two");
    (void)qstringStack1[1];

    QStack<QString> qstringStack2;
    qstringStack2 << "one" << "two";
    (void)qstringStack2[1];
    // FIXME: The following should have a containerOutOfBounds suppression #9242
    (void)qstringStack2[3];
    // cppcheck-suppress ignoredReturnValue
    qstringStack2.startsWith("one");
    // cppcheck-suppress ignoredReturnValue
    qstringStack2.endsWith("one");
    // cppcheck-suppress ignoredReturnValue
    qstringStack2.count();
    // cppcheck-suppress ignoredReturnValue
    qstringStack2.length();
    // cppcheck-suppress ignoredReturnValue
    qstringStack2.size();
    // cppcheck-suppress ignoredReturnValue
    qstringStack2.at(5);
    // cppcheck-suppress invalidFunctionArg
    (void)qstringStack2.at(-5);
}

QStack<int>::iterator QStack2()
{
    QStack<int> qstack1;
    QStack<int> qstack2;
    // cppcheck-suppress mismatchingContainers
    for (QStack<int>::iterator it = qstack1.begin(); it != qstack2.end(); ++it)
    {}

    QStack<int>::iterator it = qstack1.begin();
    // cppcheck-suppress returnDanglingLifetime
    return it;
}

void QStack3()
{
    QStack<int> intStack;
    intStack.push(1);
    // cppcheck-suppress ignoredReturnValue
    intStack.top();
    intStack.pop();
}

// Verify that Qt macros do not result in syntax errors, false positives or other issues.
class MacroTest1 : public QObject {
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
    QString str = MacroTest2::tr("hello");
    QByteArray ba = str.toLatin1();
    printf(ba.data());

#ifndef QT_NO_DEPRECATED
    str = MacroTest2::trUtf8("test2");
    ba = str.toLatin1();
    printf(ba.data());
#endif
}

void MacroTest3()
{
    QByteArray message = QByteArrayLiteral("Test1");
    message += QByteArrayLiteral("Test2");
    QVERIFY2(2 >= 0, message.constData());
}

void validCode(int * pIntPtr, QString & qstrArg, double d)
{
    Q_UNUSED(d)
    if (QFile::exists("test")) {}

    if (pIntPtr != Q_NULLPTR) {
        *pIntPtr = 5;
    }

    if (pIntPtr && *pIntPtr == 1) {
        forever {
        }
    } else if (pIntPtr && *pIntPtr == 2) {
        Q_FOREVER {}
    }

    if (Q_LIKELY(pIntPtr)) {}
    if (Q_UNLIKELY(!pIntPtr)) {}

    printf(QT_TR_NOOP("Hi"));

    // cppcheck-suppress checkLibraryFunction
    Q_DECLARE_LOGGING_CATEGORY(logging_category_test);
    QT_FORWARD_DECLARE_CLASS(forwardDeclaredClass);
    QT_FORWARD_DECLARE_STRUCT(forwardDeclaredStruct);

    //#9650
    QString qstr1(qstrArg);
    if (qstr1.length() == 1) {} else {
        qstr1.chop(1);
        if (qstr1.length() == 1) {}
    }
    if (qstr1.length() == 1) {} else {
        qstr1.remove(1);
        if (qstr1.length() == 1) {}
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

namespace {
    class C : public QObject {
        Q_OBJECT
    public:
        explicit C(QObject* parent = nullptr) : QObject(parent) {}
        void signal() {}
    };
    class D : public QObject {
        Q_OBJECT
    public:
        D() {
            connect(new C(this), &C::signal, this, &D::slot);
        }
        void slot() {};
    };

    // findFunction11
    class Fred : public QObject {
        Q_OBJECT
    private slots:
        void foo();
    };
    void Fred::foo() {}

    // bitfields14
    class X {
    signals:
    };

    // simplifyQtSignalsSlots1
    class Counter1 : public QObject {
        Q_OBJECT
    public:
        Counter1() {
            m_value = 0;
        }
        int value() const {
            return m_value;
        }
    public slots:
        void setValue(int value);
    signals:
        void valueChanged(int newValue);
    private:
        int m_value;
    };
    void Counter1::setValue(int value) {
        if (value != m_value) {
            m_value = value;
            emit valueChanged(value);
        }
    }

    class Counter2 : public QObject {
        Q_OBJECT
    public:
        Counter2() {
            m_value = 0;
        }
        int value() const {
            return m_value;
        }
    public Q_SLOTS:
        void setValue(int value);
    Q_SIGNALS:
        void valueChanged(int newValue);
    private:
        int m_value;
    };
    void Counter2::setValue(int value) {
        if (value != m_value) {
            m_value = value;
            emit valueChanged(value);
        }
    }

    class MyObject1 : public QObject {
        MyObject1() {}
        ~MyObject1() {}
    public slots:
    signals:
        void test() {}
    };

    class MyObject2 : public QObject {
        Q_OBJECT
    public slots:
    };

    // simplifyQtSignalsSlots2
    namespace Foo { class Bar; }
    class Foo::Bar : public QObject { private slots: };

    // Q_PROPERTY with templates inducing a ',' should not produce a preprocessorErrorDirective
    class AssocProperty : public QObject {
    public:
        Q_PROPERTY(QHash<QString, int> hash READ hash WRITE setHash)
    };
}
