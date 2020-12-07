/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
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

#include <QApplication>
#include <QString>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QSettings>
#include "checkthread.h"
#include "erroritem.h"
#include "threadresult.h"
#include "cppcheck.h"
#include "common.h"

static bool executeCommand(std::string exe, std::vector<std::string> args, std::string redirect, std::string *output)
{
    output->clear();

    QStringList args2;
    for (std::string arg: args)
        args2 << QString::fromStdString(arg);

    QProcess process;
    process.start(QString::fromStdString(exe), args2);
    process.waitForFinished();

    if (redirect == "2>&1") {
        QString s1 = process.readAllStandardOutput();
        QString s2 = process.readAllStandardError();
        *output = (s1 + "\n" + s2).toStdString();
    } else
        *output = process.readAllStandardOutput().toStdString();

    if (redirect.compare(0,3,"2> ") == 0) {
        std::ofstream fout(redirect.substr(3));
        fout << process.readAllStandardError().toStdString();
    }
    return process.exitCode() == 0;
}


CheckThread::CheckThread(ThreadResult &result) :
    mState(Ready),
    mResult(result),
    mCppcheck(result, true, executeCommand),
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
            for (const QString& file : mFiles)
                files2[file.toStdString()] = 0;
            mCppcheck.analyseWholeProgram(buildDir, files2);
        }
        mFiles.clear();
        emit done();
        return;
    }

    QString file = mResult.getNextFile();
    while (!file.isEmpty() && mState == Running) {
        qDebug() << "Checking file" << file;
        mCppcheck.check(file.toStdString());
        runAddonsAndTools(nullptr, file);
        emit fileChecked(file);

        if (mState == Running)
            file = mResult.getNextFile();
    }

    ImportProject::FileSettings fileSettings = mResult.getNextFileSettings();
    while (!fileSettings.filename.empty() && mState == Running) {
        file = QString::fromStdString(fileSettings.filename);
        qDebug() << "Checking file" << file;
        mCppcheck.check(fileSettings);
        runAddonsAndTools(&fileSettings, QString::fromStdString(fileSettings.filename));
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

void CheckThread::runAddonsAndTools(const ImportProject::FileSettings *fileSettings, const QString &fileName)
{
    foreach (const QString addon, mAddonsAndTools) {
        if (addon == CLANG_ANALYZER || addon == CLANG_TIDY) {
            if (!fileSettings)
                continue;

            if (!fileSettings->cfg.empty() && fileSettings->cfg.compare(0,5,"Debug") != 0)
                continue;

            QStringList args;
            for (std::list<std::string>::const_iterator incIt = fileSettings->includePaths.begin(); incIt != fileSettings->includePaths.end(); ++incIt)
                args << ("-I" + QString::fromStdString(*incIt));
            for (std::list<std::string>::const_iterator i = fileSettings->systemIncludePaths.begin(); i != fileSettings->systemIncludePaths.end(); ++i)
                args << "-isystem" << QString::fromStdString(*i);
            foreach (QString def, QString::fromStdString(fileSettings->defines).split(";")) {
                args << ("-D" + def);
            }
            foreach (const std::string& U, fileSettings->undefs) {
                args << QString::fromStdString("-U" + U);
            }

            const QString clangPath = CheckThread::clangTidyCmd();
            if (!clangPath.isEmpty()) {
                QDir dir(clangPath + "/../lib/clang");
                foreach (QString ver, dir.entryList()) {
                    QString includePath = dir.absolutePath() + '/' + ver + "/include";
                    if (ver[0] != '.' && QDir(includePath).exists()) {
                        args << "-isystem" << includePath;
                        break;
                    }
                }
            }

#ifdef Q_OS_WIN
            // To create compile_commands.json in windows see:
            // https://bitsmaker.gitlab.io/post/clang-tidy-from-vs2015/

            foreach (QString includePath, mClangIncludePaths) {
                if (!includePath.isEmpty()) {
                    includePath.replace("\\", "/");
                    args << "-isystem" << includePath.trimmed();
                }
            }

            args << "-U__STDC__" << "-fno-ms-compatibility";
#endif

            if (!fileSettings->standard.empty())
                args << ("-std=" + QString::fromStdString(fileSettings->standard));
            else {
                switch (mCppcheck.settings().standards.cpp) {
                case Standards::CPP03:
                    args << "-std=c++03";
                    break;
                case Standards::CPP11:
                    args << "-std=c++11";
                    break;
                case Standards::CPP14:
                    args << "-std=c++14";
                    break;
                case Standards::CPP17:
                    args << "-std=c++17";
                    break;
                case Standards::CPP20:
                    args << "-std=c++20";
                    break;
                }
            }

            QString analyzerInfoFile;

            const std::string &buildDir = mCppcheck.settings().buildDir;
            if (!buildDir.empty()) {
                analyzerInfoFile = QString::fromStdString(AnalyzerInformation::getAnalyzerInfoFile(buildDir, fileSettings->filename, fileSettings->cfg));

                QStringList args2(args);
                args2.insert(0,"-E");
                args2 << fileName;
                QProcess process;
                process.start(clangCmd(),args2);
                process.waitForFinished();
                const QByteArray &ba = process.readAllStandardOutput();
                const quint16 chksum = qChecksum(ba.data(), ba.length());

                QFile f1(analyzerInfoFile + '.' + addon + "-E");
                if (f1.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream in1(&f1);
                    const quint16 oldchksum = in1.readAll().toInt();
                    if (oldchksum == chksum) {
                        QFile f2(analyzerInfoFile + '.' + addon + "-results");
                        if (f2.open(QIODevice::ReadOnly | QIODevice::Text)) {
                            QTextStream in2(&f2);
                            parseClangErrors(addon, fileName, in2.readAll());
                            continue;
                        }
                    }
                    f1.close();
                }
                f1.open(QIODevice::WriteOnly | QIODevice::Text);
                QTextStream out1(&f1);
                out1 << chksum;

                QFile::remove(analyzerInfoFile + '.' + addon + "-results");
            }

            if (addon == CLANG_ANALYZER) {
                /*
                // Using clang
                args.insert(0,"--analyze");
                args.insert(1, "-Xanalyzer");
                args.insert(2, "-analyzer-output=text");
                args << fileName;
                */
                // Using clang-tidy
                args.insert(0,"-checks=-*,clang-analyzer-*");
                args.insert(1, fileName);
                args.insert(2, "--");
            } else {
                args.insert(0,"-checks=*,-clang-analyzer-*,-llvm*");
                args.insert(1, fileName);
                args.insert(2, "--");
            }

            {
                const QString cmd(clangTidyCmd());
                QString debug(cmd.contains(" ") ? ('\"' + cmd + '\"') : cmd);
                foreach (QString arg, args) {
                    if (arg.contains(" "))
                        debug += " \"" + arg + '\"';
                    else
                        debug += ' ' + arg;
                }
                qDebug() << debug;

                if (!analyzerInfoFile.isEmpty()) {
                    QFile f(analyzerInfoFile + '.' + addon + "-cmd");
                    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
                        QTextStream out(&f);
                        out << debug;
                    }
                }
            }

            QProcess process;
            process.start(clangTidyCmd(), args);
            process.waitForFinished(600*1000);
            const QString errout(process.readAllStandardOutput() + "\n\n\n" + process.readAllStandardError());
            if (!analyzerInfoFile.isEmpty()) {
                QFile f(analyzerInfoFile + '.' + addon + "-results");
                if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream out(&f);
                    out << errout;
                }
            }

            parseClangErrors(addon, fileName, errout);
        }
    }
}

