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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "preprocessor.h"
#include "tokenize.h"
#include "token.h"
#include "filelister.h"

#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <vector>
#include <set>
#include <stack>

Preprocessor::Preprocessor(Settings *settings, ErrorLogger *errorLogger) : _settings(settings), _errorLogger(errorLogger)
{

}

void Preprocessor::writeError(const std::string &fileName, const int linenr, ErrorLogger *errorLogger, const std::string &errorType, const std::string &errorText)
{
    if (!errorLogger)
        return;

    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    ErrorLogger::ErrorMessage::FileLocation loc;
    loc.line = linenr;
    loc.file = fileName;
    locationList.push_back(loc);
    errorLogger->reportErr(ErrorLogger::ErrorMessage(locationList,
                           "error",
                           errorText,
                           errorType));
}

static unsigned char readChar(std::istream &istr)
{
    unsigned char ch = (unsigned char)istr.get();

    // Handling of newlines..
    if (ch == '\r')
    {
        ch = '\n';
        if ((char)istr.peek() == '\n')
            (void)istr.get();
    }

    return ch;
}

/** Just read the code into a string. Perform simple cleanup of the code */
std::string Preprocessor::read(std::istream &istr, const std::string &filename, Settings *settings)
{
    // Get filedata from stream..
    bool ignoreSpace = true;

    // need space.. #if( => #if (
    bool needSpace = false;

    // For the error report
    int lineno = 1;

    // handling <backspace><newline>
    // when this is encountered the <backspace><newline> will be "skipped".
    // on the next <newline>, extra newlines will be added
    unsigned int newlines = 0;

    std::ostringstream code;
    for (unsigned char ch = readChar(istr); istr.good(); ch = readChar(istr))
    {
        if (ch == '\n')
            ++lineno;

        // Replace assorted special chars with spaces..
        if (((ch & 0x80) == 0) && (ch != '\n') && (std::isspace(ch) || std::iscntrl(ch)))
            ch = ' ';

        // Skip spaces after ' ' and after '#'
        if (ch == ' ' && ignoreSpace)
            continue;
        ignoreSpace = bool(ch == ' ' || ch == '#' || ch == '\n');

        if (needSpace)
        {
            if (ch == '(')
                code << " ";
            else if (!std::isalpha(ch))
                needSpace = false;
        }
        if (ch == '#')
            needSpace = true;

        // <backspace><newline>..
        if (ch == '\\')
        {
            unsigned char chNext = 0;
            for (;;)
            {
                chNext = (unsigned char)istr.peek();
                if (chNext != '\n' && chNext != '\r' &&
                    (std::isspace(chNext) || std::iscntrl(chNext)))
                {
                    // Skip whitespace between <backspace> and <newline>
                    (void)readChar(istr);
                    continue;
                }

                break;
            }

            if (chNext == '\n' || chNext == '\r')
            {
                ++newlines;
                (void)readChar(istr);   // Skip the "<backspace><newline>"
            }
            else
                code << "\\";
        }

        // Just some code..
        else
        {
            code << std::string(1, ch);

            // if there has been <backspace><newline> sequences, add extra newlines..
            if (ch == '\n' && newlines > 0)
            {
                code << std::string(newlines, '\n');
                newlines = 0;
            }
        }
    }

    return removeParantheses(removeComments(code.str(), filename, settings));
}

static bool hasbom(const std::string &str)
{
    return bool(str.size() > 3 &&
                static_cast<unsigned char>(str[0]) == 0xef &&
                static_cast<unsigned char>(str[1]) == 0xbb &&
                static_cast<unsigned char>(str[2]) == 0xbf);
}


std::string Preprocessor::removeComments(const std::string &str, const std::string &filename, Settings *settings)
{
    // For the error report
    int lineno = 1;

    // handling <backspace><newline>
    // when this is encountered the <backspace><newline> will be "skipped".
    // on the next <newline>, extra newlines will be added
    unsigned int newlines = 0;
    std::ostringstream code;
    unsigned char previous = 0;
    std::vector<std::string> suppressionIDs;

    for (std::string::size_type i = hasbom(str) ? 3 : 0; i < str.length(); ++i)
    {
        unsigned char ch = str[i];
        if (ch & 0x80)
            throw std::runtime_error("The code contains characters that are unhandled");

        if (str.compare(i, 6, "#error") == 0 || str.compare(i, 8, "#warning") == 0)
        {
            if (str.compare(i, 6, "#error") == 0)
                code << "#error";

            i = str.find("\n", i);
            if (i == std::string::npos)
                break;

            --i;
            continue;
        }

        // We have finished a line that didn't contain any comment
        // (the '\n' is swallowed when a // comment is detected)
        if (ch == '\n' && !suppressionIDs.empty())
        {
            // Add the suppressions.
            for (size_t j(0); j < suppressionIDs.size(); ++j)
                settings->nomsg.addSuppression(suppressionIDs[j], filename, lineno);
            suppressionIDs.clear();
        }

        // Remove comments..
        if (str.compare(i, 2, "//", 0, 2) == 0)
        {
            size_t commentStart = i + 2;
            i = str.find('\n', i);
            if (i == std::string::npos)
                break;

            if (settings && settings->_inlineSuppressions)
            {
                std::string comment(str, commentStart, i - commentStart);
                std::istringstream iss(comment);
                std::string word;
                iss >> word;
                if (word == "cppcheck-suppress")
                {
                    iss >> word;
                    if (iss)
                        suppressionIDs.push_back(word);
                }
            }

            code << "\n";
            previous = '\n';
            ++lineno;
        }
        else if (str.compare(i, 2, "/*", 0, 2) == 0)
        {
            unsigned char chPrev = 0;
            ++i;
            while (i < str.length() && (chPrev != '*' || ch != '/'))
            {
                chPrev = ch;
                ++i;
                ch = str[i];
                if (ch == '\n')
                {
                    ++newlines;
                    ++lineno;
                }
            }
        }

        // String or char constants..
        else if (ch == '\"' || ch == '\'')
        {
            code << std::string(1, ch);
            unsigned char chNext;
            do
            {
                ++i;
                chNext = str[i];
                if (chNext == '\\')
                {
                    ++i;
                    char chSeq = str[i];
                    if (chSeq == '\n')
                        ++newlines;
                    else
                    {
                        code << std::string(1, chNext);
                        code << std::string(1, chSeq);
                        previous = chSeq;
                    }
                }
                else
                {
                    code << std::string(1, chNext);
                    previous = chNext;
                }
            }
            while (i < str.length() && chNext != ch && chNext != '\n');
        }


        // Just some code..
        else
        {
            if (ch == ' ' && previous == ' ')
            {
                // Skip double white space
            }
            else
            {
                code << std::string(1, ch);
                previous = ch;
            }


            // if there has been <backspace><newline> sequences, add extra newlines..
            if (ch == '\n')
            {
                ++lineno;
                if (newlines > 0)
                {
                    code << std::string(newlines, '\n');
                    newlines = 0;
                    previous = '\n';
                }
            }
        }
    }

    return code.str();
}


