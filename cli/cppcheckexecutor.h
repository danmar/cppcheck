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

#ifndef CPPCHECKEXECUTOR_H
#define CPPCHECKEXECUTOR_H

#include "errorlogger.h"
#include "settings.h"
#include <ctime>
#include <vector>
#include <list>
#include <string>

class CppCheck;

/**
 * This class works as an example of how CppCheck can be used in external
 * programs without very little knowledge of the internal parts of the
 * program itself. If you wish to use cppcheck e.g. as a part of IDE,
 * just rewrite this class for your needs and possibly use other methods
 * from CppCheck class instead the ones used here.
 */
class CppCheckExecutor : public ErrorLogger
{
public:
    /**
     * Constructor
     */
    CppCheckExecutor();

    /**
     * Destructor
     */
    virtual ~CppCheckExecutor();

    /**
     * Starts the checking.
     *
     * @param argc from main()
     * @param argv from main()
     * @return EXIT_FAILURE if arguments are invalid or no input files
     *         were found.
     *         If errors are found and --error-exitcode is used,
     *         given value is returned instead of default 0.
     *         If no errors are found, 0 is returned.
     */
    virtual int check(int argc, const char* const argv[]);

    /**
     * Information about progress is directed here. This should be
     * called by the CppCheck class only.
     *
     * @param outmsg Progress message e.g. "Checking main.cpp..."
     */
    virtual void reportOut(const std::string &outmsg);

    /** xml output of errors */
    virtual void reportErr(const ErrorLogger::ErrorMessage &msg);

    void reportProgress(const std::string &filename, const char stage[], const unsigned int value);

    /**
     * Information about how many files have been checked
     *
     * @param fileindex This many files have been checked.
     * @param filecount This many files there are in total.
     * @param sizedone The sum of sizes of the files checked.
     * @param sizetotal The total sizes of the files.
     */
    static void reportStatus(unsigned int fileindex, unsigned int filecount, long sizedone, long sizetotal);

protected:

    /**
     * Helper function to print out errors. Appends a line change.
     * @param errmsg String printed to error stream
     */
    virtual void reportErr(const std::string &errmsg);

    /**
     * @brief Parse command line args and get settings and file lists
     * from there.
     *
     * @param argc argc from main()
     * @param argv argv from main()
     * @return false when errors are found in the input
     */
    bool parseFromArgs(CppCheck *cppcheck, int argc, const char* const argv[]);

    /**
     * check() will setup this in the beginning of check().
     */
    Settings _settings;

private:

    /**
     * Used to filter out duplicate error messages.
     */
    std::list<std::string> _errorList;

    /**
     * Report progress time
     */
    std::time_t time1;

    /**
     * Has --errorlist been given?
     */
    bool errorlist;

    /**
     * List of files to check.
     */
    std::vector<std::string> _filenames;

    /**
     * Sizes of files in _filenames.
     */
    std::map<std::string, long> _filesizes;
};

#endif // CPPCHECKEXECUTOR_H
