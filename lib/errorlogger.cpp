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

#include "errorlogger.h"

#include "color.h"
#include "cppcheck.h"
#include "mathlib.h"
#include "path.h"
#include "token.h"
#include "tokenlist.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <memory>
#include <sstream> // IWYU pragma: keep
#include <string>
#include <utility>

#include <tinyxml2.h>

ErrorMessage::ErrorMessage()
    : severity(Severity::none), cwe(0U), certainty(Certainty::normal), hash(0)
{}

ErrorMessage::ErrorMessage(std::list<FileLocation> callStack, std::string file1, Severity::SeverityType severity, const std::string &msg, std::string id, Certainty certainty) :
    callStack(std::move(callStack)), // locations for this error message
    id(std::move(id)),               // set the message id
    file0(std::move(file1)),
    severity(severity),   // severity for this error message
    cwe(0U),
    certainty(certainty),
    hash(0)
{
    // set the summary and verbose messages
    setmsg(msg);
}



ErrorMessage::ErrorMessage(std::list<FileLocation> callStack, std::string file1, Severity::SeverityType severity, const std::string &msg, std::string id, const CWE &cwe, Certainty certainty) :
    callStack(std::move(callStack)), // locations for this error message
    id(std::move(id)),               // set the message id
    file0(std::move(file1)),
    severity(severity),   // severity for this error message
    cwe(cwe.id),
    certainty(certainty),
    hash(0)
{
    // set the summary and verbose messages
    setmsg(msg);
}

ErrorMessage::ErrorMessage(const std::list<const Token*>& callstack, const TokenList* list, Severity::SeverityType severity, std::string id, const std::string& msg, Certainty certainty)
    : id(std::move(id)), severity(severity), cwe(0U), certainty(certainty), hash(0)
{
    // Format callstack
    for (std::list<const Token *>::const_iterator it = callstack.cbegin(); it != callstack.cend(); ++it) {
        // --errorlist can provide null values here
        if (!(*it))
            continue;

        callStack.emplace_back(*it, list);
    }

    if (list && !list->getFiles().empty())
        file0 = list->getFiles()[0];

    setmsg(msg);
}


ErrorMessage::ErrorMessage(const std::list<const Token*>& callstack, const TokenList* list, Severity::SeverityType severity, std::string id, const std::string& msg, const CWE &cwe, Certainty certainty)
    : id(std::move(id)), severity(severity), cwe(cwe.id), certainty(certainty)
{
    // Format callstack
    for (const Token *tok: callstack) {
        // --errorlist can provide null values here
        if (!tok)
            continue;

        callStack.emplace_back(tok, list);
    }

    if (list && !list->getFiles().empty())
        file0 = list->getFiles()[0];

    setmsg(msg);

    hash = 0; // calculateWarningHash(list, hashWarning.str());
}

ErrorMessage::ErrorMessage(const ErrorPath &errorPath, const TokenList *tokenList, Severity::SeverityType severity, const char id[], const std::string &msg, const CWE &cwe, Certainty certainty)
    : id(id), severity(severity), cwe(cwe.id), certainty(certainty)
{
    // Format callstack
    for (const ErrorPathItem& e: errorPath) {
        const Token *tok = e.first;
        std::string info = e.second;

        if (info.compare(0,8,"$symbol:") == 0 && info.find("\n") < info.size()) {
            const std::string::size_type pos = info.find("\n");
            const std::string &symbolName = info.substr(8, pos - 8);
            info = replaceStr(info.substr(pos+1), "$symbol", symbolName);
        }

        // --errorlist can provide null values here
        if (tok)
            callStack.emplace_back(tok, info, tokenList);
    }

    if (tokenList && !tokenList->getFiles().empty())
        file0 = tokenList->getFiles()[0];

    setmsg(msg);

    hash = 0; // calculateWarningHash(tokenList, hashWarning.str());
}

