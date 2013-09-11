/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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
#include "path.h"
#include "cppcheck.h"
#include "tokenlist.h"
#include "token.h"

#include <tinyxml2.h>

#include <cassert>
#include <sstream>
#include <vector>

InternalError::InternalError(const Token *tok, const std::string &errorMsg) :
    token(tok), errorMessage(errorMsg)
{
}

ErrorLogger::ErrorMessage::ErrorMessage()
    : _severity(Severity::none), _inconclusive(false)
{
}

ErrorLogger::ErrorMessage::ErrorMessage(const std::list<FileLocation> &callStack, Severity::SeverityType severity, const std::string &msg, const std::string &id, bool inconclusive) :
    _callStack(callStack), // locations for this error message
    _id(id),               // set the message id
    _severity(severity),   // severity for this error message
    _inconclusive(inconclusive)
{
    // set the summary and verbose messages
    setmsg(msg);
}

ErrorLogger::ErrorMessage::ErrorMessage(const std::list<const Token*>& callstack, const TokenList* list, Severity::SeverityType severity, const std::string& id, const std::string& msg, bool inconclusive)
    : _id(id), _severity(severity), _inconclusive(inconclusive)
{
    // Format callstack
    for (std::list<const Token *>::const_iterator it = callstack.begin(); it != callstack.end(); ++it) {
        // --errorlist can provide null values here
        if (!(*it))
            continue;

        _callStack.push_back(ErrorLogger::ErrorMessage::FileLocation(*it, list));
    }

    if (list && !list->getFiles().empty())
        file0 = list->getFiles()[0];

    setmsg(msg);
}

void ErrorLogger::ErrorMessage::setmsg(const std::string &msg)
{
    // If a message ends to a '\n' and contains only a one '\n'
    // it will cause the _verboseMessage to be empty which will show
    // as an empty message to the user if --verbose is used.
    // Even this doesn't cause problems with messages that have multiple
    // lines, none of the the error messages should end into it.
    assert(!(msg[msg.size()-1]=='\n'));

    // The summary and verbose message are separated by a newline
    // If there is no newline then both the summary and verbose messages
    // are the given message
    const std::string::size_type pos = msg.find("\n");
    if (pos == std::string::npos) {
        _shortMessage = msg;
        _verboseMessage = msg;
    } else {
        _shortMessage = msg.substr(0, pos);
        _verboseMessage = msg.substr(pos + 1);
    }
}

std::string ErrorLogger::ErrorMessage::serialize() const
{
    // Serialize this message into a simple string
    std::ostringstream oss;
    oss << _id.length() << " " << _id;
    oss << Severity::toString(_severity).length() << " " << Severity::toString(_severity);
    if (_inconclusive) {
        const std::string inconclusive("inconclusive");
        oss << inconclusive.length() << " " << inconclusive;
    }
    oss << _shortMessage.length() << " " << _shortMessage;
    oss << _verboseMessage.length() << " " << _verboseMessage;
    oss << _callStack.size() << " ";

    for (std::list<ErrorLogger::ErrorMessage::FileLocation>::const_iterator tok = _callStack.begin(); tok != _callStack.end(); ++tok) {
        std::ostringstream smallStream;
        smallStream << (*tok).line << ":" << (*tok).getfile();
        oss << smallStream.str().length() << " " << smallStream.str();
    }

    return oss.str();
}

