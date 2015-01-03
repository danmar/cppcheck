/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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
#include <QMessageBox>
#include <QSettings>
#include <QFileInfo>
#include "translationhandler.h"

// Provide own translations for standard buttons. This (garbage) code is needed to enforce them to appear in .ts files even after "lupdate gui.pro"
static void unused()
{
    QT_TRANSLATE_NOOP("QDialogButtonBox", "OK");
    QT_TRANSLATE_NOOP("QDialogButtonBox", "Cancel");
    QT_TRANSLATE_NOOP("QDialogButtonBox", "Close");
    QT_TRANSLATE_NOOP("QDialogButtonBox", "Save");
}

TranslationHandler::TranslationHandler(QObject *parent) :
    QObject(parent),
    mCurrentLanguage("en"),
    mTranslator(NULL)
{
    // Add our available languages
    // Keep this list sorted
    AddTranslation(QT_TRANSLATE_NOOP("MainWindow", "Chinese (Simplified)"), "cppcheck_zh_CN");
    AddTranslation(QT_TRANSLATE_NOOP("MainWindow", "Dutch"), "cppcheck_nl");
    AddTranslation(QT_TRANSLATE_NOOP("MainWindow", "English"), "cppcheck_en");
    AddTranslation(QT_TRANSLATE_NOOP("MainWindow", "Finnish"), "cppcheck_fi");
    AddTranslation(QT_TRANSLATE_NOOP("MainWindow", "French"), "cppcheck_fr");
    AddTranslation(QT_TRANSLATE_NOOP("MainWindow", "German"), "cppcheck_de");
    AddTranslation(QT_TRANSLATE_NOOP("MainWindow", "Italian"), "cppcheck_it");
    AddTranslation(QT_TRANSLATE_NOOP("MainWindow", "Japanese"), "cppcheck_ja");
    AddTranslation(QT_TRANSLATE_NOOP("MainWindow", "Korean"), "cppcheck_ko");
    AddTranslation(QT_TRANSLATE_NOOP("MainWindow", "Russian"), "cppcheck_ru");
    AddTranslation(QT_TRANSLATE_NOOP("MainWindow", "Serbian"), "cppcheck_sr");
    AddTranslation(QT_TRANSLATE_NOOP("MainWindow", "Spanish"), "cppcheck_es");
    AddTranslation(QT_TRANSLATE_NOOP("MainWindow", "Swedish"), "cppcheck_sv");
}

TranslationHandler::~TranslationHandler()
{
}

const QStringList TranslationHandler::GetNames() const
{
    QStringList names;
    foreach(TranslationInfo translation, mTranslations) {
        names.append(translation.mName);
    }
    return names;
}

bool TranslationHandler::SetLanguage(const QString &code)
{
    bool failure = false;
    QString error;

    //If English is the language
    if (code == "en") {
        //Just remove all extra translators
        if (mTranslator) {
            qApp->removeTranslator(mTranslator);
            delete mTranslator;
            mTranslator = NULL;
        }

        mCurrentLanguage = code;
        return true;
    }

    //Make sure the translator is otherwise valid
    int index = GetLanguageIndexByCode(code);
    if (index == -1) {
        error = QObject::tr("Unknown language specified!");
        failure = true;
    }

    // Make sure there is a translator
    if (!mTranslator && !failure)
        mTranslator = new QTranslator(this);

    //Load the new language
    const QString appPath = QFileInfo(QCoreApplication::applicationFilePath()).canonicalPath();

    QSettings settings;
    QString datadir = settings.value("DATADIR").toString();
    if (datadir.isEmpty())
        datadir = appPath;

    QString translationFile;
    if (QFile::exists(datadir + "/lang/" + mTranslations[index].mFilename + ".qm"))
        translationFile = datadir + "/lang/" + mTranslations[index].mFilename + ".qm";

    else if (QFile::exists(datadir + "/" + mTranslations[index].mFilename + ".qm"))
        translationFile = datadir + "/" + mTranslations[index].mFilename + ".qm";

    else
        translationFile = appPath + "/" + mTranslations[index].mFilename + ".qm";

    if (!mTranslator->load(translationFile) && !failure) {
        translationFile += ".qm";
        //If it failed, lets check if the default file exists
        if (!QFile::exists(translationFile)) {
            error = QObject::tr("Language file %1 not found!");
            error = error.arg(translationFile);
            failure = true;
        }

        //If file exists, there's something wrong with it
        error = QObject::tr("Failed to load translation for language %1 from file %2");
        error = error.arg(mTranslations[index].mName);
        error = error.arg(translationFile);
    }

    if (failure) {
        const QString msg(tr("Failed to change the user interface language:"
                             "\n\n%1\n\n"
                             "The user interface language has been reset to English. Open "
                             "the Preferences-dialog to select any of the available "
                             "languages.").arg(error));
        QMessageBox msgBox(QMessageBox::Warning,
                           tr("Cppcheck"),
                           msg,
                           QMessageBox::Ok);
        msgBox.exec();
        return false;
    }

    qApp->installTranslator(mTranslator);

    mCurrentLanguage = code;

    return true;
}

QString TranslationHandler::GetCurrentLanguage() const
{
    return mCurrentLanguage;
}

QString TranslationHandler::SuggestLanguage() const
{
    //Get language from system locale's name ie sv_SE or zh_CN
    QString language = QLocale::system().name();
    //qDebug()<<"Your language is"<<language;

    //And see if we can find it from our list of language files
    int index = GetLanguageIndexByCode(language);

    //If nothing found, return English
    if (index < 0) {
        return "en";
    }

    return language;
}

void TranslationHandler::AddTranslation(const char *name, const char *filename)
{
    TranslationInfo info;
    info.mName = name;
    info.mFilename = filename;
    int codeLength = QString(filename).length() - QString(filename).indexOf('_') - 1;
    info.mCode = QString(filename).right(codeLength);
    mTranslations.append(info);
}

int TranslationHandler::GetLanguageIndexByCode(const QString &code) const
{
    int index = -1;
    for (int i = 0; i < mTranslations.size(); i++) {
        if (mTranslations[i].mCode == code) {
            index = i;
            break;
        } else if (mTranslations[i].mCode == code.left(2)) {
            index = i;
            break;
        }
    }
    return index;
}