ErrorMessage::ErrorMessage(const tinyxml2::XMLElement * const errmsg)
    : severity(Severity::none),
    cwe(0U),
    certainty(Certainty::normal)
{
    const char * const unknown = "<UNKNOWN>";

    const char *attr = errmsg->Attribute("id");
    id = attr ? attr : unknown;

    attr = errmsg->Attribute("severity");
    severity = attr ? Severity::fromString(attr) : Severity::none;

    attr = errmsg->Attribute("cwe");
    std::istringstream(attr ? attr : "0") >> cwe.id;

    attr = errmsg->Attribute("inconclusive");
    certainty = (attr && (std::strcmp(attr, "true") == 0)) ? Certainty::inconclusive : Certainty::normal;

    attr = errmsg->Attribute("msg");
    mShortMessage = attr ? attr : "";

    attr = errmsg->Attribute("verbose");
    mVerboseMessage = attr ? attr : "";

    attr = errmsg->Attribute("hash");
    std::istringstream(attr ? attr : "0") >> hash;

    for (const tinyxml2::XMLElement *e = errmsg->FirstChildElement(); e; e = e->NextSiblingElement()) {
        if (std::strcmp(e->Name(),"location")==0) {
            const char *strfile = e->Attribute("file");
            const char *strinfo = e->Attribute("info");
            const char *strline = e->Attribute("line");
            const char *strcolumn = e->Attribute("column");

            const char *file = strfile ? strfile : unknown;
            const char *info = strinfo ? strinfo : "";
            const int line = strline ? std::atoi(strline) : 0;
            const int column = strcolumn ? std::atoi(strcolumn) : 0;
            callStack.emplace_front(file, info, line, column);
        } else if (std::strcmp(e->Name(),"symbol")==0) {
            mSymbolNames += e->GetText();
        }
    }
}

void ErrorMessage::setmsg(const std::string &msg)
{
    // If a message ends to a '\n' and contains only a one '\n'
    // it will cause the mVerboseMessage to be empty which will show
    // as an empty message to the user if --verbose is used.
    // Even this doesn't cause problems with messages that have multiple
    // lines, none of the error messages should end into it.
    assert(!endsWith(msg,'\n'));

    // The summary and verbose message are separated by a newline
    // If there is no newline then both the summary and verbose messages
    // are the given message
    const std::string::size_type pos = msg.find('\n');
    const std::string symbolName = mSymbolNames.empty() ? std::string() : mSymbolNames.substr(0, mSymbolNames.find('\n'));
    if (pos == std::string::npos) {
        mShortMessage = replaceStr(msg, "$symbol", symbolName);
        mVerboseMessage = replaceStr(msg, "$symbol", symbolName);
    } else if (msg.compare(0,8,"$symbol:") == 0) {
        mSymbolNames += msg.substr(8, pos-7);
        setmsg(msg.substr(pos + 1));
    } else {
        mShortMessage = replaceStr(msg.substr(0, pos), "$symbol", symbolName);
        mVerboseMessage = replaceStr(msg.substr(pos + 1), "$symbol", symbolName);
    }
}

Suppressions::ErrorMessage ErrorMessage::toSuppressionsErrorMessage() const
{
    Suppressions::ErrorMessage ret;
    ret.hash = hash;
    ret.errorId = id;
    if (!callStack.empty()) {
        ret.setFileName(callStack.back().getfile(false));
        ret.lineNumber = callStack.back().line;
    } else {
        ret.lineNumber = Suppressions::Suppression::NO_LINE;
    }
    ret.certainty = certainty;
    ret.symbolNames = mSymbolNames;
    return ret;
}


