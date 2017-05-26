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

#include "errorlogger.h"

#include "cppcheck.h"
#include "mathlib.h"
#include "path.h"
#include "token.h"
#include "tokenlist.h"
#include "utils.h"

#include <tinyxml2.h>
#include <array>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iomanip>

InternalError::InternalError(const Token *tok, const std::string &errorMsg, Type type) :
    token(tok), errorMessage(errorMsg)
{
    switch (type) {
    case SYNTAX:
        id = "syntaxError";
        break;
    case INTERNAL:
        id = "cppcheckError";
        break;
    }
}

ErrorLogger::ErrorMessage::ErrorMessage()
    : _severity(Severity::none), _cwe(0U), _inconclusive(false)
{
}

ErrorLogger::ErrorMessage::ErrorMessage(const std::list<FileLocation> &callStack, const std::string& file1, Severity::SeverityType severity, const std::string &msg, const std::string &id, bool inconclusive) :
    _callStack(callStack), // locations for this error message
    _id(id),               // set the message id
    file0(file1),
    _severity(severity),   // severity for this error message
    _cwe(0U),
    _inconclusive(inconclusive)
{
    // set the summary and verbose messages
    setmsg(msg);
}



ErrorLogger::ErrorMessage::ErrorMessage(const std::list<FileLocation> &callStack, const std::string& file1, Severity::SeverityType severity, const std::string &msg, const std::string &id, const CWE &cwe, bool inconclusive) :
    _callStack(callStack), // locations for this error message
    _id(id),               // set the message id
    file0(file1),
    _severity(severity),   // severity for this error message
    _cwe(cwe.id),
    _inconclusive(inconclusive)
{
    // set the summary and verbose messages
    setmsg(msg);
}

ErrorLogger::ErrorMessage::ErrorMessage(const std::list<const Token*>& callstack, const TokenList* list, Severity::SeverityType severity, const std::string& id, const std::string& msg, bool inconclusive)
    : _id(id), _severity(severity), _cwe(0U), _inconclusive(inconclusive)
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


ErrorLogger::ErrorMessage::ErrorMessage(const std::list<const Token*>& callstack, const TokenList* list, Severity::SeverityType severity, const std::string& id, const std::string& msg, const CWE &cwe, bool inconclusive)
    : _id(id), _severity(severity), _cwe(cwe.id), _inconclusive(inconclusive)
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

ErrorLogger::ErrorMessage::ErrorMessage(const ErrorPath &errorPath, const TokenList *tokenList, Severity::SeverityType severity, const char id[], const std::string &msg, const CWE &cwe, bool inconclusive)
    : _id(id), _severity(severity), _cwe(cwe.id), _inconclusive(inconclusive)
{
    // Format callstack
    for (ErrorPath::const_iterator it = errorPath.begin(); it != errorPath.end(); ++it) {
        const Token *tok = it->first;
        const std::string &info = it->second;

        // --errorlist can provide null values here
        if (tok)
            _callStack.push_back(ErrorLogger::ErrorMessage::FileLocation(tok, info, tokenList));
    }

    if (tokenList && !tokenList->getFiles().empty())
        file0 = tokenList->getFiles()[0];

    setmsg(msg);
}

ErrorLogger::ErrorMessage::ErrorMessage(const tinyxml2::XMLElement * const errmsg)
    : _id(errmsg->Attribute("id")),
      _severity(Severity::fromString(errmsg->Attribute("severity"))),
      _cwe(0U),
      _inconclusive(false),
      _shortMessage(errmsg->Attribute("msg")),
      _verboseMessage(errmsg->Attribute("verbose"))
{
    const char *attr = errmsg->Attribute("cwe");
    std::istringstream(attr ? attr : "0") >> _cwe.id;
    attr = errmsg->Attribute("inconclusive");
    _inconclusive = attr && (std::strcmp(attr, "true") == 0);
    for (const tinyxml2::XMLElement *e = errmsg->FirstChildElement(); e; e = e->NextSiblingElement()) {
        if (std::strcmp(e->Name(),"location")==0) {
            const char *strfile = e->Attribute("file");
            const char *strinfo = e->Attribute("info");
            const char *strline = e->Attribute("line");
            _callStack.push_back(ErrorLogger::ErrorMessage::FileLocation(strfile, strinfo ? strinfo : "", std::atoi(strline)));
        }
    }
}

