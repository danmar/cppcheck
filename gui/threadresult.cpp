/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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

#include <QFile>
#include <QString>
#include <QMutexLocker>
#include <QList>
#include <QStringList>
#include <QDebug>
#include "erroritem.h"
#include "errorlogger.h"
#include "threadresult.h"

// The maximum value for the progress bar
const double PROGRESS_MAX = 1024.0;

ThreadResult::ThreadResult() : mMaxProgress(0), mProgress(0), mFilesChecked(0), mTotalFiles(0)
{
    //ctor
}

ThreadResult::~ThreadResult()
{
    //dtor
}

void ThreadResult::reportOut(const std::string &outmsg)
{
    emit Log(QString::fromStdString(outmsg));
}

void ThreadResult::FileChecked(const QString &file)
{
    QMutexLocker locker(&mutex);

    mProgress += QFile(file).size();
    mFilesChecked ++;

    if (mMaxProgress > 0)
    {
        const int value = static_cast<int>(PROGRESS_MAX * mProgress / mMaxProgress);
        const QString description = tr("%1 of %2 files checked").arg(mFilesChecked).arg(mTotalFiles);

        emit Progress(value, description);
    }
}

void ThreadResult::reportErr(const ErrorLogger::ErrorMessage &msg)
{
    QMutexLocker locker(&mutex);

    QList<unsigned int> lines;
    QStringList files;

    for (std::list<ErrorLogger::ErrorMessage::FileLocation>::const_iterator tok = msg._callStack.begin();
         tok != msg._callStack.end();
         ++tok)
    {
        files << QString((*tok).getfile(false).c_str());
        lines << (*tok).line;
    }

    ErrorItem item;
    item.file = QString(callStackToString(msg._callStack).c_str());
    item.files = files;
    item.id = QString(msg._id.c_str());
    item.lines = lines;
    item.summary = QString::fromStdString(msg.shortMessage());
    item.message = QString::fromStdString(msg.verboseMessage());
    item.severity = msg._severity;
    item.inconclusive = msg._inconclusive;

    if (msg._severity != Severity::debug)
        emit Error(item);
    else
        emit DebugError(item);
}

QString ThreadResult::GetNextFile()
{
    QMutexLocker locker(&mutex);
    if (mFiles.size() == 0)
    {
        return "";
    }

    return mFiles.takeFirst();
}

void ThreadResult::SetFiles(const QStringList &files)
{
    QMutexLocker locker(&mutex);
    mFiles = files;
    mProgress = 0;
    mFilesChecked = 0;
    mTotalFiles = files.size();

    // Determine the total size of all of the files to check, so that we can
    // show an accurate progress estimate
    quint64 sizeOfFiles = 0;
    foreach(const QString& file, files)
    {
        sizeOfFiles += QFile(file).size();
    }
    mMaxProgress = sizeOfFiles;
}

void ThreadResult::ClearFiles()
{
    QMutexLocker locker(&mutex);
    mFiles.clear();
    mFilesChecked = 0;
    mTotalFiles = 0;
}

int ThreadResult::GetFileCount()
{
    QMutexLocker locker(&mutex);
    return mFiles.size();
}
