/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
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
#include <QFileInfo>
#include "translationhandler.h"
#include "common.h"


// Provide own translations for standard buttons. This (garbage) code is needed to enforce them to appear in .ts files even after "lupdate gui.pro"
static void unused()
{
// NOTE: Keeping semi-colons at end of macro for style preference
#if ((QT_VERSION >= 0x040000)&&(QT_VERSION < 0x050000))
    Q_UNUSED(QT_TRANSLATE_NOOP("QDialogButtonBox", "OK"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QDialogButtonBox", "Cancel"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QDialogButtonBox", "Close"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QDialogButtonBox", "Save"));
#elif ((QT_VERSION >= 0x050000)&&(QT_VERSION < 0x060000))
    Q_UNUSED(QT_TRANSLATE_NOOP("QPlatformTheme", "OK"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QPlatformTheme", "Cancel"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QPlatformTheme", "Close"));
    Q_UNUSED(QT_TRANSLATE_NOOP("QPlatformTheme", "Save"));
#else
#error Unsupported Qt version.
#endif
}

TranslationHandler::TranslationHandler(QObject *parent) :
    QObject(parent),
    mCurrentLanguage("en"),
    mTranslator(nullptr)
{
    // Add our available languages
    // Keep this list sorted
    addTranslation("Chinese (Simplified)", "cppcheck_zh_CN");
    addTranslation("Dutch", "cppcheck_nl");
    addTranslation("English", "cppcheck_en");
    addTranslation("Finnish", "cppcheck_fi");
    addTranslation("French", "cppcheck_fr");
    addTranslation("German", "cppcheck_de");
    addTranslation("Italian", "cppcheck_it");
    addTranslation("Japanese", "cppcheck_ja");
    addTranslation("Korean", "cppcheck_ko");
    addTranslation("Russian", "cppcheck_ru");
    addTranslation("Serbian", "cppcheck_sr");
    addTranslation("Spanish", "cppcheck_es");
    addTranslation("Swedish", "cppcheck_sv");
}

TranslationHandler::~TranslationHandler()
{
}

const QStringList TranslationHandler::getNames() const
{
    QStringList names;
    foreach (TranslationInfo translation, mTranslations) {
        names.append(translation.mName);
    }
    return names;
}

bool TranslationHandler::setLanguage(const QString &code)
{
    bool failure = false;
    QString error;

    //If English is the language
    if (code == "en") {
        //Just remove all extra translators
        if (mTranslator) {
            qApp->removeTranslator(mTranslator);
            delete mTranslator;
            mTranslator = nullptr;
        }

        mCurrentLanguage = code;
        return true;
    }

    //Make sure the translator is otherwise valid
    int index = getLanguageIndexByCode(code);
    if (index == -1) {
        error = QObject::tr("Unknown language specified!");
        failure = true;
    } else {
        // Make sure there is a translator
        if (!mTranslator && !failure)
            mTranslator = new QTranslator(this);

        //Load the new language
        const QString appPath = QFileInfo(QCoreApplication::applicationFilePath()).canonicalPath();

        QString datadir = getDataDir();

        QString translationFile;
        if (QFile::exists(datadir + "/lang/" + mTranslations[index].mFilename + ".qm"))
            translationFile = datadir + "/lang/" + mTranslations[index].mFilename + ".qm";

        else if (QFile::exists(datadir + "/" + mTranslations[index].mFilename + ".qm"))
            translationFile = datadir + "/" + mTranslations[index].mFilename + ".qm";

        else
            translationFile = appPath + "/" + mTranslations[index].mFilename + ".qm";

        if (!mTranslator->load(translationFile) && !failure) {
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

QString TranslationHandler::getCurrentLanguage() const
{
    return mCurrentLanguage;
}

QString TranslationHandler::suggestLanguage() const
{
    //Get language from system locale's name ie sv_SE or zh_CN
    QString language = QLocale::system().name();
    //qDebug()<<"Your language is"<<language;

    //And see if we can find it from our list of language files
    int index = getLanguageIndexByCode(language);

    //If nothing found, return English
    if (index < 0) {
        return "en";
    }

    return language;
}

void TranslationHandler::addTranslation(const char *name, const char *filename)
{
    TranslationInfo info;
    info.mName = name;
    info.mFilename = filename;
    int codeLength = QString(filename).length() - QString(filename).indexOf('_') - 1;
    info.mCode = QString(filename).right(codeLength);
    mTranslations.append(info);
}

int TranslationHandler::getLanguageIndexByCode(const QString &code) const
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
