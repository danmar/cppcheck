#include "translationhandler.h"

#include <QApplication>
#include <QDebug>

TranslationHandler::TranslationHandler(QObject *parent) :
        QObject(parent),
        mCurrentLanguage(-1),
        mTranslator(new QTranslator(this))
{
    //Add our default languages
    mNames  << QObject::tr("English")
    << QObject::tr("Finnish")
    << QObject::tr("Swedish")
    << QObject::tr("German")
    << QObject::tr("Russian");

    mFiles  << "cppcheck_en"
    << "cppcheck_fi"
    << "cppcheck_se"
    << "cppcheck_de"
    << "cppcheck_ru";

    //Load english as a fallback language
    QTranslator *english = new QTranslator();
    if (english->load("cppcheck_en"))
    {
        qApp->installTranslator(english);
    }
    else
    {
        qDebug() << "Failed to load english translation!";
        delete english;
    }
}

TranslationHandler::~TranslationHandler()
{
}

const QStringList TranslationHandler::GetNames()
{
    return mNames;
}

const QStringList TranslationHandler::GetFiles()
{
    return mFiles;
}

bool TranslationHandler::SetLanguage(const int index, QString &error)
{
    //If english is the language
    if (index == 0)
    {
        //Just remove all extra translators
        if (mTranslator)
        {
            qApp->removeTranslator(mTranslator);
        }

        mCurrentLanguage = index;
        return true;
    }

    //Make sure the translator is otherwise valid
    if (index >= mNames.size())
    {
        error = QObject::tr("Incorrect language specified!");
        return false;
    }


    //Load the new language
    if (!mTranslator->load(mFiles[index]))
    {
        error = QObject::tr("Failed to load language from file %1");
        error = error.arg(mFiles[index]);
        return false;
    }

    qApp->installTranslator(mTranslator);

    mCurrentLanguage = index;

    return true;
}

int TranslationHandler::GetCurrentLanguage() const
{
    return mCurrentLanguage;
}

