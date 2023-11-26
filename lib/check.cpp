/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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

#include "check.h"

#include "errorlogger.h"
#include "settings.h"
#include "token.h"
#include "tokenize.h"
#include "vfvalue.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <utility>

//---------------------------------------------------------------------------

Check::Check(const std::string &aname)
    : mName(aname)
{
    {
        const auto it = std::find_if(instances().begin(), instances().end(), [&](const Check *i) {
            return i->name() == aname;
        });
        if (it != instances().end())
            throw std::runtime_error("'" + aname + "' instance already exists");
    }

    // make sure the instances are sorted
    const auto it = std::find_if(instances().begin(), instances().end(), [&](const Check* i) {
        return i->name() > aname;
    });
    if (it == instances().end())
        instances().push_back(this);
    else
        instances().insert(it, this);
}

void Check::writeToErrorList(const ErrorMessage &errmsg)
{
    std::cout << errmsg.toXML() << std::endl;
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
