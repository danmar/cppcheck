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


void Check::reportError(const std::list<const Token *> &callstack, Severity::SeverityType severity, const std::string &id, const std::string &msg, const CWE &cwe, Certainty certainty)
{
    const ErrorMessage errmsg(callstack, mTokenizer ? &mTokenizer->list : nullptr, severity, id, msg, cwe, certainty);
    if (mErrorLogger)
        mErrorLogger->reportErr(errmsg);
    else
        writeToErrorList(errmsg);
}

void Check::reportError(const ErrorPath &errorPath, Severity::SeverityType severity, const char id[], const std::string &msg, const CWE &cwe, Certainty certainty)
{
    const ErrorMessage errmsg(errorPath, mTokenizer ? &mTokenizer->list : nullptr, severity, id, msg, cwe, certainty);
    if (mErrorLogger)
        mErrorLogger->reportErr(errmsg);
    else
        writeToErrorList(errmsg);
}

bool Check::wrongData(const Token *tok, const char *str)
{
    if (mSettings->daca)
        reportError(tok, Severity::debug, "DacaWrongData", "Wrong data detected by condition " + std::string(str));
    return true;
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

std::string Check::getMessageId(const ValueFlow::Value &value, const char id[])
{
    if (value.condition != nullptr)
        return id + std::string("Cond");
    if (value.safe)
        return std::string("safe") + (char)std::toupper(id[0]) + (id + 1);
    return id;
}

ErrorPath Check::getErrorPath(const Token* errtok, const ValueFlow::Value* value, std::string bug) const
{
    ErrorPath errorPath;
    if (!value) {
        errorPath.emplace_back(errtok, std::move(bug));
    } else if (mSettings->verbose || mSettings->xml || !mSettings->templateLocation.empty()) {
        errorPath = value->errorPath;
        errorPath.emplace_back(errtok, std::move(bug));
    } else {
        if (value->condition)
            errorPath.emplace_back(value->condition, "condition '" + value->condition->expressionString() + "'");
        //else if (!value->isKnown() || value->defaultArg)
        //    errorPath = value->callstack;
        errorPath.emplace_back(errtok, std::move(bug));
    }
    return errorPath;
}

void Check::logChecker(const char id[])
{
    reportError(nullptr, Severity::none, "logChecker", id);
}

