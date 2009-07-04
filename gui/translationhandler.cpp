/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */

#include "translationhandler.h"

#include <QApplication>
#include <QFile>
#include <QDebug>

TranslationHandler::TranslationHandler(QObject *parent) :
        QObject(parent),
        mCurrentLanguage(-1),
        mTranslator(new QTranslator(this))
{
    //Add our default languages
    mNames  << "English"
    << "Finnish"
    << "Swedish"
    << "German"
    << "Russian";

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
        qDebug() << "Failed to load English translation!";
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
    int index = mFiles.indexOf(file);

    //If nothing found, return english
    if (index < 0)
    {
        return 0;
    }

    return index;
}