std::string Preprocessor::removeParantheses(const std::string &str)
{
    if (str.find("\n#if") == std::string::npos && str.compare(0, 3, "#if") != 0)
        return str;

    std::istringstream istr(str.c_str());
    std::ostringstream ret;
    std::string line;
    while (std::getline(istr, line))
    {
        if (line.compare(0, 3, "#if") == 0 || line.compare(0, 5, "#elif") == 0)
        {
            std::string::size_type pos;
            pos = 0;
            while ((pos = line.find(" (", pos)) != std::string::npos)
                line.erase(pos, 1);
            pos = 0;
            while ((pos = line.find("( ", pos)) != std::string::npos)
                line.erase(pos + 1, 1);
            pos = 0;
            while ((pos = line.find(" )", pos)) != std::string::npos)
                line.erase(pos, 1);
            pos = 0;
            while ((pos = line.find(") ", pos)) != std::string::npos)
                line.erase(pos + 1, 1);

            // Remove inner paranthesis "((..))"..
            pos = 0;
            while ((pos = line.find("((", pos)) != std::string::npos)
            {
                ++pos;
                std::string::size_type pos2 = line.find_first_of("()", pos + 1);
                if (pos2 != std::string::npos && line[pos2] == ')')
                {
                    line.erase(pos2, 1);
                    line.erase(pos, 1);
                }
            }

            // "#if(A) => #if A", but avoid "#if (defined A) || defined (B)"
            if (line.compare(0, 4, "#if(") == 0 && line[line.length() - 1] == ')')
            {
                int ind = 0;
                for (std::string::size_type i = 0; i < line.length(); ++i)
                {
                    if (line[i] == '(')
                        ++ind;
                    else if (line[i] == ')')
                    {
                        --ind;
                        if (ind == 0)
                        {
                            if (i == line.length() - 1)
                            {
                                line[3] = ' ';
                                line.erase(line.length() - 1);
                            }
                            break;
                        }
                    }
                }
            }

            if (line.compare(0, 4, "#if(") == 0)
                line.insert(3, " ");
            else if (line.compare(0, 4, "#elif(") == 0)
                line.insert(5, " ");
        }
        ret << line << "\n";
    }

    return ret.str();
}



static void _removeAsm(std::string &str, const std::string::size_type pos)
{
    unsigned int newlines = 0;
    bool instr = false;
    int parlevel = 0;
    std::string::size_type pos2 = pos + 1;
    while (pos2 < str.length())
    {
        if (str[pos2] == '\"')
            instr = !instr;

        else if (str[pos2] == '\n')
            ++newlines;

        else if (!instr)
        {
            if (str[pos2] == '(')
                ++parlevel;
            else if (str[pos2] == ')')
            {
                if (parlevel <= 1)
                    break;
                --parlevel;
            }
        }

        ++pos2;
    }
    str.erase(pos + 1, pos2 - pos);
    str.insert(pos, std::string(newlines, '\n'));
}

void Preprocessor::removeAsm(std::string &str)
{
    std::string::size_type pos = 0;
    while ((pos = str.find("\nasm(", pos)) != std::string::npos)
    {
        _removeAsm(str, pos++);
        str.insert(pos, "asm()");
    }

    pos = 0;
    while ((pos = str.find("\nasm (", pos)) != std::string::npos)
    {
        _removeAsm(str, pos++);
        str.insert(pos, "asm()");
    }

    pos = 0;
    while ((pos = str.find("\nasm __volatile(", pos)) != std::string::npos)
        _removeAsm(str, pos);

    pos = 0;
    while ((pos = str.find("\nasm __volatile (", pos)) != std::string::npos)
        _removeAsm(str, pos);
}


void Preprocessor::preprocess(std::istream &istr, std::map<std::string, std::string> &result, const std::string &filename, const std::list<std::string> &includePaths)
{
    std::list<std::string> configs;
    std::string data;
    preprocess(istr, data, configs, filename, includePaths);
    for (std::list<std::string>::const_iterator it = configs.begin(); it != configs.end(); ++it)
        result[ *it ] = Preprocessor::getcode(data, *it, filename, _errorLogger);
}

std::string Preprocessor::removeSpaceNearNL(const std::string &str)
{
    std::string tmp;
    int prev = -1;
    for (unsigned int i = 0; i < str.size(); i++)
    {
        if (str[i] == ' ' &&
            ((i > 0 && tmp[prev] == '\n') ||
             (i + 1 < str.size() && str[i+1] == '\n')
            )
           )
        {
            // Ignore space that has new line in either side of it
        }
        else
        {
            tmp.append(1, str[i]);
            ++prev;
        }
    }

    return tmp;
}

std::string Preprocessor::replaceIfDefined(const std::string &str)
{
    std::string ret(str);
    std::string::size_type pos;

    pos = 0;
    while ((pos = ret.find("#if defined(", pos)) != std::string::npos)
    {
        std::string::size_type pos2 = ret.find(")", pos + 9);
        if (pos2 > ret.length() - 1)
            break;
        if (ret[pos2+1] == '\n')
        {
            ret.erase(pos2, 1);
            ret.erase(pos + 3, 9);
            ret.insert(pos + 3, "def ");
        }
        ++pos;
    }

    pos = 0;
    while ((pos = ret.find("#if !defined(", pos)) != std::string::npos)
    {
        std::string::size_type pos2 = ret.find(")", pos + 9);
        if (pos2 > ret.length() - 1)
            break;
        if (ret[pos2+1] == '\n')
        {
            ret.erase(pos2, 1);
            ret.erase(pos + 3, 10);
            ret.insert(pos + 3, "ndef ");
        }
        ++pos;
    }

    pos = 0;
    while ((pos = ret.find("#elif defined(", pos)) != std::string::npos)
    {
        std::string::size_type pos2 = ret.find(")", pos + 9);
        if (pos2 > ret.length() - 1)
            break;
        if (ret[pos2+1] == '\n')
        {
            ret.erase(pos2, 1);
            ret.erase(pos + 6, 8);
        }
        ++pos;
    }

    return ret;
}

