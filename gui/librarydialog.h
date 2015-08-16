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

#ifndef LIBRARYDIALOG_H
#define LIBRARYDIALOG_H

#include <QDialog>
#include <QFile>

namespace Ui {
    class LibraryDialog;
}

class LibraryDialog : public QDialog {
    Q_OBJECT

public:
    explicit LibraryDialog(QWidget *parent = 0);
    ~LibraryDialog();

    struct Function {
        Function() : noreturn(false), gccPure(false), gccConst(false),
            leakignore(false), useretval(false) {
        }

        QString name;
        bool noreturn;
        bool gccPure;
        bool gccConst;
        bool leakignore;
        bool useretval;
        struct {
            QString scan;
            QString secure;
        } formatstr;
        struct Arg {
            Arg() : nr(0), notbool(false), notnull(false), notuninit(false),
                formatstr(false), strz(false) {
            }

            QString name;
            unsigned int nr;
            static const unsigned int ANY;
            bool notbool;
            bool notnull;
            bool notuninit;
            bool formatstr;
            bool strz;
            QString valid;
            struct {
                QString type;
                QString arg;
                QString arg2;
            } minsize;
        };
        QList<struct Arg> args;
    };

private slots:
    void openCfg();
    void selectFunction(int row);

private:
    Ui::LibraryDialog *ui;

    void updateui();
    bool loadFile(QFile &file);
    QList<struct Function> functions;
};

#endif // LIBRARYDIALOG_H