std::string ErrorMessage::serialize() const
{
    // Serialize this message into a simple string
    std::ostringstream oss;
    oss << id.length() << " " << id;
    oss << Severity::toString(severity).length() << " " << Severity::toString(severity);
    oss << MathLib::toString(cwe.id).length() << " " << MathLib::toString(cwe.id);
    oss << MathLib::toString(hash).length() << " " << MathLib::toString(hash);
    oss << file0.size() << " " << file0;
    if (certainty == Certainty::inconclusive) {
        const std::string text("inconclusive");
        oss << text.length() << " " << text;
    }

    const std::string saneShortMessage = fixInvalidChars(mShortMessage);
    const std::string saneVerboseMessage = fixInvalidChars(mVerboseMessage);

    oss << saneShortMessage.length() << " " << saneShortMessage;
    oss << saneVerboseMessage.length() << " " << saneVerboseMessage;
    oss << callStack.size() << " ";

    for (std::list<ErrorMessage::FileLocation>::const_iterator loc = callStack.cbegin(); loc != callStack.cend(); ++loc) {
        std::ostringstream smallStream;
        smallStream << (*loc).line << '\t' << (*loc).column << '\t' << (*loc).getfile(false) << '\t' << loc->getOrigFile(false) << '\t' << loc->getinfo();
        oss << smallStream.str().length() << " " << smallStream.str();
    }

    return oss.str();
}

void ErrorMessage::deserialize(const std::string &data)
{
    // TODO: clear all fields
    certainty = Certainty::normal;
    callStack.clear();

    std::istringstream iss(data);
    std::array<std::string, 7> results;
    std::size_t elem = 0;
    while (iss.good() && elem < 7) {
        unsigned int len = 0;
        if (!(iss >> len))
            throw InternalError(nullptr, "Internal Error: Deserialization of error message failed - invalid length");

        if (iss.get() != ' ')
            throw InternalError(nullptr, "Internal Error: Deserialization of error message failed - invalid separator");

        if (!iss.good())
            throw InternalError(nullptr, "Internal Error: Deserialization of error message failed - premature end of data");

        std::string temp;
        if (len > 0) {
            temp.resize(len);
            iss.read(&temp[0], len);

            if (!iss.good())
                throw InternalError(nullptr, "Internal Error: Deserialization of error message failed - premature end of data");

            if (temp == "inconclusive") {
                certainty = Certainty::inconclusive;
                continue;
            }
        }

        results[elem++] = temp;
    }

    if (!iss.good())
        throw InternalError(nullptr, "Internal Error: Deserialization of error message failed - premature end of data");

    if (elem != 7)
        throw InternalError(nullptr, "Internal Error: Deserialization of error message failed - insufficient elements");

    id = std::move(results[0]);
    severity = Severity::fromString(results[1]);
    unsigned long long tmp = 0;
    if (!results[2].empty()) {
        try {
            tmp = MathLib::toULongNumber(results[2]);
        }
        catch (const InternalError&) {
            throw InternalError(nullptr, "Internal Error: Deserialization of error message failed - invalid CWE ID");
        }
        if (tmp > std::numeric_limits<unsigned short>::max())
            throw InternalError(nullptr, "Internal Error: Deserialization of error message failed - CWE ID is out of range");
    }
    cwe.id = static_cast<unsigned short>(tmp);
    hash = 0;
    if (!results[3].empty()) {
        try {
            hash = MathLib::toULongNumber(results[3]);
        }
        catch (const InternalError&) {
            throw InternalError(nullptr, "Internal Error: Deserialization of error message failed - invalid hash");
        }
    }
    file0 = std::move(results[4]);
    mShortMessage = std::move(results[5]);
    mVerboseMessage = std::move(results[6]);

    unsigned int stackSize = 0;
    if (!(iss >> stackSize))
        throw InternalError(nullptr, "Internal Error: Deserialization of error message failed - invalid stack size");

    if (iss.get() != ' ')
        throw InternalError(nullptr, "Internal Error: Deserialization of error message failed - invalid separator");

    if (stackSize == 0)
        return;

    while (iss.good()) {
        unsigned int len = 0;
        if (!(iss >> len))
            throw InternalError(nullptr, "Internal Error: Deserialization of error message failed - invalid length (stack)");

        if (iss.get() != ' ')
            throw InternalError(nullptr, "Internal Error: Deserialization of error message failed - invalid separator (stack)");

        std::string temp;
        if (len > 0) {
            temp.resize(len);
            iss.read(&temp[0], len);

            if (!iss.good())
                throw InternalError(nullptr, "Internal Error: Deserialization of error message failed - premature end of data (stack)");
        }

        std::vector<std::string> substrings;
        substrings.reserve(5);
        for (std::string::size_type pos = 0; pos < temp.size() && substrings.size() < 5; ++pos) {
            if (substrings.size() == 4) {
                substrings.push_back(temp.substr(pos));
                break;
            }
            const std::string::size_type start = pos;
            pos = temp.find("\t", pos);
            if (pos == std::string::npos) {
                substrings.push_back(temp.substr(start));
                break;
            }
            substrings.push_back(temp.substr(start, pos - start));
        }
        if (substrings.size() < 4)
            throw InternalError(nullptr, "Internal Error: Deserializing of error message failed");

        // (*loc).line << '\t' << (*loc).column << '\t' << (*loc).getfile(false) << '\t' << loc->getOrigFile(false) << '\t' << loc->getinfo();

        ErrorMessage::FileLocation loc(substrings[3], MathLib::toLongNumber(substrings[0]), MathLib::toLongNumber(substrings[1]));
        loc.setfile(std::move(substrings[2]));
        if (substrings.size() == 5)
            loc.setinfo(substrings[4]);

        callStack.push_back(std::move(loc));

        if (callStack.size() >= stackSize)
            break;
    }
}

