// Cppcheck - A tool for static C/C++ code analysis
// Copyright (C) 2007-2016 Cppcheck team.
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

options::options(int argc, const char* argv[])
    :_options(argv + 1, argv + argc)
    ,_which_test("")
    ,_quiet(_options.count("-q") != 0)
{
    _options.erase("-q");
    if (! _options.empty()) {
        _which_test = *_options.rbegin();
    }
}

bool options::quiet() const
{
    return _quiet;
}

const std::string& options::which_test() const
{
    return _which_test;
}
