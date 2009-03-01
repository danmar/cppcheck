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


#include "resultsview.h"
#include <QDebug>
#include <QVBoxLayout>

ResultsView::ResultsView(QSettings &settings)
{
    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

    mProgress = new QProgressBar();
    layout->addWidget(mProgress);
    mProgress->setMinimum(0);

    mTree = new ResultsTree(settings);
    layout->addWidget(mTree);

}

ResultsView::~ResultsView()
{
    //dtor
}


void ResultsView::Clear()
{
    mTree->Clear();
}


void ResultsView::CurrentFile(const QString &filename)
{
    mProgress->setFormat(filename);
}

void ResultsView::Progress(int value, int max)
{
    mProgress->setMaximum(max);
    mProgress->setValue(value);
}

void ResultsView::Error(const QString &filename,
                        const QString &severity,
                        const QString &message)
{
    qDebug() << "Error" << filename << severity << message;
    mTree->AddErrorItem(filename, severity, message);
}
