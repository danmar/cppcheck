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


#include "checkdialog.h"
#include <QHBoxLayout>

#include <QStringList>
#include <QLabel>
#include <QIntValidator>
#include <QFileDialog>
#include <QDirIterator>
#include <QDebug>

CheckDialog::CheckDialog(QSettings &programSettings) :
        mSettings(programSettings)
{
    QPushButton *cancel = new QPushButton("Cancel");
    QPushButton *ok = new QPushButton("Ok");

    //Main layout
    QVBoxLayout *layout = new QVBoxLayout();

    //Layout for ok/cancel buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();


    //File selection tree
    layout->addWidget(new QLabel("Select files to check"));
    mFileTree = new QTreeView();
    layout->addWidget(mFileTree);
    mFileTree->setModel(&mModel);
    mFileTree->scrollTo(mModel.index(programSettings.value("Check path", QDir::currentPath()).toString()));
    mFileTree->setSelectionMode(QAbstractItemView::ExtendedSelection);

    //Number of jobs
    mJobs = new QLineEdit(programSettings.value("Check threads", 1).toString());
    layout->addWidget(new QLabel("Number of threads"));
    layout->addWidget(mJobs);
    mJobs->setValidator(new QIntValidator(this));

    //Debug
    mDebug = AddCheckbox(layout, "Debug", "Check debug", false);

    //Show All
    mShowAll = AddCheckbox(layout, "Show all", "Check show all", false);

    //Check Coding Style
    mCheckCodingStyle = AddCheckbox(layout, "Check Coding Style", "Check coding style", false);

    //Errors only
    mErrorsOnly = AddCheckbox(layout, "Errors only", "Check errors only", false);

    //Verbose
    mVerbose = AddCheckbox(layout, "Verbose", "Check verbose", false);

    //Force
    mForce = AddCheckbox(layout, "Force", "Check force", false);

    //XML
    mXml = AddCheckbox(layout, "XML", "Check xml", false);

    //Unused functions
    mUnusedFunctions = AddCheckbox(layout, "Unused functions", "Check unused functions", false);

    //Security
    mSecurity = AddCheckbox(layout, "Security", "Check security", false);

    //Vcl
    mVcl = AddCheckbox(layout, "Vcl", "Check vcl", false);






    buttonLayout->addWidget(ok);
    buttonLayout->addWidget(cancel);
    layout->addLayout(buttonLayout);


    connect(ok, SIGNAL(clicked()),
            this, SLOT(accept()));
    connect(cancel, SIGNAL(clicked()),
            this, SLOT(reject()));

    setWindowTitle("Select files to check");
    setLayout(layout);

    LoadSettings();

}

CheckDialog::~CheckDialog()
{
    SaveSettings();
}





QStringList CheckDialog::GetSelectedFiles()
{
    QModelIndexList indexes = mFileTree->selectionModel()->selectedRows();
    QStringList list;
    QModelIndex index;

    foreach(index, indexes)
    {
        list << mModel.filePath(index);
    }

    return list;
}


Qt::CheckState CheckDialog::BoolToCheckState(bool yes)
{
    if (yes)
    {
        return Qt::Checked;
    }
    return Qt::Unchecked;
}

bool CheckDialog::CheckStateToBool(Qt::CheckState state)
{
    if (state == Qt::Checked)
    {
        return true;
    }
    return false;
}

QCheckBox* CheckDialog::AddCheckbox(QVBoxLayout *layout,
                                    const QString &label,
                                    const QString &settings,
                                    bool value)
{
    QCheckBox *result = new QCheckBox(label);
    result->setCheckState(BoolToCheckState(mSettings.value(settings, value).toInt()));

    //layout->addWidget(new QLabel(label));
    layout->addWidget(result);

    return result;
}

Settings CheckDialog::GetSettings()
{
    Settings result;
    result._debug = CheckStateToBool(mDebug->checkState());
    result._showAll = CheckStateToBool(mShowAll->checkState());
    result._checkCodingStyle = CheckStateToBool(mCheckCodingStyle->checkState());
    result._errorsOnly = CheckStateToBool(mErrorsOnly->checkState());
    result._verbose = CheckStateToBool(mVerbose->checkState());
    result._force = CheckStateToBool(mForce->checkState());
    result._xml = CheckStateToBool(mXml->checkState());
    result._unusedFunctions = CheckStateToBool(mUnusedFunctions->checkState());
    result._security = CheckStateToBool(mSecurity->checkState());
    result._vcl = CheckStateToBool(mVcl->checkState());
    result._jobs = mJobs->text().toInt();
    return result;
}

QString CheckDialog::GetDefaultPath()
{
    QStringList list = GetSelectedFiles();
    int len = 9999;
    QString file, shortest;

    if (list.size() == 0)
    {
        return QDir::currentPath();
    }

    foreach(file, list)
    {
        if (file.size() < len)
        {
            shortest = file;
            len = file.size();
        }
    }

    return shortest;
}

void CheckDialog::LoadSettings()
{
    /*
    if (mSettings.value("Check dialog maximized",false).toBool())
    {
        showMaximized();
    }
    else
    {*/
    resize(mSettings.value("Check dialog width", 800).toInt(), mSettings.value("Check dialog height", 600).toInt());
    //}

    for (int i = 0;i < mModel.columnCount();i++)
    {
        //mFileTree.columnWidth(i);
        QString temp = QString("Check dialog column %1 width").arg(i);
        mFileTree->setColumnWidth(i, mSettings.value(temp, 800 / mModel.columnCount()).toInt());
    }
}

void CheckDialog::SaveSettings()
{
    mSettings.setValue("Check dialog width", size().width());
    mSettings.setValue("Check dialog height", size().height());
    //mSettings.setValue("Check dialog maximized", isMaximized());

    for (int i = 0;i < mModel.columnCount();i++)
    {
        QString temp = QString("Check dialog column %1 width").arg(i);
        mSettings.setValue(temp, mFileTree->columnWidth(i));
    }
}

void CheckDialog::SaveCheckboxValues()
{
    mSettings.setValue("Check threads", mJobs->text().toInt());
    SaveCheckboxValue(mDebug, "Check debug");
    SaveCheckboxValue(mShowAll, "Check show all");
    SaveCheckboxValue(mCheckCodingStyle, "Check coding style");
    SaveCheckboxValue(mErrorsOnly, "Check errors only");
    SaveCheckboxValue(mVerbose, "Check verbose");
    SaveCheckboxValue(mForce, "Check force");
    SaveCheckboxValue(mXml, "Check xml");
    SaveCheckboxValue(mUnusedFunctions, "Check unused functions");
    SaveCheckboxValue(mSecurity, "Check security");
    SaveCheckboxValue(mVcl, "Check vcl");
}

void CheckDialog::SaveCheckboxValue(QCheckBox *box, const QString &name)
{
    mSettings.setValue(name, CheckStateToBool(box->checkState()));
}
