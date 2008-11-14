/*
 * c++check - c/c++ syntax checking
 * Copyright (C) 2007-2008 Daniel Marjamäki and Reijo Tomperi
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

#ifndef CPPCHECK_H
#define CPPCHECK_H

#include <string>
#include "settings.h"

/**
 * This is the base class which will use other classes to do
 * static code analysis for C and C++ code to find possible
 * errors or places that could be improved.
 */
class CppCheck
{
    public:
        CppCheck();
        virtual ~CppCheck();
        void check(int argc, char* argv[]);

    private:
        void checkFile(const std::string &code, const char FileName[], unsigned int FileId);
        Settings _settings;
};

#endif // CPPCHECK_H
