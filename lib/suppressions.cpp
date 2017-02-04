/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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
#include "path.h"

#include <algorithm>
#include <sstream>
#include <stack>
#include <cctype>   // std::isdigit, std::isalnum, etc

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

std::string Suppressions::addSuppressionLine(const std::string &line)
{
    std::istringstream lineStream(line);
    std::string id;
    std::string file;
    unsigned int lineNumber = 0;
    if (std::getline(lineStream, id, ':')) {
        if (std::getline(lineStream, file)) {
            // If there is not a dot after the last colon in "file" then
            // the colon is a separator and the contents after the colon
            // is a line number..

            // Get position of last colon
            const std::string::size_type pos = file.rfind(':');

            // if a colon is found and there is no dot after it..
            if (pos != std::string::npos &&
                file.find('.', pos) == std::string::npos) {
                // Try to parse out the line number
                try {
                    std::istringstream istr1(file.substr(pos+1));
                    istr1 >> lineNumber;
                } catch (...) {
                    lineNumber = 0;
                }

                if (lineNumber > 0) {
                    file.erase(pos);
                }
            }
        }
    }

    // We could perhaps check if the id is valid and return error if it is not
    const std::string errmsg(addSuppression(id, Path::fromNativeSeparators(file), lineNumber));
    if (!errmsg.empty())
        return errmsg;

    return "";
}

bool Suppressions::FileMatcher::match(const std::string &pattern, const std::string &name)
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

std::string Suppressions::FileMatcher::addFile(const std::string &name, unsigned int line)
{
    if (name.find_first_of("*?") != std::string::npos) {
        for (std::string::const_iterator i = name.begin(); i != name.end(); ++i) {
            if (*i == '*') {
                std::string::const_iterator j = i + 1;
                if (j != name.end() && (*j == '*' || *j == '?')) {
                    return "Failed to add suppression. Syntax error in glob.";
                }
            }
        }
        _globs[name][line] = false;
    } else if (name.empty()) {
        _globs["*"][0U] = false;
    } else {
        _files[Path::simplifyPath(name)][line] = false;
    }
    return "";
}

bool Suppressions::FileMatcher::isSuppressed(const std::string &file, unsigned int line)
{
    if (isSuppressedLocal(file, line))
        return true;

    for (std::map<std::string, std::map<unsigned int, bool> >::iterator g = _globs.begin(); g != _globs.end(); ++g) {
        if (match(g->first, file)) {
            std::map<unsigned int, bool>::iterator l = g->second.find(0U);
            if (l != g->second.end()) {
                l->second = true;
                return true;
            }
            l = g->second.find(line);
            if (l != g->second.end()) {
                l->second = true;
                return true;
            }
        }
    }

    return false;
}

bool Suppressions::FileMatcher::isSuppressedLocal(const std::string &file, unsigned int line)
{
    std::map<std::string, std::map<unsigned int, bool> >::iterator f = _files.find(Path::fromNativeSeparators(file));
    if (f != _files.end()) {
        std::map<unsigned int, bool>::iterator l = f->second.find(0U);
        if (l != f->second.end()) {
            l->second = true;
            return true;
        }
        l = f->second.find(line);
        if (l != f->second.end()) {
            l->second = true;
            return true;
        }
    }

    return false;
}

std::string Suppressions::addSuppression(const std::string &errorId, const std::string &file, unsigned int line)
{
    // Check that errorId is valid..
    if (errorId.empty()) {
        return "Failed to add suppression. No id.";
    }
    if (errorId != "*") {
        for (std::string::size_type pos = 0; pos < errorId.length(); ++pos) {
            if (errorId[pos] < 0 || (!std::isalnum(errorId[pos]) && errorId[pos] != '_')) {
                return "Failed to add suppression. Invalid id \"" + errorId + "\"";
            }
            if (pos == 0 && std::isdigit(errorId[pos])) {
                return "Failed to add suppression. Invalid id \"" + errorId + "\"";
            }
        }
    }

    return _suppressions[errorId].addFile(file, line);
}

bool Suppressions::isSuppressed(const std::string &errorId, const std::string &file, unsigned int line)
{
    if (errorId != "unmatchedSuppression" && _suppressions.find("*") != _suppressions.end())
        if (_suppressions["*"].isSuppressed(file, line))
            return true;

    std::map<std::string, FileMatcher>::iterator suppression = _suppressions.find(errorId);
    if (suppression == _suppressions.end())
        return false;

    return suppression->second.isSuppressed(file, line);
}

bool Suppressions::isSuppressedLocal(const std::string &errorId, const std::string &file, unsigned int line)
{
    if (errorId != "unmatchedSuppression" && _suppressions.find("*") != _suppressions.end())
        if (_suppressions["*"].isSuppressedLocal(file, line))
            return true;

    std::map<std::string, FileMatcher>::iterator suppression = _suppressions.find(errorId);
    if (suppression == _suppressions.end())
        return false;

    return suppression->second.isSuppressedLocal(file, line);
}

std::list<Suppressions::SuppressionEntry> Suppressions::getUnmatchedLocalSuppressions(const std::string &file, const bool unusedFunctionChecking) const
{
    std::list<SuppressionEntry> result;
    for (std::map<std::string, FileMatcher>::const_iterator i = _suppressions.begin(); i != _suppressions.end(); ++i) {
        if (!unusedFunctionChecking && i->first == "unusedFunction")
            continue;

        std::map<std::string, std::map<unsigned int, bool> >::const_iterator f = i->second._files.find(Path::fromNativeSeparators(file));
        if (f != i->second._files.end()) {
            for (std::map<unsigned int, bool>::const_iterator l = f->second.begin(); l != f->second.end(); ++l) {
                if (!l->second) {
                    result.push_back(SuppressionEntry(i->first, f->first, l->first));
                }
            }
        }
    }
    return result;
}

std::list<Suppressions::SuppressionEntry> Suppressions::getUnmatchedGlobalSuppressions(const bool unusedFunctionChecking) const
{
    std::list<SuppressionEntry> result;
    for (std::map<std::string, FileMatcher>::const_iterator i = _suppressions.begin(); i != _suppressions.end(); ++i) {
        if (!unusedFunctionChecking && i->first == "unusedFunction")
            continue;

        // global suppressions..
        for (std::map<std::string, std::map<unsigned int, bool> >::const_iterator g = i->second._globs.begin(); g != i->second._globs.end(); ++g) {
            for (std::map<unsigned int, bool>::const_iterator l = g->second.begin(); l != g->second.end(); ++l) {
                if (!l->second) {
                    result.push_back(SuppressionEntry(i->first, g->first, l->first));
                }
            }
        }
    }
    return result;
}
