/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2025 Cppcheck team.
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
#include "filesettings.h"
#include "path.h"
#include "pathmatch.h"
#include "utils.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"
#include "settings.h"

#include <algorithm>
#include <cctype>   // std::isdigit, std::isalnum, etc
#include <cstring>
#include <functional> // std::bind, std::placeholders
#include <sstream>
#include <utility>

#include "xml.h"

static const char ID_CHECKERSREPORT[] = "checkersReport";

SuppressionList::ErrorMessage SuppressionList::ErrorMessage::fromErrorMessage(const ::ErrorMessage &msg, const std::set<std::string> &macroNames)
{
    SuppressionList::ErrorMessage ret;
    ret.hash = msg.hash;
    ret.errorId = msg.id;
    if (!msg.callStack.empty()) {
        ret.setFileName(msg.callStack.back().getfile(false));
        ret.lineNumber = msg.callStack.back().line;
    } else {
        ret.setFileName(msg.file0);
        ret.lineNumber = SuppressionList::Suppression::NO_LINE;
    }
    ret.certainty = msg.certainty;
    ret.symbolNames = msg.symbolNames();
    ret.macroNames = macroNames;
    return ret;
}

static bool isAcceptedErrorIdChar(char c)
{
    switch (c) {
    case '_':
    case '-':
    case '.':
    case '*':
        return true;
    default:
        return c > 0 && std::isalnum(c);
    }
}

std::string SuppressionList::parseFile(std::istream &istr)
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

        std::string::size_type pos = 0;
        while (pos < line.size() && std::isspace(line[pos]))
            ++pos;
        if (pos == line.size())
            continue;

        // Skip comments
        if (line[pos] == '#')
            continue;
        if (pos < line.size() - 1 && line[pos] == '/' && line[pos + 1] == '/')
            continue;

        std::string errmsg(addSuppressionLine(line));
        if (!errmsg.empty())
            return errmsg;
    }

    return "";
}


std::string SuppressionList::parseXmlFile(const char *filename)
{
    tinyxml2::XMLDocument doc;
    const tinyxml2::XMLError error = doc.LoadFile(filename);
    if (error != tinyxml2::XML_SUCCESS)
        return std::string("failed to load suppressions XML '") + filename + "' (" + tinyxml2::XMLDocument::ErrorIDToName(error) + ").";

    const tinyxml2::XMLElement * const rootnode = doc.FirstChildElement();
    if (!rootnode)
        return std::string("failed to load suppressions XML '") + filename + "' (no root node found).";
    // TODO: check for proper root node 'suppressions'
    for (const tinyxml2::XMLElement * e = rootnode->FirstChildElement(); e; e = e->NextSiblingElement()) {
        if (std::strcmp(e->Name(), "suppress") != 0)
            return std::string("invalid suppression xml file '") + filename + "', expected 'suppress' element but got a '" + e->Name() + "'.";

        Suppression s;
        for (const tinyxml2::XMLElement * e2 = e->FirstChildElement(); e2; e2 = e2->NextSiblingElement()) {
            const char *name = e2->Name();
            const char *text = empty_if_null(e2->GetText());
            if (std::strcmp(name, "id") == 0)
                s.errorId = text;
            else if (std::strcmp(name, "fileName") == 0)
                s.fileName = Path::simplifyPath(text);
            else if (std::strcmp(name, "lineNumber") == 0)
                s.lineNumber = strToInt<int>(text);
            else if (std::strcmp(name, "symbolName") == 0)
                s.symbolName = text;
            else if (*text && std::strcmp(name, "hash") == 0)
                s.hash = strToInt<std::size_t>(text);
            else
                return std::string("unknown element '") + name + "' in suppressions XML '" + filename + "', expected id/fileName/lineNumber/symbolName/hash.";
        }

        std::string err = addSuppression(std::move(s));
        if (!err.empty())
            return err;
    }

    return "";
}