bool ErrorLogger::ErrorMessage::deserialize(const std::string &data)
{
    _inconclusive = false;
    _callStack.clear();
    std::istringstream iss(data);
    std::vector<std::string> results;
    while (iss.good()) {
        unsigned int len = 0;
        if (!(iss >> len))
            return false;

        iss.get();
        std::string temp;
        for (unsigned int i = 0; i < len && iss.good(); ++i) {
            char c = static_cast<char>(iss.get());
            temp.append(1, c);
        }

        if (temp == "inconclusive") {
            _inconclusive = true;
            continue;
        }

        results.push_back(temp);
        if (results.size() == 4)
            break;
    }

    _id = results[0];
    _severity = Severity::fromString(results[1]);
    _shortMessage = results[2];
    _verboseMessage = results[3];

    unsigned int stackSize = 0;
    if (!(iss >> stackSize))
        return false;

    while (iss.good()) {
        unsigned int len = 0;
        if (!(iss >> len))
            return false;

        iss.get();
        std::string temp;
        for (unsigned int i = 0; i < len && iss.good(); ++i) {
            char c = static_cast<char>(iss.get());
            temp.append(1, c);
        }

        ErrorLogger::ErrorMessage::FileLocation loc;
        loc.setfile(temp.substr(temp.find(':') + 1));
        temp = temp.substr(0, temp.find(':'));
        std::istringstream fiss(temp);
        fiss >> loc.line;

        _callStack.push_back(loc);

        if (_callStack.size() >= stackSize)
            break;
    }

    return true;
}

std::string ErrorLogger::ErrorMessage::getXMLHeader(int xml_version)
{
    // xml_version 1 is the default xml format

    tinyxml2::XMLPrinter printer;

    // standard xml header
    printer.PushDeclaration("xml version=\"1.0\" encoding=\"UTF-8\"");

    // header
    printer.OpenElement("results");
    // version 2 header
    if (xml_version == 2) {
        printer.PushAttribute("version", xml_version);
        printer.OpenElement("cppcheck");
        printer.PushAttribute("version", CppCheck::version());
        printer.CloseElement();
        printer.OpenElement("errors");
    }

    return std::string(printer.CStr()) + '>';
}

std::string ErrorLogger::ErrorMessage::getXMLFooter(int xml_version)
{
    return (xml_version<=1) ? "</results>" : "    </errors>\n</results>";
}

std::string ErrorLogger::ErrorMessage::toXML(bool verbose, int version) const
{
    // The default xml format
    if (version == 1) {
        // No inconclusive messages in the xml version 1
        if (_inconclusive)
            return "";

        tinyxml2::XMLPrinter printer(0, false, 1);
        printer.OpenElement("error");
        if (!_callStack.empty()) {
            printer.PushAttribute("file", _callStack.back().getfile().c_str());
            printer.PushAttribute("line", _callStack.back().line);
        }
        printer.PushAttribute("id", _id.c_str());
        printer.PushAttribute("severity", (_severity == Severity::error ? "error" : "style"));
        printer.PushAttribute("msg", (verbose ? _verboseMessage : _shortMessage).c_str());
        printer.CloseElement();
        return printer.CStr();
    }

    // The xml format you get when you use --xml-version=2
    else if (version == 2) {
        tinyxml2::XMLPrinter printer(0, false, 2);
        printer.OpenElement("error");
        printer.PushAttribute("id", _id.c_str());
        printer.PushAttribute("severity", Severity::toString(_severity).c_str());
        printer.PushAttribute("msg", _shortMessage.c_str());
        printer.PushAttribute("verbose", _verboseMessage.c_str());
        if (_inconclusive)
            printer.PushAttribute("inconclusive", "true");

        for (std::list<FileLocation>::const_reverse_iterator it = _callStack.rbegin(); it != _callStack.rend(); ++it) {
            printer.OpenElement("location");
            printer.PushAttribute("file", (*it).getfile().c_str());
            printer.PushAttribute("line", (*it).line);
            printer.CloseElement();
        }
        printer.CloseElement();
        return printer.CStr();
    }

    return "";
}

void ErrorLogger::ErrorMessage::findAndReplace(std::string &source, const std::string &searchFor, const std::string &replaceWith)
{
    std::string::size_type index = 0;
    while ((index = source.find(searchFor, index)) != std::string::npos) {
        source.replace(index, searchFor.length(), replaceWith);
        index += replaceWith.length() - searchFor.length() + 1;
    }
}

