/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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

#ifndef THREADEXECUTOR_H
#define THREADEXECUTOR_H

#include <vector>
#include <string>
#include <list>
#include "settings.h"
#include "errorlogger.h"

#if (defined(__GNUC__) || defined(__sun)) && !defined(__MINGW32__)
#define THREADING_MODEL_FORK
#endif

/// @addtogroup CLI
/// @{

/**
 * This class will take a list of filenames and settings and check then
 * all files using threads.
 */
class ThreadExecutor : public ErrorLogger {
public:
    ThreadExecutor(const std::vector<std::string> &filenames, const std::map<std::string, long> &filesizes, Settings &settings, ErrorLogger &_errorLogger);
    virtual ~ThreadExecutor();
    unsigned int check();
    virtual void reportOut(const std::string &outmsg);
    virtual void reportErr(const ErrorLogger::ErrorMessage &msg);

    /**
     * @brief Add content to a file, to be used in unit testing.
     *
     * @param path File name (used as a key to link with real file).
     * @param content If the file would be a real file, this should be
     * the content of the file.
     */
    void addFileContent(const std::string &path, const std::string &content);

private:
    const std::vector<std::string> &_filenames;
    const std::map<std::string, long> &_filesizes;
    Settings &_settings;
    ErrorLogger &_errorLogger;
    unsigned int _fileCount;

    /** @brief Key is file name, and value is the content of the file */
    std::map<std::string, std::string> _fileContents;

#ifdef THREADING_MODEL_FORK
private:
    /**
     * Read from the pipe, parse and handle what ever is in there.
     *@return -1 in case of error
     *         0 if there is nothing in the pipe to be read
     *         1 if we did read something
     */
    int handleRead(int rpipe, unsigned int &result);
    void writeToPipe(char type, const std::string &data);
    /**
     * Write end of status pipe, different for each child.
     * Not used in master process.
     */
    int _wpipe;
    std::list<std::string> _errorList;
public:
    /**
     * @return true if support for threads exist.
     */
    static bool isEnabled() {
        return true;
    }
#else
public:
    /**
     * @return true if support for threads exist.
     */
    static bool isEnabled() {
        return false;
    }
#endif

private:
    /** disabled copy constructor */
    ThreadExecutor(const ThreadExecutor &);

    /** disabled assignment operator */
    void operator=(const ThreadExecutor &);
};

/// @}

#endif // THREADEXECUTOR_H
