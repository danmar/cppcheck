/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis, Vesa Pikki
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

        // We are not handling UTF and stuff like that. Code is supposed to plain simple text.
        if (ch < 0)
            continue;

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

Preprocessor::Preprocessor()
{

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
        if ((ch != '\n') && (std::isspace(ch) || std::iscntrl(ch)))
            ch = ' ';

        // Skip spaces after ' ' and after '#'
        if (ch == ' ' && ignoreSpace)
            continue;
        ignoreSpace = bool(ch == ' ' || ch == '#' || ch == '\n');

        if (needSpace)
        {
            if (ch == '(')
                code << " ";
            else if (! std::isalpha(ch))
                needSpace = false;
        }
        if (ch == '#')
            needSpace = true;

        // <backspace><newline>..
        if (ch == '\\')
        {
            char chNext = 0;
            while (true)
            {
                chNext = (char)istr.peek();
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

    return removeComments(code.str());
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
    for (std::string::size_type i = 0; i < str.length(); ++i)
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
                    code << "\n";
                    previous = '\n';
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
            while (i < str.length() && chNext != ch);
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

void Preprocessor::preprocess(std::istream &istr, std::map<std::string, std::string> &result, const std::string &filename, const std::list<std::string> &includePaths, ErrorLogger *errorLogger)
{
    std::list<std::string> configs;
    std::string data;
    preprocess(istr, data, configs, filename, includePaths);
    for (std::list<std::string>::const_iterator it = configs.begin(); it != configs.end(); ++it)
        result[ *it ] = Preprocessor::getcode(data, *it, filename, errorLogger);
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
    std::string::size_type pos = 0;
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

    handleIncludes(processedFile, filename, includePaths);

    processedFile = replaceIfDefined(processedFile);

    // Get all possible configurations..
    resultConfigurations = getcfgs(processedFile);
}



// Get the DEF in this line: "#ifdef DEF"
std::string Preprocessor::getdef(std::string line, bool def)
{
    // If def is true, the line must start with "#ifdef"
    if (def && line.find("#ifdef ") != 0 && line.find("#if ") != 0 && line.find("#elif ") != 0)
    {
        return "";
    }

    // If def is false, the line must start with "#ifndef"
    if (!def && line.find("#ifndef ") != 0)
    {
        return "";
    }

    // Remove the "#ifdef" or "#ifndef"
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

    // How deep into included files are we currently parsing?
    // 0=>Source file, 1=>Included by source file, 2=>included by header that was included by source file, etc
    int filelevel = 0;

    std::istringstream istr(filedata);
    std::string line;
    while (getline(istr, line))
    {
        if (line.substr(0, 6) == "#file ")
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
                if (! def.empty())
                    def += ";";
                def += *it;
            }

            if (std::find(ret.begin(), ret.end(), def) == ret.end())
                ret.push_back(def);
        }

        if (line.find("#else") == 0 && ! deflist.empty())
        {
            std::string def((deflist.back() == "1") ? "0" : "1");
            deflist.pop_back();
            deflist.push_back(def);
        }

        if (line.find("#endif") == 0 && ! deflist.empty())
            deflist.pop_back();
    }

    return ret;
}



bool Preprocessor::match_cfg_def(std::string cfg, const std::string &def)
{
    if (def == "0")
        return false;

    if (def == "1")
        return true;

    if (cfg.empty())
        return false;

    while (! cfg.empty())
    {
        if (cfg.find(";") == std::string::npos)
            return bool(cfg == def);
        std::string _cfg = cfg.substr(0, cfg.find(";"));
        if (_cfg == def)
            return true;
        cfg.erase(0, cfg.find(";") + 1);
    }

    return false;
}


std::string Preprocessor::getcode(const std::string &filedata, std::string cfg, const std::string &filename, ErrorLogger *errorLogger)
{
    std::ostringstream ret;

    bool match = true;
    std::list<bool> matching_ifdef;
    std::list<bool> matched_ifdef;

    std::istringstream istr(filedata);
    std::string line;
    while (getline(istr, line))
    {
        std::string def = getdef(line, true);
        std::string ndef = getdef(line, false);

        if (line.find("#elif ") == 0)
        {
            if (matched_ifdef.back())
            {
                matching_ifdef.back() = false;
            }
            else
            {
                if (match_cfg_def(cfg, def))
                {
                    matching_ifdef.back() = true;
                    matched_ifdef.back() = true;
                }
            }
        }

        else if (! def.empty())
        {
            matching_ifdef.push_back(match_cfg_def(cfg, def));
            matched_ifdef.push_back(matching_ifdef.back());
        }

        else if (! ndef.empty())
        {
            matching_ifdef.push_back(! match_cfg_def(cfg, ndef));
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
                 line.find("#define") == 0)
        {
            // We must not remove #file tags or line numbers
            // are corrupted. File tags are removed by the tokenizer.
        }
        else if (!match ||
                 line[0] == '#')
        {
            // Remove #if, #else, #pragma etc, leaving only
            // #define, #file and #endfile. and also lines
            // which are not part of this configuration.
            line = "";
        }

        ret << line << "\n";
    }

    return expandMacros(ret.str(), filename, errorLogger);
}

std::string Preprocessor::getHeaderFileName(const std::string &str)
{
    std::string result;
    std::string::size_type i = str.find("\"");
    if (i == std::string::npos)
        return result;

    for (i = i + 1; i < str.length(); ++i)
    {
        if (str[i] == '"')
            break;

        result.append(1, str[i]);
    }

    return result;
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

        filename = getHeaderFileName(filename);
        if (filename.length() == 0)
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

        if (processedFile.length() == 0)
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
    }
}