std::string ErrorMessage::getXMLHeader(const std::string& productName)
{
    tinyxml2::XMLPrinter printer;

    // standard xml header
    printer.PushDeclaration("xml version=\"1.0\" encoding=\"UTF-8\"");

    // header
    printer.OpenElement("results", false);

    printer.PushAttribute("version", 2);
    printer.OpenElement("cppcheck", false);
    if (!productName.empty())
        printer.PushAttribute("product-name", productName.c_str());
    printer.PushAttribute("version", CppCheck::version());
    printer.CloseElement(false);
    printer.OpenElement("errors", false);

    return std::string(printer.CStr()) + '>';
}

std::string ErrorMessage::getXMLFooter()
{
    return "    </errors>\n</results>";
}

// There is no utf-8 support around but the strings should at least be safe for to tinyxml2.
// See #5300 "Invalid encoding in XML output" and  #6431 "Invalid XML created - Invalid encoding of string literal "
std::string ErrorMessage::fixInvalidChars(const std::string& raw)
{
    std::string result;
    result.reserve(raw.length());
    std::string::const_iterator from=raw.cbegin();
    while (from!=raw.cend()) {
        if (std::isprint(static_cast<unsigned char>(*from))) {
            result.push_back(*from);
        } else {
            std::ostringstream es;
            // straight cast to (unsigned) doesn't work out.
            const unsigned uFrom = (unsigned char)*from;
            es << '\\' << std::setbase(8) << std::setw(3) << std::setfill('0') << uFrom;
            result += es.str();
        }
        ++from;
    }
    return result;
}

