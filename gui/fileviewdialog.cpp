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

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QFile>
#include <QByteArray>
#include <QMessageBox>
#include "fileviewdialog.h"

FileViewDialog::FileViewDialog(const QString &file, QWidget *parent)
        : QDialog(parent)
{
    QString title = FormatTitle(file);
    setWindowTitle(title);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *quit = new QPushButton(tr("Close"));
    QTextEdit *edit = new QTextEdit();
    edit->setReadOnly(true);

    mainLayout->addWidget(edit);
    mainLayout->addLayout(btnLayout);
    btnLayout->addStretch();
    btnLayout->addWidget(quit);
    setLayout(mainLayout);

    connect(quit, SIGNAL(clicked()), this, SLOT(close()));

    LoadTextFile(file, edit);
}

void FileViewDialog::LoadTextFile(const QString &filename, QTextEdit *edit)
{
    QFile file(filename);
    if (!file.exists())
    {
        QString msg(tr("Could not find the file:\n"));
        msg += filename;
        QMessageBox msgbox(QMessageBox::Critical,
                           tr("Cppcheck"),
                           msg,
                           QMessageBox::Ok,
                           this);
        msgbox.exec();
        return;
    }

    file.open(QIODevice::ReadOnly | QIODevice::Text);
    if (!file.isReadable())
    {
        QString msg(tr("Could not read the file:\n"));
        msg += filename;
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

QString FileViewDialog::FormatTitle(const QString &filename)
{
    QString title(filename);
    if (title.startsWith(":"))
    {
        title.remove(0, 1);
    }
    return title;
}
