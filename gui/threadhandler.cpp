/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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

#include "threadhandler.h"

#include "checkthread.h"
#include "common.h"
#include "resultsview.h"
#include "settings.h"

#include <string>
#include <unordered_set>
#include <utility>

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QSettings>
#include <QTextStream>
#include <QVariant>

ThreadHandler::ThreadHandler(QObject *parent) :
    QObject(parent)
{
    setThreadCount(1);
}

ThreadHandler::~ThreadHandler()
{
    removeThreads();
}

void ThreadHandler::clearFiles()
{
    mLastFiles.clear();
    mResults.clearFiles();
    mAnalyseWholeProgram = false;
    mAddonsAndTools.clear();
    mSuppressions.clear();
}

void ThreadHandler::setFiles(const QStringList &files)
{
    mResults.setFiles(files);
    mLastFiles = files;
}

void ThreadHandler::setProject(const ImportProject &prj)
{
    mResults.setProject(prj);
    mLastFiles.clear();
}

void ThreadHandler::setCheckFiles(bool all)
{
    if (mRunningThreadCount == 0) {
        mResults.setFiles(getReCheckFiles(all));
    }
}

void ThreadHandler::setCheckFiles(const QStringList& files)
{
    if (mRunningThreadCount == 0) {
        mResults.setFiles(files);
    }
}

void ThreadHandler::check(const Settings &settings)
{
    if (mResults.getFileCount() == 0 || mRunningThreadCount > 0 || settings.jobs == 0) {
        qDebug() << "Can't start checking if there's no files to check or if check is in progress.";
        emit done();
        return;
    }

    setThreadCount(settings.jobs);

    mRunningThreadCount = mThreads.size();

    if (mResults.getFileCount() < mRunningThreadCount) {
        mRunningThreadCount = mResults.getFileCount();
    }

    QStringList addonsAndTools = mAddonsAndTools;
    for (const std::string& addon: settings.addons) {
        QString s = QString::fromStdString(addon);
        if (!addonsAndTools.contains(s))
            addonsAndTools << s;
    }

    for (int i = 0; i < mRunningThreadCount; i++) {
        mThreads[i]->setAddonsAndTools(addonsAndTools);
        mThreads[i]->setSuppressions(mSuppressions);
        mThreads[i]->setClangIncludePaths(mClangIncludePaths);
        mThreads[i]->check(settings);
    }

    // Date and time when checking starts..
    mCheckStartTime = QDateTime::currentDateTime();

    mAnalyseWholeProgram = true;

    mTimer.start();
}

bool ThreadHandler::isChecking() const
{
    return mRunningThreadCount > 0;
}

void ThreadHandler::setThreadCount(const int count)
{
    if (mRunningThreadCount > 0 ||
        count == mThreads.size() ||
        count <= 0) {
        return;
    }

    //Remove unused old threads
    removeThreads();
    //Create new threads
    for (int i = mThreads.size(); i < count; i++) {
        mThreads << new CheckThread(mResults);
        connect(mThreads.last(), &CheckThread::done,
                this, &ThreadHandler::threadDone);
        connect(mThreads.last(), &CheckThread::fileChecked,
                &mResults, &ThreadResult::fileChecked);
    }
}


void ThreadHandler::removeThreads()
{
    for (CheckThread* thread : mThreads) {
        if (thread->isRunning()) {
            thread->terminate();
            thread->wait();
        }
        disconnect(thread, &CheckThread::done,
                   this, &ThreadHandler::threadDone);
        disconnect(thread, &CheckThread::fileChecked,
                   &mResults, &ThreadResult::fileChecked);
        delete thread;
    }

    mThreads.clear();
    mAnalyseWholeProgram = false;
}

void ThreadHandler::threadDone()
{
    if (mRunningThreadCount == 1 && mAnalyseWholeProgram) {
        mThreads[0]->analyseWholeProgram(mLastFiles);
        mAnalyseWholeProgram = false;
        return;
    }

    mRunningThreadCount--;
    if (mRunningThreadCount == 0) {
        emit done();

        mScanDuration = mTimer.elapsed();

        // Set date/time used by the recheck
        if (!mCheckStartTime.isNull()) {
            mLastCheckTime = mCheckStartTime;
            mCheckStartTime = QDateTime();
        }
    }
}

void ThreadHandler::stop()
{
    mCheckStartTime = QDateTime();
    mAnalyseWholeProgram = false;
    for (CheckThread* thread : mThreads) {
        thread->stop();
    }
}

void ThreadHandler::initialize(ResultsView *view)
{
    connect(&mResults, &ThreadResult::progress,
            view, &ResultsView::progress);

    connect(&mResults, &ThreadResult::error,
            view, &ResultsView::error);

    connect(&mResults, &ThreadResult::log,
            this, &ThreadHandler::log);

    connect(&mResults, &ThreadResult::debugError,
            this, &ThreadHandler::debugError);
}

void ThreadHandler::loadSettings(const QSettings &settings)
{
    setThreadCount(settings.value(SETTINGS_CHECK_THREADS, 1).toInt());
}

void ThreadHandler::saveSettings(QSettings &settings) const
{
    settings.setValue(SETTINGS_CHECK_THREADS, mThreads.size());
}

bool ThreadHandler::hasPreviousFiles() const
{
    return !mLastFiles.isEmpty();
}

int ThreadHandler::getPreviousFilesCount() const
{
    return mLastFiles.size();
}

int ThreadHandler::getPreviousScanDuration() const
{
    return mScanDuration;
}

QStringList ThreadHandler::getReCheckFiles(bool all) const
{
    if (mLastCheckTime.isNull() || all)
        return mLastFiles;

    std::set<QString> modified;
    std::set<QString> unmodified;

    QStringList files;
    for (int i = 0; i < mLastFiles.size(); ++i) {
        if (needsReCheck(mLastFiles[i], modified, unmodified))
            files.push_back(mLastFiles[i]);
    }
    return files;
}

bool ThreadHandler::needsReCheck(const QString &filename, std::set<QString> &modified, std::set<QString> &unmodified) const
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
            const int i = line.indexOf("\"");
            if (i > 0) {
                line.remove(i,line.length());
                line = QFileInfo(filename).absolutePath() + "/" + line;
                if (needsReCheck(line, modified, unmodified)) {
                    modified.insert(std::move(line));
                    return true;
                }
            }
        }
    }

    return false;
}

QDateTime ThreadHandler::getCheckStartTime() const
{
    return mCheckStartTime;
}

void ThreadHandler::setCheckStartTime(QDateTime checkStartTime)
{
    mCheckStartTime = std::move(checkStartTime);
}