std::vector<SuppressionList::Suppression> SuppressionList::parseMultiSuppressComment(const std::string &comment, std::string *errorMessage)
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

        const std::string symbolNameString = "symbolName=";

        while (iss) {
            std::string word;
            iss >> word;
            if (!iss)
                break;
            if (word.find_first_not_of("+-*/%#;") == std::string::npos)
                break;
            if (startsWith(word, symbolNameString)) {
                s.symbolName = word.substr(symbolNameString.size());
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

SuppressionList::Suppression SuppressionList::parseLine(const std::string &line)
{
    std::istringstream lineStream;
    SuppressionList::Suppression suppression;

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
                    suppression.lineNumber = SuppressionList::Suppression::NO_LINE;
                }

                if (suppression.lineNumber != SuppressionList::Suppression::NO_LINE) {
                    suppression.fileName.erase(pos);
                }
            }
        }
    }

    suppression.fileName = Path::simplifyPath(suppression.fileName);

    return suppression;
}

std::string SuppressionList::addSuppressionLine(const std::string &line)
{
    return addSuppression(parseLine(line));
}

std::string SuppressionList::addSuppression(SuppressionList::Suppression suppression)
{
    std::lock_guard<std::mutex> lg(mSuppressionsSync);

    // Check if suppression is already in list
    auto foundSuppression = std::find_if(mSuppressions.begin(), mSuppressions.end(),
                                         std::bind(&Suppression::isSameParameters, &suppression, std::placeholders::_1));
    if (foundSuppression != mSuppressions.end()) {
        return "suppression '" + suppression.toString() + "' already exists";
    }

    // Check that errorId is valid..
    if (suppression.errorId.empty() && suppression.hash == 0)
        return "Failed to add suppression. No id.";

    for (std::string::size_type pos = 0; pos < suppression.errorId.length(); ++pos) {
        if (!isAcceptedErrorIdChar(suppression.errorId[pos])) {
            return "Failed to add suppression. Invalid id \"" + suppression.errorId + "\"";
        }
        if (pos == 0 && std::isdigit(suppression.errorId[pos])) {
            return "Failed to add suppression. Invalid id \"" + suppression.errorId + "\"";
        }
    }

    if (!isValidGlobPattern(suppression.errorId))
        return "Failed to add suppression. Invalid glob pattern '" + suppression.errorId + "'.";
    if (!isValidGlobPattern(suppression.fileName))
        return "Failed to add suppression. Invalid glob pattern '" + suppression.fileName + "'.";

    mSuppressions.push_back(std::move(suppression));

    return "";
}

std::string SuppressionList::addSuppressions(std::list<Suppression> suppressions)
{
    for (auto &newSuppression : suppressions) {
        auto errmsg = addSuppression(std::move(newSuppression));
        if (!errmsg.empty())
            return errmsg;
    }
    return "";
}

bool SuppressionList::updateSuppressionState(const SuppressionList::Suppression& suppression)
{
    std::lock_guard<std::mutex> lg(mSuppressionsSync);

    // Check if suppression is already in list
    auto foundSuppression = std::find_if(mSuppressions.begin(), mSuppressions.end(),
                                         std::bind(&Suppression::isSameParameters, &suppression, std::placeholders::_1));
    if (foundSuppression != mSuppressions.end()) {
        if (suppression.checked)
            foundSuppression->checked = true;
        if (suppression.matched)
            foundSuppression->matched = true;
        return true;
    }

    return false;
}

void SuppressionList::ErrorMessage::setFileName(std::string s)
{
    mFileName = Path::simplifyPath(std::move(s));
}