std::string ErrorMessage::toXML() const
{
    tinyxml2::XMLPrinter printer(nullptr, false, 2);
    printer.OpenElement("error", false);
    printer.PushAttribute("id", id.c_str());
    printer.PushAttribute("severity", Severity::toString(severity).c_str());
    printer.PushAttribute("msg", fixInvalidChars(mShortMessage).c_str());
    printer.PushAttribute("verbose", fixInvalidChars(mVerboseMessage).c_str());
    if (cwe.id)
        printer.PushAttribute("cwe", cwe.id);
    if (hash)
        printer.PushAttribute("hash", MathLib::toString(hash).c_str());
    if (certainty == Certainty::inconclusive)
        printer.PushAttribute("inconclusive", "true");

    if (!file0.empty())
        printer.PushAttribute("file0", file0.c_str());

    for (std::list<FileLocation>::const_reverse_iterator it = callStack.crbegin(); it != callStack.crend(); ++it) {
        printer.OpenElement("location", false);
        printer.PushAttribute("file", (*it).getfile().c_str());
        printer.PushAttribute("line", std::max((*it).line,0));
        printer.PushAttribute("column", (*it).column);
        if (!it->getinfo().empty())
            printer.PushAttribute("info", fixInvalidChars(it->getinfo()).c_str());
        printer.CloseElement(false);
    }
    for (std::string::size_type pos = 0; pos < mSymbolNames.size();) {
        const std::string::size_type pos2 = mSymbolNames.find('\n', pos);
        std::string symbolName;
        if (pos2 == std::string::npos) {
            symbolName = mSymbolNames.substr(pos);
            pos = pos2;
        } else {
            symbolName = mSymbolNames.substr(pos, pos2-pos);
            pos = pos2 + 1;
        }
        printer.OpenElement("symbol", false);
        printer.PushText(symbolName.c_str());
        printer.CloseElement(false);
    }
    printer.CloseElement(false);
    return printer.CStr();
}

/**
 * Replace all occurrences of searchFor with replaceWith in the
 * given source.
 * @param source The string to modify
 * @param searchFor What should be searched for
 * @param replaceWith What will replace the found item
 */
static void findAndReplace(std::string &source, const std::string &searchFor, const std::string &replaceWith)
{
    std::string::size_type index = 0;
    while ((index = source.find(searchFor, index)) != std::string::npos) {
        source.replace(index, searchFor.length(), replaceWith);
        index += replaceWith.length();
    }
}

// TODO: read info from some shared resource instead?
static std::string readCode(const std::string &file, int linenr, int column, const char endl[])
{
    std::ifstream fin(file);
    std::string line;
    while (linenr > 0 && std::getline(fin,line)) {
        linenr--;
    }
    const std::string::size_type endPos = line.find_last_not_of("\r\n\t ");
    if (endPos + 1 < line.size())
        line.erase(endPos + 1);
    std::string::size_type pos = 0;
    while ((pos = line.find('\t', pos)) != std::string::npos)
        line[pos] = ' ';
    return line + endl + std::string((column>0 ? column-1 : 0), ' ') + '^';
}

static void replaceColors(std::string& source)
{
    static const std::string reset_str = ::toString(Color::Reset);
    findAndReplace(source, "{reset}", reset_str);
    static const std::string bold_str = ::toString(Color::Bold);
    findAndReplace(source, "{bold}", bold_str);
    static const std::string dim_str = ::toString(Color::Dim);
    findAndReplace(source, "{dim}", dim_str);
    static const std::string red_str = ::toString(Color::FgRed);
    findAndReplace(source, "{red}", red_str);
    static const std::string green_str = ::toString(Color::FgGreen);
    findAndReplace(source, "{green}", green_str);
    static const std::string blue_str = ::toString(Color::FgBlue);
    findAndReplace(source, "{blue}", blue_str);
    static const std::string magenta_str = ::toString(Color::FgMagenta);
    findAndReplace(source, "{magenta}", magenta_str);
    static const std::string default_str = ::toString(Color::FgDefault);
    findAndReplace(source, "{default}", default_str);
}

