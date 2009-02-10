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

ErrorLogger::ErrorMessage::ErrorMessage(const std::list<FileLocation> &callStack, const std::string &severity, const std::string &msg, const std::string &id)
{
    _callStack = callStack;
    _severity = severity;
    _msg = msg;
    _id = id;
}

std::string ErrorLogger::ErrorMessage::toXML() const
{
    std::ostringstream xml;
    xml << "<error";
    xml << " file=\"" << _callStack.back().file << "\"";
    xml << " line=\"" << _callStack.back().line << "\"";
    xml << " id=\"" << _id << "\"";
    xml << " severity=\"" << _severity << "\"";
    xml << " msg=\"" << _msg << "\"";
    xml << "/>";
    return xml.str();
}

std::string ErrorLogger::ErrorMessage::toText() const
{
    std::ostringstream text;
    text << callStackToString(_callStack) << ": (" << _severity << ") " << _msg;
    return text.str();
}

void ErrorLogger::_writemsg(const Tokenizer *tokenizer, const Token *tok, const char severity[], const std::string msg, const std::string &id)
{
    std::list<const Token *> callstack;
    callstack.push_back(tok);
    _writemsg(tokenizer, callstack, severity, msg, id);
}

void ErrorLogger::_writemsg(const Tokenizer *tokenizer, const std::list<const Token *> &callstack, const char severity[], const std::string msg, const std::string &id)
{
    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    for (std::list<const Token *>::const_iterator tok = callstack.begin(); tok != callstack.end(); ++tok)
    {
        ErrorLogger::ErrorMessage::FileLocation loc;
        loc.file = tokenizer->file(*tok);
        loc.line = (*tok)->linenr();
        locationList.push_back(loc);
    }

    reportErr(ErrorLogger::ErrorMessage(locationList, severity, msg, id));
}


void ErrorLogger::_writemsg(const std::string msg, const std::string &id)
{
    std::ostringstream xml;
    xml << "<error";
    xml << " id=\"" << id << "\"";
    xml << " msg=\"" << msg << "\"";
    xml << ">";

    std::list<ErrorLogger::ErrorMessage::FileLocation> empty;
    empty.push_back(ErrorLogger::ErrorMessage::FileLocation());
    reportErr(ErrorLogger::ErrorMessage(empty, "severity", msg, "id"));
}

std::string ErrorLogger::callStackToString(const std::list<ErrorLogger::ErrorMessage::FileLocation> &callStack)
{
    std::ostringstream ostr;
    for (std::list<ErrorLogger::ErrorMessage::FileLocation>::const_iterator tok = callStack.begin(); tok != callStack.end(); ++tok)
        ostr << (tok == callStack.begin() ? "" : " -> ") << "[" << (*tok).file << ":" << (*tok).line << "]";
    return ostr.str();
}
