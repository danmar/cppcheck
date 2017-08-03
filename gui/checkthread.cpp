/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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

#include <QString>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include "checkthread.h"
#include "threadresult.h"
#include "cppcheck.h"

CheckThread::CheckThread(ThreadResult &result) :
    mState(Ready),
    mResult(result),
    mCppcheck(result, true),
    mAnalyseWholeProgram(false)
{
    //ctor
}

CheckThread::~CheckThread()
{
    //dtor
}

void CheckThread::check(const Settings &settings)
{
    mFiles.clear();
    mCppcheck.settings() = settings;
    start();
}

void CheckThread::analyseWholeProgram(const QStringList &files)
{
    mFiles = files;
    mAnalyseWholeProgram = true;
    start();
}

void CheckThread::run()
{
    mState = Running;

    if (!mFiles.isEmpty() || mAnalyseWholeProgram) {
        mAnalyseWholeProgram = false;
        qDebug() << "Whole program analysis";
        const std::string &buildDir = mCppcheck.settings().buildDir;
        if (!buildDir.empty()) {
            std::map<std::string,std::size_t> files2;
            for (QString file : mFiles)
                files2[file.toStdString()] = 0;
            mCppcheck.analyseWholeProgram(buildDir, files2);
        }
        mFiles.clear();
        emit done();
        return;
    }

    QString addonPath;
    if (QFileInfo(mDataDir + "/threadsafety.py").exists())
        addonPath = mDataDir;
    else if (QDir(mDataDir + "/addons").exists())
        addonPath = mDataDir + "/addons";
    else if (mDataDir.endsWith("/cfg")) {
        if (QDir(mDataDir.mid(0,mDataDir.size()-3) + "addons").exists())
            addonPath = mDataDir.mid(0,mDataDir.size()-3) + "addons";
    }

    bool needDump = mAddons.contains("y2038") || mAddons.contains("threadsafety") || mAddons.contains("cert") || mAddons.contains("misra");
    QString file = mResult.getNextFile();
    while (!file.isEmpty() && mState == Running) {
        qDebug() << "Checking file" << file;
        mCppcheck.check(file.toStdString());
        if (!mAddons.isEmpty()) {
            if (needDump) {
                mCppcheck.settings().dump = true;
                mCppcheck.check(file.toStdString());
                mCppcheck.settings().dump = false;
            }
            foreach (const QString addon, mAddons) {
                if (addon == "clang")
                    continue;
                QProcess process;
                QString a;
                if (QFileInfo(addonPath + '/' + addon + ".py").exists())
                    a = addonPath + '/' + addon + ".py";
                else if (QFileInfo(addonPath + '/' + addon + '/' + addon + ".py").exists())
                    a = addonPath + '/' + addon + '/' + addon + ".py";
                else
                    continue;
                QString dumpFile = file + ".dump";
                QString cmd = "python " + a + ' ' + dumpFile;
                qDebug() << cmd;
                process.start(cmd);
                process.waitForFinished();
                parseAddonErrors(process.readAllStandardError(), addon);
            }
        }
        emit fileChecked(file);

        if (mState == Running)
            file = mResult.getNextFile();
    }

    ImportProject::FileSettings fileSettings = mResult.getNextFileSettings();
    while (!fileSettings.filename.empty() && mState == Running) {
        file = QString::fromStdString(fileSettings.filename);
        qDebug() << "Checking file" << file;
        mCppcheck.check(fileSettings);
        if (!mAddons.isEmpty()) {
            if (needDump) {
                mCppcheck.settings().dump = true;
                mCppcheck.check(fileSettings);
                mCppcheck.settings().dump = false;
            }
            foreach (const QString addon, mAddons) {
                QProcess process;
                if (addon == "clang") {
                    QString cmd("clang --analyze");
                    for (std::list<std::string>::const_iterator I = fileSettings.includePaths.begin(); I != fileSettings.includePaths.end(); ++I)
                        cmd += " -I" + QString::fromStdString(*I);
                    foreach (QString D, QString::fromStdString(fileSettings.defines).split(";"))
                    cmd += " -D" + D;
                    QString fileName = QString::fromStdString(fileSettings.filename);
                    if (fileName.endsWith(".cpp"))
                        cmd += " -std=c++11";
                    cmd += ' ' + fileName;
                    qDebug() << cmd;
                    process.start(cmd);
                    process.waitForFinished(600*1000);
                    parseClangErrors(process.readAllStandardError());
                } else {
                    QString a;
                    if (QFileInfo(addonPath + '/' + addon + ".py").exists())
                        a = addonPath + '/' + addon + ".py";
                    else if (QFileInfo(addonPath + '/' + addon + '/' + addon + ".py").exists())
                        a = addonPath + '/' + addon + '/' + addon + ".py";
                    else
                        continue;
                    QString dumpFile = QString::fromStdString(fileSettings.filename + ".dump");
                    QString cmd = "python " + a + ' ' + dumpFile;
                    qDebug() << cmd;
                    process.start(cmd);
                    process.waitForFinished();
                    parseAddonErrors(process.readAllStandardError(), addon);
                }
            }
        }
        emit fileChecked(file);

        if (mState == Running)
            fileSettings = mResult.getNextFileSettings();
    }

    if (mState == Running)
        mState = Ready;
    else
        mState = Stopped;

    emit done();
}

void CheckThread::stop()
{
    mState = Stopping;
    mCppcheck.terminate();
}

void CheckThread::parseAddonErrors(QString err, QString tool)
{
    QTextStream in(&err, QIODevice::ReadOnly);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QRegExp r1("\\[([^:]+):([0-9]+)\\](.*)");
        if (!r1.exactMatch(line))
            continue;
        const std::string &filename = r1.cap(1).toStdString();
        const int lineNumber = r1.cap(2).toInt();

        std::string message, id;
        QRegExp r2("(.*)\\[([a-zA-Z0-9\\-\\._]+)\\]");
        if (r2.exactMatch(r1.cap(3))) {
            message = r2.cap(1).toStdString();
            id = tool.toStdString() + '-' + r2.cap(2).toStdString();
        } else {
            message = r1.cap(3).toStdString();
            id = tool.toStdString();
        }
        std::list<ErrorLogger::ErrorMessage::FileLocation> callstack;
        callstack.push_back(ErrorLogger::ErrorMessage::FileLocation(filename, lineNumber));
        ErrorLogger::ErrorMessage errmsg(callstack, filename, Severity::style, message, id, false);
        mResult.reportErr(errmsg);
    }
}

void CheckThread::parseClangErrors(QString err)
{
    QTextStream in(&err, QIODevice::ReadOnly);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QRegExp r("([^:]+):([0-9]+):[0-9]+: (warning|error): (.*)");
        if (!r.exactMatch(line))
            continue;
        const std::string filename = r.cap(1).toStdString();
        const int lineNumber = r.cap(2).toInt();
        Severity::SeverityType severity = (r.cap(3) == "error") ? Severity::error : Severity::warning;
        const std::string message = r.cap(4).toStdString();
        const std::string id = "clang";
        std::list<ErrorLogger::ErrorMessage::FileLocation> callstack;
        callstack.push_back(ErrorLogger::ErrorMessage::FileLocation(filename, lineNumber));
        ErrorLogger::ErrorMessage errmsg(callstack, filename, severity, message, id, false);
        mResult.reportErr(errmsg);
    }
}
