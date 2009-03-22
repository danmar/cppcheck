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


#include "settingsdialog.h"
#include <QLabel>
#include <QDebug>

SettingsDialog::SettingsDialog(QSettings &programSettings) :
        mSettings(programSettings)
{
    QPushButton *cancel = new QPushButton(tr("Cancel"));
    QPushButton *ok = new QPushButton(tr("Ok"));

    //Main layout
    QVBoxLayout *layout = new QVBoxLayout();

    //Layout for ok/cancel buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    //Number of jobs
    QHBoxLayout *jobsLayout = new QHBoxLayout();
    mJobs = new QLineEdit(programSettings.value(tr("Check threads"), 1).toString());
    jobsLayout->addWidget(new QLabel(tr("Number of threads: ")));
    jobsLayout->addWidget(mJobs);
    mJobs->setValidator(new QIntValidator(this));
    layout->addLayout(jobsLayout);


    //Show All
    mShowAll = AddCheckbox(layout, tr("Show all"), tr("Check show all"), false);

    //Check Coding Style
    mCheckCodingStyle = AddCheckbox(layout, tr("Check Coding Style"), tr("Check coding style"), false);

    //Errors only
    mErrorsOnly = AddCheckbox(layout, tr("Errors only"), tr("Check errors only"), false);

    //Verbose
    mVerbose = AddCheckbox(layout, tr("Verbose"), tr("Check verbose"), false);

    //Force
    mForce = AddCheckbox(layout, tr("Force"), tr("Check force"), false);

    //Unused functions
    mUnusedFunctions = AddCheckbox(layout, tr("Unused functions"), tr("Check unused functions"), false);

    //Security
    mSecurity = AddCheckbox(layout, tr("Security"), tr("Check security"), false);



    buttonLayout->addWidget(ok);
    buttonLayout->addWidget(cancel);
    layout->addLayout(buttonLayout);


    connect(ok, SIGNAL(clicked()),
            this, SLOT(accept()));
    connect(cancel, SIGNAL(clicked()),
            this, SLOT(reject()));

    setWindowTitle(tr("Settings"));
    setLayout(layout);
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
    mSettings.setValue(tr("Check threads"), mJobs->text().toInt());
    SaveCheckboxValue(mShowAll, tr("Check show all"));
    SaveCheckboxValue(mCheckCodingStyle, tr("Check coding style"));
    SaveCheckboxValue(mErrorsOnly, tr("Check errors only"));
    SaveCheckboxValue(mVerbose, tr("Check verbose"));
    SaveCheckboxValue(mForce, tr("Check force"));
    SaveCheckboxValue(mUnusedFunctions, tr("Check unused functions"));
    SaveCheckboxValue(mSecurity, tr("Check security"));
}

void SettingsDialog::SaveCheckboxValue(QCheckBox *box, const QString &name)
{
    mSettings.setValue(name, CheckStateToBool(box->checkState()));
}
