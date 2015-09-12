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

#ifndef FILEVIEW_DIALOG_H
#define FILEVIEW_DIALOG_H

#include <QDialog>
#include <QString>
#include "ui_file.h"

class QWidget;
class QTextEdit;

/// @addtogroup GUI
/// @{


/**
* @brief File view -dialog.
* This dialog shows text files. It is used for showing the license file and
* the authors list.
*
*/
class FileViewDialog : public QDialog {
    Q_OBJECT
public:
    FileViewDialog(const QString &file,
                   const QString &title,
                   QWidget *parent = 0);


protected:

    /**
    * @brief Load text file contents to edit control.
    *
    * @param filename File to load.
    * @param edit Control where to load the file contents.
    */
    void LoadTextFile(const QString &filename, QTextEdit *edit);

    /**
    * @brief Format dialog title from filename.
    *
    * @param filename File to load.
    */
    QString FormatTitle(const QString &filename);

    Ui::Fileview mUI;
};
/// @}
#endif // FILEVIEW_DIALOG_H