void ErrorLogger::ErrorMessage::setmsg(const std::string &msg)
{
    // If a message ends to a '\n' and contains only a one '\n'
    // it will cause the _verboseMessage to be empty which will show
    // as an empty message to the user if --verbose is used.
    // Even this doesn't cause problems with messages that have multiple
    // lines, none of the the error messages should end into it.
    assert(!endsWith(msg,'\n'));

    // The summary and verbose message are separated by a newline
    // If there is no newline then both the summary and verbose messages
    // are the given message
    const std::string::size_type pos = msg.find('\n');
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
    oss << MathLib::toString(_cwe.id).length() << " " << MathLib::toString(_cwe.id);
    if (_inconclusive) {
        const std::string inconclusive("inconclusive");
        oss << inconclusive.length() << " " << inconclusive;
    }

    const std::string saneShortMessage = fixInvalidChars(_shortMessage);
    const std::string saneVerboseMessage = fixInvalidChars(_verboseMessage);

    oss << saneShortMessage.length() << " " << saneShortMessage;
    oss << saneVerboseMessage.length() << " " << saneVerboseMessage;
    oss << _callStack.size() << " ";

    for (std::list<ErrorLogger::ErrorMessage::FileLocation>::const_iterator loc = _callStack.begin(); loc != _callStack.end(); ++loc) {
        std::ostringstream smallStream;
        smallStream << (*loc).line << ':' << (*loc).getfile() << '\t' << loc->getinfo();
        oss << smallStream.str().length() << " " << smallStream.str();
    }

    return oss.str();
}

bool ErrorLogger::ErrorMessage::deserialize(const std::string &data)
{
    _inconclusive = false;
    _callStack.clear();
    std::istringstream iss(data);
    std::array<std::string, 5> results;
    std::size_t elem = 0;
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

        results[elem++] = temp;
        if (elem == 5)
            break;
    }

    if (elem != 5)
        throw InternalError(0, "Internal Error: Deserialization of error message failed");

    _id = results[0];
    _severity = Severity::fromString(results[1]);
    std::istringstream scwe(results[2]);
    scwe >> _cwe.id;
    _shortMessage = results[3];
    _verboseMessage = results[4];

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
            const char c = static_cast<char>(iss.get());
            temp.append(1, c);
        }

        const std::string::size_type colonPos = temp.find(':');
        if (colonPos == std::string::npos)
            throw InternalError(0, "Internal Error: No colon found in <filename:line> pattern");
        const std::string::size_type tabPos = temp.find('\t');
        if (tabPos == std::string::npos)
            throw InternalError(0, "Internal Error: No tab found in <filename:line> pattern");

        const std::string tempinfo = temp.substr(tabPos + 1);
        temp.erase(tabPos);
        const std::string tempfile = temp.substr(colonPos + 1);
        temp.erase(colonPos);
        const std::string templine = temp;
        ErrorLogger::ErrorMessage::FileLocation loc;
        loc.setfile(tempfile);
        loc.setinfo(tempinfo);
        std::istringstream fiss(templine);
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
    printer.OpenElement("results", false);
    // version 2 header
    if (xml_version == 2) {
        printer.PushAttribute("version", xml_version);
        printer.OpenElement("cppcheck", false);
        printer.PushAttribute("version", CppCheck::version());
        printer.CloseElement(false);
        printer.OpenElement("errors", false);
    }

    return std::string(printer.CStr()) + '>';
}

std::string ErrorLogger::ErrorMessage::getXMLFooter(int xml_version)
{
    return (xml_version<=1) ? "</results>" : "    </errors>\n</results>";
}