void Preprocessor::preprocess(std::istream &istr, std::string &processedFile, std::list<std::string> &resultConfigurations, const std::string &filename, const std::list<std::string> &includePaths)
{
    processedFile = read(istr, filename, _settings);

    // Replace all tabs with spaces..
    std::replace(processedFile.begin(), processedFile.end(), '\t', ' ');

    // Remove all indentation..
    if (!processedFile.empty() && processedFile[0] == ' ')
        processedFile.erase(0, processedFile.find_first_not_of(" "));

    // Remove space characters that are after or before new line character
    processedFile = removeSpaceNearNL(processedFile);

    // Remove asm(...)
    removeAsm(processedFile);

    // Replace "defined A" with "defined(A)"
    {
        std::istringstream istr(processedFile.c_str());
        std::ostringstream ostr;
        std::string line;
        while (std::getline(istr, line))
        {
            if (line.compare(0, 4, "#if ") == 0 || line.compare(0, 6, "#elif ") == 0)
            {
                std::string::size_type pos = 0;
                while ((pos = line.find(" defined ")) != std::string::npos)
                {
                    line[pos+8] = '(';
                    pos = line.find_first_of(" |&", pos + 8);
                    if (pos == std::string::npos)
                        line += ")";
                    else
                        line.insert(pos, ")");
                }
            }
            ostr << line << "\n";
        }
        processedFile = ostr.str();
    }

    handleIncludes(processedFile, filename, includePaths);

    processedFile = replaceIfDefined(processedFile);

    // Get all possible configurations..
    resultConfigurations = getcfgs(processedFile, filename);
}



// Get the DEF in this line: "#ifdef DEF"
std::string Preprocessor::getdef(std::string line, bool def)
{
    // If def is true, the line must start with "#ifdef"
    if (def && line.find("#ifdef ") != 0 && line.find("#if ") != 0 && line.find("#elif ") != 0 && line.find("#if defined ") != 0)
    {
        return "";
    }

    // If def is false, the line must start with "#ifndef"
    if (!def && line.find("#ifndef ") != 0)
    {
        return "";
    }

    // Remove the "#ifdef" or "#ifndef"
    if (line.find("#if defined ") == 0)
        line.erase(0, 11);
    else
        line.erase(0, line.find(" "));

    // Remove all spaces.
    std::string::size_type pos = 0;
    while ((pos = line.find(" ", pos)) != std::string::npos)
    {
        const unsigned char chprev = (pos > 0) ? line[pos-1] : (unsigned char)0;
        const unsigned char chnext = (pos + 1 < line.length()) ? line[pos+1] : (unsigned char)0;
        if ((std::isalnum(chprev) || chprev == '_') && (std::isalnum(chnext) || chnext == '_'))
            ++pos;
        else
            line.erase(pos, 1);
    }

    // The remaining string is our result.
    return line;
}



std::list<std::string> Preprocessor::getcfgs(const std::string &filedata, const std::string &filename)
{
    std::list<std::string> ret;
    ret.push_back("");

    std::list<std::string> deflist;

    // constants defined through "#define" in the code..
    std::set<std::string> defines;

    // How deep into included files are we currently parsing?
    // 0=>Source file, 1=>Included by source file, 2=>included by header that was included by source file, etc
    int filelevel = 0;

    bool includeguard = false;

    unsigned int linenr = 0;
    std::istringstream istr(filedata);
    std::string line;
    while (getline(istr, line))
    {
        ++linenr;

        if (line.compare(0, 6, "#file ") == 0)
        {
            includeguard = true;
            ++filelevel;
            continue;
        }

        else if (line == "#endfile")
        {
            includeguard = false;
            if (filelevel > 0)
                --filelevel;
            continue;
        }

        if (line.compare(0, 8, "#define ") == 0 && line.find("(", 8) == std::string::npos)
        {
            if (line.find(" ", 8) == std::string::npos)
                defines.insert(line.substr(8));
            else
            {
                std::string s = line.substr(8);
                s[s.find(" ")] = '=';
                defines.insert(s);
            }
        }

        if (!line.empty() && line.compare(0, 3, "#if") != 0)
            includeguard = false;

        if (includeguard)
            continue;

        std::string def = getdef(line, true) + getdef(line, false);
        if (!def.empty())
        {
            int par = 0;
            for (std::string::size_type pos = 0; pos < def.length(); ++pos)
            {
                if (def[pos] == '(')
                    ++par;
                else if (def[pos] == ')')
                {
                    --par;
                    if (par < 0)
                        break;
                }
            }
            if (par != 0)
            {
                std::ostringstream line;
                line << __LINE__;

                ErrorLogger::ErrorMessage errmsg;
                ErrorLogger::ErrorMessage::FileLocation loc;
                loc.file = filename;
                loc.line = linenr;
                errmsg._callStack.push_back(loc);
                errmsg._severity = "error";
                errmsg._msg = "mismatching number of '(' and ')' in this line: " + def;
                errmsg._id  = "preprocessor" + line.str();
                _errorLogger->reportErr(errmsg);
                ret.clear();
                return ret;
            }

            // Replace defined constants
            {
                std::map<std::string, std::string> varmap;
                for (std::set<std::string>::const_iterator it = defines.begin(); it != defines.end(); ++it)
                {
                    std::string::size_type pos = it->find("=");
                    if (pos == std::string::npos)
                        continue;
                    const std::string varname(it->substr(0, pos));
                    const std::string value(it->substr(pos + 1));
                    varmap[varname] = value;
                }

                simplifyCondition(varmap, def, false);
            }

            if (! deflist.empty() && line.find("#elif ") == 0)
                deflist.pop_back();
            deflist.push_back(def);
            def = "";
            for (std::list<std::string>::const_iterator it = deflist.begin(); it != deflist.end(); ++it)
            {
                if (*it == "0")
                    break;
                if (*it == "1")
                    continue;

                // don't add "T;T":
                // treat two and more similar nested conditions as one
                if (def != *it)
                {
                    if (! def.empty())
                        def += ";";
                    def += *it;
                }
            }
            if (std::find(ret.begin(), ret.end(), def) == ret.end())
                ret.push_back(def);
        }

        else if (line.find("#else") == 0 && ! deflist.empty())
        {
            std::string def((deflist.back() == "1") ? "0" : "1");
            deflist.pop_back();
            deflist.push_back(def);
        }

        else if (line.find("#endif") == 0 && ! deflist.empty())
            deflist.pop_back();
    }

    // Remove defined constants from ifdef configurations..
    for (std::list<std::string>::iterator it = ret.begin(); it != ret.end(); ++it)
    {
        std::string cfg(*it);
        for (std::set<std::string>::const_iterator it2 = defines.begin(); it2 != defines.end(); ++it2)
        {
            std::string::size_type pos = 0;

            // Get name of define
            std::string defineName(*it2);
            if (defineName.find("=") != std::string::npos)
                defineName.erase(defineName.find("="));

            // Remove ifdef configurations that match the defineName
            while ((pos = cfg.find(defineName, pos)) != std::string::npos)
            {
                std::string::size_type pos1 = pos;
                ++pos;
                if (pos1 > 0 && cfg[pos1-1] != ';')
                    continue;
                std::string::size_type pos2 = pos1 + defineName.length();
                if (pos2 < cfg.length() && cfg[pos2] != ';')
                    continue;
                --pos;
                cfg.erase(pos, defineName.length());
            }
        }
        if (cfg.length() != it->length())
        {
            while (cfg.length() > 0 && cfg[0] == ';')
                cfg.erase(0, 1);

            while (cfg.length() > 0 && cfg[cfg.length()-1] == ';')
                cfg.erase(cfg.length() - 1);

            std::string::size_type pos = 0;
            while ((pos = cfg.find(";;", pos)) != std::string::npos)
                cfg.erase(pos, 1);

            *it = cfg;
        }
    }

    // convert configurations: "defined(A) && defined(B)" => "A;B"
    for (std::list<std::string>::iterator it = ret.begin(); it != ret.end(); ++it)
    {
        std::string s(*it);

        if (s.find("&&") != std::string::npos)
        {
            Tokenizer tokenizer(_settings, _errorLogger);
            std::istringstream istr(s.c_str());
            if (!tokenizer.tokenize(istr, filename.c_str()))
            {
                std::ostringstream line;
                line << __LINE__;

                ErrorLogger::ErrorMessage errmsg;
                ErrorLogger::ErrorMessage::FileLocation loc;
                loc.file = filename;
                loc.line = 1;
                errmsg._callStack.push_back(loc);
                errmsg._severity = "error";
                errmsg._msg = "Error parsing this: " + s;
                errmsg._id  = "preprocessor" + line.str();
                _errorLogger->reportErr(errmsg);
            }


            const Token *tok = tokenizer.tokens();
            std::list<std::string> varList;
            while (tok)
            {
                if (Token::Match(tok, "defined ( %var% )"))
                {
                    varList.push_back(tok->strAt(2));
                    tok = tok->tokAt(4);
                    if (tok && tok->str() == "&&")
                    {
                        tok = tok->next();
                    }
                }
                else if (Token::Match(tok, "%var% ;"))
                {
                    varList.push_back(tok->str());
                    tok = tok->tokAt(2);
                }
                else
                {
                    break;
                }
            }

            varList.sort();
            s = "";
            for (std::list<std::string>::iterator varIter = varList.begin(); varIter != varList.end(); ++varIter)
            {
                if (!s.empty())
                    s += ";";

                s += *varIter;
            }

            if (!s.empty())
                *it = s;
        }
    }

    // Remove duplicates from the ret list..
    ret.sort();
    ret.unique();

    // cleanup unhandled configurations..
    for (std::list<std::string>::iterator it = ret.begin(); it != ret.end();)
    {
        const std::string s(*it + ";");

        bool unhandled = false;

        for (std::string::size_type pos = 0; pos < s.length(); ++pos)
        {
            const unsigned char c = s[pos];

            // ok with ";"
            if (c == ';')
                continue;

            // identifier..
            if (std::isalpha(c) || c == '_')
            {
                while (std::isalnum(s[pos]) || s[pos] == '_')
                    ++pos;
                if (s[pos] == '=')
                {
                    ++pos;
                    while (std::isdigit(s[pos]))
                        ++pos;
                    if (s[pos] != ';')
                    {
                        unhandled = true;
                        break;
                    }
                }

                --pos;
                continue;
            }

            // not ok..
            else
            {
                unhandled = true;
                break;
            }
        }

        if (unhandled)
        {
            // unhandled ifdef configuration..
            if (_errorLogger && _settings && _settings->_debug)
                _errorLogger->reportOut("unhandled configuration: " + *it);

            ret.erase(it++);
        }
        else
        {
            ++it;
        }
    }

    return ret;
}


