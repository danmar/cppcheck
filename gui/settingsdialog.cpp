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


#include "settingsdialog.h"
#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QDebug>
#include <QTabWidget>
#include "applicationdialog.h"

SettingsDialog::SettingsDialog(QSettings &programSettings, ApplicationList &list,
                               QWidget *parent) :
        QDialog(parent),
        mSettings(programSettings),
        mApplications(list)
{
    mTempApplications.Copy(list);
    //Create a layout for the settings dialog
    QVBoxLayout *dialoglayout = new QVBoxLayout();

    //Create a tabwidget and add it to dialogs layout
    QTabWidget *tabs = new QTabWidget();
    dialoglayout->addWidget(tabs);

    //Add ok and cancel buttons
    QPushButton *cancel = new QPushButton(tr("Cancel"));
    QPushButton *ok = new QPushButton(tr("Ok"));

    //Add a layout for ok/cancel buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    buttonLayout->addStretch();
    buttonLayout->addWidget(ok);
    buttonLayout->addWidget(cancel);
    //Add button layout to the main dialog layout
    dialoglayout->addLayout(buttonLayout);

    //Connect OK buttons
    connect(ok, SIGNAL(clicked()),
            this, SLOT(Ok()));
    connect(cancel, SIGNAL(clicked()),
            this, SLOT(reject()));

    //Begin adding tabs and tab content

    //General tab
    QWidget *general = new QWidget();
    tabs->addTab(general, tr("General"));

    //layout for general tab
    QVBoxLayout *layout = new QVBoxLayout();

    //Number of jobs
    QHBoxLayout *jobsLayout = new QHBoxLayout();
    mJobs = new QLineEdit(programSettings.value(tr("Check threads"), 1).toString());
    mJobs->setValidator(new QIntValidator(1, 9999, this));

    jobsLayout->addWidget(new QLabel(tr("Number of threads: ")));
    jobsLayout->addWidget(mJobs);

    layout->addLayout(jobsLayout);

    //Force
    mForce = AddCheckbox(layout,
                         tr("Force checking on files that have \"too many\" configurations"),
                         tr("Check force"),
                         false);

    mShowFullPath = AddCheckbox(layout,
                                tr("Show full path of files"),
                                tr("Show full path"),
                                false);

    layout->addStretch();
    general->setLayout(layout);

    //Add tab for setting user startable applications
    QWidget *applications = new QWidget();
    tabs->addTab(applications, tr("Applications"));

    QVBoxLayout *appslayout = new QVBoxLayout();
    mListWidget = new QListWidget();
    appslayout->addWidget(mListWidget);
    applications->setLayout(appslayout);

    QPushButton *add = new QPushButton(tr("Add application"));
    appslayout->addWidget(add);
    connect(add, SIGNAL(clicked()),
            this, SLOT(AddApplication()));

    QPushButton *del = new QPushButton(tr("Delete application"));
    appslayout->addWidget(del);
    connect(del, SIGNAL(clicked()),
            this, SLOT(DeleteApplication()));

    QPushButton *modify = new QPushButton(tr("Modify application"));
    appslayout->addWidget(modify);
    connect(modify, SIGNAL(clicked()),
            this, SLOT(ModifyApplication()));

    QPushButton *def = new QPushButton(tr("Make default application"));
    appslayout->addWidget(def);
    connect(def, SIGNAL(clicked()),
            this, SLOT(DefaultApplication()));

    connect(mListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
            this, SLOT(ModifyApplication()));

    mListWidget->setSortingEnabled(false);
    PopulateListWidget();

    //report tab
    QWidget *report = new QWidget();
    tabs->addTab(report, tr("Reports"));

    QVBoxLayout *reportlayout = new QVBoxLayout();
    mSaveAllErrors = AddCheckbox(reportlayout,
                                 tr("Save all errors when creating report"),
                                 tr("Save all errors"),
                                 false);

    mSaveFullPath = AddCheckbox(reportlayout,
                                tr("Save full path to files in reports"),
                                tr("Save full path"),
                                false);
    reportlayout->addStretch();
    report->setLayout(reportlayout);
    setLayout(dialoglayout);
    setWindowTitle(tr("Settings"));
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

QCheckBox* SettingsDialog::AddCheckbox(QVBoxLayout *layout,
                                       const QString &label,
                                       const QString &settings,
                                       bool value)
{
    QCheckBox *result = new QCheckBox(label);
    result->setCheckState(BoolToCheckState(mSettings.value(settings, value).toBool()));
    layout->addWidget(result);
    return result;
}

void SettingsDialog::LoadSettings()
{
    resize(mSettings.value(tr("Check dialog width"), 800).toInt(), mSettings.value(tr("Check dialog height"), 600).toInt());
}

void SettingsDialog::SaveSettings()
{
    mSettings.setValue(tr("Check dialog width"), size().width());
    mSettings.setValue(tr("Check dialog height"), size().height());
}

void SettingsDialog::SaveCheckboxValues()
{
    int jobs = mJobs->text().toInt();
    if (jobs <= 0)
    {
        jobs = 1;
    }

    mSettings.setValue(tr("Check threads"), jobs);
    SaveCheckboxValue(mForce, tr("Check force"));
    SaveCheckboxValue(mSaveAllErrors, tr("Save all errors"));
    SaveCheckboxValue(mSaveFullPath, tr("Save full path"));
    SaveCheckboxValue(mShowFullPath, tr("Show full path"));
}

void SettingsDialog::SaveCheckboxValue(QCheckBox *box, const QString &name)
{
    mSettings.setValue(name, CheckStateToBool(box->checkState()));
}

void SettingsDialog::AddApplication()
{
    ApplicationDialog dialog("", "", tr("Add a new application"), this);

    if (dialog.exec() == QDialog::Accepted)
    {
        mTempApplications.AddApplicationType(dialog.GetName(), dialog.GetPath());
        mListWidget->addItem(dialog.GetName());
    }
}

void SettingsDialog::DeleteApplication()
{

    QList<QListWidgetItem *> selected = mListWidget->selectedItems();
    QListWidgetItem *item = 0;

    foreach(item, selected)
    {
        mTempApplications.RemoveApplication(mListWidget->row(item));
        mListWidget->clear();
        PopulateListWidget();
    }
}

void SettingsDialog::ModifyApplication()
{
    QList<QListWidgetItem *> selected = mListWidget->selectedItems();
    QListWidgetItem *item = 0;
    foreach(item, selected)
    {
        int row = mListWidget->row(item);

        ApplicationDialog dialog(mTempApplications.GetApplicationName(row),
                                 mTempApplications.GetApplicationPath(row),
                                 tr("Modify an application"));

        if (dialog.exec() == QDialog::Accepted)
        {
            mTempApplications.SetApplicationType(row, dialog.GetName(), dialog.GetPath());
            item->setText(dialog.GetName());
        }
    }
}

void SettingsDialog::DefaultApplication()
{
    QList<QListWidgetItem *> selected = mListWidget->selectedItems();
    if (selected.size() > 0)
    {
        int index = mListWidget->row(selected[0]);
        mTempApplications.MoveFirst(index);
        mListWidget->clear();
        PopulateListWidget();
    }
}

void SettingsDialog::PopulateListWidget()
{
    for (int i = 0; i < mTempApplications.GetApplicationCount(); i++)
    {
        mListWidget->addItem(mTempApplications.GetApplicationName(i));
    }

    // If list contains items select first item
    if (mTempApplications.GetApplicationCount())
    {
        mListWidget->setCurrentRow(0);
    }
}

void SettingsDialog::Ok()
{
    mApplications.Copy(mTempApplications);
    accept();
}

bool SettingsDialog::ShowFullPath()
{
    return CheckStateToBool(mShowFullPath->checkState());
}

bool SettingsDialog::SaveFullPath()
{
    return CheckStateToBool(mSaveFullPath->checkState());
}

bool SettingsDialog::SaveAllErrors()
{
    return CheckStateToBool(mSaveAllErrors->checkState());
}