std::string ErrorMessage::toString(bool verbose, const std::string &templateFormat, const std::string &templateLocation) const
{
    // Save this ErrorMessage in plain text.

    // No template is given
    if (templateFormat.empty()) {
        std::string text;
        if (!callStack.empty()) {
            text += ErrorLogger::callStackToString(callStack);
            text += ": ";
        }
        if (severity != Severity::none) {
            text += '(';
            text += Severity::toString(severity);
            if (certainty == Certainty::inconclusive)
                text += ", inconclusive";
            text += ") ";
        }
        text += (verbose ? mVerboseMessage : mShortMessage);
        return text;
    }

    // template is given. Reformat the output according to it
    std::string result = templateFormat;
    // Support a few special characters to allow to specific formatting, see http://sourceforge.net/apps/phpbb/cppcheck/viewtopic.php?f=4&t=494&sid=21715d362c0dbafd3791da4d9522f814
    // Substitution should be done first so messages from cppcheck never get translated.
    findAndReplace(result, "\\b", "\b");
    findAndReplace(result, "\\n", "\n");
    findAndReplace(result, "\\r", "\r");
    findAndReplace(result, "\\t", "\t");

    replaceColors(result);
    findAndReplace(result, "{id}", id);

    std::string::size_type pos1 = result.find("{inconclusive:");
    while (pos1 != std::string::npos) {
        const std::string::size_type pos2 = result.find('}', pos1+1);
        const std::string replaceFrom = result.substr(pos1,pos2-pos1+1);
        const std::string replaceWith = (certainty == Certainty::inconclusive) ? result.substr(pos1+14, pos2-pos1-14) : std::string();
        findAndReplace(result, replaceFrom, replaceWith);
        pos1 = result.find("{inconclusive:", pos1);
    }
    findAndReplace(result, "{severity}", Severity::toString(severity));
    findAndReplace(result, "{cwe}", MathLib::toString(cwe.id));
    findAndReplace(result, "{message}", verbose ? mVerboseMessage : mShortMessage);
    if (!callStack.empty()) {
        if (result.find("{callstack}") != std::string::npos)
            findAndReplace(result, "{callstack}", ErrorLogger::callStackToString(callStack));
        findAndReplace(result, "{file}", callStack.back().getfile());
        findAndReplace(result, "{line}", MathLib::toString(callStack.back().line));
        findAndReplace(result, "{column}", MathLib::toString(callStack.back().column));
        if (result.find("{code}") != std::string::npos) {
            const std::string::size_type pos = result.find('\r');
            const char *endl;
            if (pos == std::string::npos)
                endl = "\n";
            else if (pos+1 < result.size() && result[pos+1] == '\n')
                endl = "\r\n";
            else
                endl = "\r";
            findAndReplace(result, "{code}", readCode(callStack.back().getOrigFile(), callStack.back().line, callStack.back().column, endl));
        }
    } else {
        findAndReplace(result, "{callstack}", emptyString);
        findAndReplace(result, "{file}", "nofile");
        findAndReplace(result, "{line}", "0");
        findAndReplace(result, "{column}", "0");
        findAndReplace(result, "{code}", emptyString);
    }

    if (!templateLocation.empty() && callStack.size() >= 2U) {
        for (const FileLocation &fileLocation : callStack) {
            std::string text = templateLocation;

            findAndReplace(text, "\\b", "\b");
            findAndReplace(text, "\\n", "\n");
            findAndReplace(text, "\\r", "\r");
            findAndReplace(text, "\\t", "\t");

            replaceColors(text);
            findAndReplace(text, "{file}", fileLocation.getfile());
            findAndReplace(text, "{line}", MathLib::toString(fileLocation.line));
            findAndReplace(text, "{column}", MathLib::toString(fileLocation.column));
            findAndReplace(text, "{info}", fileLocation.getinfo().empty() ? mShortMessage : fileLocation.getinfo());
            if (text.find("{code}") != std::string::npos) {
                const std::string::size_type pos = text.find('\r');
                const char *endl;
                if (pos == std::string::npos)
                    endl = "\n";
                else if (pos+1 < text.size() && text[pos+1] == '\n')
                    endl = "\r\n";
                else
                    endl = "\r";
                findAndReplace(text, "{code}", readCode(fileLocation.getOrigFile(), fileLocation.line, fileLocation.column, endl));
            }
            result += '\n' + text;
        }
    }

    return result;
}

bool ErrorLogger::reportUnmatchedSuppressions(const std::list<Suppressions::Suppression> &unmatched)
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

        std::list<ErrorMessage::FileLocation> callStack;
        if (!s.fileName.empty())
            callStack.emplace_back(s.fileName, s.lineNumber, 0);
        reportErr(ErrorMessage(callStack, emptyString, Severity::information, "Unmatched suppression: " + s.errorId, "unmatchedSuppression", Certainty::normal));
        err = true;
    }
    return err;
}

