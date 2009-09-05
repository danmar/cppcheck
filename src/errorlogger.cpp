/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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

ErrorLogger::ErrorMessage::ErrorMessage()
{

}
#include <iostream>
ErrorLogger::ErrorMessage::ErrorMessage(const std::list<FileLocation> &callStack, const std::string &severity, const std::string &msg, const std::string &id)
{
    _callStack = callStack;
    _severity = severity;
    _msg = msg;
    _id = id;
}

std::string ErrorLogger::ErrorMessage::serialize() const
{
    std::ostringstream oss;
    oss << _id.length() << " " << _id;
    oss << _severity.length() << " " << _severity;
    oss << _msg.length() << " " << _msg;
    oss << _callStack.size() << " ";

    for (std::list<ErrorLogger::ErrorMessage::FileLocation>::const_iterator tok = _callStack.begin(); tok != _callStack.end(); ++tok)
    {
        std::ostringstream smallStream;
        smallStream << (*tok).line << ":" << (*tok).getfile();
        oss << smallStream.str().length() << " " << smallStream.str();
    }
    return oss.str();
}

bool ErrorLogger::ErrorMessage::deserialize(const std::string &data)
{
    _callStack.clear();
    std::istringstream iss(data);
    std::vector<std::string> results;
    while (iss.good())
    {
        unsigned int len = 0;
        if (!(iss >> len))
            return false;

        iss.get();
        std::string temp;
        for (unsigned int i = 0; i < len && iss.good(); ++i)
        {
            char c = static_cast<char>(iss.get());
            temp.append(1, c);
        }

        results.push_back(temp);
        if (results.size() == 3)
            break;
    }

    _id = results[0];
    _severity = results[1];
    _msg = results[2];

    unsigned int stackSize = 0;
    if (!(iss >> stackSize))
        return false;

    while (iss.good())
    {
        unsigned int len = 0;
        if (!(iss >> len))
            return false;

        iss.get();
        std::string temp;
        for (unsigned int i = 0; i < len && iss.good(); ++i)
        {
            char c = static_cast<char>(iss.get());
            temp.append(1, c);
        }

        ErrorLogger::ErrorMessage::FileLocation loc;
        loc.file = temp.substr(temp.find(':') + 1);
        temp = temp.substr(0, temp.find(':'));
        std::istringstream fiss(temp);
        fiss >> loc.line;

        _callStack.push_back(loc);

        if (_callStack.size() >= stackSize)
            break;
    }

    return true;
}

std::string ErrorLogger::ErrorMessage::getXMLHeader()
{
    return  "<?xml version=\"1.0\"?>\n"
            "<results>";
}

std::string ErrorLogger::ErrorMessage::getXMLFooter()
{
    return "</results>";
}

std::string ErrorLogger::ErrorMessage::toXML() const
{
    std::ostringstream xml;
    xml << "<error";
    if (!_callStack.empty())
    {
        xml << " file=\"" << _callStack.back().getfile() << "\"";
        xml << " line=\"" << _callStack.back().line << "\"";
    }
    xml << " id=\"" << _id << "\"";
    xml << " severity=\"" << _severity << "\"";

    // Replace characters in message
    std::string m(_msg);
    std::string::size_type pos = 0;
    while ((pos = m.find_first_of("<>&\"", pos)) != std::string::npos)
    {
        if (m[pos] == '<')
            m.insert(pos + 1, "&lt;");
        else if (m[pos] == '>')
            m.insert(pos + 1, "&gt;");
        else if (m[pos] == '&')
            m.insert(pos + 1, "&amp;");
        else if (m[pos] == '"')
            m.insert(pos + 1, "&quot;");
        m.erase(pos, 1);
        ++pos;
    }

    xml << " msg=\"" << m << "\"";
    xml << "/>";
    return xml.str();
}

void ErrorLogger::ErrorMessage::findAndReplace(std::string &source, const std::string &searchFor, const std::string &replaceWith)
{
    std::string::size_type index = 0;
    while ((index = source.find(searchFor, index)) != std::string::npos)
    {
        source.replace(index, searchFor.length(), replaceWith);
        index += replaceWith.length() - searchFor.length() + 1;
    }
}

std::string ErrorLogger::ErrorMessage::toText(const std::string &outputFormat) const
{
    if (outputFormat.length() == 0)
    {
        std::ostringstream text;
        if (!_callStack.empty())
            text << callStackToString(_callStack) << ": ";
        if (!_severity.empty())
            text << "(" << _severity << ") ";
        text << _msg;
        return text.str();
    }
    else
    {
        std::string result = outputFormat;
        findAndReplace(result, "{id}", _id);
        findAndReplace(result, "{severity}", _severity);
        findAndReplace(result, "{message}", _msg);

        if (!_callStack.empty())
        {
            std::ostringstream oss;
            oss << _callStack.back().line;
            findAndReplace(result, "{line}", oss.str());
            findAndReplace(result, "{file}", _callStack.back().getfile());
        }
        else
        {
            findAndReplace(result, "{file}", "");
            findAndReplace(result, "{line}", "");
        }

        return result;
    }
}

void ErrorLogger::_writemsg(const Tokenizer *tokenizer, const Token *tok, const char severity[], const std::string &msg, const std::string &id)
{
    std::list<const Token *> callstack;
    callstack.push_back(tok);
    _writemsg(tokenizer, callstack, severity, msg, id);
}

void ErrorLogger::_writemsg(const Tokenizer *tokenizer, const std::list<const Token *> &callstack, const char severity[], const std::string &msg, const std::string &id)
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




std::string ErrorLogger::callStackToString(const std::list<ErrorLogger::ErrorMessage::FileLocation> &callStack)
{
    std::ostringstream ostr;
    for (std::list<ErrorLogger::ErrorMessage::FileLocation>::const_iterator tok = callStack.begin(); tok != callStack.end(); ++tok)
        ostr << (tok == callStack.begin() ? "" : " -> ") << "[" << (*tok).getfile() << ":" << (*tok).line << "]";
    return ostr.str();
}


std::string ErrorLogger::ErrorMessage::FileLocation::getfile() const
{
    std::string f(file);

    // replace "/ab/.." with "/"..
    std::string::size_type pos = 0;
    while ((pos = f.find("..", pos + 1)) != std::string::npos)
    {
        // position must be at least 4..
        if (pos < 4)
            continue;

        // Previous char must be a separator..
        if (f[pos-1] != '/' && f[pos-2] != '\\')
            continue;

        // Next char must be a separator..
        if (f[pos+2] != '/' && f[pos+2] != '\\')
            continue;

        // Locate previous separator..
        std::string::size_type sep = f.find_last_of("/\\", pos - 2);
        if (sep == std::string::npos)
            continue;

        // Delete substring..
        f.erase(sep, pos + 2 - sep);
        pos = sep;
    }

    return f;
}

