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

#ifndef APPLICATIONDIALOG_H
#define APPLICATIONDIALOG_H

#include <QDialog>
#include <QString>
#include "application.h"
#include "ui_application.h"

class QWidget;

/// @addtogroup GUI
/// @{

/**
* @brief Dialog to edit a startable application.
* User can open errors with user specified applications. This is a dialog
* to modify/add an application to open errors with.
*
*/
class ApplicationDialog : public QDialog {
    Q_OBJECT

public:
    /**
    * @brief Constructor.
    * @param title Title for the dialog.
    * @param app Application definition.
    * @param parent Parent widget.
    */
    ApplicationDialog(const QString &title, Application &app,
                      QWidget *parent = 0);
    virtual ~ApplicationDialog();

protected slots:

    void Ok();

    /**
    * @brief Slot to browse for an application
    *
    */
    void Browse();

protected:

    /**
    * @brief UI from the Qt designer
    *
    */
    Ui::ApplicationDialog mUI;

private:

    /**
    * @brief Underlying Application
    */
    Application& mApplication;
};
/// @}
#endif // APPLICATIONDIALOG_H