bool SuppressionList::Suppression::parseComment(std::string comment, std::string *errorMessage)
{
    if (comment.size() < 2)
        return false;

    if (comment.compare(comment.size() - 2, 2, "*/") == 0)
        comment.erase(comment.size() - 2, 2);

    std::string::size_type extraPos = comment.find(';');
    std::string::size_type extraDelimiterSize = 1;

    if (extraPos == std::string::npos) {
        extraPos = comment.find("//", 2);
        extraDelimiterSize = 2;
    }

    if (extraPos != std::string::npos) {
        extraComment = trim(comment.substr(extraPos + extraDelimiterSize));
        for (auto it = extraComment.begin(); it != extraComment.end();)
            it = *it & 0x80 ? extraComment.erase(it) : it + 1;
        comment.erase(extraPos);
    }

    const std::set<std::string> cppchecksuppress{
        "cppcheck-suppress",
        "cppcheck-suppress-begin",
        "cppcheck-suppress-end",
        "cppcheck-suppress-file",
        "cppcheck-suppress-macro"
    };

    std::istringstream iss(comment.substr(2));
    std::string word;
    iss >> word;
    if (!cppchecksuppress.count(word))
        return false;

    iss >> errorId;
    if (!iss)
        return false;

    const std::string symbolNameString = "symbolName=";

    while (iss) {
        iss >> word;
        if (!iss)
            break;
        if (word.find_first_not_of("+-*/%#;") == std::string::npos)
            break;
        if (startsWith(word, symbolNameString))
            symbolName = word.substr(symbolNameString.size());
        else if (errorMessage && errorMessage->empty())
            *errorMessage = "Bad suppression attribute '" + word + "'. You can write comments in the comment after a ; or //. Valid suppression attributes; symbolName=sym";
    }
    return true;
}

SuppressionList::Suppression::Result SuppressionList::Suppression::isSuppressed(const SuppressionList::ErrorMessage &errmsg) const
{
    if (type == SuppressionList::Type::macro) {
        if (errmsg.macroNames.count(macroName) == 0)
            return Result::None;
        if (hash > 0 && hash != errmsg.hash)
            return Result::Checked;
        if (!errorId.empty() && !matchglob(errorId, errmsg.errorId))
            return Result::Checked;
    } else {
        if ((SuppressionList::Type::unique == type) && (lineNumber != NO_LINE) && (lineNumber != errmsg.lineNumber)) {
            if (!thisAndNextLine || lineNumber + 1 != errmsg.lineNumber)
                return Result::None;
        }
        if (!fileName.empty() && !PathMatch::match(fileName, errmsg.getFileName()))
            return Result::None;
        if (hash > 0 && hash != errmsg.hash)
            return Result::Checked;
        // the empty check is a hack to allow wildcard suppressions on IDs to be marked as checked
        if (!errorId.empty() && (errmsg.errorId.empty() || !matchglob(errorId, errmsg.errorId)))
            return Result::Checked;
        if ((SuppressionList::Type::block == type) && ((errmsg.lineNumber < lineBegin) || (errmsg.lineNumber > lineEnd)))
            return Result::Checked;
    }
    if (!symbolName.empty()) {
        bool matchedSymbol = false;
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
            if (matchglob(symbolName, symname)) {
                matchedSymbol = true;
                break;
            }
        }
        if (!matchedSymbol)
            return Result::Checked;
    }
    return Result::Matched;
}

bool SuppressionList::Suppression::isMatch(const SuppressionList::ErrorMessage &errmsg)
{
    switch (isSuppressed(errmsg)) {
    case Result::None:
        return false;
    case Result::Checked:
        checked = true;
        return false;
    case Result::Matched:
        checked = true;
        matched = true;
        return true;
    }
    cppcheck::unreachable();
}

bool SuppressionList::isSuppressed(const SuppressionList::ErrorMessage &errmsg, bool global)
{
    std::lock_guard<std::mutex> lg(mSuppressionsSync);

    const bool unmatchedSuppression(errmsg.errorId == "unmatchedSuppression");
    bool returnValue = false;
    for (Suppression &s : mSuppressions) {
        if (!global && !s.isLocal())
            continue;
        if (unmatchedSuppression && s.errorId != errmsg.errorId)
            continue;
        if (s.isMatch(errmsg))
            returnValue = true;
    }
    return returnValue;
}

bool SuppressionList::isSuppressedExplicitly(const SuppressionList::ErrorMessage &errmsg, bool global)
{
    std::lock_guard<std::mutex> lg(mSuppressionsSync);

    for (Suppression &s : mSuppressions) {
        if (!global && !s.isLocal())
            continue;
        if (s.errorId != errmsg.errorId) // Error id must match exactly
            continue;
        if (s.isMatch(errmsg))
            return true;
    }
    return false;
}

