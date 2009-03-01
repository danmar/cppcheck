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


#include "checkthread.h"
#include <QDebug>

CheckThread::CheckThread() : mCppCheck(*this)
{
    //ctor
}

CheckThread::~CheckThread()
{
    //dtor
}

void CheckThread::SetSettings(Settings settings)
{
    mCppCheck.settings(settings);
}

void CheckThread::AddFile(const QString &file)
{
    mCppCheck.addFile(file.toStdString());
}

void CheckThread::ClearFiles()
{
    mCppCheck.clearFiles();
}

void CheckThread::run()
{
    mCppCheck.check();
    emit Done();
}


void CheckThread::reportOut(const std::string &outmsg)
{
    emit CurrentFile(QString(outmsg.c_str()));
}

void CheckThread::reportErr(const ErrorLogger::ErrorMessage &msg)
{
    emit Error(QString(callStackToString(msg._callStack).c_str()),
               QString(msg._severity.c_str()),
               QString(msg._msg.c_str()));
    /*
    qDebug()<<"Error: ";
    qDebug()<<QString(callStackToString(msg._callStack).c_str());
    qDebug()<<QString(msg._severity.c_str());
    qDebug()<<QString(msg._msg.c_str());
    qDebug()<<endl;
    */

}

void CheckThread::reportStatus(unsigned int index, unsigned int max)
{
    emit Progress(index, max);
}
