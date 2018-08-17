/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2018 Cppcheck team.
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

#include "suppressions.h"

#include "errorlogger.h"
#include "mathlib.h"
#include "path.h"

#include <tinyxml2.h>

#include <algorithm>
#include <cctype>   // std::isdigit, std::isalnum, etc
#include <stack>
#include <sstream>
#include <utility>

class ErrorLogger;

static bool isValidGlobPattern(const std::string &pattern)
{
    for (std::string::const_iterator i = pattern.begin(); i != pattern.end(); ++i) {
        if (*i == '*' || *i == '?') {
            std::string::const_iterator j = i + 1;
            if (j != pattern.end() && (*j == '*' || *j == '?')) {
                return false;
            }
        }
    }
    return true;
}

static bool isAcceptedErrorIdChar(char c)
{
    switch (c) {
    case '_':
    case '-':
    case '.':
        return true;
    default:
        return std::isalnum(c);
    }
}

std::string Suppressions::parseFile(std::istream &istr)
{
    // Change '\r' to '\n' in the istr
    std::string filedata;
    std::string line;
    while (std::getline(istr, line))
        filedata += line + "\n";
    std::replace(filedata.begin(), filedata.end(), '\r', '\n');

    // Parse filedata..
    std::istringstream istr2(filedata);
    while (std::getline(istr2, line)) {
        // Skip empty lines
        if (line.empty())
            continue;

        // Skip comments
        if (line.length() >= 2 && line[0] == '/' && line[1] == '/')
            continue;

        const std::string errmsg(addSuppressionLine(line));
        if (!errmsg.empty())
            return errmsg;
    }

    return "";
}


std::string Suppressions::parseXmlFile(const char *filename)
{
    tinyxml2::XMLDocument doc;
    const tinyxml2::XMLError error = doc.LoadFile(filename);
    if (error == tinyxml2::XML_ERROR_FILE_NOT_FOUND)
        return "File not found";
    if (error != tinyxml2::XML_SUCCESS)
        return "Failed to parse XML file";

    const tinyxml2::XMLElement * const rootnode = doc.FirstChildElement();
    for (const tinyxml2::XMLElement * e = rootnode->FirstChildElement(); e; e = e->NextSiblingElement()) {
        if (std::strcmp(e->Name(), "suppress") != 0)
            return "Invalid suppression xml file format, expected <suppress> element but got a <" + std::string(e->Name()) + '>';

        Suppression s;
        for (const tinyxml2::XMLElement * e2 = e->FirstChildElement(); e2; e2 = e2->NextSiblingElement()) {
            const char *text = e2->GetText() ? e2->GetText() : "";
            if (std::strcmp(e2->Name(), "id") == 0)
                s.errorId = text;
            else if (std::strcmp(e2->Name(), "fileName") == 0)
                s.fileName = text;
            else if (std::strcmp(e2->Name(), "lineNumber") == 0)
                s.lineNumber = std::atoi(text);
            else if (std::strcmp(e2->Name(), "symbolName") == 0)
                s.symbolName = text;
            else
                return "Unknown suppression element <" + std::string(e2->Name()) + ">, expected <id>/<fileName>/<lineNumber>/<symbolName>";
        }

        const std::string err = addSuppression(s);
        if (!err.empty())
            return err;
    }

    return "";
}

std::string Suppressions::addSuppressionLine(const std::string &line)
{
    std::istringstream lineStream(line);
    Suppressions::Suppression suppression;
    if (std::getline(lineStream, suppression.errorId, ':')) {
        if (std::getline(lineStream, suppression.fileName)) {
            // If there is not a dot after the last colon in "file" then
            // the colon is a separator and the contents after the colon
            // is a line number..

            // Get position of last colon
            const std::string::size_type pos = suppression.fileName.rfind(':');

            // if a colon is found and there is no dot after it..
            if (pos != std::string::npos &&
                suppression.fileName.find('.', pos) == std::string::npos) {
                // Try to parse out the line number
                try {
                    std::istringstream istr1(suppression.fileName.substr(pos+1));
                    istr1 >> suppression.lineNumber;
                } catch (...) {
                    suppression.lineNumber = 0;
                }

                if (suppression.lineNumber > 0) {
                    suppression.fileName.erase(pos);
                }
            }
        }
    }

    suppression.fileName = Path::simplifyPath(suppression.fileName);

    return addSuppression(suppression);
}

