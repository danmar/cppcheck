/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis
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

#include <algorithm>

#include <sstream>
#include <fstream>

#ifdef __BORLANDC__
#include <ctype>
#endif


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

    // For the error report
    int lineno = 1;

    // handling <backspace><newline>
    // when this is encountered the <backspace><newline> will be "skipped".
    // on the next <newline>, extra newlines will be added
    unsigned int newlines = 0;

    std::ostringstream code;
    for (char ch = readChar(istr); istr.good(); ch = readChar(istr))
    {
        if (ch < 0)
            continue;

        if (ch == '\n')
            ++lineno;

        // Replace assorted special chars with spaces..
        if ((ch != '\n') && (isspace(ch) || iscntrl(ch)))
            ch = ' ';

        // Skip spaces after ' ' and after '#'
        if (ch == ' ' && ignoreSpace)
            continue;
        ignoreSpace = bool(ch == ' ' || ch == '#' || ch == '/');

        // Remove comments..
        if (ch == '/')
        {
            char chNext = readChar(istr);

            if (chNext == '/')
            {
                while (istr.good() && ch != '\n')
                    ch = readChar(istr);
                code << "\n";
                ++lineno;
            }

            else if (chNext == '*')
            {
                char chPrev = 0;
                while (istr.good() && (chPrev != '*' || ch != '/'))
                {
                    chPrev = ch;
                    ch = readChar(istr);
                    if (ch == '\n')
                    {
                        code << "\n";
                        ++lineno;
                    }
                }
            }

            else
            {
                if (chNext == '\n')
                    ++lineno;
                code << std::string(1, ch) << std::string(1, chNext);
            }
        }

        // String or char constants..
        else if (ch == '\"' || ch == '\'')
        {
            code << std::string(1, ch);
            char chNext;
            do
            {
                chNext = (char)istr.get();
                if (chNext == '\\')
                {
                    char chSeq = readChar(istr);
                    if (chSeq == '\n')
                        ++newlines;
                    else
                    {
                        code << std::string(1, chNext);
                        code << std::string(1, chSeq);
                    }
                }
                else
                    code << std::string(1, chNext);
            }
            while (istr.good() && chNext != ch);
        }
        /*
                // char constants..
                else if (ch == '\'')
                {
                    code << "\'";
                    ch = readChar(istr);
                    code << std::string(1, ch);
                    if (ch == '\\')
                    {
                        ch = readChar(istr);
                        code << std::string(1, ch);
                    }
                    ch = readChar(istr);
                    code << "\'";
                }
        */
        // <backspace><newline>..
        else if (ch == '\\')
        {
            char chNext = (char)istr.peek();
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

    return code.str();
}
#include <iostream>
void Preprocessor::preprocess(std::istream &istr, std::map<std::string, std::string> &result, const std::string &filename)
{
    std::list<std::string> configs;
    std::string data;
    preprocess(istr, data, configs, filename);
    for (std::list<std::string>::const_iterator it = configs.begin(); it != configs.end(); ++it)
        result[ *it ] = Preprocessor::getcode(data, *it);
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

void Preprocessor::preprocess(std::istream &istr, std::string &processedFile, std::list<std::string> &resultConfigurations, const std::string &filename)
{
    processedFile = read(istr);

    // Replace all tabs with spaces..
    std::replace(processedFile.begin(), processedFile.end(), '\t', ' ');

    // Remove all indentation..
    if (!processedFile.empty() && processedFile[0] == ' ')
        processedFile.erase(0, processedFile.find_first_not_of(" "));

    // Remove space characters that are after or before new line character
    processedFile = removeSpaceNearNL(processedFile);

    handleIncludes(processedFile, filename);

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

    std::istringstream istr(filedata);
    std::string line;
    while (getline(istr, line))
    {
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


std::string Preprocessor::getcode(const std::string &filedata, std::string cfg)
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

        else if (line == "#endif")
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
        if (! match)
            line = "";

        if (line.find("#if") == 0 ||
            line.find("#else") == 0 ||
            line.find("#elif") == 0 ||
            line.find("#endif") == 0)
            line = "";

        ret << line << "\n";
    }

    return expandMacros(ret.str());
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

void Preprocessor::handleIncludes(std::string &code, const std::string &filename)
{
//    std::string line;
    std::string path = filename;
    path.erase(1 + path.find_last_of("\\/"));
    std::string::size_type pos = 0;
    while ((pos = code.find("#include", pos)) != std::string::npos)
    {
        // Accept only includes that are at the start of a line
        if (pos > 0 && code[pos-1] != '\n')
        {
            pos += 8; // length of "#include"
            continue;
        }

        std::string::size_type end = code.find("\n", pos);
        std::string filename = code.substr(pos, end - pos);

        // Remove #include clause
        code.erase(pos, end - pos);

        filename = getHeaderFileName(filename);
        if (filename.length() == 0)
            continue;

        // filename contains now a file name e.g. "menu.h"
        filename = path + filename;
        std::ifstream fin(filename.c_str());
        std::string processedFile = Preprocessor::read(fin);
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

public:
    Macro(const std::string &macro)
            : _macro(macro)
    {
        // Tokenize the macro to make it easier to handle
        std::istringstream istr(macro.c_str());
        tokenizer.tokenize(istr, "");

        // macro name..
        if (tokens() && tokens()->isName())
            _name = tokens()->str();

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

    const std::string &name() const
    {
        return _name;
    }

    const std::string code(const std::vector<std::string> &params2) const
    {
        std::string macrocode;

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
                while ((tok = tok->next()) != NULL)
                {
                    std::string str = tok->str();
                    if (str == "##")
                        continue;
                    if (tok->isName())
                    {
                        for (unsigned int i = 0; i < _params.size(); ++i)
                        {
                            if (str == _params[i])
                            {
                                str = params2[i];
                                break;
                            }
                        }
                    }
                    macrocode += str;
                    if (Token::Match(tok, "%type% %var%"))
                        macrocode += " ";
                }
            }
        }

        return macrocode;
    }
};


#include <iostream>

std::string Preprocessor::expandMacros(std::string code)
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
                const std::string substr(code.substr(pos1, 7 + macro.name().length()));
                if (substr == "#undef " + macro.name())
                    break;
                else
                    continue;
            }

            // String or char..
            if (code[pos1] == '\"' || code[pos1] == '\'')
            {
                //char ch = code[pos1];
                ++pos1;
                while (code[pos1] != ch)
                {
                    if (code[pos1] == '\\')
                        ++pos1;
                    ++pos1;

                    if (!code[pos1])
                    {
                        // TODO, this code is here, because there is currently a bug in cppcheck
                        // Once it has been sorted out, this if can be removed
                        std::cout << "\n\n####### There is a bug in preprocessor.cpp that can cause crash, shutting down.\n\n" << std::endl;
                        exit(0);
                    }
                }
                continue;
            }

            // Matching the macroname?
            const std::string substr(code.substr(pos1, macro.name().length()));
            if (code.substr(pos1, macro.name().length()) != macro.name())
                continue;

            // Previous char must not be alphanumeric or '_'
            if (pos1 != 0 && (isalnum(code[pos1-1]) || code[pos1-1] == '_'))
                continue;

            // The char after the macroname must not be alphanumeric or '_'
            if (pos1 + macro.name().length() < code.length())
            {
                std::string::size_type pos2 = pos1 + macro.name().length();
                if (isalnum(code[pos2]) || code[pos2] == '_')
                    continue;
            }

            std::vector<std::string> params;
            std::string::size_type pos2 = pos1 + macro.name().length();
            if (macro.params().size() && pos2 >= code.length())
                continue;
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

                    if (parlevel == 1 && code[pos2] == ',')
                    {
                        params.push_back(par);
                        par = "";
                    }
                    else if (parlevel >= 1)
                    {
                        par += std::string(1, code[pos2]);
                    }
                }
            }

            // Same number of parameters..
            if (params.size() != macro.params().size())
                continue;

            // Create macro code..
            const std::string macrocode(macro.code(params));

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

