/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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

//---------------------------------------------------------------------------
// 64-bit portability
//---------------------------------------------------------------------------

#include "check.h"

#include <iostream>

//---------------------------------------------------------------------------

Check::Check(const std::string &aname)
    : _tokenizer(0), _settings(0), _errorLogger(0), _name(aname)
{
    for (std::list<Check*>::iterator i = instances().begin(); i != instances().end(); ++i) {
        if ((*i)->name() > aname) {
            instances().insert(i, this);
            return;
        }
    }
    instances().push_back(this);
}

void Check::reportError(const ErrorLogger::ErrorMessage &errmsg)
{
    std::cout << errmsg.toXML(true, 1) << std::endl;
}

std::list<Check *> &Check::instances()
{
#ifdef __SVR4
    // Under Solaris, destructors are called in wrong order which causes a segmentation fault.
    // This fix ensures pointer remains valid and reachable until program terminates.
    static std::list<Check *> *_instances= new std::list<Check *>;
    return *_instances;
#else
    static std::list<Check *> _instances;
    return _instances;
#endif
}
