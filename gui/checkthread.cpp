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

#include <QString>
#include <QDebug>
#include "checkthread.h"
#include "threadresult.h"
#include "cppcheck.h"

CheckThread::CheckThread(ThreadResult &result) :
    mState(Ready),
    mResult(result),
    mCppcheck(result, true)
{
    //ctor
}

CheckThread::~CheckThread()
{
    //dtor
}

void CheckThread::Check(const Settings &settings)
{
    mCppcheck.settings() = settings;
    start();
}

void CheckThread::run()
{
    mState = Running;
    QString file;
    file = mResult.GetNextFile();

    while (!file.isEmpty() && mState == Running) {
        qDebug() << "Checking file" << file;
        mCppcheck.check(file.toStdString());
        emit FileChecked(file);

        if (mState == Running)
            file = mResult.GetNextFile();
    }
    if (mState == Running)
        mState = Ready;
    else
        mState = Stopped;

    emit Done();
}

void CheckThread::stop()
{
    mState = Stopping;
    mCppcheck.terminate();
}