void Preprocessor::simplifyCondition(const std::map<std::string, std::string> &variables, std::string &condition, bool match)
{
    Tokenizer tokenizer;
    std::istringstream istr(("(" + condition + ")").c_str());
    tokenizer.tokenize(istr, "");

    // replace variable names with values..
    for (Token *tok = const_cast<Token *>(tokenizer.tokens()); tok; tok = tok->next())
    {
        if (!tok->isName())
            continue;

        if (Token::Match(tok, "defined ( %var% )"))
        {
            if (variables.find(tok->strAt(2)) != variables.end())
                tok->str("1");
            else if (match)
                tok->str("0");
            else
                continue;
            tok->deleteNext();
            tok->deleteNext();
            tok->deleteNext();
            continue;
        }

        if (Token::Match(tok, "defined %var%"))
        {
            if (variables.find(tok->strAt(1)) != variables.end())
                tok->str("1");
            else if (match)
                tok->str("0");
            else
                continue;
            tok->deleteNext();
            continue;
        }

        const std::map<std::string, std::string>::const_iterator it = variables.find(tok->str());
        if (it != variables.end())
        {
            if (it->second.empty())
                tok->deleteThis();
            else
                tok->str(it->second);
        }
    }

    // simplify calculations..
    bool modified = true;
    while (modified)
    {
        modified = false;
        tokenizer.simplifyCalculations();
        for (Token *tok = const_cast<Token *>(tokenizer.tokens()); tok; tok = tok->next())
        {
            if (Token::Match(tok, "! %num%"))
            {
                tok->deleteThis();
                tok->str(tok->str() == "0" ? "1" : "0");
                modified = true;
            }
        }
    }

    if (Token::simpleMatch(tokenizer.tokens(), "( 1 )") ||
        Token::simpleMatch(tokenizer.tokens(), "( 1 ||"))
        condition = "1";
    else if (Token::simpleMatch(tokenizer.tokens(), "( 0 )"))
        condition = "0";
}

bool Preprocessor::match_cfg_def(const std::map<std::string, std::string> &cfg, std::string def)
{
    //std::cout << "cfg: \"" << cfg << "\"  ";
    //std::cout << "def: \"" << def << "\"";

    simplifyCondition(cfg, def, true);

    if (cfg.find(def) != cfg.end())
        return true;

    if (def == "0")
        return false;

    if (def == "1")
        return true;

    return false;
}


