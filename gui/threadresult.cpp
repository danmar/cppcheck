/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
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

#include "threadresult.h"

#include "common.h"
#include "erroritem.h"
#include "errorlogger.h"
#include "errortypes.h"

#include <QFile>

ThreadResult::ThreadResult() : QObject(), ErrorLogger(), mMaxProgress(0), mProgress(0), mFilesChecked(0), mTotalFiles(0)
{
    //ctor
}

ThreadResult::~ThreadResult()
{
    //dtor
}

void ThreadResult::reportOut(const std::string &outmsg, Color)
{
    emit log(QString::fromStdString(outmsg));
}

void ThreadResult::fileChecked(const QString &file)
{
    QMutexLocker locker(&mutex);

    mProgress += QFile(file).size();
    mFilesChecked++;

    if (mMaxProgress > 0) {
        const int value = static_cast<int>(PROGRESS_MAX * mProgress / mMaxProgress);
        const QString description = tr("%1 of %2 files checked").arg(mFilesChecked).arg(mTotalFiles);

        emit progress(value, description);
    }
}

void ThreadResult::reportErr(const ErrorMessage &msg)
{
    QMutexLocker locker(&mutex);
    const ErrorItem item(msg);
    if (msg.severity != Severity::debug)
        emit error(item);
    else
        emit debugError(item);
}

QString ThreadResult::getNextFile()
{
    QMutexLocker locker(&mutex);
    if (mFiles.isEmpty()) {
        return QString();
    }

    return mFiles.takeFirst();
}

ImportProject::FileSettings ThreadResult::getNextFileSettings()
{
    QMutexLocker locker(&mutex);
    if (mFileSettings.empty()) {
        return ImportProject::FileSettings();
    }
    const ImportProject::FileSettings fs = mFileSettings.front();
    mFileSettings.pop_front();
    return fs;
}

void ThreadResult::setFiles(const QStringList &files)
{
    QMutexLocker locker(&mutex);
    mFiles = files;
    mProgress = 0;
    mFilesChecked = 0;
    mTotalFiles = files.size();

    // Determine the total size of all of the files to check, so that we can
    // show an accurate progress estimate
    quint64 sizeOfFiles = 0;
    for (const QString& file : files) {
        sizeOfFiles += QFile(file).size();
    }
    mMaxProgress = sizeOfFiles;
}

void ThreadResult::setProject(const ImportProject &prj)
{
    QMutexLocker locker(&mutex);
    mFiles.clear();
    mFileSettings = prj.fileSettings;
    mProgress = 0;
    mFilesChecked = 0;
    mTotalFiles = prj.fileSettings.size();

    // Determine the total size of all of the files to check, so that we can
    // show an accurate progress estimate
    quint64 sizeOfFiles = 0;
    for (const ImportProject::FileSettings& fs : prj.fileSettings) {
        sizeOfFiles += QFile(QString::fromStdString(fs.filename)).size();
    }
    mMaxProgress = sizeOfFiles;
}

void ThreadResult::clearFiles()
{
    QMutexLocker locker(&mutex);
    mFiles.clear();
    mFileSettings.clear();
    mFilesChecked = 0;
    mTotalFiles = 0;
}

int ThreadResult::getFileCount() const
{
    QMutexLocker locker(&mutex);
    return mFiles.size() + mFileSettings.size();
}
