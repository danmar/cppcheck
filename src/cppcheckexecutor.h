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

#ifndef CPPCHECKEXECUTOR_H
#define CPPCHECKEXECUTOR_H

#include "errorlogger.h"

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
     * @return amount of errors found in the checking or 0
     * if no errors were found. If parsing of the arguments failed
     * and checking is not even started, 1 is returned to indicate
     * about an error.
     */
    unsigned int check(int argc, const char* const argv[]);



    /**
     * Information about progress is directed here. This should be
     * called by the CppCheck class only.
     *
     * @param outmsg, E.g. "Checking main.cpp..."
     */
    virtual void reportOut(const std::string &outmsg);

    /** xml output of errors */
    virtual void reportErr(const ErrorLogger::ErrorMessage &msg);

private:

    /**
     * Helper function to print out errors. Appends a line change.
     * @param errmsg, string to printed to error stream
     */
    void reportErr(const std::string &errmsg);

    bool _useXML;
};

#endif // CPPCHECKEXECUTOR_H
