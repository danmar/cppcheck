// Cppcheck - A tool for static C/C++ code analysis
// Copyright (C) 2007-2017 Cppcheck team.
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

#include <iterator>

options::options(int argc, const char* const argv[])
    :mOptions(argv + 1, argv + argc)
    ,mWhichTest("")
    ,mQuiet(mOptions.count("-q") != 0)
    ,mHelp(mOptions.count("-h") != 0 || mOptions.count("--help"))
{
    mOptions.erase("-q");
    mOptions.erase("-h");
    mOptions.erase("--help");
    if (! mOptions.empty()) {
        mWhichTest = *mOptions.rbegin();
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

const std::string& options::which_test() const
{
    return mWhichTest;
}
