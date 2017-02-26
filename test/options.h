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

#ifndef OPTIONS_H
#define OPTIONS_H

#include <set>
#include <string>

/**
 * @brief Class to parse command-line parameters for ./testrunner .
 * Has getters for available switches and parameters.
 * See test/testoptions.cpp for sample usage.
 */
class options {
public:
    /** Call from main() to populate object */
    options(int argc, const char* argv[]);
    /** Don't print the name of each method being tested. */
    bool quiet() const;
    /** Which test should be run. Empty string means 'all tests' */
    const std::string& which_test() const;

private:
    options();
    options(const options& non_copy);
    const options& operator =(const options& non_assign);

private:
    std::set<std::string> _options;
    std::string _which_test;
    const bool _quiet;
};

#endif
