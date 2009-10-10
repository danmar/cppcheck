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
#include <iostream>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <vector>
#include <set>

Preprocessor::Preprocessor(const Settings *settings, ErrorLogger *errorLogger) : _settings(settings), _errorLogger(errorLogger)
{

}

void Preprocessor::writeError(const std::string &fileName, const std::string &code, size_t endPos, ErrorLogger *errorLogger, const std::string &errorType, const std::string &errorText)
{
    if (!errorLogger)
    {
        return;
    }

    // line number in parsed code
    unsigned int lineno = 1;

    // The current token being parsed
    std::string CurrentToken;

    // lineNumbers holds line numbers for files in fileIndexes
    // every time an include file is complitely parsed, last item in the vector
    // is removed and lineno is set to point to that value.
    std::vector<unsigned int> lineNumbers;

    // fileIndexes holds index for _files vector about currently parsed files
    // every time an include file is complitely parsed, last item in the vector
    // is removed and FileIndex is set to point to that value.
    std::vector<std::string> fileIndexes;

    // FileIndex. What file in the _files vector is read now?
    std::string FileIndex = fileName;

    // Read one byte at a time from code and create tokens
    if (endPos > code.length())
        endPos = code.length();

    for (size_t codePos = 0; codePos < endPos; ++codePos)
    {

        char ch = code[codePos];

        // UTF / extended ASCII => The output from the preprocessor should only be standard ASCII
        if (ch < 0)
        {
            /**
             * @todo handle utf better:
             * - remove characters that are redundant
             * - convert needed characters to standard ASCII
             */

            // Not sure how to handle this character. Bailing out.
            if (ch < 0)
                continue;
        }

        // char/string..
        if (ch == '\'' || ch == '\"')
        {
            std::string line;

            // read char
            bool special = false;
            char c = ch;
            do
            {
                // Append token..
                line += c;

                if (c == '\n')
                    ++lineno;

                // Special sequence '\.'
                if (special)
                    special = false;
                else
                    special = (c == '\\');

                // Get next character
                ++codePos;
                c = code[codePos];
            }
            while (codePos < endPos && (special || c != ch));
            line += ch;

            // Handle #file "file.h"
            if (CurrentToken == "#file")
            {
                // Extract the filename
                line = line.substr(1, line.length() - 2);

                // Has this file been tokenized already?
                ++lineno;
                fileIndexes.push_back(FileIndex);
                FileIndex = FileLister::simplifyPath(line.c_str());
                lineNumbers.push_back(lineno);
                lineno = 0;
            }

            CurrentToken.clear();

            continue;
        }

        if (strchr("+-*/%&|^?!=<>[](){};:,.~\n ", ch))
        {
            if (ch == '.' &&
                CurrentToken.length() > 0 &&
                std::isdigit(CurrentToken[0]))
            {
                // Don't separate doubles "5.4"
            }
            else if (strchr("+-", ch) &&
                     CurrentToken.length() > 0 &&
                     std::isdigit(CurrentToken[0]) &&
                     CurrentToken[CurrentToken.length()-1] == 'e')
            {
                // Don't separate doubles "4.2e+10"
            }
            else
            {
                if (CurrentToken == "#file")
                {
                    // Handle this where strings are handled
                    continue;
                }
                else if (CurrentToken == "#endfile")
                {
                    if (lineNumbers.empty() || fileIndexes.empty())
                    {
                        std::cerr << "####### Preprocessor bug! #######\n";
                        std::exit(0);
                    }

                    lineno = lineNumbers.back();
                    lineNumbers.pop_back();
                    FileIndex = fileIndexes.back();
                    fileIndexes.pop_back();
                    CurrentToken.clear();
                    continue;
                }

                // If token contains # characters, split it up
                std::string temp;
                for (std::string::size_type i = 0; i < CurrentToken.length(); ++i)
                {
                    if (CurrentToken[i] == '#' && CurrentToken.length() + 1 > i && CurrentToken[i+1] == '#')
                    {
                        temp.clear();
                        ++i;
                    }
                    else
                        temp += CurrentToken[i];
                }

                CurrentToken.clear();

                if (ch == '\n')
                {
                    ++lineno;
                    continue;
                }
                else if (ch == ' ')
                {
                    continue;
                }

                CurrentToken += ch;
                // Add "++", "--" or ">>" token
                if ((ch == '+' || ch == '-' || ch == '>') && (code[codePos+1] == ch))
                {
                    ++codePos;
                    CurrentToken += code[codePos];
                }
                CurrentToken.clear();
                continue;
            }
        }

        CurrentToken += ch;
    }


    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    ErrorLogger::ErrorMessage::FileLocation loc;
    loc.line = lineno;
    loc.file = FileIndex;
    locationList.push_back(loc);
    errorLogger->reportErr(
        ErrorLogger::ErrorMessage(locationList,
                                  "error",
                                  errorText,
                                  errorType));
}

