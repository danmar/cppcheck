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


#include "threadhandler.h"
#include <QDebug>

ThreadHandler::ThreadHandler() : mThreadCount(1), mRunningThreadCount(0)
{
    SetThreadCount(1);
}

ThreadHandler::~ThreadHandler()
{
    Stop();

    for (int i = 0;i < mThreads.size();i++)
    {
        delete mThreads[i];
    }

    mThreads.clear();
}

void ThreadHandler::ClearFiles()
{
    mResults.ClearFiles();
}

void ThreadHandler::SetFiles(const QStringList &files)
{
    mResults.SetFiles(files);
}

void ThreadHandler::Check(Settings settings)
{
    if (mResults.GetFileCount() == 0 || mRunningThreadCount > 0)
    {
        qDebug() << tr("Can't start checking if there's no files to check or if check is in progress.");
        return;
    }

    SetThreadCount(settings._jobs);


    mRunningThreadCount = mThreadCount;

    if (mResults.GetFileCount() < mRunningThreadCount)
    {
        mRunningThreadCount = mResults.GetFileCount();
    }

    qDebug() << tr("Starting") << mRunningThreadCount << tr("threads");
    for (int i = 0;i < mRunningThreadCount;i++)
    {
        mThreads[i]->Check(settings);
    }
}

void ThreadHandler::SetThreadCount(const int count)
{
    if (mRunningThreadCount > 0 ||
        count == mThreadCount ||
        count <= 0)
    {
        return;
    }

    qDebug() << tr("Setting thead count to") << count;

    mThreadCount = count;

    //Remove unused old threads
    if (mThreads.size() > count)
    {
        for (int i = count;i < mThreads.size();i++)
        {
            disconnect(mThreads.last(), SIGNAL(Done()),
                       this, SLOT(ThreadDone()));
            delete mThreads.takeLast();
        }
    }
    else
    {
        //Create new threads
        for (int i = mThreads.size();i < count;i++)
        {
            mThreads << new CheckThread(mResults);
            connect(mThreads.last(), SIGNAL(Done()),
                    this, SLOT(ThreadDone()));
        }
    }
}

void ThreadHandler::ThreadDone()
{
    mRunningThreadCount--;
    if (mRunningThreadCount == 0)
    {
        emit Done();
    }
}

void ThreadHandler::Stop()
{
    for (int i = 0;i < mThreads.size();i++)
    {
        mThreads[i]->terminate();
    }

    mResults.ClearFiles();
}

void ThreadHandler::Initialize(ResultsView *view)
{

    connect(&mResults, SIGNAL(Progress(int, int)),
            view, SLOT(Progress(int, int)));

    connect(&mResults, SIGNAL(Error(const QString &,
                                    const QString &,
                                    const QString &,
                                    const QStringList &,
                                    const QList<int> &)),
            view, SLOT(Error(const QString &,
                             const QString &,
                             const QString &,
                             const QStringList &,
                             const QList<int> &)));

}

void ThreadHandler::LoadSettings(QSettings &settings)
{
    SetThreadCount(settings.value(tr("Check threads"), 1).toInt());
}

void ThreadHandler::SaveSettings(QSettings &settings)
{
    settings.setValue(tr("Check threads"), mThreadCount);
}

