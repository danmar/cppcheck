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

#include "cppcheckexecutor.h"
#include "cppcheck.h"
#include "threadexecutor.h"
#include <fstream>
#include <iostream>
#include <cstdlib> // EXIT_SUCCESS and EXIT_FAILURE
#include <stdexcept>

CppCheckExecutor::CppCheckExecutor()
{

}

CppCheckExecutor::~CppCheckExecutor()
{

}

int CppCheckExecutor::check(int argc, const char* const argv[])
{
    CppCheck cppCheck(*this);
    try
    {
        cppCheck.parseFromArgs(argc, argv);
    }
    catch (std::runtime_error &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    _settings = cppCheck.settings();
    if (_settings._xml)
    {
        reportErr(ErrorLogger::ErrorMessage::getXMLHeader());
    }

    unsigned int returnValue = 0;
    if (_settings._jobs == 1)
    {
        // Single process
        returnValue = cppCheck.check();
    }
    else if (!ThreadExecutor::isEnabled())
    {
        std::cout << "No thread support yet implemented for this platform." << std::endl;
    }
    else
    {
        // Multiple processes
        const std::vector<std::string> &filenames = cppCheck.filenames();
        Settings settings = cppCheck.settings();
        ThreadExecutor executor(filenames, settings, *this);
        returnValue = executor.check();
    }

    if (_settings._xml)
    {
        reportErr(ErrorLogger::ErrorMessage::getXMLFooter());
    }

    if (returnValue)
        return _settings._exitCode;
    else
        return 0;
}

void CppCheckExecutor::reportErr(const std::string &errmsg)
{
    std::cerr << errmsg << std::endl;
}

void CppCheckExecutor::reportOut(const std::string &outmsg)
{
    std::cout << outmsg << std::endl;
}

void CppCheckExecutor::reportStatus(unsigned int index, unsigned int max)
{
    if (max > 1 && !_settings._errorsOnly)
    {
        std::ostringstream oss;
        oss << index << "/" << max
        << " files checked " <<
        static_cast<int>(static_cast<double>(index) / max*100)
        << "% done";
        std::cout << oss.str() << std::endl;
    }
}

void CppCheckExecutor::reportErr(const ErrorLogger::ErrorMessage &msg)
{
    if (_settings._xml)
    {
        reportErr(msg.toXML());
    }
    else
    {
        reportErr(msg.toText(_settings._outputFormat));
    }
}
