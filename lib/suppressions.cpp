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

#include "suppressions.h"

#include "errorlogger.h"
#include "mathlib.h"
#include "path.h"
#include "utils.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"

#include <algorithm>
#include <cctype>   // std::isdigit, std::isalnum, etc
#include <cstring>
#include <functional> // std::bind, std::placeholders
#include <sstream> // IWYU pragma: keep
#include <utility>

#include <tinyxml2.h>

Suppressions::ErrorMessage Suppressions::ErrorMessage::fromErrorMessage(const ::ErrorMessage &msg)
{
    Suppressions::ErrorMessage ret;
    ret.hash = msg.hash;
    ret.errorId = msg.id;
    if (!msg.callStack.empty()) {
        ret.setFileName(msg.callStack.back().getfile(false));
        ret.lineNumber = msg.callStack.back().line;
    } else {
        ret.lineNumber = Suppressions::Suppression::NO_LINE;
    }
    ret.certainty = msg.certainty;
    ret.symbolNames = msg.symbolNames();
    return ret;
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
        if (line.length() > 1 && line[0] == '#')
            continue;
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
            return "Invalid suppression xml file format, expected <suppress> element but got a \"" + std::string(e->Name()) + '\"';

        Suppression s;
        for (const tinyxml2::XMLElement * e2 = e->FirstChildElement(); e2; e2 = e2->NextSiblingElement()) {
            const char *text = e2->GetText() ? e2->GetText() : "";
            if (std::strcmp(e2->Name(), "id") == 0)
                s.errorId = text;
            else if (std::strcmp(e2->Name(), "fileName") == 0)
                s.fileName = text;
            else if (std::strcmp(e2->Name(), "lineNumber") == 0)
                s.lineNumber = strToInt<int>(text);
            else if (std::strcmp(e2->Name(), "symbolName") == 0)
                s.symbolName = text;
            else if (*text && std::strcmp(e2->Name(), "hash") == 0)
                s.hash = strToInt<std::size_t>(text);
            else
                return "Unknown suppression element \"" + std::string(e2->Name()) + "\", expected id/fileName/lineNumber/symbolName/hash";
        }

        const std::string err = addSuppression(std::move(s));
        if (!err.empty())
            return err;
    }

    return "";
}

std::vector<Suppressions::Suppression> Suppressions::parseMultiSuppressComment(const std::string &comment, std::string *errorMessage)
{
    std::vector<Suppression> suppressions;

    // If this function is called we assume that comment starts with "cppcheck-suppress[".
    const std::string::size_type start_position = comment.find('[');
    const std::string::size_type end_position = comment.find(']', start_position);
    if (end_position == std::string::npos) {
        if (errorMessage && errorMessage->empty())
            *errorMessage = "Bad multi suppression '" + comment + "'. legal format is cppcheck-suppress[errorId, errorId symbolName=arr, ...]";
        return suppressions;
    }

    // parse all suppressions
    for (std::string::size_type pos = start_position; pos < end_position;) {
        const std::string::size_type pos1 = pos + 1;
        pos = comment.find(',', pos1);
        const std::string::size_type pos2 = (pos < end_position) ? pos : end_position;
        if (pos1 == pos2)
            continue;

        Suppression s;
        std::istringstream iss(comment.substr(pos1, pos2-pos1));

        iss >> s.errorId;
        if (!iss) {
            if (errorMessage && errorMessage->empty())
                *errorMessage = "Bad multi suppression '" + comment + "'. legal format is cppcheck-suppress[errorId, errorId symbolName=arr, ...]";
            suppressions.clear();
            return suppressions;
        }

        while (iss) {
            std::string word;
            iss >> word;
            if (!iss)
                break;
            if (word.find_first_not_of("+-*/%#;") == std::string::npos)
                break;
            if (word.compare(0, 11, "symbolName=") == 0) {
                s.symbolName = word.substr(11);
            } else {
                if (errorMessage && errorMessage->empty())
                    *errorMessage = "Bad multi suppression '" + comment + "'. legal format is cppcheck-suppress[errorId, errorId symbolName=arr, ...]";
                suppressions.clear();
                return suppressions;
            }
        }

        suppressions.push_back(std::move(s));
    }

    return suppressions;
}

