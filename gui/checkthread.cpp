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
#include "erroritem.h"
#include "threadresult.h"
#include "cppcheck.h"

static const char CLANG[] = "clang";
static const char CLANGTIDY[] = "clang-tidy";

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

    const QString addonPath = getAddonPath();

    QString file = mResult.getNextFile();
    while (!file.isEmpty() && mState == Running) {
        qDebug() << "Checking file" << file;
        mCppcheck.check(file.toStdString());
        runAddons(addonPath, nullptr, file);
        emit fileChecked(file);

        if (mState == Running)
            file = mResult.getNextFile();
    }

    ImportProject::FileSettings fileSettings = mResult.getNextFileSettings();
    while (!fileSettings.filename.empty() && mState == Running) {
        file = QString::fromStdString(fileSettings.filename);
        qDebug() << "Checking file" << file;
        mCppcheck.check(fileSettings);
        runAddons(addonPath, &fileSettings, QString::fromStdString(fileSettings.filename));
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

void CheckThread::runAddons(const QString &addonPath, const ImportProject::FileSettings *fileSettings, const QString &fileName)
{
    QString dumpFile;

    foreach (const QString addon, mAddons) {
        if (addon == CLANG || addon == CLANGTIDY) {
            if (!fileSettings)
                continue;

            QStringList args;
            if (addon == CLANG)
                args << "--analyze" << "-Xanalyzer" << "-analyzer-output=text";
            else
                args << "-checks=*,-clang*,-llvm*" << fileName << "--";
#ifdef Q_OS_WIN
            // To create compile_commands.json in windows see:
            // https://bitsmaker.gitlab.io/post/clang-tidy-from-vs2015/

            // TODO: Replace the "QDir::homePath()"
            args << "-isystem" << (QDir::homePath() + "/include");
            args << "-isystem" << (QDir::homePath() + "/include/c++");
            args << "-isystem" << (QDir::homePath() + "/include/c++/i686-w64-mingw32");
            args << "-fno-ms-compatibility";
#endif
            for (std::list<std::string>::const_iterator I = fileSettings->includePaths.begin(); I != fileSettings->includePaths.end(); ++I)
                args << ("-I" + QString::fromStdString(*I));
            for (std::list<std::string>::const_iterator i = fileSettings->systemIncludePaths.begin(); i != fileSettings->systemIncludePaths.end(); ++i)
                args << "-isystem" << QString::fromStdString(*i);
            foreach (QString D, QString::fromStdString(fileSettings->defines).split(";")) {
                args << ("-D" + D);
            }
            if (!fileSettings->standard.empty())
                args << (" -std=" + QString::fromStdString(fileSettings->standard));
            if (addon == CLANG)
                args << fileName;

            const QString cmd(mClangPath.isEmpty() ? addon : (mClangPath + '/' + addon + ".exe"));
            {
                QString debug(cmd);
                foreach (QString arg, args) {
                    debug += ' ' + arg;
                }
                qDebug() << debug;
            }

            QProcess process;
            process.start(cmd, args);
            process.waitForFinished(600*1000);
            if (addon == CLANG) {
                const QString err(process.readAllStandardError());
                parseClangErrors(QString::fromStdString(fileSettings->filename), err);
            }
            else
                parseClangErrors(QString::fromStdString(fileSettings->filename), process.readAllStandardOutput());
        } else {
            QString a;
            if (QFileInfo(addonPath + '/' + addon + ".py").exists())
                a = addonPath + '/' + addon + ".py";
            else if (QFileInfo(addonPath + '/' + addon + '/' + addon + ".py").exists())
                a = addonPath + '/' + addon + '/' + addon + ".py";
            else
                continue;

            if (dumpFile.isEmpty()) {
                const std::string buildDir = mCppcheck.settings().buildDir;
                mCppcheck.settings().buildDir.clear();
                mCppcheck.settings().dump = true;
                if (!buildDir.empty()) {
                    mCppcheck.settings().dumpFile = AnalyzerInformation::getAnalyzerInfoFile(buildDir, fileName.toStdString(), fileSettings ? fileSettings->cfg : std::string()) + ".dump";
                    dumpFile = QString::fromStdString(mCppcheck.settings().dumpFile);
                } else {
                    dumpFile = fileName + ".dump";
                }
                if (fileSettings)
                    mCppcheck.check(*fileSettings);
                else
                    mCppcheck.check(fileName.toStdString());
                mCppcheck.settings().dump = false;
                mCppcheck.settings().dumpFile.clear();
                mCppcheck.settings().buildDir = buildDir;
            }

            QString cmd = "python " + a + ' ' + dumpFile;
            qDebug() << cmd;
            QProcess process;
            process.start(cmd);
            process.waitForFinished();
            parseAddonErrors(process.readAllStandardError(), addon);
        }
    }
}

void CheckThread::stop()
{
    mState = Stopping;
    mCppcheck.terminate();
}

QString CheckThread::getAddonPath() const
{
    if (QFileInfo(mDataDir + "/threadsafety.py").exists())
        return mDataDir;
    else if (QDir(mDataDir + "/addons").exists())
        return mDataDir + "/addons";
    else if (mDataDir.endsWith("/cfg")) {
        if (QDir(mDataDir.mid(0,mDataDir.size()-3) + "addons").exists())
            return mDataDir.mid(0,mDataDir.size()-3) + "addons";
    }
    return QString();
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

void CheckThread::parseClangErrors(const QString &file0, QString err)
{
    QList<ErrorItem> errorItems;
    ErrorItem errorItem;
    QRegExp r1("(.+):([0-9]+):[0-9]+: (note|warning|error|fatal error): (.*)");
    QRegExp r2("(.*)\\[([a-zA-Z0-9\\-_\\.]+)\\]");
    QTextStream in(&err, QIODevice::ReadOnly);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (!r1.exactMatch(line))
            continue;
        if (r1.cap(3) != "note") {
            errorItems.append(errorItem);
            errorItem = ErrorItem();
        }

        errorItem.errorPath.append(QErrorPathItem());
        errorItem.errorPath.last().file = r1.cap(1);
        errorItem.errorPath.last().line = r1.cap(2).toInt();
        if (r1.cap(3) == "warning")
            errorItem.severity = Severity::SeverityType::warning;
        else if (r1.cap(3) == "error" || r1.cap(3) == "fatal error")
            errorItem.severity = Severity::SeverityType::error;

        QString message,id;
        if (r2.exactMatch(r1.cap(4))) {
            message = r2.cap(1);
            id = r2.cap(2);
        } else {
            message = r1.cap(4);
            id = CLANG;
        }

        if (errorItem.errorPath.size() == 1) {
            errorItem.message = message;
            errorItem.errorId = id;
        }

        errorItem.errorPath.last().info = message;
    }
    errorItems.append(errorItem);

    foreach (const ErrorItem &e, errorItems) {
        if (e.errorPath.isEmpty())
            continue;
        std::list<ErrorLogger::ErrorMessage::FileLocation> callstack;
        foreach (const QErrorPathItem &path, e.errorPath) {
            callstack.push_back(ErrorLogger::ErrorMessage::FileLocation(path.file.toStdString(), path.info.toStdString(), path.line));
        }
        ErrorLogger::ErrorMessage errmsg(callstack, file0.toStdString(), errorItem.severity, errorItem.message.toStdString(), errorItem.errorId.toStdString(), false);
        mResult.reportErr(errmsg);
    }
}
