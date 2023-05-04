// Cppcheck - A tool for static C/C++ code analysis
// Copyright (C) 2007-2023 Cppcheck team.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "options.h"

options::options(int argc, const char* const argv[])
    : mWhichTests(argv + 1, argv + argc)
    ,mQuiet(mWhichTests.count("-q") != 0)
    ,mHelp(mWhichTests.count("-h") != 0 || mWhichTests.count("--help"))
    ,mExe(argv[0])
{
    for (std::set<std::string>::const_iterator it = mWhichTests.cbegin(); it != mWhichTests.cend();) {
        if (!(*it).empty() && (((*it)[0] == '-') || ((*it).find("::") != std::string::npos && mWhichTests.count((*it).substr(0, (*it).find("::"))))))
            it = mWhichTests.erase(it);
        else
            ++it;
    }

    if (mWhichTests.empty()) {
        mWhichTests.insert("");
    }
}

bool options::quiet() const
{
    return mQuiet;
}

bool options::help() const
{
    return mHelp;
}

const std::set<std::string>& options::which_test() const
{
    return mWhichTests;
}

const std::string& options::exe() const
{
    return mExe;
}