std::string ErrorLogger::callStackToString(const std::list<ErrorMessage::FileLocation> &callStack)
{
    std::string str;
    for (std::list<ErrorMessage::FileLocation>::const_iterator tok = callStack.cbegin(); tok != callStack.cend(); ++tok) {
        str += (tok == callStack.cbegin() ? "" : " -> ");
        str += tok->stringify();
    }
    return str;
}


ErrorMessage::FileLocation::FileLocation(const Token* tok, const TokenList* tokenList)
    : fileIndex(tok->fileIndex()), line(tok->linenr()), column(tok->column()), mOrigFileName(tokenList->getOrigFile(tok)), mFileName(tokenList->file(tok))
{}

ErrorMessage::FileLocation::FileLocation(const Token* tok, std::string info, const TokenList* tokenList)
    : fileIndex(tok->fileIndex()), line(tok->linenr()), column(tok->column()), mOrigFileName(tokenList->getOrigFile(tok)), mFileName(tokenList->file(tok)), mInfo(std::move(info))
{}

std::string ErrorMessage::FileLocation::getfile(bool convert) const
{
    if (convert)
        return Path::toNativeSeparators(mFileName);
    return mFileName;
}

std::string ErrorMessage::FileLocation::getOrigFile(bool convert) const
{
    if (convert)
        return Path::toNativeSeparators(mOrigFileName);
    return mOrigFileName;
}

void ErrorMessage::FileLocation::setfile(std::string file)
{
    mFileName = Path::fromNativeSeparators(std::move(file));
    mFileName = Path::simplifyPath(std::move(mFileName));
}

std::string ErrorMessage::FileLocation::stringify() const
{
    std::string str;
    str += '[';
    str += Path::toNativeSeparators(mFileName);
    if (line != Suppressions::Suppression::NO_LINE) {
        str += ':';
        str += std::to_string(line);
    }
    str += ']';
    return str;
}

std::string ErrorLogger::toxml(const std::string &str)
{
    std::ostringstream xml;
    for (const unsigned char c : str) {
        switch (c) {
        case '<':
            xml << "&lt;";
            break;
        case '>':
            xml << "&gt;";
            break;
        case '&':
            xml << "&amp;";
            break;
        case '\"':
            xml << "&quot;";
            break;
        case '\'':
            xml << "&apos;";
            break;
        case '\0':
            xml << "\\0";
            break;
        default:
            if (c >= ' ' && c <= 0x7f)
                xml << c;
            else
                xml << 'x';
            break;
        }
    }
    return xml.str();
}

std::string ErrorLogger::plistHeader(const std::string &version, const std::vector<std::string> &files)
{
    std::ostringstream ostr;
    ostr << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
         << "<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\r\n"
         << "<plist version=\"1.0\">\r\n"
         << "<dict>\r\n"
         << " <key>clang_version</key>\r\n"
         << "<string>cppcheck version " << version << "</string>\r\n"
         << " <key>files</key>\r\n"
         << " <array>\r\n";
    for (const std::string & file : files)
        ostr << "  <string>" << ErrorLogger::toxml(file) << "</string>\r\n";
    ostr << " </array>\r\n"
         << " <key>diagnostics</key>\r\n"
         << " <array>\r\n";
    return ostr.str();
}

static std::string plistLoc(const char indent[], const ErrorMessage::FileLocation &loc)
{
    std::ostringstream ostr;
    ostr << indent << "<dict>\r\n"
         << indent << ' ' << "<key>line</key><integer>" << loc.line << "</integer>\r\n"
         << indent << ' ' << "<key>col</key><integer>" << loc.column << "</integer>\r\n"
         << indent << ' ' << "<key>file</key><integer>" << loc.fileIndex << "</integer>\r\n"
         << indent << "</dict>\r\n";
    return ostr.str();
}