std::string ErrorLogger::ErrorMessage::toString(bool verbose, const std::string &outputFormat) const
{
    // Save this ErrorMessage in plain text.

    // No template is given
    if (outputFormat.length() == 0) {
        std::ostringstream text;
        if (!_callStack.empty())
            text << callStackToString(_callStack) << ": ";
        if (_severity != Severity::none) {
            text << '(' << Severity::toString(_severity);
            if (_inconclusive)
                text << ", inconclusive";
            text << ") ";
        }
        text << (verbose ? _verboseMessage : _shortMessage);
        return text.str();
    }

    // template is given. Reformat the output according to it
    else {
        std::string result = outputFormat;
        // Support a few special characters to allow to specific formatting, see http://sourceforge.net/apps/phpbb/cppcheck/viewtopic.php?f=4&t=494&sid=21715d362c0dbafd3791da4d9522f814
        // Substitution should be done first so messages from cppcheck never get translated.
        findAndReplace(result, "\\b", "\b");
        findAndReplace(result, "\\n", "\n");
        findAndReplace(result, "\\r", "\r");
        findAndReplace(result, "\\t", "\t");

        findAndReplace(result, "{id}", _id);
        findAndReplace(result, "{severity}", Severity::toString(_severity));
        findAndReplace(result, "{message}", verbose ? _verboseMessage : _shortMessage);
        findAndReplace(result, "{callstack}", _callStack.empty() ? "" : callStackToString(_callStack));
        if (!_callStack.empty()) {
            std::ostringstream oss;
            oss << _callStack.back().line;
            findAndReplace(result, "{line}", oss.str());
            findAndReplace(result, "{file}", _callStack.back().getfile());
        } else {
            findAndReplace(result, "{file}", "");
            findAndReplace(result, "{line}", "");
        }

        return result;
    }
}

void ErrorLogger::reportUnmatchedSuppressions(const std::list<Suppressions::SuppressionEntry> &unmatched)
{
    // Report unmatched suppressions
    for (std::list<Suppressions::SuppressionEntry>::const_iterator i = unmatched.begin(); i != unmatched.end(); ++i) {
        // don't report "unmatchedSuppression" as unmatched
        if (i->id == "unmatchedSuppression")
            continue;

        // check if this unmatched suppression is suppressed
        bool suppressed = false;
        for (std::list<Suppressions::SuppressionEntry>::const_iterator i2 = unmatched.begin(); i2 != unmatched.end(); ++i2) {
            if (i2->id == "unmatchedSuppression") {
                if ((i2->file == "*" || i2->file == i->file) &&
                    (i2->line == 0 || i2->line == i->line))
                    suppressed = true;
            }
        }

        if (suppressed)
            continue;

        std::list<ErrorLogger::ErrorMessage::FileLocation> callStack;
        callStack.push_back(ErrorLogger::ErrorMessage::FileLocation(i->file, i->line));
        reportErr(ErrorLogger::ErrorMessage(callStack, Severity::information, "Unmatched suppression: " + i->id, "unmatchedSuppression", false));
    }
}

std::string ErrorLogger::callStackToString(const std::list<ErrorLogger::ErrorMessage::FileLocation> &callStack)
{
    std::ostringstream ostr;
    for (std::list<ErrorLogger::ErrorMessage::FileLocation>::const_iterator tok = callStack.begin(); tok != callStack.end(); ++tok) {
        ostr << (tok == callStack.begin() ? "" : " -> ") << tok->stringify();
    }
    return ostr.str();
}


ErrorLogger::ErrorMessage::FileLocation::FileLocation(const Token* tok, const TokenList* list)
    : line(tok->linenr()), _file(list->file(tok))
{
}

std::string ErrorLogger::ErrorMessage::FileLocation::getfile(bool convert) const
{
    if (convert)
        return Path::toNativeSeparators(_file);
    return _file;
}

void ErrorLogger::ErrorMessage::FileLocation::setfile(const std::string &file)
{
    _file = file;
    _file = Path::fromNativeSeparators(_file);
    _file = Path::simplifyPath(_file.c_str());
}

std::string ErrorLogger::ErrorMessage::FileLocation::stringify() const
{
    std::ostringstream oss;
    oss << '[' << Path::toNativeSeparators(_file);
    if (line != 0)
        oss << ':' << line;
    oss << ']';
    return oss.str();
}
