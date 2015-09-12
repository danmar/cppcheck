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

#include <QFile>
#include <QByteArray>
#include <QMessageBox>
#include <QTextEdit>
#include "fileviewdialog.h"

FileViewDialog::FileViewDialog(const QString &file,
                               const QString &title,
                               QWidget *parent)
    : QDialog(parent)
{
    mUI.setupUi(this);


    setWindowTitle(title);
    connect(mUI.mButtons, SIGNAL(accepted()), this, SLOT(accept()));
    LoadTextFile(file, mUI.mText);
}

void FileViewDialog::LoadTextFile(const QString &filename, QTextEdit *edit)
{
    QFile file(filename);
    if (!file.exists()) {
        QString msg(tr("Could not find the file: %1"));
        msg = msg.arg(filename);

        QMessageBox msgbox(QMessageBox::Critical,
                           tr("Cppcheck"),
                           msg,
                           QMessageBox::Ok,
                           this);
        msgbox.exec();
        return;
    }

    file.open(QIODevice::ReadOnly | QIODevice::Text);
    if (!file.isReadable()) {
        QString msg(tr("Could not read the file: %1"));
        msg = msg.arg(filename);

        QMessageBox msgbox(QMessageBox::Critical,
                           tr("Cppcheck"),
                           msg,
                           QMessageBox::Ok,
                           this);
        msgbox.exec();
        return;
    }
    QByteArray filedata = file.readAll();
    file.close();

    QString filestringdata(filedata);
    edit->setPlainText(filestringdata);
}