std::string ErrorLogger::plistData(const ErrorMessage &msg)
{
    std::ostringstream plist;
    plist << "  <dict>\r\n"
          << "   <key>path</key>\r\n"
          << "   <array>\r\n";

    std::list<ErrorMessage::FileLocation>::const_iterator prev = msg.callStack.cbegin();

    for (std::list<ErrorMessage::FileLocation>::const_iterator it = msg.callStack.cbegin(); it != msg.callStack.cend(); ++it) {
        if (prev != it) {
            plist << "    <dict>\r\n"
                  << "     <key>kind</key><string>control</string>\r\n"
                  << "     <key>edges</key>\r\n"
                  << "      <array>\r\n"
                  << "       <dict>\r\n"
                  << "        <key>start</key>\r\n"
                  << "         <array>\r\n"
                  << plistLoc("          ", *prev)
                  << plistLoc("          ", *prev)
                  << "         </array>\r\n"
                  << "        <key>end</key>\r\n"
                  << "         <array>\r\n"
                  << plistLoc("          ", *it)
                  << plistLoc("          ", *it)
                  << "         </array>\r\n"
                  << "       </dict>\r\n"
                  << "      </array>\r\n"
                  << "    </dict>\r\n";
            prev = it;
        }

        std::list<ErrorMessage::FileLocation>::const_iterator next = it;
        ++next;
        const std::string message = (it->getinfo().empty() && next == msg.callStack.cend() ? msg.shortMessage() : it->getinfo());

        plist << "    <dict>\r\n"
              << "     <key>kind</key><string>event</string>\r\n"
              << "     <key>location</key>\r\n"
              << plistLoc("     ", *it)
              << "     <key>ranges</key>\r\n"
              << "     <array>\r\n"
              << "       <array>\r\n"
              << plistLoc("        ", *it)
              << plistLoc("        ", *it)
              << "       </array>\r\n"
              << "     </array>\r\n"
              << "     <key>depth</key><integer>0</integer>\r\n"
              << "     <key>extended_message</key>\r\n"
              << "     <string>" << ErrorLogger::toxml(message) << "</string>\r\n"
              << "     <key>message</key>\r\n"
              << "     <string>" << ErrorLogger::toxml(message) << "</string>\r\n"
              << "    </dict>\r\n";
    }

    plist << "   </array>\r\n"
          << "   <key>description</key><string>" << ErrorLogger::toxml(msg.shortMessage()) << "</string>\r\n"
          << "   <key>category</key><string>" << Severity::toString(msg.severity) << "</string>\r\n"
          << "   <key>type</key><string>" << ErrorLogger::toxml(msg.shortMessage()) << "</string>\r\n"
          << "   <key>check_name</key><string>" << msg.id << "</string>\r\n"
          << "   <!-- This hash is experimental and going to change! -->\r\n"
          << "   <key>issue_hash_content_of_line_in_context</key><string>" << 0 << "</string>\r\n"
          << "  <key>issue_context_kind</key><string></string>\r\n"
          << "  <key>issue_context</key><string></string>\r\n"
          << "  <key>issue_hash_function_offset</key><string></string>\r\n"
          << "  <key>location</key>\r\n"
          << plistLoc("  ", msg.callStack.back())
          << "  </dict>\r\n";
    return plist.str();
}


std::string replaceStr(std::string s, const std::string &from, const std::string &to)
{
    std::string::size_type pos1 = 0;
    while (pos1 < s.size()) {
        pos1 = s.find(from, pos1);
        if (pos1 == std::string::npos)
            return s;
        if (pos1 > 0 && (s[pos1-1] == '_' || std::isalnum(s[pos1-1]))) {
            pos1++;
            continue;
        }
        const std::string::size_type pos2 = pos1 + from.size();
        if (pos2 >= s.size())
            return s.substr(0,pos1) + to;
        if (s[pos2] == '_' || std::isalnum(s[pos2])) {
            pos1++;
            continue;
        }
        s.replace(pos1, from.size(), to);
        pos1 += to.size();
    }
    return s;
}
