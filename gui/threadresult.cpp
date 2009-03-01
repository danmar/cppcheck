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


#include "threadresult.h"

ThreadResult::ThreadResult() : mMaxProgress(0), mProgress(0)
{
    //ctor
}

ThreadResult::~ThreadResult()
{
    //dtor
}

void ThreadResult::reportOut(const std::string &outmsg)
{
    //emit CurrentFile(QString(outmsg.c_str()));
    Q_UNUSED(outmsg);
}

void ThreadResult::reportErr(const ErrorLogger::ErrorMessage &msg)
{
    QMutexLocker locker(&mutex);

    QList<int> lines;
    QStringList files;

    for (std::list<ErrorLogger::ErrorMessage::FileLocation>::const_iterator tok = msg._callStack.begin();
         tok != msg._callStack.end();
         ++tok)
    {
        files << QString((*tok).file.c_str());
        lines << (*tok).line;
    }

    emit Error(QString(callStackToString(msg._callStack).c_str()),
               QString(msg._severity.c_str()),
               QString(msg._msg.c_str()),
               files,
               lines);

    mProgress++;

    emit Progress(mProgress, mMaxProgress);

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


void ThreadResult::reportStatus(unsigned int index, unsigned int max)
{
    Q_UNUSED(index);
    Q_UNUSED(max);
}

void ThreadResult::SetFiles(const QStringList &files)
{
    //TODO we should check which of the strings in files is actually a path
    //and add the path's contents
    mFiles = files;
    mProgress = 0;
    mMaxProgress = files.size();
}

void ThreadResult::ClearFiles()
{
    mFiles.clear();
}

int ThreadResult::GetFileCount()
{
    return mFiles.size();
}

