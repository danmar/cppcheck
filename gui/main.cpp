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
#include <QCoreApplication>
#include <QTextCodec>
#include <QTranslator>
#include <QMetaType>
#include <QStringList>
#include <iostream>
#include "mainwindow.h"
#include "erroritem.h"

void ShowUsage();
bool CheckArgs(const QStringList &args);

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    if (!CheckArgs(app.arguments()))
        return 0;

    QCoreApplication::setOrganizationName("Cppcheck");
    QCoreApplication::setApplicationName("Cppcheck-GUI");

    app.setWindowIcon(QIcon(":icon.png"));

    // Register this metatype that is used to transfer error info
    qRegisterMetaType<ErrorItem>("ErrorItem");

    // Set codecs so that UTF-8 strings in sources are handled correctly.
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    MainWindow window;
    window.show();
    return app.exec();
}

// Check only arguments needing action before GUI is shown.
// Rest of the arguments are handled in MainWindow::HandleCLIParams()
bool CheckArgs(const QStringList &args)
{
    if (args.contains("-h") || args.contains("--help")) {
        ShowUsage();
        return false;
    }
    return true;
}

void ShowUsage()
{
    std::cout << "Cppcheck GUI.\n\n";
    std::cout << "Syntax:\n";
    std::cout << "    cppcheck-gui [OPTIONS] [files or paths]\n\n";
    std::cout << "Options:\n";
    std::cout << "    -h, --help    Print this help\n";
    std::cout << "    -p <file>     Open given project file and start checking it\n";
}