std::string Suppressions::addSuppressionLine(const std::string &line)
{
    std::istringstream lineStream;
    Suppressions::Suppression suppression;

    // Strip any end of line comments
    std::string::size_type endpos = std::min(line.find('#'), line.find("//"));
    if (endpos != std::string::npos) {
        while (endpos > 0 && std::isspace(line[endpos-1])) {
            endpos--;
        }
        lineStream.str(line.substr(0, endpos));
    } else {
        lineStream.str(line);
    }

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
                    suppression.lineNumber = Suppressions::Suppression::NO_LINE;
                }

                if (suppression.lineNumber != Suppressions::Suppression::NO_LINE) {
                    suppression.fileName.erase(pos);
                }
            }
        }
    }

    suppression.fileName = Path::simplifyPath(suppression.fileName);

    return addSuppression(std::move(suppression));
}

std::string Suppressions::addSuppression(Suppressions::Suppression suppression)
{
    // Check if suppression is already in list
    auto foundSuppression = std::find_if(mSuppressions.begin(), mSuppressions.end(),
                                         std::bind(&Suppression::isSameParameters, &suppression, std::placeholders::_1));
    if (foundSuppression != mSuppressions.end()) {
        // Update matched state of existing global suppression
        if (!suppression.isLocal() && suppression.matched)
            foundSuppression->matched = suppression.matched;
        return "";
    }

    // Check that errorId is valid..
    if (suppression.errorId.empty() && suppression.hash == 0)
        return "Failed to add suppression. No id.";

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

    mSuppressions.push_back(std::move(suppression));

    return "";
}

std::string Suppressions::addSuppressions(std::list<Suppression> suppressions)
{
    for (auto &newSuppression : suppressions) {
        auto errmsg = addSuppression(std::move(newSuppression));
        if (!errmsg.empty())
            return errmsg;
    }
    return "";
}

void Suppressions::ErrorMessage::setFileName(std::string s)
{
    mFileName = Path::simplifyPath(std::move(s));
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
    if (hash > 0 && hash != errmsg.hash)
        return false;
    if (!errorId.empty() && !matchglob(errorId, errmsg.errorId))
        return false;
    if (!fileName.empty() && !matchglob(fileName, errmsg.getFileName()))
        return false;
    if (lineNumber != NO_LINE && lineNumber != errmsg.lineNumber) {
        if (!thisAndNextLine || lineNumber + 1 != errmsg.lineNumber)
            return false;
    }
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
    checked = true;
    return true;
}

std::string Suppressions::Suppression::getText() const
{
    std::string ret;
    if (!errorId.empty())
        ret = errorId;
    if (!fileName.empty())
        ret += " fileName=" + fileName;
    if (lineNumber != NO_LINE)
        ret += " lineNumber=" + std::to_string(lineNumber);
    if (!symbolName.empty())
        ret += " symbolName=" + symbolName;
    if (hash > 0)
        ret += " hash=" + std::to_string(hash);
    if (ret.compare(0,1," ")==0)
        return ret.substr(1);
    return ret;
}

bool Suppressions::isSuppressed(const Suppressions::ErrorMessage &errmsg, bool global)
{
    const bool unmatchedSuppression(errmsg.errorId == "unmatchedSuppression");
    for (Suppression &s : mSuppressions) {
        if (!global && !s.isLocal())
            continue;
        if (unmatchedSuppression && s.errorId != errmsg.errorId)
            continue;
        if (s.isMatch(errmsg))
            return true;
    }
    return false;
}

bool Suppressions::isSuppressed(const ::ErrorMessage &errmsg)
{
    if (mSuppressions.empty())
        return false;
    return isSuppressed(Suppressions::ErrorMessage::fromErrorMessage(errmsg));
}