bool SuppressionList::isSuppressed(const ::ErrorMessage &errmsg, const std::set<std::string>& macroNames)
{
    {
        std::lock_guard<std::mutex> lg(mSuppressionsSync);

        if (mSuppressions.empty())
            return false;
    }
    return isSuppressed(SuppressionList::ErrorMessage::fromErrorMessage(errmsg, macroNames));
}

void SuppressionList::dump(std::ostream & out, const std::string& filePath) const
{
    std::lock_guard<std::mutex> lg(mSuppressionsSync);

    out << "  <suppressions>" << std::endl;
    for (const Suppression &suppression : mSuppressions) {
        if (suppression.isInline && !suppression.fileName.empty() && !filePath.empty() && filePath != suppression.fileName)
            continue;
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
        if (suppression.lineBegin != Suppression::NO_LINE)
            out << " lineBegin=\"" << suppression.lineBegin << '"';
        if (suppression.lineEnd != Suppression::NO_LINE)
            out << " lineEnd=\"" << suppression.lineEnd << '"';
        if (suppression.type == SuppressionList::Type::file)
            out << " type=\"file\"";
        else if (suppression.type == SuppressionList::Type::block)
            out << " type=\"block\"";
        else if (suppression.type == SuppressionList::Type::blockBegin)
            out << " type=\"blockBegin\"";
        else if (suppression.type == SuppressionList::Type::blockEnd)
            out << " type=\"blockEnd\"";
        else if (suppression.type == SuppressionList::Type::macro)
            out << " type=\"macro\"";
        if (suppression.isInline)
            out << " inline=\"true\"";
        else
            out << " inline=\"false\"";
        if (!suppression.extraComment.empty())
            out << " comment=\"" << ErrorLogger::toxml(suppression.extraComment) << "\"";
        out << " />" << std::endl;
    }
    out << "  </suppressions>" << std::endl;
}

std::list<SuppressionList::Suppression> SuppressionList::getUnmatchedLocalSuppressions(const FileWithDetails &file) const
{
    std::lock_guard<std::mutex> lg(mSuppressionsSync);

    std::list<Suppression> result;
    for (const Suppression &s : mSuppressions) {
        if (s.isInline)
            continue;
        if (s.matched)
            continue;
        if ((s.lineNumber != Suppression::NO_LINE) && !s.checked)
            continue;
        if (s.type == SuppressionList::Type::macro)
            continue;
        if (s.hash > 0)
            continue;
        if (s.errorId == ID_CHECKERSREPORT)
            continue;
        if (!s.isLocal() || !PathMatch::match(s.fileName, file.spath()))
            continue;
        result.push_back(s);
    }
    return result;
}

std::list<SuppressionList::Suppression> SuppressionList::getUnmatchedGlobalSuppressions() const
{
    std::lock_guard<std::mutex> lg(mSuppressionsSync);

    std::list<Suppression> result;
    for (const Suppression &s : mSuppressions) {
        if (s.isInline)
            continue;
        if (s.matched)
            continue;
        if (!s.checked && s.isWildcard())
            continue;
        if (s.hash > 0)
            continue;
        if (s.errorId == ID_CHECKERSREPORT)
            continue;
        if (s.isLocal())
            continue;
        result.push_back(s);
    }
    return result;
}

std::list<SuppressionList::Suppression> SuppressionList::getUnmatchedInlineSuppressions() const
{
    std::list<SuppressionList::Suppression> result;
    for (const SuppressionList::Suppression &s : SuppressionList::mSuppressions) {
        if (!s.isInline)
            continue;
        // TODO: remove this and markUnmatchedInlineSuppressionsAsChecked()?
        if (!s.checked)
            continue;
        if (s.matched)
            continue;
        if (s.hash > 0)
            continue;
        result.push_back(s);
    }
    return result;
}

