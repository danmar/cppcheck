/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis
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

#ifndef THREADEXECUTOR_H
#define THREADEXECUTOR_H

#include <vector>
#include <string>
#include "settings.h"
#include "errorlogger.h"

/**
 * This class will take a list of filenames and settings and check then
 * all files using threads.
 */
class ThreadExecutor : public ErrorLogger
{
#if 0
public:
    ThreadExecutor(const std::vector<std::string> &filenames, const Settings &settings, ErrorLogger &_errorLogger);
    virtual ~ThreadExecutor();
    unsigned int check();
    virtual void reportOut(const std::string &outmsg);
    virtual void reportErr(const ErrorLogger::ErrorMessage &msg);
    virtual void reportStatus(unsigned int index, unsigned int max);
protected:
private:
    bool handleRead(unsigned int &result);
    void writeToPipe(char type, const std::string &data);

    const std::vector<std::string> &_filenames;
    const Settings &_settings;
    ErrorLogger &_errorLogger;
    int _pipe[2];
    unsigned int _fileCount;
#else
public:
    ThreadExecutor(const std::vector<std::string> &filenames, const Settings &settings, ErrorLogger &_errorLogger) {};
    virtual ~ThreadExecutor() {};
    unsigned int check()
    {
        return 0;
    }
    virtual void reportOut(const std::string &outmsg) {}
    virtual void reportErr(const ErrorLogger::ErrorMessage &msg) {}
    virtual void reportStatus(unsigned int index, unsigned int max) {}
#endif
};

#endif // THREADEXECUTOR_H
