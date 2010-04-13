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


#include "settingsdialog.h"
#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QDebug>
#include <QTabWidget>
#include "applicationdialog.h"
#include "common.h"

SettingsDialog::SettingsDialog(QSettings *programSettings,
                               ApplicationList *list,
                               QWidget *parent) :
        QDialog(parent),
        mSettings(programSettings),
        mApplications(list),
        mTempApplications(new ApplicationList(this))
{
    mUI.setupUi(this);
    mTempApplications->Copy(list);

    connect(mUI.mButtons, SIGNAL(accepted()), this, SLOT(Ok()));
    connect(mUI.mButtons, SIGNAL(rejected()), this, SLOT(reject()));

    mUI.mJobs->setText(programSettings->value(SETTINGS_CHECK_THREADS, 1).toString());
    mUI.mForce->setCheckState(BoolToCheckState(programSettings->value(SETTINGS_CHECK_FORCE, false).toBool()));
    mUI.mShowFullPath->setCheckState(BoolToCheckState(programSettings->value(SETTINGS_SHOW_FULL_PATH, false).toBool()));
    mUI.mShowNoErrorsMessage->setCheckState(BoolToCheckState(programSettings->value(SETTINGS_SHOW_NO_ERRORS, false).toBool()));


    connect(mUI.mButtonAdd, SIGNAL(clicked()),
            this, SLOT(AddApplication()));

    connect(mUI.mButtonDelete, SIGNAL(clicked()),
            this, SLOT(DeleteApplication()));

    connect(mUI.mButtonModify, SIGNAL(clicked()),
            this, SLOT(ModifyApplication()));

    connect(mUI.mButtonDefault, SIGNAL(clicked()),
            this, SLOT(DefaultApplication()));

    connect(mUI.mListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
            this, SLOT(ModifyApplication()));

    mUI.mListWidget->setSortingEnabled(false);
    PopulateListWidget();

    mUI.mSaveAllErrors->setCheckState(BoolToCheckState(programSettings->value(SETTINGS_SAVE_ALL_ERRORS, false).toBool()));
    mUI.mSaveFullPath->setCheckState(BoolToCheckState(programSettings->value(SETTINGS_SAVE_FULL_PATH, false).toBool()));

    LoadSettings();
}

SettingsDialog::~SettingsDialog()
{
    SaveSettings();
}

Qt::CheckState SettingsDialog::BoolToCheckState(bool yes)
{
    if (yes)
    {
        return Qt::Checked;
    }
    return Qt::Unchecked;
}

bool SettingsDialog::CheckStateToBool(Qt::CheckState state)
{
    if (state == Qt::Checked)
    {
        return true;
    }
    return false;
}


void SettingsDialog::LoadSettings()
{
    resize(mSettings->value(SETTINGS_CHECK_DIALOG_WIDTH, 800).toInt(),
           mSettings->value(SETTINGS_CHECK_DIALOG_HEIGHT, 600).toInt());
}

void SettingsDialog::SaveSettings()
{
    mSettings->setValue(SETTINGS_CHECK_DIALOG_WIDTH, size().width());
    mSettings->setValue(SETTINGS_CHECK_DIALOG_HEIGHT, size().height());
}

void SettingsDialog::SaveCheckboxValues()
{
    int jobs = mUI.mJobs->text().toInt();
    if (jobs <= 0)
    {
        jobs = 1;
    }

    mSettings->setValue(SETTINGS_CHECK_THREADS, jobs);
    SaveCheckboxValue(mUI.mForce, SETTINGS_CHECK_FORCE);
    SaveCheckboxValue(mUI.mSaveAllErrors, SETTINGS_SAVE_ALL_ERRORS);
    SaveCheckboxValue(mUI.mSaveFullPath, SETTINGS_SAVE_FULL_PATH);
    SaveCheckboxValue(mUI.mShowFullPath, SETTINGS_SHOW_FULL_PATH);
    SaveCheckboxValue(mUI.mShowNoErrorsMessage, SETTINGS_SHOW_NO_ERRORS);
}

void SettingsDialog::SaveCheckboxValue(QCheckBox *box, const QString &name)
{
    mSettings->setValue(name, CheckStateToBool(box->checkState()));
}

void SettingsDialog::AddApplication()
{
    ApplicationDialog dialog("", "", tr("Add a new application"), this);

    if (dialog.exec() == QDialog::Accepted)
    {
        mTempApplications->AddApplicationType(dialog.GetName(), dialog.GetPath());
        mUI.mListWidget->addItem(dialog.GetName());
    }
}

void SettingsDialog::DeleteApplication()
{

    QList<QListWidgetItem *> selected = mUI.mListWidget->selectedItems();
    QListWidgetItem *item = 0;

    foreach(item, selected)
    {
        mTempApplications->RemoveApplication(mUI.mListWidget->row(item));
        mUI.mListWidget->clear();
        PopulateListWidget();
    }
}

void SettingsDialog::ModifyApplication()
{
    QList<QListWidgetItem *> selected = mUI.mListWidget->selectedItems();
    QListWidgetItem *item = 0;
    foreach(item, selected)
    {
        int row = mUI.mListWidget->row(item);

        ApplicationDialog dialog(mTempApplications->GetApplicationName(row),
                                 mTempApplications->GetApplicationPath(row),
                                 tr("Modify an application"));

        if (dialog.exec() == QDialog::Accepted)
        {
            mTempApplications->SetApplicationType(row, dialog.GetName(), dialog.GetPath());
            item->setText(dialog.GetName());
        }
    }
}

void SettingsDialog::DefaultApplication()
{
    QList<QListWidgetItem *> selected = mUI.mListWidget->selectedItems();
    if (selected.size() > 0)
    {
        int index = mUI.mListWidget->row(selected[0]);
        mTempApplications->MoveFirst(index);
        mUI.mListWidget->clear();
        PopulateListWidget();
    }
}

void SettingsDialog::PopulateListWidget()
{
    for (int i = 0; i < mTempApplications->GetApplicationCount(); i++)
    {
        mUI.mListWidget->addItem(mTempApplications->GetApplicationName(i));
    }

    // If list contains items select first item
    if (mTempApplications->GetApplicationCount())
    {
        mUI.mListWidget->setCurrentRow(0);
    }
}

void SettingsDialog::Ok()
{
    mApplications->Copy(mTempApplications);
    accept();
}

bool SettingsDialog::ShowFullPath()
{
    return CheckStateToBool(mUI.mShowFullPath->checkState());
}

bool SettingsDialog::SaveFullPath()
{
    return CheckStateToBool(mUI.mSaveFullPath->checkState());
}

bool SettingsDialog::SaveAllErrors()
{
    return CheckStateToBool(mUI.mSaveAllErrors->checkState());
}

bool SettingsDialog::ShowNoErrorsMessage()
{
    return CheckStateToBool(mUI.mShowNoErrorsMessage->checkState());
}