std::string Preprocessor::getcode(const std::string &filedata, std::string cfg, const std::string &filename, ErrorLogger *errorLogger)
{
    std::ostringstream ret;

    bool match = true;
    std::list<bool> matching_ifdef;
    std::list<bool> matched_ifdef;

    // Create a map for the cfg for faster access to defines
    std::map<std::string, std::string> cfgmap;
    {
        std::string::size_type pos = 0;
        for (;;)
        {
            std::string::size_type pos2 = cfg.find_first_of(";=", pos);
            if (pos2 == std::string::npos)
            {
                cfgmap[cfg.substr(pos)] = "";
                break;
            }
            if (cfg[pos2] == ';')
            {
                cfgmap[cfg.substr(pos, pos2-pos)] = "";
            }
            else
            {
                std::string::size_type pos3 = pos2;
                pos2 = cfg.find(";", pos2);
                if (pos2 == std::string::npos)
                {
                    cfgmap[cfg.substr(pos, pos3-pos)] = cfg.substr(pos3 + 1);
                    break;
                }
                else
                {
                    cfgmap[cfg.substr(pos, pos3-pos)] = cfg.substr(pos3 + 1, pos2 - pos3 - 1);
                }
            }
            pos = pos2 + 1;
        }
    }

    std::istringstream istr(filedata);
    std::string line;
    while (getline(istr, line))
    {
        if (line.compare(0, 11, "#pragma asm") == 0)
        {
            ret << "\n";
            bool found_end = false;
            while (getline(istr, line))
            {
                ret << "\n";
                if (line.compare(0, 14, "#pragma endasm") == 0)
                {
                    found_end = true;
                    break;
                }
            }
            if (!found_end)
                break;
            continue;
        }

        std::string def = getdef(line, true);
        std::string ndef = getdef(line, false);

        if (line.compare(0, 8, "#define ") == 0 && line.find("(", 8) == std::string::npos)
        {
            std::string::size_type pos = line.find(" ", 8);
            if (pos == std::string::npos)
                cfgmap[line.substr(8)] = "";
            else
                cfgmap[line.substr(8, pos - 8)] = line.substr(pos + 1);
        }

        else if (line.find("#elif ") == 0)
        {
            if (matched_ifdef.back())
            {
                matching_ifdef.back() = false;
            }
            else
            {
                if (match_cfg_def(cfgmap, def))
                {
                    matching_ifdef.back() = true;
                    matched_ifdef.back() = true;
                }
            }
        }

        else if (! def.empty())
        {
            matching_ifdef.push_back(match_cfg_def(cfgmap, def));
            matched_ifdef.push_back(matching_ifdef.back());
        }

        else if (! ndef.empty())
        {
            matching_ifdef.push_back(! match_cfg_def(cfgmap, ndef));
            matched_ifdef.push_back(matching_ifdef.back());
        }

        else if (line == "#else")
        {
            if (! matched_ifdef.empty())
                matching_ifdef.back() = ! matched_ifdef.back();
        }

        else if (line.compare(0, 6, "#endif") == 0)
        {
            if (! matched_ifdef.empty())
                matched_ifdef.pop_back();
            if (! matching_ifdef.empty())
                matching_ifdef.pop_back();
        }

        if (!line.empty() && line[0] == '#')
        {
            match = true;
            for (std::list<bool>::const_iterator it = matching_ifdef.begin(); it != matching_ifdef.end(); ++it)
                match &= bool(*it);
        }

        // #error => return ""
        if (match && line.compare(0, 6, "#error") == 0)
            return "";

        if (!match && line.compare(0, 8, "#define ") == 0)
        {
            // Remove define that is not part of this configuration
            line = "";
        }
        else if (line.compare(0, 7, "#file \"") == 0 ||
                 line.compare(0, 8, "#endfile") == 0 ||
                 line.compare(0, 8, "#define ") == 0 ||
                 line.compare(0, 6, "#undef") == 0)
        {
            // We must not remove #file tags or line numbers
            // are corrupted. File tags are removed by the tokenizer.
        }
        else if (!match || line.compare(0, 1, "#") == 0)
        {
            // Remove #if, #else, #pragma etc, leaving only
            // #define, #undef, #file and #endfile. and also lines
            // which are not part of this configuration.
            line = "";
        }

        ret << line << "\n";
    }

    return expandMacros(ret.str(), filename, errorLogger);
}

int Preprocessor::getHeaderFileName(std::string &str)
{
    std::string result;
    std::string::size_type i = str.find_first_of("<\"");
    if (i == std::string::npos)
    {
        str = "";
        return 0;
    }

    unsigned char c = str[i];
    if (c == '<')
        c = '>';

    for (i = i + 1; i < str.length(); ++i)
    {
        if (str[i] == c)
            break;

        result.append(1, str[i]);
    }

    str = result;
    if (c == '"')
        return 1;
    else
        return 2;
}


// This wrapper exists because Sun's CC does not allow a static_cast
// from extern "C" int(*)(int) to int(*)(int).
static int tolowerWrapper(int c)
{
    return std::tolower(c);
}


void Preprocessor::handleIncludes(std::string &code, const std::string &filename, const std::list<std::string> &includePaths)
{
    std::list<std::string> paths;
    std::string path;
    path = filename;
    path.erase(1 + path.find_last_of("\\/"));
    paths.push_back(path);
    std::string::size_type pos = 0;
    std::string::size_type endfilePos = 0;
    std::set<std::string> handledFiles;
    endfilePos = pos;
    while ((pos = code.find("#include", pos)) != std::string::npos)
    {
        // Accept only includes that are at the start of a line
        if (pos > 0 && code[pos-1] != '\n')
        {
            pos += 8; // length of "#include"
            continue;
        }

        // If endfile is encountered, we have moved to a next file in our stack,
        // so remove last path in our list.
        while ((endfilePos = code.find("\n#endfile", endfilePos)) != std::string::npos && endfilePos < pos)
        {
            paths.pop_back();
            endfilePos += 9; // size of #endfile
        }

        endfilePos = pos;
        std::string::size_type end = code.find("\n", pos);
        std::string filename = code.substr(pos, end - pos);

        // Remove #include clause
        code.erase(pos, end - pos);

        int headerType = getHeaderFileName(filename);
        if (headerType == 0)
            continue;

        // filename contains now a file name e.g. "menu.h"
        std::string processedFile;
        bool fileOpened = false;
        std::ifstream fin;
        for (std::list<std::string>::const_iterator iter = includePaths.begin(); iter != includePaths.end(); ++iter)
        {
            fin.open((*iter + filename).c_str());
            if (fin.is_open())
            {
                filename = *iter + filename;
                fileOpened = true;
                break;
            }
        }

        if (headerType == 1 && !fileOpened)
        {
            filename = paths.back() + filename;
            fin.open(filename.c_str());
            if (fin.is_open())
            {
                fileOpened = true;
            }
        }

        if (fileOpened)
        {
            std::string tempFile = FileLister::simplifyPath(filename.c_str());
            std::transform(tempFile.begin(), tempFile.end(), tempFile.begin(), tolowerWrapper);
            if (handledFiles.find(tempFile) != handledFiles.end())
            {
                // We have processed this file already once, skip
                // it this time to avoid ethernal loop.
                continue;
            }

            handledFiles.insert(tempFile);
            std::ifstream fin(filename.c_str());
            processedFile = Preprocessor::read(fin, filename, _settings);
            fin.close();
        }

        if (processedFile.length() > 0)
        {
            // Replace all tabs with spaces..
            std::replace(processedFile.begin(), processedFile.end(), '\t', ' ');

            // Remove all indentation..
            if (!processedFile.empty() && processedFile[0] == ' ')
                processedFile.erase(0, processedFile.find_first_not_of(" "));

            // Remove space characters that are after or before new line character
            processedFile = removeSpaceNearNL(processedFile);
            processedFile = "#file \"" + filename + "\"\n" + processedFile + "\n#endfile";
            code.insert(pos, processedFile);

            path = filename;
            path.erase(1 + path.find_last_of("\\/"));
            paths.push_back(path);
        }
        else if (!fileOpened)
        {
            if (headerType == 1 && _errorLogger && _settings && _settings->_verbose)
            {
                _errorLogger->reportOut("Include file: \"" + filename + "\" not found.");
            }
        }
    }
}