std::string Suppressions::addSuppression(const Suppressions::Suppression &suppression)
{
    // Check that errorId is valid..
    if (suppression.errorId.empty()) {
        return "Failed to add suppression. No id.";
    }
    if (suppression.errorId != "*") {
        for (std::string::size_type pos = 0; pos < suppression.errorId.length(); ++pos) {
            if (suppression.errorId[pos] < 0 || !isAcceptedErrorIdChar(suppression.errorId[pos])) {
                return "Failed to add suppression. Invalid id \"" + suppression.errorId + "\"";
            }
            if (pos == 0 && std::isdigit(suppression.errorId[pos])) {
                return "Failed to add suppression. Invalid id \"" + suppression.errorId + "\"";
            }
        }
    }

    if (!isValidGlobPattern(suppression.errorId))
        return "Failed to add suppression. Invalid glob pattern '" + suppression.errorId + "'.";
    if (!isValidGlobPattern(suppression.fileName))
        return "Failed to add suppression. Invalid glob pattern '" + suppression.fileName + "'.";

    mSuppressions.push_back(suppression);

    return "";
}

void Suppressions::ErrorMessage::setFileName(const std::string &s)
{
    mFileName = Path::simplifyPath(s);
}

bool Suppressions::Suppression::parseComment(std::string comment, std::string *errorMessage)
{
    if (comment.size() < 2)
        return false;

    if (comment.find(';') != std::string::npos)
        comment.erase(comment.find(';'));

    if (comment.find("//", 2) != std::string::npos)
        comment.erase(comment.find("//",2));

    if (comment.compare(comment.size() - 2, 2, "*/") == 0)
        comment.erase(comment.size() - 2, 2);

    std::istringstream iss(comment.substr(2));
    std::string word;
    iss >> word;
    if (word != "cppcheck-suppress")
        return false;
    iss >> errorId;
    if (!iss)
        return false;
    while (iss) {
        iss >> word;
        if (!iss)
            break;
        if (word.find_first_not_of("+-*/%#;") == std::string::npos)
            break;
        if (word.compare(0,11,"symbolName=")==0)
            symbolName = word.substr(11);
        else if (errorMessage && errorMessage->empty())
            *errorMessage = "Bad suppression attribute '" + word + "'. You can write comments in the comment after a ; or //. Valid suppression attributes; symbolName=sym";
    }
    return true;
}

bool Suppressions::Suppression::isSuppressed(const Suppressions::ErrorMessage &errmsg) const
{
    if (!errorId.empty() && !matchglob(errorId, errmsg.errorId))
        return false;
    if (!fileName.empty() && !matchglob(fileName, errmsg.getFileName()))
        return false;
    if (lineNumber > 0 && lineNumber != errmsg.lineNumber)
        return false;
    if (!symbolName.empty()) {
        for (std::string::size_type pos = 0; pos < errmsg.symbolNames.size();) {
            const std::string::size_type pos2 = errmsg.symbolNames.find('\n',pos);
            std::string symname;
            if (pos2 == std::string::npos) {
                symname = errmsg.symbolNames.substr(pos);
                pos = pos2;
            } else {
                symname = errmsg.symbolNames.substr(pos,pos2-pos);
                pos = pos2+1;
            }
            if (matchglob(symbolName, symname))
                return true;
        }
        return false;
    }
    return true;
}

bool Suppressions::Suppression::isMatch(const Suppressions::ErrorMessage &errmsg)
{
    if (!isSuppressed(errmsg))
        return false;
    matched = true;
    return true;
}

std::string Suppressions::Suppression::getText() const
{
    std::string ret;
    if (!errorId.empty())
        ret = errorId;
    if (!fileName.empty())
        ret += " fileName=" + fileName;
    if (lineNumber > 0)
        ret += " lineNumber=" + MathLib::toString(lineNumber);
    if (!symbolName.empty())
        ret += " symbolName=" + symbolName;
    if (ret.compare(0,1," ")==0)
        return ret.substr(1);
    return ret;
}

