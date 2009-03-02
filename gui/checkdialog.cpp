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
#include <QFileInfo>
#include <QDirIterator>
#include "../src/filelister.h"

CheckDialog::CheckDialog(QSettings &programSettings) :
        mSettings(programSettings)
{
    QPushButton *cancel = new QPushButton(tr("Cancel"));
    QPushButton *ok = new QPushButton(tr("Ok"));

    //Main layout
    QVBoxLayout *layout = new QVBoxLayout();

    //Layout for ok/cancel buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();


    //File selection tree
    layout->addWidget(new QLabel(tr("Select files to check")));
    mFileTree = new QTreeView();
    layout->addWidget(mFileTree);
    mFileTree->setModel(&mModel);
    mFileTree->scrollTo(mModel.index(programSettings.value(tr("Check path"), QDir::currentPath()).toString()));
    mFileTree->setSelectionMode(QAbstractItemView::ExtendedSelection);

    //Number of jobs
    mJobs = new QLineEdit(programSettings.value(tr("Check threads"), 1).toString());
    layout->addWidget(new QLabel(tr("Number of threads")));
    layout->addWidget(mJobs);
    mJobs->setValidator(new QIntValidator(this));


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

    //XML
    mXml = AddCheckbox(layout, tr("XML"), tr("Check xml"), false);

    //Unused functions
    mUnusedFunctions = AddCheckbox(layout, tr("Unused functions"), tr("Check unused functions"), false);

    //Security
    mSecurity = AddCheckbox(layout, tr("Security"), tr("Check security"), false);

    //Vcl
    mVcl = AddCheckbox(layout, tr("Vcl"), tr("Check vcl"), false);






    buttonLayout->addWidget(ok);
    buttonLayout->addWidget(cancel);
    layout->addLayout(buttonLayout);


    connect(ok, SIGNAL(clicked()),
            this, SLOT(accept()));
    connect(cancel, SIGNAL(clicked()),
            this, SLOT(reject()));

    setWindowTitle(tr("Select files to check"));
    setLayout(layout);

    LoadSettings();

}

CheckDialog::~CheckDialog()
{
    SaveSettings();
}


QStringList CheckDialog::RemoveUnacceptedFiles(const QStringList &list)
{
    QStringList result;
    QString str;
    foreach(str, list)
    {
        if (FileLister::AcceptFile(str.toStdString()))
        {
            result << str;
        }
    }

    return result;
}

QStringList CheckDialog::GetFiles(QModelIndex index)
{
    QFileInfo info(mModel.filePath(index));
    QStringList list;

    if (info.isDir())
    {
        QDirIterator it(mModel.filePath(index), QDirIterator::Subdirectories);

        while (it.hasNext())
        {
            list << it.next();
        }
    }
    else
    {
        list << mModel.filePath(index);
    }

    return list;
}

QStringList CheckDialog::RemoveDuplicates(const QStringList &list)
{
    QHash<QString, int> hash;
    QString str;
    foreach(str, list)
    {
        hash[str] = 0;
    }

    return QStringList(hash.uniqueKeys());
}

QStringList CheckDialog::GetSelectedFiles()
{
    QModelIndexList indexes = mFileTree->selectionModel()->selectedRows();
    QStringList list;
    QModelIndex index;

    foreach(index, indexes)
    {
        if (!mModel.filePath(index).isEmpty())
        {
            list << GetFiles(index);
        }
    }

    QString str;

    return RemoveUnacceptedFiles(RemoveDuplicates(list));
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
    result._debug = false;
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
    resize(mSettings.value(tr("Check dialog width"), 800).toInt(), mSettings.value(tr("Check dialog height"), 600).toInt());
    //}

    for (int i = 0;i < mModel.columnCount();i++)
    {
        //mFileTree.columnWidth(i);
        QString temp = QString(tr("Check dialog column %1 width")).arg(i);
        mFileTree->setColumnWidth(i, mSettings.value(temp, 800 / mModel.columnCount()).toInt());
    }
}

void CheckDialog::SaveSettings()
{
    mSettings.setValue(tr("Check dialog width"), size().width());
    mSettings.setValue(tr("Check dialog height"), size().height());
    //mSettings.setValue("Check dialog maximized", isMaximized());

    for (int i = 0;i < mModel.columnCount();i++)
    {
        QString temp = QString(tr("Check dialog column %1 width")).arg(i);
        mSettings.setValue(temp, mFileTree->columnWidth(i));
    }
}

void CheckDialog::SaveCheckboxValues()
{
    mSettings.setValue(tr("Check threads"), mJobs->text().toInt());
    SaveCheckboxValue(mShowAll, tr("Check show all"));
    SaveCheckboxValue(mCheckCodingStyle, tr("Check coding style"));
    SaveCheckboxValue(mErrorsOnly, tr("Check errors only"));
    SaveCheckboxValue(mVerbose, tr("Check verbose"));
    SaveCheckboxValue(mForce, tr("Check force"));
    SaveCheckboxValue(mXml, tr("Check xml"));
    SaveCheckboxValue(mUnusedFunctions, tr("Check unused functions"));
    SaveCheckboxValue(mSecurity, tr("Check security"));
}

void CheckDialog::SaveCheckboxValue(QCheckBox *box, const QString &name)
{
    mSettings.setValue(name, CheckStateToBool(box->checkState()));
}
