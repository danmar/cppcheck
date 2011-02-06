/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QApplication>
#include <QFile>
#include <QDebug>
#include <QLocale>
#include "translationhandler.h"

TranslationHandler::TranslationHandler(QObject *parent) :
    QObject(parent),
    mCurrentLanguage(-1),
    mTranslator(new QTranslator(this))
{
    // Add our available languages
    // Keep this list sorted
    AddTranslation(QT_TRANSLATE_NOOP("MainWindow", "Dutch"), "cppcheck_nl");
    AddTranslation(QT_TRANSLATE_NOOP("MainWindow", "English"), "cppcheck_en");
    AddTranslation(QT_TRANSLATE_NOOP("MainWindow", "Finnish"), "cppcheck_fi");
    AddTranslation(QT_TRANSLATE_NOOP("MainWindow", "German"), "cppcheck_de");
    AddTranslation(QT_TRANSLATE_NOOP("MainWindow", "Japanese"), "cppcheck_ja");
    AddTranslation(QT_TRANSLATE_NOOP("MainWindow", "Polish"), "cppcheck_pl");
    AddTranslation(QT_TRANSLATE_NOOP("MainWindow", "Russian"), "cppcheck_ru");
    AddTranslation(QT_TRANSLATE_NOOP("MainWindow", "Serbian"), "cppcheck_sr");
    AddTranslation(QT_TRANSLATE_NOOP("MainWindow", "Swedish"), "cppcheck_se");

    //Load English as a fallback language
    QTranslator *english = new QTranslator();
    if (english->load("cppcheck_en"))
    {
        qApp->installTranslator(english);
    }
    else
    {
        qDebug() << "Failed to load English translation!";
        delete english;
    }
}

TranslationHandler::~TranslationHandler()
{
}

const QStringList TranslationHandler::GetNames() const
{
    QStringList names;
    foreach(TranslationInfo translation, mTranslations)
    {
        names.append(translation.mName);
    }
    return names;
}

bool TranslationHandler::SetLanguage(const int index, QString &error)
{
    //If English is the language
    if (index == 2)
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
    if (index >= mTranslations.size())
    {
        error = QObject::tr("Incorrect language specified!");
        return false;
    }

    //Load the new language
    if (!mTranslator->load(mTranslations[index].mFilename))
    {
        //If it failed, lets check if the default file exists
        if (!QFile::exists(mTranslations[index].mFilename + ".qm"))
        {
            error = QObject::tr("Language file %1 not found!");
            error = error.arg(mTranslations[index].mFilename + ".qm");
            return false;
        }

        //If file exists, there's something wrong with it
        error = QObject::tr("Failed to load translation for language %1 from file %2");
        error = error.arg(mTranslations[index].mName);
        error = error.arg(mTranslations[index].mFilename + ".qm");
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

int TranslationHandler::SuggestLanguage() const
{
    /*
    Get language from system locale's name
    QLocale::languageToString would return the languages full name and we
    only want two-letter ISO 639 language code so we'll get it from
    locale's name.
    */
    QString language = QLocale::system().name().left(2);
    //qDebug()<<"Your language is"<<language;

    //catenate that to the default language filename
    QString file = QString("cppcheck_%1").arg(language);
    //qDebug()<<"Language file could be"<<file;


    //And see if we can find it from our list of language files
    int index = 0;
    for (int i = 0; i < mTranslations.size(); i++)
    {
        if (mTranslations[i].mFilename == file)
        {
            index = i;
            break;
        }
    }

    //If nothing found, return English
    if (index < 0)
    {
        return 0;
    }

    return index;
}

void TranslationHandler::AddTranslation(const char *name, const char *filename)
{
    TranslationInfo info;
    info.mName = name;
    info.mFilename = filename;
    mTranslations.append(info);
}