bool Suppressions::isSuppressed(const Suppressions::ErrorMessage &errmsg)
{
    const bool unmatchedSuppression(errmsg.errorId == "unmatchedSuppression");
    for (Suppression &s : mSuppressions) {
        if (unmatchedSuppression && s.errorId != errmsg.errorId)
            continue;
        if (s.isMatch(errmsg))
            return true;
    }
    return false;
}

bool Suppressions::isSuppressedLocal(const Suppressions::ErrorMessage &errmsg)
{
    const bool unmatchedSuppression(errmsg.errorId == "unmatchedSuppression");
    for (Suppression &s : mSuppressions) {
        if (!s.isLocal())
            continue;
        if (unmatchedSuppression && s.errorId != errmsg.errorId)
            continue;
        if (s.isMatch(errmsg))
            return true;
    }
    return false;
}

void Suppressions::dump(std::ostream & out)
{
    out << "  <suppressions>" << std::endl;
    for (const Suppression &suppression : mSuppressions) {
        out << "    <suppression";
        out << " errorId=\"" << ErrorLogger::toxml(suppression.errorId) << '"';
        if (!suppression.fileName.empty())
            out << " fileName=\"" << ErrorLogger::toxml(suppression.fileName) << '"';
        if (suppression.lineNumber > 0)
            out << " lineNumber=\"" << suppression.lineNumber << '"';
        if (!suppression.symbolName.empty())
            out << " symbolName=\"" << ErrorLogger::toxml(suppression.symbolName) << '\"';
        out << " />" << std::endl;
    }
    out << "  </suppressions>" << std::endl;
}

#include <iostream>

std::list<Suppressions::Suppression> Suppressions::getUnmatchedLocalSuppressions(const std::string &file, const bool unusedFunctionChecking) const
{
    std::list<Suppression> result;
    for (const Suppression &s : mSuppressions) {
        if (s.matched)
            continue;
        if (!unusedFunctionChecking && s.errorId == "unusedFunction")
            continue;
        if (file.empty() || !s.isLocal() || s.fileName != file)
            continue;
        result.push_back(s);
    }
    return result;
}

std::list<Suppressions::Suppression> Suppressions::getUnmatchedGlobalSuppressions(const bool unusedFunctionChecking) const
{
    std::list<Suppression> result;
    for (const Suppression &s : mSuppressions) {
        if (s.matched)
            continue;
        if (!unusedFunctionChecking && s.errorId == "unusedFunction")
            continue;
        if (s.isLocal())
            continue;
        result.push_back(s);
    }
    return result;
}

bool Suppressions::matchglob(const std::string &pattern, const std::string &name)
{
    const char *p = pattern.c_str();
    const char *n = name.c_str();
    std::stack<std::pair<const char *, const char *> > backtrack;

    for (;;) {
        bool matching = true;
        while (*p != '\0' && matching) {
            switch (*p) {
            case '*':
                // Step forward until we match the next character after *
                while (*n != '\0' && *n != p[1]) {
                    n++;
                }
                if (*n != '\0') {
                    // If this isn't the last possibility, save it for later
                    backtrack.push(std::make_pair(p, n));
                }
                break;
            case '?':
                // Any character matches unless we're at the end of the name
                if (*n != '\0') {
                    n++;
                } else {
                    matching = false;
                }
                break;
            default:
                // Non-wildcard characters match literally
                if (*n == *p) {
                    n++;
                } else if (*n == '\\' && *p == '/') {
                    n++;
                } else if (*n == '/' && *p == '\\') {
                    n++;
                } else {
                    matching = false;
                }
                break;
            }
            p++;
        }

        // If we haven't failed matching and we've reached the end of the name, then success
        if (matching && *n == '\0') {
            return true;
        }

        // If there are no other paths to try, then fail
        if (backtrack.empty()) {
            return false;
        }

        // Restore pointers from backtrack stack
        p = backtrack.top().first;
        n = backtrack.top().second;
        backtrack.pop();

        // Advance name pointer by one because the current position didn't work
        n++;
    }
}
