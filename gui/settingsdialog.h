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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#include "ui_settings.h"

/// @addtogroup GUI
/// @{

/**
* @brief Settings dialog
*
*/
class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    SettingsDialog(QSettings *programSettings,
                   ApplicationList *list,
                   QWidget *parent = 0);
    virtual ~SettingsDialog();

    /**
    * @brief Save all values to QSettings
    *
    */
    void SaveCheckboxValues();

    /**
    * @brief Get checkbox value for mShowFullPath
    *
    * @return should full path of errors be shown in the tree
    */
    bool ShowFullPath();

    /**
    * @brief Get checkbox value for mSaveFullPath
    *
    * @return should full path of files be saved when creating a report
    */
    bool SaveFullPath();


    /**
    * @brief Get checkbox value for mNoErrorsMessage
    *
    * @return Should "no errors message" be hidden
    */
    bool ShowNoErrorsMessage();

    /**
    * @brief Get checkbox value for mSaveAllErrors
    *
    * @return should all errors be saved to report
    */
    bool SaveAllErrors();

protected slots:
    /**
    * @brief Slot for clicking OK.
    *
    */
    void Ok();

    /**
    * @brief Slot for adding a new application to the list
    *
    */
    void AddApplication();

    /**
    * @brief Slot for deleting an application from the list
    *
    */
    void DeleteApplication();

    /**
    * @brief Slot for modifying an application in the list
    *
    */
    void ModifyApplication();

    /**
    * @brief Slot for making the selected application as the default (first)
    *
    */
    void DefaultApplication();
protected:

    /**
    * @brief Clear all applications from the list and re insert them from mTempApplications
    *
    */
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
    * @brief Settings
    *
    */
    QSettings *mSettings;

    /**
    * @brief List of applications user has specified
    *
    */
    ApplicationList *mApplications;

    /**
    * @brief Temporary list of applications
    * This will be copied to actual list of applications (mApplications)
    * when user clicks ok.
    */
    ApplicationList *mTempApplications;

    /**
    * @brief Dialog from UI designer
    *
    */
    Ui::Settings mUI;
private:
};
/// @}
#endif // SETTINGSDIALOG_H
