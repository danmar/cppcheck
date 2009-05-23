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


#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QSettings>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QPushButton>
#include "applicationlist.h"

#include <QListWidget>
#include <QKeyEvent>


/**
* @brief Settings dialog
*
*/
class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    SettingsDialog(QSettings &programSettings, ApplicationList &list);
    virtual ~SettingsDialog();
    void SaveCheckboxValues();

protected slots:
    void AddApplication();
    void DeleteApplication();
    void ModifyApplication();
    void DefaultApplication();
protected:
    void PopulateListWidget();
    /**
        * @brief Load saved values
        * Loads dialog size and column widths.
        *
        */
    void SaveSettings();

    /**
    * @brief Save settings
    * Save dialog size and column widths.
    *
    */
    void LoadSettings();

    /**
    * @brief Save a single checkboxes value
    *
    * @param box checkbox to save
    * @param name name for QSettings to store the value
    */
    void SaveCheckboxValue(QCheckBox *box, const QString &name);

    /**
        * @brief Add a new checkbox to layout
        *
        * @param layout layout to add to
        * @param label label for the checkbox
        * @param settings QSettings name for default value
        * @return newly created QCheckBox
        */
    QCheckBox* AddCheckbox(QVBoxLayout *layout,
                           const QString &label,
                           const QString &settings,
                           bool value);

    /**
    * @brief Convert bool to Qt::CheckState
    *
    * @param yes value to convert
    * @return value converted to Qt::CheckState
    */
    Qt::CheckState BoolToCheckState(bool yes);

    /**
    * @brief Converts Qt::CheckState to bool
    *
    * @param state Qt::CheckState to convert
    * @return converted value
    */
    bool CheckStateToBool(Qt::CheckState state);

    /**
        * @brief How many threads should cppcheck have
        *
        */
    QLineEdit *mJobs;


    /**
    * @brief Cppcheck setting
    *
    */
    QCheckBox *mForce;

    /**
    * @brief List of all applications that can be started when right clicking
    * an error
    */
    QListWidget *mListWidget;


    /**
    * @brief Settings
    *
    */
    QSettings &mSettings;
    ApplicationList &mApplications;
private:
};

#endif // SETTINGSDIALOG_H