/** @brief Class that the preprocessor uses when it expands macros. This class represents a preprocessor macro */
class PreprocessorMacro
{
private:
    /** tokens of this macro */
    Tokenizer tokenizer;

    /** macro parameters */
    std::vector<std::string> _params;

    /** name of macro */
    std::string _name;

    /** macro definition in plain text */
    const std::string _macro;

    /** does this macro take a variable number of parameters? */
    bool _variadic;

    /** prefix that is used by cppcheck to separate macro parameters. Always "__cppcheck__" */
    const std::string _prefix;

    /** The macro has parantheses but no parameters.. "AAA()" */
    bool _nopar;

    /** disabled assignment operator */
    void operator=(const PreprocessorMacro &);
public:
    /**
     * @param macro The code after #define, until end of line,
     * e.g. "A(x) foo(x);"
     */
    PreprocessorMacro(const std::string &macro)
            : _macro(macro), _prefix("__cppcheck__")
    {
        // Tokenize the macro to make it easier to handle
        std::istringstream istr(macro.c_str());
        tokenizer.createTokens(istr);

        // macro name..
        if (tokens() && tokens()->isName())
            _name = tokens()->str();

        // initialize parameters to default values
        _variadic = _nopar = false;

        std::string::size_type pos = macro.find_first_of(" (");
        if (pos != std::string::npos && macro[pos] == '(')
        {
            // Extract macro parameters
            if (Token::Match(tokens(), "%var% ( %var%"))
            {
                for (const Token *tok = tokens()->tokAt(2); tok; tok = tok->next())
                {
                    if (tok->str() == ")")
                        break;
                    if (Token::simpleMatch(tok, ". . . )"))
                    {
                        _variadic = true;
                        break;
                    }
                    if (tok->isName())
                        _params.push_back(tok->str());
                }
            }

            else if (Token::Match(tokens(), "%var% ( . . . )"))
                _variadic = true;

            else if (Token::Match(tokens(), "%var% ( )"))
                _nopar = true;
        }
    }

    /**
     * To avoid name collisions, we will rename macro variables by
     * adding _prefix in front of the name of each variable.
     * Returns the macro with converted names
     * @param result If return value is false, this is not touched. If
     * return value is true, this will contain new macro line
     * (all that comes after #define) e.g.
     * "A(__cppcheck__x) foo(__cppcheck__x);"
     * @param macro The macro which is about to cause name collision.
     * @return true if code needs to be changed, false is no changes
     * are required.
     */
    bool renameMacroVariables(std::string &result, const PreprocessorMacro &macro)
    {
        // No variables
        if (_params.empty())
            return false;

        // Already renamed
        if (_params[0].compare(0, _prefix.length(), _prefix) == 0)
            return false;

        // Check does the macro contain tokens that have
        // the same name as parameters in this macro.
        const Token *tok = macro.tokens();
        if (Token::Match(tok->next(), "("))
        {
            std::map<std::string, bool> paramMap;
            for (unsigned int i = 0; i < _params.size(); ++i)
                paramMap[_params[i]] = true;

            bool collision = false;
            tok = Token::findmatch(tok, ")", 0);
            for (; tok; tok = tok->next())
            {
                if (paramMap.find(tok->str()) != paramMap.end())
                {
                    // Name collision
                    collision = true;
                    break;
                }
            }

            if (!collision)
                return false;
        }

        result = "";
        result.append(_name);
        result.append("(");
        std::vector<std::string> values;
        for (unsigned int i = 0; i < _params.size(); ++i)
        {
            if (i > 0)
                result.append(",");
            values.push_back(_prefix + _params[i]);
            result.append(values.back());
        }

        result.append(") ");
        std::string temp;
        this->code(values, temp);
        result.append(temp);
        return true;
    }

    /** return tokens of this macro */
    const Token *tokens() const
    {
        return tokenizer.tokens();
    }

    /** read parameters of this macro */
    const std::vector<std::string> &params() const
    {
        return _params;
    }

    /** check if this is macro has a variable number of parameters */
    bool variadic() const
    {
        return _variadic;
    }

    /** Check if this macro has parantheses but no parameters */
    bool nopar() const
    {
        return _nopar;
    }

    /** name of macro */
    const std::string &name() const
    {
        return _name;
    }