std::list<SuppressionList::Suppression> SuppressionList::getSuppressions() const
{
    std::lock_guard<std::mutex> lg(mSuppressionsSync);

    return mSuppressions;
}

void SuppressionList::markUnmatchedInlineSuppressionsAsChecked(const Tokenizer &tokenizer) {
    std::lock_guard<std::mutex> lg(mSuppressionsSync);

    int currLineNr = -1;
    int currFileIdx = -1;
    for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
        if (currFileIdx != tok->fileIndex() || currLineNr != tok->linenr()) {
            currLineNr = tok->linenr();
            currFileIdx = tok->fileIndex();
            for (auto &suppression : mSuppressions) {
                if (suppression.type == SuppressionList::Type::unique) {
                    if (!suppression.checked && (suppression.lineNumber == currLineNr) && (suppression.fileName == tokenizer.list.file(tok))) {
                        suppression.checked = true;
                    }
                } else if (suppression.type == SuppressionList::Type::block) {
                    if ((!suppression.checked && (suppression.lineBegin <= currLineNr) && (suppression.lineEnd >= currLineNr) && (suppression.fileName == tokenizer.list.file(tok)))) {
                        suppression.checked = true;
                    }
                } else if (!suppression.checked && suppression.fileName == tokenizer.list.file(tok)) {
                    suppression.checked = true;
                }
            }
        }
    }
}

std::string SuppressionList::Suppression::toString() const
{
    std::string s;
    s+= errorId;
    if (!fileName.empty()) {
        s += ':';
        s += fileName;
        if (lineNumber != -1) {
            s += ':';
            s += std::to_string(lineNumber);
        }
    }
    if (!symbolName.empty()) {
        s += ':';
        s += symbolName;
    }
    return s;
}

std::string polyspace::Parser::peekToken()
{
    if (!mHasPeeked) {
        mPeeked = nextToken();
        mHasPeeked = true;
    }
    return mPeeked;
}

std::string polyspace::Parser::nextToken()
{
    if (mHasPeeked) {
        mHasPeeked = false;
        return mPeeked;
    }

    if (mComment.compare(0, 2, "/*") == 0 || mComment.compare(0, 2, "//") == 0)
        mComment = mComment.substr(2);

    std::string::size_type pos = 0;
    while (mComment[pos] == ' ') {
        pos++;
        if (pos == mComment.size()) {
            mComment = "";
            return "";
        }
    }

    if (mComment.compare(0, 2, "*/") == 0) {
        mComment = "";
        return "";
    }

    if (mComment[pos] == ':') {
        mComment = mComment.substr(pos + 1);
        return ":";
    }

    if (mComment[pos] == ',') {
        mComment = mComment.substr(pos + 1);
        return ",";
    }

    const char *stopChars;
    std::string::size_type skip;
    switch (mComment[pos]) {
    case '\"':
        stopChars = "\"";
        skip = 1;
        break;
    case '[':
        stopChars = "]";
        skip = 1;
        break;
    default:
        stopChars = " :,";
        skip = 0;
        break;
    }

    const std::string::size_type start = pos;
    pos += skip;

    if (pos == mComment.size()) {
        mComment = "";
        return "";
    }

    while (std::strchr(stopChars, mComment[pos]) == nullptr) {
        pos++;
        if (pos == mComment.size())
            break;
    }

    if (pos == mComment.size())
        skip = 0;

    const std::string token = mComment.substr(start, pos - start + skip);
    mComment = mComment.substr(pos + skip);

    return token;
}

void polyspace::Parser::finishSuppression()
{
    Suppression suppr = { mFamily, mResultName, mFilename, 0, 0 };

    switch (mKind) {
    case CommentKind::Regular:
    {
        suppr.lineBegin = mLine;
        suppr.lineEnd = mLine + mRange;
        mDone.push_back(suppr);
        return;
    }
    case CommentKind::Begin:
    {
        suppr.lineBegin = mLine;
        mStarted.push_back(suppr);
        return;
    }
    case CommentKind::End:
    {
        auto it = std::find_if(
            mStarted.begin(),
            mStarted.end(),
            [&] (const Suppression &other) {
                return suppr.matches(other);
            }
            );

        if (it == mStarted.end())
            return;

        suppr.lineBegin = it->lineBegin;
        suppr.lineEnd = mLine;
        mStarted.erase(it);
        mDone.push_back(suppr);
        return;
    }
    }
}

