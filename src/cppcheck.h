/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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

#ifndef CPPCHECK_H
#define CPPCHECK_H

#include <string>
#include <list>
#include <sstream>
#include <vector>
#include <map>
#include "settings.h"
#include "errorlogger.h"
#include "checkunusedfunctions.h"

/// @addtogroup Core
/// @{

/**
 * This is the base class which will use other classes to do
 * static code analysis for C and C++ code to find possible
 * errors or places that could be improved.
 * Usage: See check() for more info.
 */
class CppCheck : public ErrorLogger
{
public:
    /**
     * Constructor.
     */
    CppCheck(ErrorLogger &errorLogger);

    /**
     * Destructor.
     */
    virtual ~CppCheck();

    /**
     * This starts the actual checking. Note that you must call
     * parseFromArgs() or settings() and addFile() before calling this.
     * @return amount of errors found or 0 if none were found.
     */
    unsigned int check();

    /**
     * Adjust the settings before doing the check. E.g. show only
     * actual bugs or also coding style issues.
     *
     * @param settings New settings which will overwrite the old.
     */
    void settings(const Settings &settings);

    /**
     * Get copy of current settings.
     * @return a copy of current settings
     */
    Settings settings() const;

    /**
     * Add new file to be checked.
     *
     * @param path Relative or absolute path to the file to be checked,
     * e.g. "cppcheck.cpp". Note that only source files (.c, .cc or .cpp)
     * should be added to the list. Include files are gathered automatically.
     * You can also give path, e.g. "src/" which will be scanned for source
     * files recursively.
     */
    void addFile(const std::string &path);

    /**
     * Add new unreal file to be checked.
     *
     * @param path File name (used for error reporting).
     * @param content If the file would be a real file, this should be
     * the content of the file.
     */
    void addFile(const std::string &path, const std::string &content);

    /**
     * Remove all files added with addFile() and parseFromArgs().
     */
    void clearFiles();

    /**
     * Parse command line args and get settings and file lists
     * from there.
     *
     * @param argc argc from main()
     * @param argv argv from main()
     * @throw std::runtime_error when errors are found in the input
     */
    void parseFromArgs(int argc, const char* const argv[]);

    const std::vector<std::string> &filenames() const;

    virtual void reportStatus(unsigned int index, unsigned int max);

private:
    void checkFile(const std::string &code, const char FileName[]);

    /**
     * Errors and warnings are directed here.
     *
     * @param msg Errors messages are normally in format
     * "[filepath:line number] Message", e.g.
     * "[main.cpp:4] Uninitialized member variable"
     */
    virtual void reportErr(const ErrorLogger::ErrorMessage &msg);

    /**
     * Information about progress is directed here.
     *
     * @param outmsg Message to show, e.g. "Checking main.cpp..."
     */
    virtual void reportOut(const std::string &outmsg);

    std::list<std::string> _errorList;
    std::ostringstream _errout;
    Settings _settings;
    std::vector<std::string> _filenames;
    /** Key is file name, and value is the content of the file */
    std::map<std::string, std::string> _fileContents;
    CheckUnusedFunctions _checkUnusedFunctions;
    ErrorLogger *_errorLogger;

    /** Current configuration */
    std::string     cfg;

    std::list<std::string> _xmllist;
};

/// @}

#endif // CPPCHECK_H