    /**
     * get expanded code for this macro
     * @param params2 macro parameters
     * @param macrocode output string
     * @return true if the expanding was successful
     */
    bool code(const std::vector<std::string> &params2, std::string &macrocode) const
    {
        if (_nopar)
        {
            macrocode = _macro.substr(1 + _macro.find(")"));
            if (macrocode.empty())
                return true;

            std::string::size_type pos = 0;
            // Remove leading spaces
            if ((pos = macrocode.find_first_not_of(" ")) > 0)
                macrocode.erase(0, pos);
            // Remove ending newline
            if ((pos = macrocode.find_first_of("\r\n")) != std::string::npos)
                macrocode.erase(pos);
        }

        else if (_params.empty() && _variadic)
        {
            std::string s;
            for (unsigned int i = 0; i < params2.size(); ++i)
            {
                if (i > 0)
                    s += ",";
                s += params2[i];
            }

            macrocode = _macro.substr(1 + _macro.find(")"));
            if (macrocode.empty())
                return true;

            std::string::size_type pos = 0;
            // Remove leading spaces
            if ((pos = macrocode.find_first_not_of(" ")) > 0)
                macrocode.erase(0, pos);
            // Remove ending newline
            if ((pos = macrocode.find_first_of("\r\n")) != std::string::npos)
                macrocode.erase(pos);
            // Replace "__VA_ARGS__" with parameters
            pos = 0;
            while ((pos = macrocode.find("__VA_ARGS__", pos)) != std::string::npos)
            {
                macrocode.erase(pos, 11);
                macrocode.insert(pos, s);
                pos += s.length();
            }
        }

        else if (_params.empty())
        {
            std::string::size_type pos = _macro.find(" ");
            if (pos == std::string::npos)
                macrocode = "";
            else
            {
                macrocode = _macro.substr(pos + 1);
                if ((pos = macrocode.find_first_of("\r\n")) != std::string::npos)
                    macrocode.erase(pos);
            }
        }

        else
        {
            const Token *tok = tokens();
            while (tok && tok->str() != ")")
                tok = tok->next();
            if (tok)
            {
                bool optcomma = false;
                while ((tok = tok->next()) != NULL)
                {
                    std::string str = tok->str();
                    if (str == "##")
                        continue;
                    if (str[0] == '#' || tok->isName())
                    {
                        const bool stringify(str[0] == '#');
                        if (stringify)
                        {
                            str = str.erase(0, 1);
                        }
                        for (unsigned int i = 0; i < _params.size(); ++i)
                        {
                            if (str == _params[i])
                            {
                                if (_variadic && i == _params.size() - 1)
                                {
                                    str = "";
                                    for (unsigned int j = (unsigned int)_params.size() - 1; j < params2.size(); ++j)
                                    {
                                        if (optcomma || j > _params.size() - 1)
                                            str += ",";
                                        optcomma = false;
                                        str += params2[j];
                                    }
                                }
                                else if (i >= params2.size())
                                {
                                    // Macro had more parameters than caller used.
                                    macrocode = "";
                                    return false;
                                }
                                else if (stringify)
                                {
                                    const std::string &s(params2[i]);
                                    std::ostringstream ostr;
                                    ostr << "\"";
                                    for (std::string::size_type i = 0; i < s.size(); ++i)
                                    {
                                        if (s[i] == '\\' || s[i] == '\"')
                                            ostr << '\\';
                                        ostr << s[i];
                                    }
                                    str = ostr.str() + "\"";
                                }
                                else
                                    str = params2[i];

                                break;
                            }
                        }
                    }
                    if (_variadic && tok->str() == "," && tok->next() && tok->next()->str() == "##")
                    {
                        optcomma = true;
                        continue;
                    }
                    optcomma = false;
                    macrocode += str;
                    if (Token::Match(tok, "%var% %var%") ||
                        Token::Match(tok, "> >"))
                        macrocode += " ";
                }
            }
        }

        return true;
    }
};


/**
 * Skip string in line. A string begins and ends with either a " or a '
 * @param line the string
 * @param pos in=start position of string, out=end position of string
 */
static void skipstring(const std::string &line, std::string::size_type &pos)
{
    const unsigned char ch = line[pos];

    ++pos;
    while (pos < line.size() && line[pos] != ch)
    {
        if (line[pos] == '\\')
            ++pos;
        ++pos;
    }
}


/**
 * Get data from a input string. This is an extended version of std::getline.
 * The std::getline only get a single line at a time. It can therefore happen that it
 * contains a partial statement. This function ensures that the returned data
 * doesn't end in the middle of a statement. The "getlines" name indicate that
 * this function will return multiple lines if needed.
 * @param istr input stream
 * @param line output data
 * @return success
 */
static bool getlines(std::istream &istr, std::string &line)
{
    if (!istr.good())
        return false;
    line = "";
    int parlevel = 0;
    for (unsigned char ch = (unsigned char)istr.get(); istr.good(); ch = (unsigned char)istr.get())
    {
        if (ch == '\'' || ch == '\"')
        {
            line += ch;
            unsigned char c = 0;
            while (istr.good() && c != ch)
            {
                if (c == '\\')
                {
                    c = (unsigned char)istr.get();
                    if (!istr.good())
                        return true;
                    line += c;
                }

                c = (unsigned char)istr.get();
                if (!istr.good())
                    return true;
                if (c == '\n' && line.compare(0, 1, "#") == 0)
                    return true;
                line += c;
            }
            continue;
        }
        if (ch == '(')
            ++parlevel;
        else if (ch == ')')
            --parlevel;
        else if (ch == '\n')
        {
            if (line.compare(0, 1, "#") == 0)
                return true;

            if ((char)istr.peek() == '#')
            {
                line += ch;
                return true;
            }
        }
        else if (line.compare(0, 1, "#") != 0 && parlevel <= 0 && ch == ';')
        {
            line += ";";
            return true;
        }

        line += ch;
    }
    return true;
}