static char readChar(std::istream &istr)
{
    char ch = (char)istr.get();

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
std::string Preprocessor::read(std::istream &istr)
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
    for (char ch = readChar(istr); istr.good(); ch = readChar(istr))
    {
        if (ch == '\n')
            ++lineno;

        // Replace assorted special chars with spaces..
        if ((ch > 0) && (ch != '\n') && (std::isspace(ch) || std::iscntrl(ch)))
            ch = ' ';

        // Skip spaces after ' ' and after '#'
        if (ch == ' ' && ignoreSpace)
            continue;
        ignoreSpace = bool(ch == ' ' || ch == '#' || ch == '\n');

        if (needSpace)
        {
            if (ch == '(')
                code << " ";
            else if ((ch > 0) && ! std::isalpha(ch))
                needSpace = false;
        }
        if (ch == '#')
            needSpace = true;

        // <backspace><newline>..
        if (ch == '\\')
        {
            char chNext = 0;
            for (;;)
            {
                chNext = (char)istr.peek();
                if (chNext != '\n' && chNext != '\r' && (chNext > 0) &&
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

    return removeParantheses(removeComments(code.str()));
}

static bool hasbom(const std::string &str)
{
    return bool(str.size() > 3 &&
                (char)str[0] == (char)0xef &&
                (char)str[1] == (char)0xbb &&
                (char)str[2] == (char)0xbf);
}


std::string Preprocessor::removeComments(const std::string &str)
{
    // For the error report
    int lineno = 1;

    // handling <backspace><newline>
    // when this is encountered the <backspace><newline> will be "skipped".
    // on the next <newline>, extra newlines will be added
    unsigned int newlines = 0;
    std::ostringstream code;
    char previous = 0;
    for (std::string::size_type i = hasbom(str) ? 3 : 0; i < str.length(); ++i)
    {
        char ch = str[i];
        if (ch < 0)
            throw std::runtime_error("The code contains characters that are unhandled");

        // Remove comments..
        if (str.compare(i, 2, "//", 0, 2) == 0)
        {
            i = str.find('\n', i);
            if (i == std::string::npos)
                break;

            code << "\n";
            previous = '\n';
            ++lineno;
        }
        else if (str.compare(i, 2, "/*", 0, 2) == 0)
        {
            char chPrev = 0;
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
            char chNext;
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
            if (ch == '\n' && newlines > 0)
            {
                code << std::string(newlines, '\n');
                newlines = 0;
                previous = '\n';
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
        _removeAsm(str, pos);

    pos = 0;
    while ((pos = str.find("\nasm (", pos)) != std::string::npos)
        _removeAsm(str, pos);

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
    processedFile = read(istr);

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
    resultConfigurations = getcfgs(processedFile);
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
    while (line.find(" ") != std::string::npos)
        line.erase(line.find(" "), 1);

    // The remaining string is our result.
    return line;
}



std::list<std::string> Preprocessor::getcfgs(const std::string &filedata)
{
    std::list<std::string> ret;
    ret.push_back("");

    std::list<std::string> deflist;

    // constants defined through "#define" in the code..
    std::set<std::string> defines;

    // How deep into included files are we currently parsing?
    // 0=>Source file, 1=>Included by source file, 2=>included by header that was included by source file, etc
    int filelevel = 0;

    std::istringstream istr(filedata);
    std::string line;
    while (getline(istr, line))
    {
        if (line.compare(0, 6, "#file ") == 0)
        {
            ++filelevel;
            continue;
        }

        else if (line == "#endfile")
        {
            if (filelevel > 0)
                --filelevel;
            continue;
        }

        else if (line.compare(0, 8, "#define ") == 0 && line.find("(", 8) == std::string::npos)
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

        if (filelevel > 0)
            continue;

        std::string def = getdef(line, true) + getdef(line, false);
        if (!def.empty())
        {
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
            Tokenizer tokenizer;
            std::istringstream istr(s.c_str());
            tokenizer.tokenize(istr, "");


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
        const std::string &s(*it);
        if (s.find("&&") != std::string::npos || s.find("||") != std::string::npos)
        {
            // unhandled ifdef configuration..
            if (_errorLogger && _settings && _settings->_debug)
                _errorLogger->reportOut("unhandled configuration: " + s);

            ret.erase(it++);
        }
        else
            ++it;
    }

    return ret;
}



bool Preprocessor::match_cfg_def(const std::map<std::string, std::string> &cfg, std::string def)
{
    //std::cout << "cfg: \"" << cfg << "\"  ";
    //std::cout << "def: \"" << def << "\"";

    if (cfg.find(def) != cfg.end())
        return true;

    for (std::string::size_type pos = def.find("defined("); pos != std::string::npos; pos = def.find("defined(", pos + 1))
    {
        // The character before "defined" must not be '_' or alphanumeric
        unsigned char chPrev = (pos > 0) ? def[pos-1] : ' ';
        if (chPrev == '_' || std::isalnum(chPrev))
            continue;

        // Extract the parameter..
        std::string::size_type pos2 = def.find(")", pos);
        if (pos2 == std::string::npos)
            continue;

        std::string::size_type pos1 = pos + 8;
        const std::string par(def.substr(pos1, pos2 - pos1));
        const bool isdefined(cfg.find(par) != cfg.end());

        def.erase(pos, pos2 + 1 - pos);
        def.insert(pos, isdefined ? "1" : "0");
    }

    if (def.find("1||") != std::string::npos || def.find("||1") != std::string::npos)
        return true;

    while (def.find("1&&") != std::string::npos)
    {
        def.erase(def.find("1&&"), 3);
    }

    //std::cout << " => \"" << def << "\"" << std::endl;

    if (def == "0")
        return false;

    if (def == "1")
        return true;

    /*
        if (cfg.empty())
            return false;

        // remove the define values
        while (cfg.find("=") != std::string::npos)
        {
            std::string::size_type pos1 = cfg.find("=");
            std::string::size_type pos2 = cfg.find(";", pos1);
            if (pos2 == std::string::npos)
                cfg.erase(pos1);
            else
                cfg.erase(pos1, pos2 - pos1);
        }

        while (! cfg.empty())
        {
            if (cfg.find(";") == std::string::npos)
                return bool(cfg == def);
            std::string _cfg = cfg.substr(0, cfg.find(";"));
            if (_cfg == def)
                return true;
            cfg.erase(0, cfg.find(";") + 1);
        }
    */

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


        if (!match && line.find("#define") == 0)
        {
            // Remove define that is not part of this configuration
            line = "";
        }
        else if (line.find("#file \"") == 0 ||
                 line.find("#endfile") == 0 ||
                 line.find("#define") == 0 ||
                 line.find("#undef") == 0)
        {
            // We must not remove #file tags or line numbers
            // are corrupted. File tags are removed by the tokenizer.
        }
        else if (!match ||
                 line[0] == '#')
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

    char c = str[i];
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

void Preprocessor::handleIncludes(std::string &code, const std::string &filename, const std::list<std::string> &includePaths)
{
    std::list<std::string> paths;
    std::string path;
    path = filename;
    path.erase(1 + path.find_last_of("\\/"));
    paths.push_back(path);
    std::string::size_type pos = 0;
    std::string::size_type endfilePos = 0;
    std::map<std::string, bool> handledFiles;
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
        while ((endfilePos = code.find("#endfile", endfilePos)) != std::string::npos && endfilePos < pos)
        {
            paths.pop_back();
            endfilePos += 8; // size of #endfile
        }

        endfilePos = pos;
        std::string::size_type end = code.find("\n", pos);
        std::string filename = code.substr(pos, end - pos);

        // Remove #include clause
        code.erase(pos, end - pos);

        int headerType = getHeaderFileName(filename);
        if (headerType == 0)
            continue;

        std::string tempFile = filename;
        std::transform(tempFile.begin(), tempFile.end(), tempFile.begin(), static_cast < int(*)(int) > (std::tolower));
        if (handledFiles.find(tempFile) != handledFiles.end())
        {
            // We have processed this file already once, skip
            // it this time to avoid ethernal loop.
            continue;
        }

        handledFiles[ tempFile ] = true;

        // filename contains now a file name e.g. "menu.h"
        std::string processedFile;
        for (std::list<std::string>::const_iterator iter = includePaths.begin(); iter != includePaths.end(); ++iter)
        {
            std::ifstream fin;
            fin.open((*iter + filename).c_str());
            if (fin.is_open())
            {
                filename = *iter + filename;
                processedFile = Preprocessor::read(fin);
                break;
            }
        }

        if (headerType == 1 && processedFile.length() == 0)
        {
            filename = paths.back() + filename;
            std::ifstream fin(filename.c_str());
            if (fin.is_open())
                processedFile = Preprocessor::read(fin);
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
        else
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
    Tokenizer tokenizer;
    std::vector<std::string> _params;
    std::string _name;
    std::string _macro;
    bool _variadic;
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
        if (_params.size() == 0)
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

    const Token *tokens() const
    {
        return tokenizer.tokens();
    }

    const std::vector<std::string> params() const
    {
        return _params;
    }

    bool variadic() const
    {
        return _variadic;
    }

    bool nopar() const
    {
        return _nopar;
    }

    const std::string &name() const
    {
        return _name;
    }

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
                        bool stringify = false;
                        if (str[0] == '#')
                        {
                            str = str.erase(0, 1);
                            stringify = true;
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
                                    str = "\"" + params2[i] + "\"";
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
                    if (Token::Match(tok, "%type% %var%"))
                        macrocode += " ";
                }
            }
        }

        return true;
    }
};

std::string Preprocessor::expandMacros(std::string code, const std::string &filename, ErrorLogger *errorLogger)
{
    // Search for macros and expand them..
    std::string::size_type defpos = 0;
    while ((defpos = code.find("#define ", defpos)) != std::string::npos)
    {
        if (defpos > 0 && code[defpos-1] != '\n')
        {
            defpos++;
            continue;
        }

        // Get macro..
        std::string::size_type endpos = code.find("\n", defpos + 6);
        if (endpos == std::string::npos)
        {
            code.erase(defpos);
            break;
        }

        // Extract the whole macro into a separate variable "macro" and then erase it from "code"
        const PreprocessorMacro macro(code.substr(defpos + 8, endpos - defpos - 7));
        code.erase(defpos, endpos - defpos);

        // No macro name => continue
        if (macro.name() == "")
            continue;

        // Expand all macros in the code..
        char pattern[5] = "\"'# ";
        pattern[3] = macro.name().at(0);
        std::string::size_type pos1 = defpos;
        while ((pos1 = code.find_first_of(pattern, pos1 + 1)) != std::string::npos)
        {
            char ch = code[pos1];

            // #undef => break
            if (code[pos1] == '#')
            {
                // Are we at a #undef or #define?
                if (code.compare(pos1, 7, "#undef ") == 0)
                    pos1 += 7;
                else if (code.compare(pos1, 8, "#define ") == 0)
                    pos1 += 8;
                else
                    continue;

                // Compare the macroname with the macroname we're currently parsing (macro.name())
                // If it's the same macroname.. break.
                std::string::size_type pos = pos1 + macro.name().length();
                if (pos < code.length()
                    && code.compare(pos1, macro.name().length(), macro.name()) == 0
                    && !std::isalnum(code[pos]) && code[pos] != '_')
                    break;


                continue;
            }

            // String or char..
            if (code[pos1] == '\"' || code[pos1] == '\'')
            {
                // Find the end of the string/char..
                ++pos1;
                while (pos1 < code.size() && code[pos1] != ch && code[pos1] != '\n')
                {
                    if (code[pos1] == '\\')
                        ++pos1;
                    ++pos1;
                }

                // End of line/file was reached without finding pair
                if (pos1 >= code.size() || code[pos1] == '\n')
                {
                    std::string::size_type lineStart = code.rfind('\n', pos1 - 1);
                    if (lineStart != std::string::npos)
                    {
                        if (code.compare(lineStart + 1, 7, "#define") == 0)
                        {
                            // There is nothing wrong #define containing quote without
                            // a pair.
                            continue;
                        }
                    }

                    writeError(filename,
                               code,
                               pos1,
                               errorLogger,
                               "noQuoteCharPair",
                               std::string("No pair for character (") + ch + "). Can't process file. File is either invalid or unicode, which is currently not supported.");

                    return "";
                }

                continue;
            }

            // Matching the macroname?
            if (code.compare(pos1, macro.name().length(), macro.name()) != 0)
                continue;

            // Previous char must not be alphanumeric nor '_'
            if (pos1 != 0 && (std::isalnum(code[pos1-1]) || code[pos1-1] == '_'))
                continue;

            // The char after the macroname must not be alphanumeric nor '_'
            if (pos1 + macro.name().length() < code.length())
            {
                std::string::size_type pos2 = pos1 + macro.name().length();
                if (std::isalnum(code[pos2]) || code[pos2] == '_')
                    continue;
            }

            std::vector<std::string> params;
            std::string::size_type pos2 = pos1 + macro.name().length();
            if (macro.params().size() && pos2 >= code.length())
                continue;

            // Check are we in #define
            std::string::size_type startOfLine = code.rfind("\n", pos1);
            ++startOfLine;

            if (code.compare(startOfLine, 8, "#define ") == 0)
            {
                // We are inside a define, make sure we don't have name collision
                // by e.g. replacing the following code:
                // #define B(a) A(a)
                // With this:
                // #define B(2a) A(2a)
                std::string::size_type endOfLine = code.find("\n", pos1);
                startOfLine += 8;

                PreprocessorMacro tempMacro(code.substr(startOfLine, endOfLine - startOfLine));
                std::string tempMacroCode;
                if (tempMacro.renameMacroVariables(tempMacroCode, macro))
                {
                    // Change the macro and then start again from the start
                    // of the line, as code has changed.
                    code.erase(startOfLine, endOfLine - startOfLine);
                    code.insert(startOfLine, tempMacroCode);
                    pos1 = startOfLine;
                    continue;
                }
            }

            unsigned int numberOfNewlines = 0;

            if (macro.variadic() || macro.nopar() || macro.params().size())
            {
                if (code[pos2] == ' ')
                    pos2++;

                if (code[pos2] != '(')
                    continue;

                int parlevel = 0;
                std::string par;
                for (; pos2 < code.length(); ++pos2)
                {
                    if (code[pos2] == '(')
                    {
                        ++parlevel;
                        if (parlevel == 1)
                            continue;
                    }
                    else if (code[pos2] == ')')
                    {
                        --parlevel;
                        if (parlevel <= 0)
                        {
                            params.push_back(par);
                            break;
                        }
                    }
                    else if (code[pos2] == '\"' || code[pos2] == '\'')
                    {
                        par += code[pos2];
                        char ch = code[pos2];
                        ++pos2;
                        while (pos2 < code.length() && code[pos2] != ch)
                        {
                            if (code[pos2] == '\\')
                            {
                                par += code[pos2];
                                ++pos2;
                            }

                            par += code[pos2];
                            ++pos2;
                        }
                        if (pos2 == code.length())
                            break;
                        par += code[pos2];
                        continue;
                    }
                    else if (code[pos2] == '\n')
                    {
                        ++numberOfNewlines;
                        continue;
                    }

                    if (parlevel == 1 && code[pos2] == ',')
                    {
                        params.push_back(par);
                        par = "";
                    }
                    else if (code[pos2] == ' ')
                    {
                        // Add space only if it is needed
                        if (par.size() && std::isalnum(par[par.length()-1]))
                        {
                            par += ' ';
                        }
                    }
                    else if (parlevel >= 1)
                    {
                        par.append(1, code[pos2]);
                    }
                }
            }

            if (params.size() == 1 && params[0] == "")
                params.clear();

            // Same number of parameters..
            if (!macro.variadic() && params.size() != macro.params().size())
                continue;

            // Create macro code..
            std::string tempMacro;
            if (!macro.code(params, tempMacro))
            {
                // Syntax error in code


                writeError(filename,
                           code,
                           pos1,
                           errorLogger,
                           "syntaxError",
                           std::string("Syntax error. Not enough parameters for macro '") + macro.name() + "'.");


                return "";
            }

            const std::string macrocode(std::string(numberOfNewlines, '\n') + tempMacro);

            // Insert macro code..
            if (macro.variadic() || macro.nopar() || !macro.params().empty())
                ++pos2;

            code.erase(pos1, pos2 - pos1);
            code.insert(pos1, macrocode);
            pos1 += macrocode.length() - 1;
        }
    }

    // Remove all #undef..
    defpos = 0;
    while ((defpos = code.find("\n#undef ", defpos)) != std::string::npos)
    {
        ++defpos;
        std::string::size_type pos2 = code.find("\n", defpos);
        code.erase(defpos, pos2 - defpos);
    }

    return code;
}