bool polyspace::Parser::parseEntry()
{
    mFamily = nextToken();
    if (mFamily.empty())
        return false;

    if (nextToken() != ":")
        return false;

    // Parse result name, multiple names may be separated by commas
    while (!mComment.empty()) {
        mResultName = nextToken();
        if (mResultName.empty())
            return false;

        finishSuppression();

        if (peekToken() == ",") {
            (void) nextToken();
            continue;
        }

        break;
    }

    // Skip status and severity
    if (!peekToken().empty() && mPeeked[0] == '[')
        (void) nextToken();

    return true;
}

void polyspace::Parser::collect(SuppressionList &suppressions) const
{
    for (const auto &polyspaceSuppr : mDone) {
        SuppressionList::Suppression suppr;
        if (polyspaceSuppr.convert(mSettings, suppr))
            suppressions.addSuppression(std::move(suppr));
    }
}

void polyspace::Parser::parse(const std::string &comment, int line, const std::string &filename)
{
    mComment = comment;
    mLine = line;
    mFilename = filename;
    mHasPeeked = false;

    while (true) {
        const std::string kindStr = nextToken();
        if (kindStr.empty())
            return;

        if (kindStr == "polyspace") mKind = CommentKind::Regular;
        else if (kindStr == "polyspace-begin") mKind = CommentKind::Begin;
        else if (kindStr == "polyspace-end") mKind = CommentKind::End;
        else return;

        mRange = 0;
        if (peekToken()[0] == '+') {
            try { mRange = std::stoi(mPeeked.substr(1)); } catch (...) { return; }
            (void) nextToken();
        }

        while (parseEntry()) {
            if (peekToken().empty() || mPeeked[0] == '\"')
                break;
        }

        if (!peekToken().empty() && mPeeked[0] == '\"') {
            (void) nextToken();
            if (peekToken().empty())
                return;
            continue;
        }

        break;
    }
}

bool polyspace::isPolyspaceComment(const std::string &comment)
{
    const std::string polyspace = "polyspace";
    const std::string::size_type pos = comment.find_first_not_of("/* ");
    if (pos == std::string::npos)
        return false;
    return comment.compare(pos, polyspace.size(), polyspace, 0, polyspace.size()) == 0;
}

bool polyspace::Suppression::matches(const polyspace::Suppression &other) const
{
    return family == other.family && resultName == other.resultName;
}

bool polyspace::Suppression::convert(const Settings &settings, SuppressionList::Suppression &suppr) const
{
    static const std::map<std::string, std::string> map = {
        { "MISRA-C-2023", "premium-misra-c-2023-" },
        { "MISRA-CPP", "premium-misra-cpp-2008-" },
        { "MISRA-CPP-2023", "premium-misra-cpp-2023-" },
        { "CERT-C", "premium-cert-c-" },
        { "CERT-CPP", "premium-cert-cpp-" },
        { "AUTOSAR-CPP14", "premium-autosar-" },
    };

    const auto it = map.find(family);
    std::string prefix;
    if (it == map.cend()) {
        if (family == "MISRA-C3" || family == "MISRA2012") {
            if (settings.premiumArgs.empty()) {
                prefix = "misra-c2012-";
            } else {
                prefix = "premium-misra-c-2012-";
            }
        } else {
            return false;
        }
    } else {
        prefix = it->second;
    }

    suppr.errorId = prefix + resultName;
    suppr.isInline = true;
    suppr.fileName = filename;

    suppr.lineNumber = lineBegin;
    if (lineBegin == lineEnd) {
        suppr.type = SuppressionList::Type::unique;
    } else {
        suppr.type = SuppressionList::Type::block;
        suppr.lineBegin = lineBegin;
        suppr.lineEnd = lineEnd;
    }

    return true;
}
