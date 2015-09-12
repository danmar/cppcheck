/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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

#include <QObject>
#include <QFileInfo>
#include <QStringList>
#include <QDebug>
#include "common.h"
#include "settings.h"
#include "checkthread.h"
#include "threadhandler.h"
#include "resultsview.h"

ThreadHandler::ThreadHandler(QObject *parent) :
    QObject(parent),
    mScanDuration(0),
    mRunningThreadCount(0)
{
    SetThreadCount(1);
}

ThreadHandler::~ThreadHandler()
{
    RemoveThreads();
}

void ThreadHandler::ClearFiles()
{
    mLastFiles.clear();
    mResults.ClearFiles();
}

void ThreadHandler::SetFiles(const QStringList &files)
{
    mResults.SetFiles(files);
    mLastFiles = files;
}

void ThreadHandler::Check(const Settings &settings, bool recheck)
{
    if (recheck && mRunningThreadCount == 0) {
        // only recheck changed files
        mResults.SetFiles(GetReCheckFiles());
    }

    if (mResults.GetFileCount() == 0 || mRunningThreadCount > 0 || settings._jobs == 0) {
        qDebug() << "Can't start checking if there's no files to check or if check is in progress.";
        emit Done();
        return;
    }

    SetThreadCount(settings._jobs);

    mRunningThreadCount = mThreads.size();

    if (mResults.GetFileCount() < mRunningThreadCount) {
        mRunningThreadCount = mResults.GetFileCount();
    }

    for (int i = 0; i < mRunningThreadCount; i++) {
        mThreads[i]->Check(settings);
    }

    // Date and time when checking starts..
    mCheckStartTime = QDateTime::currentDateTime();

    mTime.start();
}

bool ThreadHandler::IsChecking() const
{
    return mRunningThreadCount > 0;
}

void ThreadHandler::SetThreadCount(const int count)
{
    if (mRunningThreadCount > 0 ||
        count == mThreads.size() ||
        count <= 0) {
        return;
    }

    //Remove unused old threads
    RemoveThreads();
    //Create new threads
    for (int i = mThreads.size(); i < count; i++) {
        mThreads << new CheckThread(mResults);
        connect(mThreads.last(), SIGNAL(Done()),
                this, SLOT(ThreadDone()));
        connect(mThreads.last(), SIGNAL(FileChecked(const QString &)),
                &mResults, SLOT(FileChecked(const QString &)));
    }

}


void ThreadHandler::RemoveThreads()
{
    for (int i = 0; i < mThreads.size(); i++) {
        mThreads[i]->terminate();
        disconnect(mThreads.last(), SIGNAL(Done()),
                   this, SLOT(ThreadDone()));
        disconnect(mThreads.last(), SIGNAL(FileChecked(const QString &)),
                   &mResults, SLOT(FileChecked(const QString &)));

        delete mThreads[i];
    }

    mThreads.clear();
}

void ThreadHandler::ThreadDone()
{
    mRunningThreadCount--;
    if (mRunningThreadCount == 0) {
        emit Done();

        mScanDuration = mTime.elapsed();

        // Set date/time used by the recheck
        if (!mCheckStartTime.isNull()) {
            mLastCheckTime = mCheckStartTime;
            mCheckStartTime = QDateTime();
        }
    }
}

void ThreadHandler::Stop()
{
    mCheckStartTime = QDateTime();
    for (int i = 0; i < mThreads.size(); i++) {
        mThreads[i]->stop();
    }
}

void ThreadHandler::Initialize(ResultsView *view)
{
    connect(&mResults, SIGNAL(Progress(int, const QString&)),
            view, SLOT(Progress(int, const QString&)));

    connect(&mResults, SIGNAL(Error(const ErrorItem &)),
            view, SLOT(Error(const ErrorItem &)));

    connect(&mResults, SIGNAL(Log(const QString &)),
            parent(), SLOT(Log(const QString &)));

    connect(&mResults, SIGNAL(DebugError(const ErrorItem &)),
            parent(), SLOT(DebugError(const ErrorItem &)));
}

void ThreadHandler::LoadSettings(QSettings &settings)
{
    SetThreadCount(settings.value(SETTINGS_CHECK_THREADS, 1).toInt());
}

void ThreadHandler::SaveSettings(QSettings &settings) const
{
    settings.setValue(SETTINGS_CHECK_THREADS, mThreads.size());
}

bool ThreadHandler::HasPreviousFiles() const
{
    return !mLastFiles.isEmpty();
}

int ThreadHandler::GetPreviousFilesCount() const
{
    return mLastFiles.size();
}

int ThreadHandler::GetPreviousScanDuration() const
{
    return mScanDuration;
}

QStringList ThreadHandler::GetReCheckFiles() const
{
    if (mLastCheckTime.isNull())
        return mLastFiles;

    std::set<QString> modified;
    std::set<QString> unmodified;

    QStringList files;
    for (int i = 0; i < mLastFiles.size(); ++i) {
        if (NeedsReCheck(mLastFiles[i], modified, unmodified))
            files.push_back(mLastFiles[i]);
    }
    return files;
}

bool ThreadHandler::NeedsReCheck(const QString &filename, std::set<QString> &modified, std::set<QString> &unmodified) const
{
    if (modified.find(filename) != modified.end())
        return true;

    if (unmodified.find(filename) != unmodified.end())
        return false;

    if (QFileInfo(filename).lastModified() > mLastCheckTime) {
        return true;
    }

    // Parse included files recursively
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    // prevent recursion..
    unmodified.insert(filename);

    QTextStream in(&f);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith("#include \"")) {
            line.remove(0,10);
            int i = line.indexOf("\"");
            if (i > 0) {
                line.remove(i,line.length());
                line = QFileInfo(filename).absolutePath() + "/" + line;
                if (NeedsReCheck(line, modified, unmodified)) {
                    modified.insert(line);
                    return true;
                }
            }
        }
    }

    return false;
}
