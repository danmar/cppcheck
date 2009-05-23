/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis, Vesa Pikki
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

#ifndef APPLICATIONDIALOG_H
#define APPLICATIONDIALOG_H

#include <QDialog>
#include <QLineEdit>


class ApplicationDialog : public QDialog
{
    Q_OBJECT
public:
    ApplicationDialog(const QString &name,
                      const QString &path,
                      const QString &title);
    virtual ~ApplicationDialog();
    QString GetName();
    QString GetPath();
protected slots:
    void Browse();
protected:
    QLineEdit *mName;
    QLineEdit *mPath;
private:
};

#endif // APPLICATIONDIALOG_H