class Macro
{
private:
    Tokenizer tokenizer;
    std::vector<std::string> _params;
    std::string _name;
    std::string _macro;
    bool _variadic;

public:
    Macro(const std::string &macro)
            : _macro(macro)
    {
        // Tokenize the macro to make it easier to handle
        std::istringstream istr(macro.c_str());
        tokenizer.createTokens(istr);

        // macro name..
        if (tokens() && tokens()->isName())
            _name = tokens()->str();

        _variadic = false;

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
                    if (Token::Match(tok, ". . . )"))
                    {
                        _variadic = true;
                        break;
                    }
                    if (tok->isName())
                        _params.push_back(tok->str());
                }
            }
        }
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

    const std::string &name() const
    {
        return _name;
    }

    bool code(const std::vector<std::string> &params2, std::string &macrocode) const
    {
        if (_params.empty())
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
                                    for (unsigned int j = _params.size() - 1; j < params2.size(); ++j)
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
                    if (_variadic && Token::Match(tok, ",") && tok->next() && Token::Match(tok->next(), "##"))
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
            defpos += 6;
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
        const Macro macro(code.substr(defpos + 8, endpos - defpos - 7));
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
                if (code.substr(pos1, 7) == "#undef ")
                    pos1 += 7;
                else if (code.substr(pos1, 8) == "#define ")
                    pos1 += 8;
                else
                    continue;

                // Compare the macroname with the macroname we're currently parsing (macro.name())
                // If it's the same macroname.. break.
                std::string::size_type pos = pos1 + macro.name().length();
                if (pos < code.length()
                    && code.substr(pos1, macro.name().length()) == macro.name()
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
                        if (code.substr(lineStart + 1, 7) == "#define")
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
            const std::string substr(code.substr(pos1, macro.name().length()));
            if (code.substr(pos1, macro.name().length()) != macro.name())
                continue;

            // Previous char must not be alphanumeric or '_'
            if (pos1 != 0 && (std::isalnum(code[pos1-1]) || code[pos1-1] == '_'))
                continue;

            // The char after the macroname must not be alphanumeric or '_'
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

            unsigned int numberOfNewlines = 0;

            if (macro.params().size())
            {
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
                            par += code[pos2];
                            if (code[pos2] == '\\')
                            {
                                par += code[pos2];
                                ++pos2;
                            }
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

                    }
                    else if (parlevel >= 1)
                    {
                        par.append(1, code[pos2]);
                    }
                }
            }

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
            if (!macro.params().empty())
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

