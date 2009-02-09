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

#include "errorlogger.h"
#include "tokenize.h"
#include "token.h"

#include <sstream>

void ErrorLogger::_writemsg(const Tokenizer *tokenizer, const Token *tok, const char severity[], const std::string msg, const std::string &id)
{
    std::list<const Token *> callstack;
    callstack.push_back(tok);
    _writemsg(tokenizer, callstack, severity, msg, id);
}

void ErrorLogger::_writemsg(const Tokenizer *tokenizer, const std::list<const Token *> &callstack, const char severity[], const std::string msg, const std::string &id)
{
    std::list<FileLocation> locationList;
    for (std::list<const Token *>::const_iterator tok = callstack.begin(); tok != callstack.end(); ++tok)
    {
        FileLocation loc;
        loc.file = tokenizer->file(*tok);
        loc.line = (*tok)->linenr();
        locationList.push_back(loc);
    }

    reportErr(locationList,
              id,
              severity,
              msg);
}


void ErrorLogger::_writemsg(const std::string msg, const std::string &id)
{
    std::ostringstream xml;
    xml << "<error";
    xml << " id=\"" << id << "\"";
    xml << " msg=\"" << msg << "\"";
    xml << ">";

    std::list<FileLocation> empty;
    empty.push_back(FileLocation());
    reportErr(empty, "id", "severity", msg);
}

std::string ErrorLogger::callStackToString(const std::list<FileLocation> &callStack)
{
    std::ostringstream ostr;
    for (std::list<FileLocation>::const_iterator tok = callStack.begin(); tok != callStack.end(); ++tok)
        ostr << (tok == callStack.begin() ? "" : " -> ") << "[" << (*tok).file << ":" << (*tok).line << "]";
    return ostr.str();
}
