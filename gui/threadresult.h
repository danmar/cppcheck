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


#ifndef THREADRESULT_H
#define THREADRESULT_H

#include <QMutex>
#include <QObject>
#include <QStringList>
#include "../src/errorlogger.h"

/**
* @brief Threads use this class to obtain new files to process and to publish results
*
*/
class ThreadResult : public QObject, public ErrorLogger
{
    Q_OBJECT
public:
    ThreadResult();
    virtual ~ThreadResult();
    QString GetNextFile();
    void SetFiles(const QStringList &files);
    void ClearFiles();
    int GetFileCount();

    /**
    * ErrorLogger methods
    */
    void reportOut(const std::string &outmsg);
    void reportErr(const ErrorLogger::ErrorMessage &msg);
    void reportStatus(unsigned int index, unsigned int max);
signals:
    void Progress(int value, int max);
    void Error(const QString &file,
               const QString &severity,
               const QString &message,
               const QStringList &files,
               const QList<int> &lines);

protected:
    mutable QMutex mutex;
    QStringList mFiles;
    int mMaxProgress;
    int mProgress;
private:
};

#endif // THREADRESULT_H