void Suppressions::dump(std::ostream & out) const
{
    out << "  <suppressions>" << std::endl;
    for (const Suppression &suppression : mSuppressions) {
        out << "    <suppression";
        out << " errorId=\"" << ErrorLogger::toxml(suppression.errorId) << '"';
        if (!suppression.fileName.empty())
            out << " fileName=\"" << ErrorLogger::toxml(suppression.fileName) << '"';
        if (suppression.lineNumber != Suppression::NO_LINE)
            out << " lineNumber=\"" << suppression.lineNumber << '"';
        if (!suppression.symbolName.empty())
            out << " symbolName=\"" << ErrorLogger::toxml(suppression.symbolName) << '\"';
        if (suppression.hash > 0)
            out << " hash=\"" << suppression.hash << '\"';
        out << " />" << std::endl;
    }
    out << "  </suppressions>" << std::endl;
}

std::list<Suppressions::Suppression> Suppressions::getUnmatchedLocalSuppressions(const std::string &file, const bool unusedFunctionChecking) const
{
    std::string tmpFile = Path::simplifyPath(file);
    std::list<Suppression> result;
    for (const Suppression &s : mSuppressions) {
        if (s.matched || ((s.lineNumber != Suppression::NO_LINE) && !s.checked))
            continue;
        if (s.hash > 0)
            continue;
        if (!unusedFunctionChecking && s.errorId == "unusedFunction")
            continue;
        if (tmpFile.empty() || !s.isLocal() || s.fileName != tmpFile)
            continue;
        result.push_back(s);
    }
    return result;
}

std::list<Suppressions::Suppression> Suppressions::getUnmatchedGlobalSuppressions(const bool unusedFunctionChecking) const
{
    std::list<Suppression> result;
    for (const Suppression &s : mSuppressions) {
        if (s.matched || ((s.lineNumber != Suppression::NO_LINE) && !s.checked))
            continue;
        if (s.hash > 0)
            continue;
        if (!unusedFunctionChecking && s.errorId == "unusedFunction")
            continue;
        if (s.isLocal())
            continue;
        result.push_back(s);
    }
    return result;
}

const std::list<Suppressions::Suppression> &Suppressions::getSuppressions() const
{
    return mSuppressions;
}

void Suppressions::markUnmatchedInlineSuppressionsAsChecked(const Tokenizer &tokenizer) {
    int currLineNr = -1;
    int currFileIdx = -1;
    for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
        if (currFileIdx != tok->fileIndex() || currLineNr != tok->linenr()) {
            currLineNr = tok->linenr();
            currFileIdx = tok->fileIndex();
            for (auto &suppression : mSuppressions) {
                if (!suppression.checked && (suppression.lineNumber == currLineNr) && (suppression.fileName == tokenizer.list.file(tok))) {
                    suppression.checked = true;
                }
            }
        }
    }
}

bool Suppressions::reportUnmatchedSuppressions(const std::list<Suppressions::Suppression> &unmatched, ErrorLogger &errorLogger)
{
    bool err = false;
    // Report unmatched suppressions
    for (const Suppressions::Suppression &s : unmatched) {
        // don't report "unmatchedSuppression" as unmatched
        if (s.errorId == "unmatchedSuppression")
            continue;

        // check if this unmatched suppression is suppressed
        bool suppressed = false;
        for (const Suppressions::Suppression &s2 : unmatched) {
            if (s2.errorId == "unmatchedSuppression") {
                if ((s2.fileName.empty() || s2.fileName == "*" || s2.fileName == s.fileName) &&
                    (s2.lineNumber == Suppressions::Suppression::NO_LINE || s2.lineNumber == s.lineNumber)) {
                    suppressed = true;
                    break;
                }
            }
        }

        if (suppressed)
            continue;

        std::list<::ErrorMessage::FileLocation> callStack;
        if (!s.fileName.empty())
            callStack.emplace_back(s.fileName, s.lineNumber, 0);
        errorLogger.reportErr(::ErrorMessage(callStack, emptyString, Severity::information, "Unmatched suppression: " + s.errorId, "unmatchedSuppression", Certainty::normal));
        err = true;
    }
    return err;
}
