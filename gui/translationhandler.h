#ifndef TRANSLATIONHANDLER_H
#define TRANSLATIONHANDLER_H

#include <QStringList>
#include <QTranslator>
#include <QObject>

class TranslationHandler : QObject
{
    Q_OBJECT
public:
    TranslationHandler(QObject *parent);
    virtual ~TranslationHandler();
    const QStringList GetNames();
    const QStringList GetFiles();
    bool SetLanguage(const int index, QString &error);
    int GetCurrentLanguage() const;
protected:
    int mCurrentLanguage;
    QStringList mNames;
    QStringList mFiles;
    QTranslator *mTranslator;
private:
};

#endif // TRANSLATIONHANDLER_H