void CheckThread::stop()
{
    mState = Stopping;
    Settings::terminate();
}

void CheckThread::parseClangErrors(const QString &tool, const QString &file0, QString err)
{
    QList<ErrorItem> errorItems;
    ErrorItem errorItem;
    QRegExp r1("(.+):([0-9]+):([0-9]+): (note|warning|error|fatal error): (.*)");
    QRegExp r2("(.*)\\[([a-zA-Z0-9\\-_\\.]+)\\]");
    QTextStream in(&err, QIODevice::ReadOnly);
    while (!in.atEnd()) {
        QString line = in.readLine();

        if (line.startsWith("Assertion failed:")) {
            ErrorItem e;
            e.errorPath.append(QErrorPathItem());
            e.errorPath.last().file   = file0;
            e.errorPath.last().line   = 1;
            e.errorPath.last().column = 1;
            e.errorId = tool + "-internal-error";
            e.file0 = file0;
            e.message = line;
            e.severity = Severity::information;
            errorItems.append(e);
            continue;
        }

        if (!r1.exactMatch(line))
            continue;
        if (r1.cap(4) != "note") {
            errorItems.append(errorItem);
            errorItem = ErrorItem();
            errorItem.file0 = r1.cap(1);
        }

        errorItem.errorPath.append(QErrorPathItem());
        errorItem.errorPath.last().file = r1.cap(1);
        errorItem.errorPath.last().line = r1.cap(2).toInt();
        errorItem.errorPath.last().column = r1.cap(3).toInt();
        if (r1.cap(4) == "warning")
            errorItem.severity = Severity::SeverityType::warning;
        else if (r1.cap(4) == "error" || r1.cap(4) == "fatal error")
            errorItem.severity = Severity::SeverityType::error;

        QString message,id;
        if (r2.exactMatch(r1.cap(5))) {
            message = r2.cap(1);
            const QString id1(r2.cap(2));
            if (id1.startsWith("clang"))
                id = id1;
            else
                id = tool + '-' + r2.cap(2);
            if (tool == CLANG_TIDY) {
                if (id1.startsWith("performance"))
                    errorItem.severity = Severity::SeverityType::performance;
                else if (id1.startsWith("portability"))
                    errorItem.severity = Severity::SeverityType::portability;
                else if (id1.startsWith("cert") || (id1.startsWith("misc") && !id1.contains("unused")))
                    errorItem.severity = Severity::SeverityType::warning;
                else
                    errorItem.severity = Severity::SeverityType::style;
            }
        } else {
            message = r1.cap(5);
            id = CLANG_ANALYZER;
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
        Suppressions::ErrorMessage errorMessage;
        errorMessage.setFileName(e.errorPath.back().file.toStdString());
        errorMessage.lineNumber = e.errorPath.back().line;
        errorMessage.errorId = e.errorId.toStdString();
        errorMessage.symbolNames = e.symbolNames.toStdString();

        if (isSuppressed(errorMessage))
            continue;

        std::list<ErrorMessage::FileLocation> callstack;
        foreach (const QErrorPathItem &path, e.errorPath) {
            callstack.push_back(ErrorMessage::FileLocation(path.file.toStdString(), path.info.toStdString(), path.line, path.column));
        }
        const std::string f0 = file0.toStdString();
        const std::string msg = e.message.toStdString();
        const std::string id = e.errorId.toStdString();
        ErrorMessage errmsg(callstack, f0, e.severity, msg, id, false);
        mResult.reportErr(errmsg);
    }
}

bool CheckThread::isSuppressed(const Suppressions::ErrorMessage &errorMessage) const
{
    foreach (const Suppressions::Suppression &suppression, mSuppressions) {
        if (suppression.isSuppressed(errorMessage))
            return true;
    }
    return false;
}

QString CheckThread::clangCmd()
{
    QString path = QSettings().value(SETTINGS_CLANG_PATH,QString()).toString();
    if (!path.isEmpty())
        path += '/';
    path += "clang";
#ifdef Q_OS_WIN
    path += ".exe";
#endif

    QProcess process;
    process.start(path, QStringList() << "--version");
    process.waitForFinished();
    if (process.exitCode() == 0)
        return path;

#ifdef Q_OS_WIN
    // Try to autodetect clang
    if (QFileInfo("C:/Program Files/LLVM/bin/clang.exe").exists())
        return "C:/Program Files/LLVM/bin/clang.exe";
#endif

    return QString();
}

QString CheckThread::clangTidyCmd()
{
    QString path = QSettings().value(SETTINGS_CLANG_PATH,QString()).toString();
    if (!path.isEmpty())
        path += '/';
    path += "clang-tidy";
#ifdef Q_OS_WIN
    path += ".exe";
#endif

    QProcess process;
    process.start(path, QStringList() << "--version");
    process.waitForFinished();
    if (process.exitCode() == 0)
        return path;

#ifdef Q_OS_WIN
    // Try to autodetect clang-tidy
    if (QFileInfo("C:/Program Files/LLVM/bin/clang-tidy.exe").exists())
        return "C:/Program Files/LLVM/bin/clang-tidy.exe";
#endif

    return QString();
}
