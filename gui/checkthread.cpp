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

            if (!fileSettings->cfg.empty() && fileSettings->cfg.compare(0,5,"Debug") != 0)
                continue;

            QStringList args;
            for (std::list<std::string>::const_iterator I = fileSettings->includePaths.begin(); I != fileSettings->includePaths.end(); ++I)
                args << ("-I" + QString::fromStdString(*I));
            for (std::list<std::string>::const_iterator i = fileSettings->systemIncludePaths.begin(); i != fileSettings->systemIncludePaths.end(); ++i)
                args << "-isystem" << QString::fromStdString(*i);
            foreach (QString D, QString::fromStdString(fileSettings->defines).split(";")) {
                args << ("-D" + D);
            }

            if (!mClangPath.isEmpty()) {
                QDir dir(mClangPath + "/../lib/clang");
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

            QString analyzerInfoFile;

            const std::string &buildDir = mCppcheck.settings().buildDir;
            if (!buildDir.empty()) {
                analyzerInfoFile = QString::fromStdString(AnalyzerInformation::getAnalyzerInfoFile(buildDir, fileSettings->filename, fileSettings->cfg));

                const QString cmd(mClangPath.isEmpty() ? QString("clang") : (mClangPath + "/clang.exe"));
                QStringList args2(args);
                args2.insert(0,"-E");
                args2 << fileName;
                QProcess process;
                process.start(cmd,args2);
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

            if (addon == CLANG) {
                args.insert(0,"--analyze");
                args.insert(1, "-Xanalyzer");
                args.insert(2, "-analyzer-output=text");
                args << fileName;
            } else {
                args.insert(0,"-checks=*,-clang*,-llvm*");
                args.insert(1, fileName);
                args.insert(2, "--");
            }

            const QString cmd(mClangPath.isEmpty() ? addon : (mClangPath + '/' + addon + ".exe"));
            {
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
            process.start(cmd, args);
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

            const QString python = mPythonPath.isEmpty() ? QString("python") : mPythonPath;
            QStringList args;
            args << a << dumpFile;
            qDebug() << python << args;

            QProcess process;
            process.start(python, args);
            process.waitForFinished();
            const QString errout(process.readAllStandardError());
            QFile f(dumpFile + '-' + addon + "-results");
            if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&f);
                out << errout;
                f.close();
            }
            parseAddonErrors(errout, addon);
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
    else if (QDir(mDataDir + "/../addons").exists())
        return mDataDir + "/../addons";
    else if (mDataDir.endsWith("/cfg")) {
        if (QDir(mDataDir.mid(0,mDataDir.size()-3) + "addons").exists())
            return mDataDir.mid(0,mDataDir.size()-3) + "addons";
    }
    return QString();
}

void CheckThread::parseAddonErrors(QString err, QString tool)
{
    Q_UNUSED(tool);
    QTextStream in(&err, QIODevice::ReadOnly);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QRegExp r1("\\[([a-zA-Z]?:?[^:]+):([0-9]+)\\][ ][(]([a-z]+)[)]: (.+) \\[([a-zA-Z0-9_\\-\\.]+)\\]");
        if (!r1.exactMatch(line))
            continue;
        const std::string &filename = r1.cap(1).toStdString();
        const int lineNumber = r1.cap(2).toInt();
        const std::string severity = r1.cap(3).toStdString();
        const std::string message = r1.cap(4).toStdString();
        const std::string id = r1.cap(5).toStdString();

        std::list<ErrorLogger::ErrorMessage::FileLocation> callstack;
        callstack.push_back(ErrorLogger::ErrorMessage::FileLocation(filename, lineNumber));
        ErrorLogger::ErrorMessage errmsg(callstack, filename, Severity::fromString(severity), message, id, false);
        mResult.reportErr(errmsg);
    }
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
            e.errorPath.last().file = file0;
            e.errorPath.last().line = 1;
            e.errorPath.last().col  = 1;
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
        errorItem.errorPath.last().col  = r1.cap(3).toInt();
        if (r1.cap(4) == "warning")
            errorItem.severity = Severity::SeverityType::warning;
        else if (r1.cap(4) == "error" || r1.cap(4) == "fatal error")
            errorItem.severity = Severity::SeverityType::error;

        QString message,id;
        if (r2.exactMatch(r1.cap(5))) {
            message = r2.cap(1);
            id = tool + '-' + r2.cap(2);
            if (r2.cap(2) == "performance")
                errorItem.severity = Severity::SeverityType::performance;
            else if (r2.cap(2) == "portability")
                errorItem.severity = Severity::SeverityType::portability;
            else if (r2.cap(2) == "readability")
                errorItem.severity = Severity::SeverityType::style;
        } else {
            message = r1.cap(5);
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
        if (mSuppressions.contains(e.errorId))
            continue;
        std::list<ErrorLogger::ErrorMessage::FileLocation> callstack;
        foreach (const QErrorPathItem &path, e.errorPath) {
            callstack.push_back(ErrorLogger::ErrorMessage::FileLocation(path.file.toStdString(), path.info.toStdString(), path.line));
        }
        const std::string f0 = file0.toStdString();
        const std::string msg = e.message.toStdString();
        const std::string id = e.errorId.toStdString();
        ErrorLogger::ErrorMessage errmsg(callstack, f0, e.severity, msg, id, false);
        mResult.reportErr(errmsg);
    }
}
