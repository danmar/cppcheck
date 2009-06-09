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


#include "resultsview.h"
#include <QDebug>
#include <QVBoxLayout>
#include <QFile>
#include <QMessageBox>

ResultsView::ResultsView(QSettings &settings, ApplicationList &list) :
        mErrorsFound(false),
        mShowNoErrorsMessage(true)
{
    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

    mProgress = new QProgressBar();
    layout->addWidget(mProgress);
    mProgress->setMinimum(0);
    mProgress->setVisible(false);

    mTree = new ResultsTree(settings, list);
    layout->addWidget(mTree);

    mShowNoErrorsMessage = settings.value(tr("Show no errors message"), true).toBool();

}

ResultsView::~ResultsView()
{
    //dtor
}


void ResultsView::Clear()
{
    mTree->Clear();
    mErrorsFound = false;

    //Clear the progressbar
    mProgress->setMaximum(100);
    mProgress->setValue(0);
}



void ResultsView::Progress(int value, int max)
{
    mProgress->setMaximum(max);
    mProgress->setValue(value);
    if (value >= max)
    {
        mProgress->setVisible(false);
        //Should we inform user of non visible/not found errors?
        if (mShowNoErrorsMessage)
        {   //Tell user that we found no errors
            if (!mErrorsFound)
            {
                QMessageBox msg(QMessageBox::Information,
                                tr("Cppcheck"),
                                tr("No errors found."),
                                QMessageBox::Ok,
                                this);

                msg.exec();
            } //If we have errors but they aren't visible, tell user about it
            else if (!mTree->VisibleErrors())
            {
                QString text = tr("Errors were found, but they are configured to be hidden.\n"\
                                  "To toggle what kind of errors are shown, open view menu.");
                QMessageBox msg(QMessageBox::Information,
                                tr("Cppcheck"),
                                text,
                                QMessageBox::Ok,
                                this);

                msg.exec();
            }
        }
    }
    else
    {
        mProgress->setVisible(true);
    }
}

void ResultsView::Error(const QString &file,
                        const QString &severity,
                        const QString &message,
                        const QStringList &files,
                        const QVariantList &lines,
                        const QString &id)
{
    mErrorsFound = true;
    mTree->AddErrorItem(file, severity, message, files, lines, id);
    emit GotResults();
}

void ResultsView::ShowResults(ShowTypes type, bool show)
{
    mTree->ShowResults(type, show);
}

void ResultsView::CollapseAllResults()
{
    mTree->collapseAll();
}

void ResultsView::ExpandAllResults()
{
    mTree->expandAll();
}

void ResultsView::Save(const QString &filename, bool xml)
{
    if (!mErrorsFound)
    {
        QMessageBox msgBox;
        msgBox.setText("No errors found, nothing to save.");
        msgBox.exec();
    }

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return;
    }

    QTextStream out(&file);
    mTree->SaveResults(out, xml);
}


void ResultsView::UpdateSettings(bool showFullPath,
                                 bool saveFullPath,
                                 bool saveAllErrors,
                                 bool showNoErrorsMessage)
{
    mTree->UpdateSettings(showFullPath, saveFullPath, saveAllErrors);
    mShowNoErrorsMessage = showNoErrorsMessage;
}

void ResultsView::SetCheckDirectory(const QString &dir)
{
    mTree->SetCheckDirectory(dir);
}

void ResultsView::CheckingStarted()
{
    mProgress->setVisible(true);
}


