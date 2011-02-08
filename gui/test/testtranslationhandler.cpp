#include <QtTest>
#include <QObject>
#include "translationhandler.h"

class TestTranslationHandler: public QObject
{
    Q_OBJECT

private slots:
    void construct();
};

void TestTranslationHandler::construct()
{
    TranslationHandler handler;
    QCOMPARE(10, handler.GetNames().size());
    QCOMPARE(QString("en"), handler.GetCurrentLanguage());
}

#include "testtranslationhandler.moc"
QTEST_MAIN(TestTranslationHandler)
