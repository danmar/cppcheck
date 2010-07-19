/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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
#include <QLineEdit>
#include <QString>
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
class ApplicationDialog : public QDialog
{
    Q_OBJECT
public:
    /**
    * @brief Constructor
    *
    * @param name Default name for the application to start
    * @param path Path for the application
    * @param title Title for the dialog
    * @param parent Parent widget
    */
    ApplicationDialog(const QString &name,
                      const QString &path,
                      const QString &title,
                      QWidget *parent = 0);
    virtual ~ApplicationDialog();

    /**
    * @brief Get modified name
    * This is just a name to display the application. This has nothing to do
    * with executing the application.
    *
    * @return Modified name
    */
    QString GetName();

    /**
    * @brief Get modified path
    * This also contains all parameters user wants to specify.
    * @return Modified path
    */
    QString GetPath();
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
};
/// @}
#endif // APPLICATIONDIALOG_H
