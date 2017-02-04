/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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
#include <QCoreApplication>
#include <QTextCodec>
#include <QMetaType>
#include <QStringList>
#include <QSettings>
#ifdef _WIN32
#include <QMessageBox>
#else
#include <iostream>
#endif
#include "cppcheck.h"
#include "common.h"
#include "mainwindow.h"
#include "erroritem.h"
#include "aboutdialog.h"
#include "translationhandler.h"


static void ShowUsage();
static void ShowVersion();
static bool CheckArgs(const QStringList &args);

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

#if QT_VERSION < 0x050000
    // Set codecs so that UTF-8 strings in sources are handled correctly.
    // This is ONLY needed for Qt versions 4.x.
    // Qt 5.x assumes UTF-8 by default.
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif

    QCoreApplication::setOrganizationName("Cppcheck");
    QCoreApplication::setApplicationName("Cppcheck-GUI");

    QSettings* settings = new QSettings("Cppcheck", "Cppcheck-GUI", &app);

    // Set data dir..
    foreach (const QString arg, app.arguments()) {
        if (arg.startsWith("--data-dir=")) {
            settings->setValue("DATADIR", arg.mid(11));
            return 0;
        }
    }

    TranslationHandler* th = new TranslationHandler(&app);
    th->SetLanguage(settings->value(SETTINGS_LANGUAGE, th->SuggestLanguage()).toString());

    if (!CheckArgs(app.arguments()))
        return 0;

    app.setWindowIcon(QIcon(":cppcheck-gui.png"));

    // Register this metatype that is used to transfer error info
    qRegisterMetaType<ErrorItem>("ErrorItem");

    MainWindow window(th, settings);
    window.show();
    return app.exec();
}

// Check only arguments needing action before GUI is shown.
// Rest of the arguments are handled in MainWindow::HandleCLIParams()
static bool CheckArgs(const QStringList &args)
{
    if (args.contains("-h") || args.contains("--help")) {
        ShowUsage();
        return false;
    }
    if (args.contains("-v") || args.contains("--version")) {
        ShowVersion();
        return false;
    }
    return true;
}

static void ShowUsage()
{
    QString helpMessage = MainWindow::tr(
                              "Cppcheck GUI.\n\n"
                              "Syntax:\n"
                              "    cppcheck-gui [OPTIONS] [files or paths]\n\n"
                              "Options:\n"
                              "    -h, --help              Print this help\n"
                              "    -p <file>               Open given project file and start checking it\n"
                              "    -l <file>               Open given results xml file\n"
                              "    -d <directory>          Specify the directory that was checked to generate the results xml specified with -l\n"
                              "    -v, --version           Show program version\n"
                              "    --data-dir=<directory>  Specify directory where GUI datafiles are located (translations, cfg)");
#if defined(_WIN32)
    QMessageBox msgBox(QMessageBox::Information,
                       MainWindow::tr("Cppcheck GUI - Command line parameters"),
                       helpMessage,
                       QMessageBox::Ok
                      );
    (void)msgBox.exec();
#else
    std::cout << helpMessage.toStdString() << std::endl;
#endif
}

static void ShowVersion()
{
#if defined(_WIN32)
    AboutDialog *dlg = new AboutDialog(CppCheck::version(), CppCheck::extraVersion(), 0);
    dlg->exec();
    delete dlg;
#else
    std::string versionMessage("Cppcheck ");
    versionMessage += CppCheck::version();
    const char * extraVersion = CppCheck::extraVersion();
    if (*extraVersion != 0)
        versionMessage += std::string(" (") + extraVersion + ")";

    std::cout << versionMessage << std::endl;
#endif
}