// There is no utf-8 support around but the strings should at least be safe for to tinyxml2.
// See #5300 "Invalid encoding in XML output" and  #6431 "Invalid XML created - Invalid encoding of string literal "
std::string ErrorLogger::ErrorMessage::fixInvalidChars(const std::string& raw)
{
    std::string result;
    result.reserve(raw.length());
    std::string::const_iterator from=raw.begin();
    while (from!=raw.end()) {
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

std::string ErrorLogger::ErrorMessage::toXML(bool verbose, int version) const
{
    // The default xml format
    if (version == 1) {
        // No inconclusive messages in the xml version 1
        if (_inconclusive)
            return "";

        tinyxml2::XMLPrinter printer(0, false, 1);
        printer.OpenElement("error", false);
        if (!_callStack.empty()) {
            printer.PushAttribute("file", _callStack.back().getfile().c_str());
            printer.PushAttribute("line", _callStack.back().line);
        }
        printer.PushAttribute("id", _id.c_str());
        printer.PushAttribute("severity", (_severity == Severity::error ? "error" : "style"));
        printer.PushAttribute("msg", fixInvalidChars(verbose ? _verboseMessage : _shortMessage).c_str());
        printer.CloseElement(false);
        return printer.CStr();
    }

    // The xml format you get when you use --xml-version=2
    else if (version == 2) {
        tinyxml2::XMLPrinter printer(0, false, 2);
        printer.OpenElement("error", false);
        printer.PushAttribute("id", _id.c_str());
        printer.PushAttribute("severity", Severity::toString(_severity).c_str());
        printer.PushAttribute("msg", fixInvalidChars(_shortMessage).c_str());
        printer.PushAttribute("verbose", fixInvalidChars(_verboseMessage).c_str());
        if (_cwe.id)
            printer.PushAttribute("cwe", _cwe.id);
        if (_inconclusive)
            printer.PushAttribute("inconclusive", "true");

        for (std::list<FileLocation>::const_reverse_iterator it = _callStack.rbegin(); it != _callStack.rend(); ++it) {
            printer.OpenElement("location", false);
            if (!file0.empty() && (*it).getfile() != file0)
                printer.PushAttribute("file0", Path::toNativeSeparators(file0).c_str());
            printer.PushAttribute("file", (*it).getfile().c_str());
            printer.PushAttribute("line", (*it).line);
            if (!it->getinfo().empty())
                printer.PushAttribute("info", it->getinfo().c_str());
            printer.CloseElement(false);
        }
        printer.CloseElement(false);
        return printer.CStr();
    }

    return "";
}

void ErrorLogger::ErrorMessage::findAndReplace(std::string &source, const std::string &searchFor, const std::string &replaceWith)
{
    std::string::size_type index = 0;
    while ((index = source.find(searchFor, index)) != std::string::npos) {
        source.replace(index, searchFor.length(), replaceWith);
        index += replaceWith.length();
    }
}

std::string ErrorLogger::ErrorMessage::toString(bool verbose, const std::string &outputFormat) const
{
    // Save this ErrorMessage in plain text.

    // No template is given
    if (outputFormat.empty()) {
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

    else if (outputFormat == "daca2") {
        // This is a clang-like output format for daca2
        std::ostringstream text;
        if (_callStack.empty()) {
            text << "nofile:0:0: ";
        } else {
            const ErrorLogger::ErrorMessage::FileLocation &loc = _callStack.back();
            text << loc.getfile() << ':' << loc.line << ':' << loc.col << ": ";
        }

        if (_inconclusive)
            text << "inconclusive ";
        text << Severity::toString(_severity) << ": ";

        text << (verbose ? _verboseMessage : _shortMessage)
             << " [" << _id << ']';

        if (_callStack.size() <= 1U)
            return text.str();

        for (std::list<FileLocation>::const_iterator loc = _callStack.begin(); loc != _callStack.end(); ++loc)
            text << std::endl
                 << loc->getfile()
                 << ':'
                 << loc->line
                 << ':'
                 << loc->col
                 << ": note: "
                 << (loc->getinfo().empty() ? _shortMessage : loc->getinfo());
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
        findAndReplace(result, "{callstack}", _callStack.empty() ? emptyString : callStackToString(_callStack));
        if (!_callStack.empty()) {
            std::ostringstream oss;
            oss << _callStack.back().line;
            findAndReplace(result, "{line}", oss.str());
            findAndReplace(result, "{file}", _callStack.back().getfile());
        } else {
            findAndReplace(result, "{file}", emptyString);
            findAndReplace(result, "{line}", emptyString);
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
                    (i2->line == 0 || i2->line == i->line)) {
                    suppressed = true;
                    break;
                }
            }
        }

        if (suppressed)
            continue;

        const std::list<ErrorLogger::ErrorMessage::FileLocation> callStack = make_container< std::list<ErrorLogger::ErrorMessage::FileLocation> > ()
                << ErrorLogger::ErrorMessage::FileLocation(i->file, i->line);
        reportErr(ErrorLogger::ErrorMessage(callStack, emptyString, Severity::information, "Unmatched suppression: " + i->id, "unmatchedSuppression", false));
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


ErrorLogger::ErrorMessage::FileLocation::FileLocation(const Token* tok, const TokenList* tokenList)
    : fileIndex(tok->fileIndex()), line(tok->linenr()), col(tok->col()), _file(tokenList->file(tok))
{
}

ErrorLogger::ErrorMessage::FileLocation::FileLocation(const Token* tok, const std::string &info, const TokenList* tokenList)
    : fileIndex(tok->fileIndex()), line(tok->linenr()), col(tok->col()), _file(tokenList->file(tok)), _info(info)
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
    _file = Path::simplifyPath(_file);
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

std::string ErrorLogger::toxml(const std::string &str)
{
    std::ostringstream xml;
    const bool isstring(str[0] == '\"');
    for (std::size_t i = 0U; i < str.length(); i++) {
        char c = str[i];
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
        case '\0':
            xml << "\\0";
            break;
        default:
            if (!isstring || (c >= ' ' && c <= 'z'))
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
    for (unsigned int i = 0; i < files.size(); ++i)
        ostr << "  <string>" << ErrorLogger::toxml(files[i]) << "</string>\r\n";
    ostr       << " </array>\r\n"
               << " <key>diagnostics</key>\r\n"
               << " <array>\r\n";
    return ostr.str();
}

static std::string plistLoc(const char indent[], const ErrorLogger::ErrorMessage::FileLocation &loc)
{
    std::ostringstream ostr;
    ostr << indent << "<dict>\r\n"
         << indent << ' ' << "<key>line</key><integer>" << loc.line << "</integer>\r\n"
         << indent << ' ' << "<key>col</key><integer>" << loc.col << "</integer>\r\n"
         << indent << ' ' << "<key>file</key><integer>" << loc.fileIndex << "</integer>\r\n"
         << indent << "</dict>\r\n";
    return ostr.str();
}

std::string ErrorLogger::plistData(const ErrorLogger::ErrorMessage &msg)
{
    std::ostringstream plist;
    plist << "  <dict>\r\n"
          << "   <key>path</key>\r\n"
          << "   <array>\r\n";

    std::list<ErrorLogger::ErrorMessage::FileLocation>::const_iterator prev = msg._callStack.begin();

    for (std::list<ErrorLogger::ErrorMessage::FileLocation>::const_iterator it = msg._callStack.begin(); it != msg._callStack.end(); ++it) {
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

        std::list<ErrorLogger::ErrorMessage::FileLocation>::const_iterator next = it;
        ++next;
        const std::string message = (it->getinfo().empty() && next == msg._callStack.end() ? msg.shortMessage() : it->getinfo());

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
              << "     <key>message</key>\r"
              << "     <string>" << ErrorLogger::toxml(message) << "</string>\r\n"
              << "    </dict>\r\n";
    }

    plist << "   </array>\r\n"
          << "   <key>description</key><string>" << ErrorLogger::toxml(msg.shortMessage()) << "</string>\r\n"
          << "   <key>category</key><string>" << Severity::toString(msg._severity) << "</string>\r\n"
          << "   <key>type</key><string>" << ErrorLogger::toxml(msg.shortMessage()) << "</string>\r\n"
          << "   <key>check_name</key><string>" << msg._id << "</string>\r\n"
          << "   <!-- This hash is experimental and going to change! -->\r\n"
          << "   <key>issue_hash_content_of_line_in_context</key><string>" << 0 << "</string>\r\n"
          << "  <key>issue_context_kind</key><string></string>\r\n"
          << "  <key>issue_context</key><string></string>\r\n"
          << "  <key>issue_hash_function_offset</key><string></string>\r\n"
          << "  <key>location</key>\r\n"
          << plistLoc("  ", msg._callStack.back())
          << "  </dict>\r\n";
    return plist.str();
}