std::string Preprocessor::expandMacros(const std::string &code, std::string filename, ErrorLogger *errorLogger)
{
    // Search for macros and expand them..
    // --------------------------------------------

    // Available macros (key=macroname, value=macro).
    std::map<std::string, PreprocessorMacro *> macros;

    // Current line number
    unsigned int linenr = 1;

    // linenr, filename
    std::stack< std::pair<unsigned int, std::string> > fileinfo;

    // output stream
    std::ostringstream ostr;

    // read code..
    std::istringstream istr(code.c_str());
    std::string line;
    while (getlines(istr, line))
    {
        // defining a macro..
        if (line.compare(0, 8, "#define ") == 0)
        {
            PreprocessorMacro *macro = new PreprocessorMacro(line.substr(8));
            if (macro->name().empty())
                delete macro;
            else
            {
                std::map<std::string, PreprocessorMacro *>::iterator it;
                it = macros.find(macro->name());
                if (it != macros.end())
                    delete it->second;
                macros[macro->name()] = macro;
            }
            line = "\n";
        }

        // undefining a macro..
        else if (line.compare(0, 7, "#undef ") == 0)
        {
            std::map<std::string, PreprocessorMacro *>::iterator it;
            it = macros.find(line.substr(7));
            if (it != macros.end())
            {
                delete it->second;
                macros.erase(it);
            }
            line = "\n";
        }

        // entering a file, update position..
        else if (line.compare(0, 7, "#file \"") == 0)
        {
            fileinfo.push(std::pair<unsigned int, std::string>(linenr, filename));
            filename = line.substr(7, line.length() - 8);
            linenr = 0;
            line += "\n";
        }

        // leaving a file, update position..
        else if (line == "#endfile")
        {
            if (!fileinfo.empty())
            {
                linenr = fileinfo.top().first;
                filename = fileinfo.top().second;
                fileinfo.pop();
            }
            line += "\n";
        }

        // all other preprocessor directives are just replaced with a newline
        else if (line.compare(0, 1, "#") == 0)
        {
            line += "\n";
        }

        // expand macros..
        else
        {
            // Limit for each macro.
            // The limit specify a position in the "line" variable.
            // For a "recursive macro" where the expanded text contains
            // the macro again, the macro should not be expanded again.
            // The limits are used to prevent recursive expanding.
            // * When a macro is expanded its limit position is set to
            //   the last expanded character.
            // * macros are only allowed to be expanded when the
            //   the position is beyond the limit.
            // * The limit is relative to the end of the "line"
            //   variable. Inserting and deleting text before the limit
            //   without updating the limit is safe.
            // * when pos goes beyond a limit the limit needs to be
            //   deleted because it is unsafe to insert/delete text
            //   after the limit otherwise
            std::map<const PreprocessorMacro *, unsigned int> limits;

            // pos is the current position in line
            std::string::size_type pos = 0;

            // scan line to see if there are any macros to expand..
            while (pos < line.size())
            {
                if (line[pos] == '\n')
                    ++linenr;

                // skip strings..
                if (line[pos] == '\"' || line[pos] == '\'')
                {
                    const char ch = line[pos];

                    skipstring(line, pos);
                    ++pos;

                    if (pos >= line.size())
                    {
                        writeError(filename,
                                   linenr,
                                   errorLogger,
                                   "noQuoteCharPair",
                                   std::string("No pair for character (") + ch + "). Can't process file. File is either invalid or unicode, which is currently not supported.");

                        std::map<std::string, PreprocessorMacro *>::iterator it;
                        for (it = macros.begin(); it != macros.end(); ++it)
                            delete it->second;

                        return "";
                    }

                    continue;
                }

                if (!std::isalpha(line[pos]) && line[pos] != '_')
                    ++pos;

                // found an identifier..
                // the "while" is used in case the expanded macro will immediately call another macro
                while (pos < line.length() && (std::isalpha(line[pos]) || line[pos] == '_'))
                {
                    // pos1 = start position of macro
                    const std::string::size_type pos1 = pos++;

                    // find the end of the identifier
                    while (pos < line.size() && (std::isalnum(line[pos]) || line[pos] == '_'))
                        ++pos;

                    // get identifier
                    const std::string id = line.substr(pos1, pos - pos1);

                    // is there a macro with this name?
                    std::map<std::string, PreprocessorMacro *>::const_iterator it;
                    it = macros.find(id);
                    if (it == macros.end())
                        break;  // no macro with this name exist

                    const PreprocessorMacro * const macro = it->second;

                    // check that pos is within allowed limits for this
                    // macro
                    {
                        const std::map<const PreprocessorMacro *, unsigned int>::const_iterator it2 = limits.find(macro);
                        if (it2 != limits.end() && pos <= line.length() - it2->second)
                            break;
                    }

                    // get parameters from line..
                    std::vector<std::string> params;
                    std::string::size_type pos2 = pos;
                    if (macro->params().size() && pos2 >= line.length())
                        break;

                    // number of newlines within macro use
                    unsigned int numberOfNewlines = 0;

                    // if the macro has parantheses, get parameters
                    if (macro->variadic() || macro->nopar() || macro->params().size())
                    {
                        if (line[pos2] == ' ')
                            pos2++;

                        if (line[pos2] != '(')
                            break;

                        // parantheses level
                        int parlevel = 0;

                        // current parameter
                        std::string par;

                        // is the end paranthesis found?
                        bool endFound = false;

                        // scan for parameters..
                        for (; pos2 < line.length(); ++pos2)
                        {
                            // increase paranthesis level
                            if (line[pos2] == '(')
                            {
                                ++parlevel;
                                if (parlevel == 1)
                                    continue;
                            }

                            // decrease paranthesis level
                            else if (line[pos2] == ')')
                            {
                                --parlevel;
                                if (parlevel <= 0)
                                {
                                    endFound = true;
                                    params.push_back(par);
                                    break;
                                }
                            }

                            // string
                            else if (line[pos2] == '\"' || line[pos2] == '\'')
                            {
                                const std::string::size_type p = pos2;
                                skipstring(line, pos2);
                                if (pos2 == line.length())
                                    break;
                                par += line.substr(p, pos2 + 1 - p);
                                continue;
                            }

                            // count newlines. the expanded macro must have the same number of newlines
                            else if (line[pos2] == '\n')
                            {
                                ++numberOfNewlines;
                                continue;
                            }

                            // new parameter
                            if (parlevel == 1 && line[pos2] == ',')
                            {
                                params.push_back(par);
                                par = "";
                            }

                            // spaces are only added if needed
                            else if (line[pos2] == ' ')
                            {
                                // Add space only if it is needed
                                if (par.size() && std::isalnum(par[par.length()-1]))
                                {
                                    par += ' ';
                                }
                            }

                            // add character to current parameter
                            else if (parlevel >= 1)
                            {
                                par.append(1, line[pos2]);
                            }
                        }

                        // something went wrong so bail out
                        if (!endFound)
                            break;
                    }

                    // Just an empty parameter => clear
                    if (params.size() == 1 && params[0] == "")
                        params.clear();

                    // Check that it's the same number of parameters..
                    if (!macro->variadic() && params.size() != macro->params().size())
                        break;

                    // Create macro code..
                    std::string tempMacro;
                    if (!macro->code(params, tempMacro))
                    {
                        // Syntax error in code
                        writeError(filename,
                                   linenr,
                                   errorLogger,
                                   "syntaxError",
                                   std::string("Syntax error. Not enough parameters for macro '") + macro->name() + "'.");

                        std::map<std::string, PreprocessorMacro *>::iterator it;
                        for (it = macros.begin(); it != macros.end(); ++it)
                            delete it->second;

                        return "";
                    }

                    // make sure number of newlines remain the same..
                    const std::string macrocode(std::string(numberOfNewlines, '\n') + tempMacro);

                    // Insert macro code..
                    if (macro->variadic() || macro->nopar() || !macro->params().empty())
                        ++pos2;

                    // Remove old limits
                    for (std::map<const PreprocessorMacro *, unsigned int>::iterator iter = limits.begin();
                         iter != limits.end();)
                    {
                        if ((line.length() - pos1) < iter->second)
                        {
                            // We have gone past this limit, so just delete it
                            limits.erase(iter++);
                        }
                        else
                        {
                            ++iter;
                        }
                    }

                    // don't allow this macro to be expanded again before pos2
                    limits[macro] = line.length() - pos2;

                    // erase macro
                    line.erase(pos1, pos2 - pos1);

                    // insert expanded macro code
                    line.insert(pos1, macrocode);

                    // position = start position.
                    pos = pos1;
                }
            }
        }

        // the line has been processed in various ways. Now add it to the output stream
        ostr << line;

        // update linenr
        for (std::string::size_type p = 0; p < line.length(); ++p)
        {
            if (line[p] == '\n')
                ++linenr;
        }
    }

    {
        std::map<std::string, PreprocessorMacro *>::iterator it;
        for (it = macros.begin(); it != macros.end(); ++it)
            delete it->second;
    }

    return ostr.str();
}

