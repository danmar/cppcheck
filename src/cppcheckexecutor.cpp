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

#include "cppcheckexecutor.h"
#include "cppcheck.h"
#include <fstream>
#include <iostream>

CppCheckExecutor::CppCheckExecutor()
{
    _useXML = false;
}

CppCheckExecutor::~CppCheckExecutor()
{
    //dtor
}

unsigned int CppCheckExecutor::check(int argc, const char* const argv[])
{
    CppCheck cppCheck(*this);
    std::string result = cppCheck.parseFromArgs(argc, argv);
    if (result.length() == 0)
    {
        if (cppCheck.settings()._xml)
        {
            _useXML = true;
            reportErr("<?xml version=\"1.0\"?>");
            reportErr("<results>");
        }

        unsigned int returnValue = cppCheck.check();
        if (_useXML)
        {
            reportErr("</results>");
        }

        return returnValue;
    }
    else
    {
        std::cout << result;
        return 1;
    }
}

void CppCheckExecutor::reportErr(const std::string &errmsg)
{
    std::cerr << errmsg << std::endl;
}

void CppCheckExecutor::reportOut(const std::string &outmsg)
{
    std::cout << outmsg << std::endl;
}

void CppCheckExecutor::reportErr(const std::list<FileLocation> &callStack, const std::string &id, const std::string &severity, const std::string &msg)
{
    if (_useXML)
    {
        std::ostringstream xml;
        xml << "<error";
        xml << " file=\"" << callStack.back().file << "\"";
        xml << " line=\"" << callStack.back().line << "\"";
        xml << " id=\"" << id << "\"";
        xml << " severity=\"" << severity << "\"";
        xml << " msg=\"" << msg << "\"";
        xml << "/>";
        reportErr(xml.str());
    }
    else
    {
        std::ostringstream text;

        text << ErrorLogger::callStackToString(callStack) << ": (" << severity << ") " << msg;
        reportErr(text.str());
    }
}
