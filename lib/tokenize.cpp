/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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


//---------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(disable: 4503)
#endif

#include "tokenize.h"
#include "token.h"
#include "mathlib.h"
#include "settings.h"
#include "errorlogger.h"
#include "check.h"
#include "path.h"
#include "symboldatabase.h"

#include <locale>
#include <fstream>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <list>
#include <cassert>
#include <algorithm>
#include <cctype>
#include <stack>
#include <stdexcept>    // for std::runtime_error

//---------------------------------------------------------------------------

Tokenizer::Tokenizer()
    : _settings(0), _errorLogger(0)
{
    // No tokens to start with
    _tokens = 0;
    _tokensBack = 0;

    // is there any templates?
    _codeWithTemplates = false;

    // symbol database
    _symbolDatabase = NULL;

    // variable count
    _varId = 0;
}

Tokenizer::Tokenizer(const Settings *settings, ErrorLogger *errorLogger)
    : _settings(settings), _errorLogger(errorLogger)
{
    // make sure settings are specified
    assert(_settings);

    // No tokens to start with
    _tokens = 0;
    _tokensBack = 0;

    // is there any templates?
    _codeWithTemplates = false;

    // symbol database
    _symbolDatabase = NULL;

    // variable count
    _varId = 0;
}

Tokenizer::~Tokenizer()
{
    deallocateTokens();
    delete _symbolDatabase;
}

//---------------------------------------------------------------------------

// Helper functions..


//---------------------------------------------------------------------------

const Token *Tokenizer::tokens() const
{
    return _tokens;
}


const std::vector<std::string> *Tokenizer::getFiles() const
{
    return &_files;
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// addtoken
// add a token. Used by 'Tokenizer'
//---------------------------------------------------------------------------

void Tokenizer::addtoken(const char str[], const unsigned int lineno, const unsigned int fileno, bool split)
{
    if (str[0] == 0)
        return;

    // If token contains # characters, split it up
    if (split && strstr(str, "##"))
    {
        std::string temp;
        for (unsigned int i = 0; str[i]; ++i)
        {
            if (strncmp(&str[i], "##", 2) == 0)
            {
                addtoken(temp.c_str(), lineno, fileno, false);
                temp.clear();
                addtoken("##", lineno, fileno, false);
                ++i;
            }
            else
                temp += str[i];
        }
        addtoken(temp.c_str(), lineno, fileno, false);
        return;
    }

    // Replace hexadecimal value with decimal
    std::ostringstream str2;
    if (strncmp(str, "0x", 2) == 0 || strncmp(str, "0X", 2) == 0)
    {
        str2 << std::strtoul(str + 2, NULL, 16);
    }
    else
    {
        str2 << str;
    }

    if (_tokensBack)
    {
        _tokensBack->insertToken(str2.str().c_str());
    }
    else
    {
        _tokens = new Token(&_tokensBack);
        _tokensBack = _tokens;
        _tokensBack->str(str2.str());
    }

    _tokensBack->linenr(lineno);
    _tokensBack->fileIndex(fileno);
}

void Tokenizer::addtoken(const Token * tok, const unsigned int lineno, const unsigned int fileno)
{
    if (tok == 0)
        return;

    // Replace hexadecimal value with decimal
    std::ostringstream str2;
    if (strncmp(tok->str().c_str(), "0x", 2) == 0)
    {
        str2 << std::strtoul(tok->str().c_str() + 2, NULL, 16);
    }
    else
    {
        str2 << tok->str();
    }

    if (_tokensBack)
    {
        _tokensBack->insertToken(str2.str().c_str());
    }
    else
    {
        _tokens = new Token(&_tokensBack);
        _tokensBack = _tokens;
        _tokensBack->str(str2.str());
    }

    _tokensBack->linenr(lineno);
    _tokensBack->fileIndex(fileno);
    _tokensBack->isUnsigned(tok->isUnsigned());
    _tokensBack->isSigned(tok->isSigned());
    _tokensBack->isLong(tok->isLong());
    _tokensBack->isUnused(tok->isUnused());
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// SizeOfType - gives the size of a type
//---------------------------------------------------------------------------



unsigned int Tokenizer::sizeOfType(const Token *type) const
{
    if (!type || type->str().empty())
        return 0;

    if (type->str()[0] == '"')
        return static_cast<unsigned int>(Token::getStrLength(type) + 1);

    std::map<std::string, unsigned int>::const_iterator it = _typeSize.find(type->str());
    if (it == _typeSize.end())
        return 0;
    else if (type->isLong())
    {
        if (type->str() == "double")
            return sizeof(long double);
        else if (type->str() == "long")
            return sizeof(long long);
    }

    return it->second;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// InsertTokens - Copy and insert tokens
//---------------------------------------------------------------------------

void Tokenizer::insertTokens(Token *dest, const Token *src, unsigned int n)
{
    while (n > 0)
    {
        dest->insertToken(src->str().c_str());
        dest = dest->next();
        dest->fileIndex(src->fileIndex());
        dest->linenr(src->linenr());
        dest->varId(src->varId());
        dest->isUnsigned(src->isUnsigned());
        dest->isSigned(src->isSigned());
        dest->isLong(src->isLong());
        src  = src->next();
        --n;
    }
}
//---------------------------------------------------------------------------

Token *Tokenizer::copyTokens(Token *dest, const Token *first, const Token *last)
{
    std::stack<Token *> links;
    Token *tok2 = dest;
    for (const Token *tok = first; tok != last->next(); tok = tok->next())
    {
        tok2->insertToken(tok->str());
        tok2 = tok2->next();
        tok2->fileIndex(dest->fileIndex());
        tok2->linenr(dest->linenr());
        tok2->isUnsigned(tok->isUnsigned());
        tok2->isSigned(tok->isSigned());
        tok2->isLong(tok->isLong());

        // Check for links and fix them up
        if (tok2->str() == "(" || tok2->str() == "[" || tok2->str() == "{")
            links.push(tok2);
        else if (tok2->str() == ")" || tok2->str() == "]" || tok2->str() == "}")
        {
            Token * link = links.top();

            tok2->link(link);
            link->link(tok2);

            links.pop();
        }
    }
    return tok2;
}

//---------------------------------------------------------------------------
// Tokenize - tokenizes a given file.
//---------------------------------------------------------------------------

void Tokenizer::createTokens(std::istream &code)
{
    // line number in parsed code
    unsigned int lineno = 1;

    // The current token being parsed
    std::string CurrentToken;

    // lineNumbers holds line numbers for files in fileIndexes
    // every time an include file is completely parsed, last item in the vector
    // is removed and lineno is set to point to that value.
    std::vector<unsigned int> lineNumbers;

    // fileIndexes holds index for _files vector about currently parsed files
    // every time an include file is completely parsed, last item in the vector
    // is removed and FileIndex is set to point to that value.
    std::vector<unsigned int> fileIndexes;

    // FileIndex. What file in the _files vector is read now?
    unsigned int FileIndex = 0;

    // Read one byte at a time from code and create tokens
    for (char ch = (char)code.get(); code.good(); ch = (char)code.get())
    {
        // char/string..
        // multiline strings are not handled. The preprocessor should handle that for us.
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

                // Special sequence '\.'
                if (special)
                    special = false;
                else
                    special = (c == '\\');

                // Get next character
                c = (char)code.get();
            }
            while (code.good() && (special || c != ch));
            line += ch;

            // Handle #file "file.h"
            if (CurrentToken == "#file")
            {
                // Extract the filename
                line = line.substr(1, line.length() - 2);

                // Has this file been tokenized already?
                ++lineno;
                bool foundOurfile = false;
                fileIndexes.push_back(FileIndex);
                for (unsigned int i = 0; i < _files.size(); i++)
                {
                    if (Path::sameFileName(_files[i].c_str(), line.c_str()))
                    {
                        // Use this index
                        foundOurfile = true;
                        FileIndex = i;
                    }
                }

                if (!foundOurfile)
                {
                    // The "_files" vector remembers what files have been tokenized..
                    _files.push_back(Path::simplifyPath(line.c_str()));
                    FileIndex = static_cast<unsigned int>(_files.size() - 1);
                }

                lineNumbers.push_back(lineno);
                lineno = 0;
            }
            else
            {
                // Add previous token
                addtoken(CurrentToken.c_str(), lineno, FileIndex);

                // Add content of the string
                addtoken(line.c_str(), lineno, FileIndex);
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
                     (CurrentToken[CurrentToken.length()-1] == 'e' ||
                      CurrentToken[CurrentToken.length()-1] == 'E'))
            {
                // Don't separate doubles "4.2e+10"
            }
            else if (CurrentToken.empty() && ch == '.' && std::isdigit(code.peek()))
            {
                // tokenize .125 into 0.125
                CurrentToken = "0";
            }
            else if (ch=='&' && CurrentToken.empty() && code.peek() == '&')
            {
                // &&
                ch = (char)code.get();
                addtoken("&&", lineno, FileIndex, true);
                continue;
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
                        cppcheckError(0);
                        deallocateTokens();
                        return;
                    }

                    lineno = lineNumbers.back();
                    lineNumbers.pop_back();
                    FileIndex = fileIndexes.back();
                    fileIndexes.pop_back();
                    CurrentToken.clear();
                    continue;
                }

                addtoken(CurrentToken.c_str(), lineno, FileIndex, true);

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
                if ((ch == '+' || ch == '-' || ch == '>') && (code.peek() == ch))
                    CurrentToken += (char)code.get();
                addtoken(CurrentToken.c_str(), lineno, FileIndex);
                CurrentToken.clear();
                continue;
            }
        }

        CurrentToken += ch;
    }
    addtoken(CurrentToken.c_str(), lineno, FileIndex, true);
    _tokens->assignProgressValues();
}

void Tokenizer::duplicateTypedefError(const Token *tok1, const Token *tok2, const std::string &type)
{
    if (!(_settings->_checkCodingStyle))
        return;

    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    ErrorLogger::ErrorMessage::FileLocation loc;
    loc.line = tok1->linenr();
    loc.setfile(file(tok1));
    locationList.push_back(loc);
    loc.line = tok2->linenr();
    loc.setfile(file(tok2));
    locationList.push_back(loc);

    const ErrorLogger::ErrorMessage errmsg(locationList,
                                           Severity::style,
                                           std::string(type + " '" + tok2->str() +
                                                   "' hides typedef with same name"),
                                           "variableHidingTypedef");

    if (_errorLogger)
        _errorLogger->reportErr(errmsg);
    else
        Check::reportError(errmsg);
}

void Tokenizer::duplicateDeclarationError(const Token *tok1, const Token *tok2, const std::string &type)
{
    if (!(_settings->_checkCodingStyle))
        return;

    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    ErrorLogger::ErrorMessage::FileLocation loc;
    loc.line = tok1->linenr();
    loc.setfile(file(tok1));
    locationList.push_back(loc);
    loc.line = tok2->linenr();
    loc.setfile(file(tok2));
    locationList.push_back(loc);

    const ErrorLogger::ErrorMessage errmsg(locationList,
                                           Severity::style,
                                           std::string(type + " '" + tok2->str() +
                                                   "' forward declaration unnecessary, already declared"),
                                           "unnecessaryForwardDeclaration");

    if (_errorLogger)
        _errorLogger->reportErr(errmsg);
    else
        Check::reportError(errmsg);
}

// check if this statement is a duplicate definition
bool Tokenizer::duplicateTypedef(Token **tokPtr, const Token *name, const Token *typeDef)
{
    // check for an end of definition
    const Token * tok = *tokPtr;
    if (tok && Token::Match(tok->next(), ";|,|[|=|)|>|(|{"))
    {
        const Token * end = tok->next();

        if (end->str() == "[")
        {
            end = end->link()->next();
        }
        else if (end->str() == ",")
        {
            // check for derived class
            if (Token::Match(tok->previous(), "public|private|protected"))
                return false;

            // find end of definition
            int level = 0;
            while (end && end->next() && (!Token::Match(end->next(), ";|)|>") ||
                                          (end->next()->str() == ")" && level == 0)))
            {
                if (end->next()->str() == "(")
                    level++;
                else if (end->next()->str() == ")")
                    level--;

                end = end->next();
            }
            end = end->next();
        }
        else if (end->str() == "(")
        {
            if (tok->previous()->str().find("operator")  == 0)
            {
                // conversion operator
                return false;
            }
            else if (tok->previous()->str() == "typedef")
            {
                // typedef of function returning this type
                return false;
            }
            else if (Token::Match(tok->previous(), "public:|private:|protected:"))
            {
                return false;
            }
            else if (tok->previous()->str() == ">")
            {
                if (!Token::Match(tok->tokAt(-2), "%type%"))
                    return false;

                if (!Token::Match(tok->tokAt(-3), ",|<"))
                    return false;

                duplicateTypedefError(*tokPtr, name, "Template instantiation");
                *tokPtr = end->link();
                return true;
            }
            else if (Token::Match(tok->previous(), "%type%"))
            {
                if (end->link()->next()->str() == "{")
                {
                    duplicateTypedefError(*tokPtr, name, "Function");
                    *tokPtr = end->link()->next()->link();
                    return true;
                }
            }
        }

        if (end)
        {
            if (Token::simpleMatch(end, ") {")) // function parameter ?
            {
                // look backwards
                if (Token::Match(tok->previous(), "%type%") &&
                    !Token::Match(tok->previous(), "return|new|const"))
                {
                    duplicateTypedefError(*tokPtr, name, "Function parameter");
                    // duplicate definition so skip entire function
                    *tokPtr = end->next()->link();
                    return true;
                }
            }
            else if (end->str() == ">") // template parameter ?
            {
                // look backwards
                if (Token::Match(tok->previous(), "%type%") &&
                    !Token::Match(tok->previous(), "return|new|const|volatile"))
                {
                    // duplicate definition so skip entire template
                    while (end && end->str() != "{")
                        end = end->next();
                    if (end)
                    {
                        duplicateTypedefError(*tokPtr, name, "Template parameter");
                        *tokPtr = end->link();
                        return true;
                    }
                }
            }
            else
            {
                // look backwards
                if (Token::Match(tok->previous(), "typedef|}|>") ||
                    (tok->previous()->str() == "*" && tok->next()->str() != "(") ||
                    (Token::Match(tok->previous(), "%type%") &&
                     (!Token::Match(tok->previous(), "return|new|const|friend|public|private|protected|throw|extern") &&
                      !Token::simpleMatch(tok->tokAt(-2), "friend class"))))
                {
                    // scan backwards for the end of the previous statement
                    int level = (tok->previous()->str() == "}") ? 1 : 0;
                    while (tok && tok->previous() && (!Token::Match(tok->previous(), ";|{") || (level != 0)))
                    {
                        if (tok->previous()->str() == "}")
                        {
                            tok = tok->previous()->link();
                        }
                        else if (tok->previous()->str() == "typedef")
                        {
                            duplicateTypedefError(*tokPtr, name, "Typedef");
                            return true;
                        }
                        else if (tok->previous()->str() == "enum")
                        {
                            duplicateTypedefError(*tokPtr, name, "Enum");
                            return true;
                        }
                        else if (tok->previous()->str() == "struct")
                        {
                            if (tok->strAt(-2) == "typedef" &&
                                tok->next()->str() == "{" &&
                                typeDef->strAt(3) != "{")
                            {
                                // declaration after forward declaration
                                return true;
                            }
                            else if (tok->next()->str() != ";")
                            {
                                duplicateTypedefError(*tokPtr, name, "Struct");
                                return true;
                            }
                            else
                            {
                                // forward declaration after declaration
                                duplicateDeclarationError(*tokPtr, name, "Struct");
                                return false;
                            }
                        }
                        else if (tok->previous()->str() == "union")
                        {
                            if (tok->next()->str() != ";")
                            {
                                duplicateTypedefError(*tokPtr, name, "Union");
                                return true;
                            }
                            else
                            {
                                // forward declaration after declaration
                                duplicateDeclarationError(*tokPtr, name, "Union");
                                return false;
                            }
                        }
                        else if (tok->previous()->str() == "class")
                        {
                            if (tok->next()->str() != ";")
                            {
                                duplicateTypedefError(*tokPtr, name, "Class");
                                return true;
                            }
                            else
                            {
                                // forward declaration after declaration
                                duplicateDeclarationError(*tokPtr, name, "Class");
                                return false;
                            }
                        }
                        else if (tok->previous()->str() == "{")
                            level--;

                        tok = tok->previous();
                    }

                    duplicateTypedefError(*tokPtr, name, "Variable");
                    return true;
                }
            }
        }
    }

    return false;
}

void Tokenizer::unsupportedTypedef(const Token *tok) const
{
    if (!_settings->debugwarnings)
        return;

    std::ostringstream str;
    const Token *tok1 = tok;
    int level = 0;
    while (tok)
    {
        if (level == 0 && tok->str() == ";")
            break;
        else if (tok->str() == "{")
            level++;
        else if (tok->str() == "}")
            level--;

        if (tok != tok1)
            str << " ";
        str << tok->str();
        tok = tok->next();
    }
    if (tok)
        str << " ;";
    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    ErrorLogger::ErrorMessage::FileLocation loc;
    loc.line = tok1->linenr();
    loc.setfile(file(tok1));
    locationList.push_back(loc);

    const ErrorLogger::ErrorMessage errmsg(locationList,
                                           Severity::debug,
                                           "Failed to parse \'" + str.str() + "\'. The checking continues anyway.",
                                           "debug");

    if (_errorLogger)
        _errorLogger->reportErr(errmsg);
    else
        Check::reportError(errmsg);
}

Token * Tokenizer::deleteInvalidTypedef(Token *typeDef)
{
    Token *tok = NULL;
    int level = 0;

    // remove typedef but leave ;
    while (typeDef->next())
    {
        if (level == 0 && typeDef->next()->str() == ";")
            break;
        else if (typeDef->next()->str() == "{")
            level++;
        else if (typeDef->next()->str() == "}")
            level--;
        typeDef->deleteNext();
    }

    if (typeDef != _tokens)
    {
        tok = typeDef->previous();
        tok->deleteNext();
    }
    else
    {
        _tokens->deleteThis();
        tok = _tokens;
    }

    return tok;
}

struct Space
{
    bool isNamespace;
    std::string className;
    const Token * classEnd;
};

static Token *splitDefinitionFromTypedef(Token *tok)
{
    Token *tok1;
    std::string name;
    bool isConst = false;

    if (tok->next()->str() == "const")
    {
        tok->next()->deleteThis();
        isConst = true;
    }

    if (tok->tokAt(2)->str() == "{") // unnamed
    {
        tok1 = tok->tokAt(2)->link();

        if (tok1 && tok1->next())
        {
            // use typedef name if available
            if (Token::Match(tok1->next(), "%type%"))
                name = tok1->next()->str();
            else // create a unique name
            {
                static long count = 0;
                name = "Unnamed" + MathLib::toString<long>(count++);
            }
            tok->tokAt(1)->insertToken(name.c_str());
        }
        else
            return NULL;
    }
    else if (tok->strAt(3) == ":")
    {
        tok1 = tok->tokAt(4);
        while (tok1 && tok1->str() != "{")
            tok1 = tok1->next();
        if (!tok1)
            return NULL;

        tok1 = tok1->link();

        name = tok->tokAt(2)->str();
    }
    else // has a name
    {
        tok1 = tok->tokAt(3)->link();

        if (!tok1)
            return NULL;

        name = tok->tokAt(2)->str();
    }

    tok1->insertToken(";");
    tok1 = tok1->next();

    if (tok1->next()->str() == ";" && tok1 && tok1->previous()->str() == "}")
    {
        tok->deleteThis();
        tok1->deleteThis();
        return NULL;
    }
    else
    {
        tok1->insertToken("typedef");
        tok1 = tok1->next();
        Token * tok3 = tok1;
        if (isConst)
        {
            tok1->insertToken("const");
            tok1 = tok1->next();
        }
        tok1->insertToken(tok->next()->str()); // struct, union or enum
        tok1 = tok1->next();
        tok1->insertToken(name.c_str());
        tok->deleteThis();
        tok = tok3;
    }

    return tok;
}

/* This function is called when processing function related typedefs.
 * If simplifyTypedef generates an "Internal Error" message and the
 * code that generated it deals in some way with functions, then this
 * fucntion will probably need to be extended to handle a new function
 * related pattern */
static Token *processFunc(Token *tok2, bool inOperator)
{
    if (tok2->next() && tok2->next()->str() != ")" &&
        tok2->next()->str() != ",")
    {
        // skip over tokens for some types of canonicalization
        if (Token::Match(tok2->next(), "( * %type% ) ("))
            tok2 = tok2->tokAt(5)->link();
        else if (Token::Match(tok2->next(), "* ( * %type% ) ("))
            tok2 = tok2->tokAt(6)->link();
        else if (Token::Match(tok2->next(), "* ( * %type% ) ;"))
            tok2 = tok2->tokAt(5);
        else if (Token::Match(tok2->next(), "* ( %type% [") &&
                 Token::Match(tok2->tokAt(4)->link(), "] ) ;|="))
            tok2 = tok2->tokAt(4)->link()->next();
        else if (Token::Match(tok2->next(), "* ( * %type% ("))
            tok2 = tok2->tokAt(5)->link()->next();
        else if (Token::Match(tok2->next(), "* [") &&
                 Token::simpleMatch(tok2->tokAt(2)->link(), "] ;"))
            tok2 = tok2->next();
        else
        {
            if (tok2->next()->str() == "(")
                tok2 = tok2->next()->link();
            else if (!inOperator && !Token::Match(tok2->next(), "[|>|;"))
            {
                tok2 = tok2->next();

                while (Token::Match(tok2, "*|&") &&
                       !Token::Match(tok2->next(), ")|>"))
                    tok2 = tok2->next();

                // skip over namespace
                while (Token::Match(tok2, "%var% ::"))
                    tok2 = tok2->tokAt(2);

                if (tok2->str() == "(" &&
                    tok2->link()->next()->str() == "(")
                {
                    tok2 = tok2->link();

                    if (tok2->next()->str() == "(")
                        tok2 = tok2->next()->link();
                }

                // skip over typedef parameter
                if (tok2->next()->str() == "(")
                {
                    tok2 = tok2->next()->link();

                    if (tok2->next()->str() == "(")
                        tok2 = tok2->next()->link();
                }
            }
        }
    }
    return tok2;
}

void Tokenizer::simplifyTypedef()
{
    std::vector<Space> spaceInfo;
    bool isNamespace = false;
    std::string className;
    bool hasClass = false;
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (_errorLogger && !_files.empty())
            _errorLogger->reportProgress(_files[0], "Tokenize (typedef)", tok->progressValue());

        if (Token::Match(tok, "class|struct|namespace %any%") &&
            (!tok->previous() || (tok->previous() && tok->previous()->str() != "enum")))
        {
            isNamespace = (tok->str() == "namespace");
            hasClass = true;
            className = tok->next()->str();
            continue;
        }
        else if (hasClass && tok->str() == ";")
        {
            hasClass = false;
            continue;
        }
        else if (hasClass && tok->str() == "{")
        {
            Space info;
            info.isNamespace = isNamespace;
            info.className = className;
            info.classEnd = tok->link();
            spaceInfo.push_back(info);

            hasClass = false;
            continue;
        }
        else if (!spaceInfo.empty() && tok->str() == "}" && spaceInfo.back().classEnd == tok)
        {
            spaceInfo.pop_back();
            continue;
        }
        else if (tok->str() != "typedef")
            continue;

        // check for syntax errors
        if (tok->previous() && tok->previous()->str() == "(")
        {
            syntaxError(tok);
            continue;
        }

        // pull struct, union, enum or class definition out of typedef
        // use typedef name for unnamed struct, union, enum or class
        if (Token::Match(tok->next(), "const| struct|enum|union|class %type% {") ||
            Token::Match(tok->next(), "const| struct|enum|union|class {"))
        {
            Token *tok1 = splitDefinitionFromTypedef(tok);
            if (!tok1)
                continue;
            tok = tok1;
        }
        else if (Token::Match(tok->next(), "const| struct|class %type% :"))
        {
            Token *tok1 = tok;
            while (tok1 && tok1->str() != ";" && tok1->str() != "{")
                tok1 = tok1->next();
            if (tok1 && tok1->str() == "{")
            {
                tok1 = splitDefinitionFromTypedef(tok);
                if (!tok1)
                    continue;
                tok = tok1;
            }
        }

        /** @todo add support for struct and union */
        if (Token::Match(tok, "typedef enum %type% %type% ;") && tok->strAt(2) == tok->strAt(3))
        {
            tok->deleteThis();
            tok->deleteThis();
            tok->deleteThis();
            tok->deleteThis();
            continue;
        }

        Token *typeName;
        std::list<std::string> pointers;
        Token *typeStart = 0;
        Token *typeEnd = 0;
        Token *argStart = 0;
        Token *argEnd = 0;
        Token *arrayStart = 0;
        Token *arrayEnd = 0;
        Token *specStart = 0;
        Token *specEnd = 0;
        Token *typeDef = tok;
        Token *argFuncRetStart = 0;
        Token *argFuncRetEnd = 0;
        Token *const1 = 0;
        Token *const2 = 0;
        Token *funcStart = 0;
        Token *funcEnd = 0;
        int offset = 1;
        bool function = false;
        bool functionPtr = false;
        bool functionRef = false;
        bool functionRetFuncPtr = false;
        bool functionPtrRetFuncPtr = false;
        bool ptrToArray = false;
        bool refToArray = false;
        bool ptrMember = false;
        bool typeOf = false;
        Token *namespaceStart = 0;
        Token *namespaceEnd = 0;

        // check for invalid input
        if (!tok->next())
        {
            syntaxError(tok);
            return;
        }

        if (Token::simpleMatch(tok->next(), "::") ||
            Token::Match(tok->next(), "%type%"))
        {
            typeStart = tok->next();
            offset = 1;

            while (Token::Match(tok->tokAt(offset), "const|signed|unsigned"))
                offset++;

            typeEnd = tok->tokAt(offset++);

            bool atEnd = false;
            while (!atEnd)
            {
                if (Token::simpleMatch(tok->tokAt(offset), "::"))
                    typeEnd = tok->tokAt(offset++);

                if (Token::Match(tok->tokAt(offset), "%type%") &&
                    tok->tokAt(offset + 1) && !Token::Match(tok->tokAt(offset + 1), "[|;|,|("))
                    typeEnd = tok->tokAt(offset++);
                else if (Token::simpleMatch(tok->tokAt(offset), "const ("))
                {
                    typeEnd = tok->tokAt(offset++);
                    atEnd = true;
                }
                else
                    atEnd = true;
            }
        }
        else
            continue; // invalid input

        // check for invalid input
        if (!tok->tokAt(offset))
        {
            syntaxError(tok);
            return;
        }

        // check for template
        if (tok->tokAt(offset)->str() == "<")
        {
            int level = 1;
            int paren = 0;
            typeEnd = tok->tokAt(offset + 1);
            for (; typeEnd ; typeEnd = typeEnd->next())
            {
                if (typeEnd->str() == ">")
                {
                    if (paren == 0)
                    {
                        level--;
                        if (level == 0)
                            break;
                    }
                }
                else if (typeEnd->str() == "<")
                {
                    if (paren == 0)
                        level++;
                }
                else if (typeEnd->str() == "(")
                    paren++;
                else if (typeEnd->str() == ")")
                    paren--;
            }

            while (typeEnd && Token::Match(typeEnd->next(), ":: %type%"))
                typeEnd = typeEnd->tokAt(2);

            if (!typeEnd)
            {
                // internal error
                return;
            }

            while (Token::Match(typeEnd->next(), "const|volatile"))
                typeEnd = typeEnd->next();

            tok = typeEnd;
            offset = 1;
        }

        // check for pointers and references
        while (Token::Match(tok->tokAt(offset), "*|&|const"))
            pointers.push_back(tok->tokAt(offset++)->str());

        if (Token::Match(tok->tokAt(offset), "%type%"))
        {
            // found the type name
            typeName = tok->tokAt(offset++);

            // check for array
            if (tok->tokAt(offset) && tok->tokAt(offset)->str() == "[")
            {
                arrayStart = tok->tokAt(offset);

                bool atEnd = false;
                while (!atEnd)
                {
                    while (tok->tokAt(offset + 1) && !Token::Match(tok->tokAt(offset + 1), ";|,"))
                        offset++;

                    if (!tok->tokAt(offset + 1))
                        return; // invalid input
                    else if (tok->tokAt(offset + 1)->str() == ";")
                        atEnd = true;
                    else if (tok->tokAt(offset)->str() == "]")
                        atEnd = true;
                    else
                        offset++;
                }

                arrayEnd = tok->tokAt(offset++);
            }

            // check for end or another
            if (Token::Match(tok->tokAt(offset), ";|,"))
                tok = tok->tokAt(offset);

            // or a function typedef
            else if (Token::simpleMatch(tok->tokAt(offset), "("))
            {
                // unhandled typedef, skip it and continue
                if (typeName->str() == "void")
                {
                    unsupportedTypedef(typeDef);
                    tok = deleteInvalidTypedef(typeDef);
                    continue;
                }

                function = true;
                if (tok->tokAt(offset)->link()->next())
                {
                    argStart = tok->tokAt(offset);
                    argEnd = tok->tokAt(offset)->link();
                    tok = argEnd->next();
                }
                else
                {
                    // internal error
                    continue;
                }
            }

            // unhandled typedef, skip it and continue
            else
            {
                unsupportedTypedef(typeDef);
                tok = deleteInvalidTypedef(typeDef);
                continue;
            }
        }

        // typeof: typedef __typeof__ ( ... ) type;
        else if (Token::simpleMatch(tok->tokAt(offset - 1), "__typeof__ (") &&
                 Token::Match(tok->tokAt(offset)->link(), ") %type% ;"))
        {
            argStart = tok->tokAt(offset);
            argEnd = tok->tokAt(offset)->link();
            typeName = tok->tokAt(offset)->link()->next();
            tok = typeName->next();
            typeOf = true;
        }

        // function: typedef ... ( .... type )( ... );
        else if (tok->tokAt(offset)->str() == "(" &&
                 Token::Match(tok->tokAt(offset)->link()->previous(), "%type% ) (") &&
                 Token::Match(tok->tokAt(offset)->link()->next()->link(), ") const|volatile|;"))
        {
            funcStart = tok->tokAt(offset + 1);
            funcEnd = tok->tokAt(offset)->link()->tokAt(-2);
            typeName = tok->tokAt(offset)->link()->previous();
            argStart = tok->tokAt(offset)->link()->next();
            argEnd = tok->tokAt(offset)->link()->next()->link();
            tok = argEnd->next();
            Token *spec = tok;
            if (Token::Match(spec, "const|volatile"))
            {
                specStart = spec;
                specEnd = spec;
                while (Token::Match(spec->next(), "const|volatile"))
                {
                    specEnd = spec->next();
                    spec = specEnd;
                }
                tok = specEnd->next();
            }
        }

        else if (Token::Match(tok->tokAt(offset), "( %type% ("))
        {
            function = true;
            if (tok->tokAt(offset)->link()->next())
            {
                typeName = tok->tokAt(offset + 1);
                argStart = tok->tokAt(offset + 2);
                argEnd = tok->tokAt(offset + 2)->link();
                tok = tok->tokAt(offset)->link()->next();
            }
            else
            {
                // internal error
                continue;
            }
        }

        // pointer to function returning pointer to function
        else if (Token::Match(tok->tokAt(offset), "( * ( * %type% ) ("))
        {
            functionPtrRetFuncPtr = true;

            typeName = tok->tokAt(offset + 4);
            argStart = tok->tokAt(offset + 6);
            argEnd = tok->tokAt(offset + 6)->link();

            argFuncRetStart = argEnd->tokAt(2);
            argFuncRetEnd = argEnd->tokAt(2)->link();

            tok = argFuncRetEnd->next();
        }

        // function returning pointer to function
        else if (Token::Match(tok->tokAt(offset), "( * %type% (") &&
                 Token::simpleMatch(tok->tokAt(offset + 3)->link(), ") ) ("))
        {
            functionRetFuncPtr = true;

            typeName = tok->tokAt(offset + 2);
            argStart = tok->tokAt(offset + 3);
            argEnd = tok->tokAt(offset + 3)->link();

            argFuncRetStart = argEnd->tokAt(2);
            argFuncRetEnd = argEnd->tokAt(2)->link();

            tok = argFuncRetEnd->next();
        }
        else if (Token::Match(tok->tokAt(offset), "( * ( %type% ) ("))
        {
            functionRetFuncPtr = true;

            typeName = tok->tokAt(offset + 3);
            argStart = tok->tokAt(offset + 5);
            argEnd = tok->tokAt(offset + 5)->link();

            argFuncRetStart = argEnd->tokAt(2);
            argFuncRetEnd = argEnd->tokAt(2)->link();

            tok = argFuncRetEnd->next();
        }

        // pointer/reference to array
        else if (Token::Match(tok->tokAt(offset), "( *|& %type% ) ["))
        {
            ptrToArray = (tok->tokAt(offset + 1)->str() == "*");
            refToArray = (tok->tokAt(offset + 1)->str() == "&");
            typeName = tok->tokAt(offset + 2);
            arrayStart = tok->tokAt(offset + 4);
            arrayEnd = arrayStart->link();
            tok = arrayEnd->next();
        }

        // pointer to class member
        else if (Token::Match(tok->tokAt(offset), "( %type% :: * %type% ) ;"))
        {
            namespaceStart = tok->tokAt(offset + 1);
            namespaceEnd = tok->tokAt(offset + 2);
            ptrMember = true;
            typeName = tok->tokAt(offset + 4);
            tok = tok->tokAt(offset + 6);
        }

        // unhandled typedef, skip it and continue
        else
        {
            unsupportedTypedef(typeDef);
            tok = deleteInvalidTypedef(typeDef);
            continue;
        }

        bool done = false;
        bool ok = true;

        while (!done)
        {
            std::string pattern = typeName->str();
            int scope = 0;
            bool inScope = true;
            bool exitThisScope = false;
            int exitScope = 0;
            bool simplifyType = false;
            bool inMemberFunc = false;
            int memberScope = 0;
            std::size_t classLevel = spaceInfo.size();

            for (Token *tok2 = tok; tok2; tok2 = tok2->next())
            {
                // check for end of scope
                if (tok2->str() == "}")
                {
                    // check for end of member function
                    if (inMemberFunc)
                    {
                        memberScope--;
                        if (memberScope == 0)
                            inMemberFunc = false;
                    }

                    if (classLevel > 0 && tok2 == spaceInfo[classLevel - 1].classEnd)
                    {
                        --classLevel;
                        pattern.clear();

                        for (std::size_t i = classLevel; i < spaceInfo.size(); i++)
                            pattern += (spaceInfo[i].className + " :: ");

                        pattern += typeName->str();
                    }
                    else
                    {
                        scope--;
                        if (scope < 0)
                            inScope = false;

                        if (exitThisScope)
                        {
                            if (scope < exitScope)
                                exitThisScope = false;
                        }
                    }
                }

                // check for operator typedef
                /** @todo add support for multi-token operators */
                else if (tok2->str() == "operator" &&
                         tok2->next()->str() == typeName->str() &&
                         tok2->strAt(2) == "(" &&
                         Token::Match(tok2->tokAt(2)->link(), ") const| {"))
                {
                    // check for qualifier
                    if (tok2->previous()->str() == "::")
                    {
                        // check for available and matching class name
                        if (!spaceInfo.empty() && classLevel < spaceInfo.size() &&
                            tok2->strAt(-2) == spaceInfo[classLevel].className)
                        {
                            tok2 = tok2->next();
                            simplifyType = true;
                        }
                    }
                }

                // check for member functions
                else if (Token::Match(tok2, ") const| {"))
                {
                    const Token *func = tok2->link()->previous();

                    /** @todo add support for multi-token operators */
                    if (func->previous()->str() == "operator")
                        func = func->previous();

                    // check for qualifier
                    if (func->previous()->str() == "::")
                    {
                        // check for available and matching class name
                        if (!spaceInfo.empty() && classLevel < spaceInfo.size() &&
                            func->strAt(-2) == spaceInfo[classLevel].className)
                        {
                            memberScope = 0;
                            inMemberFunc = true;
                        }
                    }
                }

                // check for entering a new scope
                else if (tok2->str() == "{")
                {
                    // keep track of scopes within member function
                    if (inMemberFunc)
                        memberScope++;

                    scope++;
                }

                // check for typedef that can be substituted
                else if (Token::Match(tok2, pattern.c_str()) ||
                         (inMemberFunc && tok2->str() == typeName->str()))
                {
                    std::string pattern1;

                    // member function class variables don't need qualification
                    if (inMemberFunc && tok2->str() == typeName->str())
                        pattern1 = tok2->str();
                    else
                        pattern1 = pattern;

                    if (pattern1.find("::") != std::string::npos) // has a "something ::"
                    {
                        for (std::size_t i = classLevel; i < spaceInfo.size(); i++)
                        {
                            tok2->deleteNext();
                            tok2->deleteNext();
                        }
                        simplifyType = true;
                    }
                    else if ((inScope && !exitThisScope) || inMemberFunc)
                    {
                        if (Token::simpleMatch(tok2->previous(), "::"))
                        {
                            // Don't replace this typename if it's preceded by "::" unless it's a namespace
                            if (!spaceInfo.empty() && (tok2->tokAt(-2)->str() == spaceInfo[0].className) && spaceInfo[0].isNamespace)
                            {
                                tok2 = tok2->tokAt(-3);
                                tok2->deleteNext();
                                tok2->deleteNext();
                                tok2 = tok2->next();
                                simplifyType = true;
                            }
                        }
                        else if (Token::Match(tok2->previous(), "case %type% :"))
                        {
                            tok2 = tok2->next();
                        }
                        else if (duplicateTypedef(&tok2, typeName, typeDef))
                        {
                            exitScope = scope;

                            // skip to end of scope if not already there
                            if (tok2->str() != "}")
                            {
                                int level = 0;
                                while (tok2->next() && (tok2->next()->str() != "}" || level))
                                {
                                    if (tok2->next()->str() == "{")
                                        level++;
                                    else if (tok2->next()->str() == "}")
                                        level--;

                                    tok2 = tok2->next();
                                }
                            }
                        }
                        else if (tok2->previous()->str() != ".")
                        {
                            simplifyType = true;
                        }
                    }
                }

                if (simplifyType)
                {
                    // There are 2 categories of typedef substitutions:
                    // 1. variable declarations that preserve the variable name like
                    //    global, local, and function parameters
                    // 2. not variable declarations that have no name like derived
                    //    classes, casts, operators, and template parameters

                    // try to determine which category this substitution is
                    bool isDerived = false;
                    bool inCast = false;
                    bool inTemplate = false;
                    bool inOperator = false;
                    bool inSizeof = false;

                    // check for derived class: class A : some_typedef {
                    isDerived = Token::Match(tok2->previous(), "public|protected|private %type% {|,");

                    // check for cast: (some_typedef) A or static_cast<some_typedef>(A)
                    // todo: check for more complicated casts like: (const some_typedef *)A
                    if ((tok2->previous()->str() == "(" && tok2->next()->str() == ")" && tok2->strAt(-2) != "sizeof") ||
                        (tok2->previous()->str() == "<" && Token::simpleMatch(tok2->next(), "> (")))
                        inCast = true;

                    // check for template parameters: t<some_typedef> t1
                    else if (Token::Match(tok2->previous(), "<|,") &&
                             Token::Match(tok2->next(), "&|*| &|*| >|,"))
                        inTemplate = true;

                    else if (Token::Match(tok2->tokAt(-2), "sizeof ( %type% )"))
                        inSizeof = true;

                    // check for operator
                    if (Token::simpleMatch(tok2->previous(), "operator") ||
                        Token::simpleMatch(tok2->tokAt(-2), "operator const"))
                        inOperator = true;

                    // skip over class or struct in derived class declaration
                    if (isDerived && Token::Match(typeStart, "class|struct"))
                        typeStart = typeStart->next();

                    // start substituting at the typedef name by replacing it with the type
                    tok2->str(typeStart->str());
                    tok2 = copyTokens(tok2, typeStart->next(), typeEnd);

                    if (!pointers.empty())
                    {
                        std::list<std::string>::const_iterator iter;
                        for (iter = pointers.begin(); iter != pointers.end(); ++iter)
                        {
                            tok2->insertToken(*iter);
                            tok2 = tok2->next();
                        }
                    }

                    if (funcStart && funcEnd)
                    {
                        tok2->insertToken("(");
                        tok2 = tok2->next();
                        Token *tok3 = tok2;
                        tok2 = copyTokens(tok2, funcStart, funcEnd);

                        if (!inCast)
                            tok2 = processFunc(tok2, inOperator);

                        tok2->insertToken(")");
                        tok2 = tok2->next();
                        Token::createMutualLinks(tok2, tok3);

                        tok2 = copyTokens(tok2, argStart, argEnd);

                        if (specStart)
                        {
                            Token *spec = specStart;
                            tok2->insertToken(spec->str());
                            tok2 = tok2->next();
                            while (spec != specEnd)
                            {
                                spec = spec->next();
                                tok2->insertToken(spec->str());
                                tok2 = tok2->next();
                            }
                        }
                    }

                    else if (functionPtr || functionRef || function)
                    {
                        // don't add parenthesis around function names because it
                        // confuses other simplifications
                        bool needParen = true;
                        if (!inTemplate && function && tok2->next()->str() != "*")
                            needParen = false;
                        if (needParen)
                        {
                            tok2->insertToken("(");
                            tok2 = tok2->next();
                        }
                        Token *tok3 = tok2;
                        if (namespaceStart)
                        {
                            const Token *tok4 = namespaceStart;

                            while (tok4 != namespaceEnd)
                            {
                                tok2->insertToken(tok4->str());
                                tok2 = tok2->next();
                                tok4 = tok4->next();
                            }
                            tok2->insertToken(namespaceEnd->str());
                            tok2 = tok2->next();
                        }
                        if (functionPtr)
                        {
                            tok2->insertToken("*");
                            tok2 = tok2->next();
                        }
                        else if (functionRef)
                        {
                            tok2->insertToken("&");
                            tok2 = tok2->next();
                        }

                        if (const1)
                        {
                            tok2->insertToken(const1->str());
                            tok2 = tok2->next();
                            if (const2)
                            {
                                tok2->insertToken(const2->str());
                                tok2 = tok2->next();
                            }
                        }

                        if (!inCast)
                            tok2 = processFunc(tok2, inOperator);

                        if (needParen)
                        {
                            tok2->insertToken(")");
                            tok2 = tok2->next();
                            Token::createMutualLinks(tok2, tok3);
                        }

                        tok2 = copyTokens(tok2, argStart, argEnd);

                        if (inTemplate)
                            tok2 = tok2->next();

                        if (specStart)
                        {
                            Token *spec = specStart;
                            tok2->insertToken(spec->str());
                            tok2 = tok2->next();
                            while (spec != specEnd)
                            {
                                spec = spec->next();
                                tok2->insertToken(spec->str());
                                tok2 = tok2->next();
                            }
                        }
                    }
                    else if (functionRetFuncPtr || functionPtrRetFuncPtr)
                    {
                        tok2->insertToken("(");
                        tok2 = tok2->next();
                        Token *tok3 = tok2;
                        tok2->insertToken("*");
                        tok2 = tok2->next();

                        Token * tok4 = 0;
                        if (functionPtrRetFuncPtr)
                        {
                            tok2->insertToken("(");
                            tok2 = tok2->next();
                            tok4 = tok2;
                            tok2->insertToken("*");
                            tok2 = tok2->next();
                        }

                        // skip over variable name if there
                        if (!inCast)
                        {
                            if (tok2->next()->str() != ")")
                                tok2 = tok2->next();
                        }

                        if (tok4 && functionPtrRetFuncPtr)
                        {
                            tok2->insertToken(")");
                            tok2 = tok2->next();
                            Token::createMutualLinks(tok2, tok4);
                        }

                        tok2 = copyTokens(tok2, argStart, argEnd);

                        tok2->insertToken(")");
                        tok2 = tok2->next();
                        Token::createMutualLinks(tok2, tok3);

                        tok2 = copyTokens(tok2, argFuncRetStart, argFuncRetEnd);
                    }
                    else if (ptrToArray || refToArray)
                    {
                        tok2->insertToken("(");
                        tok2 = tok2->next();
                        Token *tok3 = tok2;

                        if (ptrToArray)
                            tok2->insertToken("*");
                        else
                            tok2->insertToken("&");
                        tok2 = tok2->next();

                        // skip over name
                        if (tok2->next()->str() != ")")
                        {
                            if (tok2->next()->str() != "(")
                                tok2 = tok2->next();

                            // check for function and skip over args
                            if (tok2->next()->str() == "(")
                                tok2 = tok2->next()->link();

                            // check for array
                            if (tok2->next()->str() == "[")
                                tok2 = tok2->next()->link();
                        }
                        else
                        {
                            // syntax error
                        }

                        tok2->insertToken(")");
                        Token::createMutualLinks(tok2->next(), tok3);
                    }
                    else if (ptrMember)
                    {
                        if (Token::simpleMatch(tok2, "* ("))
                        {
                            tok2->insertToken("*");
                            tok2 = tok2->next();
                        }
                        else
                        {
                            tok2->insertToken("(");
                            tok2 = tok2->next();
                            Token *tok3 = tok2;

                            const Token *tok4 = namespaceStart;

                            while (tok4 != namespaceEnd)
                            {
                                tok2->insertToken(tok4->str());
                                tok2 = tok2->next();
                                tok4 = tok4->next();
                            }
                            tok2->insertToken(namespaceEnd->str());
                            tok2 = tok2->next();

                            tok2->insertToken("*");
                            tok2 = tok2->next();

                            // skip over name
                            tok2 = tok2->next();

                            tok2->insertToken(")");
                            tok2 = tok2->next();
                            Token::createMutualLinks(tok2, tok3);
                        }
                    }
                    else if (typeOf)
                    {
                        tok2 = copyTokens(tok2, argStart, argEnd);
                    }
                    else if (tok2->tokAt(2) && tok2->tokAt(2)->str() == "[")
                    {
                        while (tok2->tokAt(2) && tok2->tokAt(2)->str() == "[")
                            tok2 = tok2->tokAt(2)->link()->previous();
                    }

                    if (arrayStart && arrayEnd)
                    {
                        do
                        {
                            if (!inCast && !inSizeof)
                                tok2 = tok2->next();

                            // reference to array?
                            if (tok2->str() == "&")
                            {
                                tok2 = tok2->previous();
                                tok2->insertToken("(");
                                tok2 = tok2->tokAt(3);
                                tok2->insertToken(")");
                                tok2 = tok2->next();
                                Token::createMutualLinks(tok2, tok2->tokAt(-3));
                            }

                            tok2 = copyTokens(tok2, arrayStart, arrayEnd);
                            tok2 = tok2->next();

                            if (tok2->str() == "=")
                            {
                                if (tok2->next()->str() == "{")
                                    tok2 = tok2->next()->link()->next();
                                else if (tok2->next()->str().at(0) == '\"')
                                    tok2 = tok2->next()->next();
                            }
                        }
                        while (Token::Match(tok2, ", %var% ;|'|=|,"));
                    }

                    simplifyType = false;
                }
            }

            if (tok->str() == ";")
                done = true;
            else if (tok->str() == ",")
            {
                arrayStart = 0;
                arrayEnd = 0;
                offset = 1;
                pointers.clear();

                while (Token::Match(tok->tokAt(offset), "*|&"))
                    pointers.push_back(tok->tokAt(offset++)->str());

                if (Token::Match(tok->tokAt(offset), "%type%"))
                {
                    typeName = tok->tokAt(offset++);

                    if (tok->tokAt(offset) && tok->tokAt(offset)->str() == "[")
                    {
                        arrayStart = tok->tokAt(offset);

                        bool atEnd = false;
                        while (!atEnd)
                        {
                            while (tok->tokAt(offset + 1) && !Token::Match(tok->tokAt(offset + 1), ";|,"))
                                offset++;

                            if (!tok->tokAt(offset + 1))
                                return; // invalid input
                            else if (tok->tokAt(offset + 1)->str() == ";")
                                atEnd = true;
                            else if (tok->tokAt(offset)->str() == "]")
                                atEnd = true;
                            else
                                offset++;
                        }

                        arrayEnd = tok->tokAt(offset++);
                    }

                    if (Token::Match(tok->tokAt(offset), ";|,"))
                        tok = tok->tokAt(offset);
                    else
                    {
                        // we encountered a typedef we don't support yet so just continue
                        done = true;
                        ok = false;
                    }
                }
                else
                {
                    // we encountered a typedef we don't support yet so just continue
                    done = true;
                    ok = false;
                }
            }
            else
            {
                // something is really wrong (internal error)
                done = true;
                ok = false;
            }
        }

        if (ok)
        {
            // remove typedef but leave ;
            while (typeDef->next() && typeDef->next() != tok)
                typeDef->deleteNext();

            if (typeDef != _tokens)
            {
                tok = typeDef->previous();
                tok->deleteNext();
            }
            else
            {
                _tokens->deleteThis();
                tok = _tokens;
            }
        }
    }
}

bool Tokenizer::tokenize(std::istream &code,
                         const char FileName[],
                         const std::string &configuration,
                         const bool preprocessorCondition)
{
    // make sure settings specified
    assert(_settings);

    _configuration = configuration;

    // The "_files" vector remembers what files have been tokenized..
    _files.push_back(Path::simplifyPath(FileName));

    createTokens(code);

    // Convert C# code
    if (_files[0].find(".cs"))
    {
        for (Token *tok = _tokens; tok; tok = tok->next())
        {
            if (Token::Match(tok, "[;{}] %type% [ ] %var% [=;]"))
            {
                tok = tok->next()->next();
                tok->str("*");
                tok->deleteNext();
            }
        }
    }

    // if MACRO
    for (const Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "if|for|while %var% ("))
        {
            syntaxError(tok);
            return false;
        }
    }

    // Simplify JAVA/C# code
    if (isJavaOrCSharp() && _files[0].find(".java") != std::string::npos)
    {
        for (Token *tok = _tokens; tok; tok = tok->next())
        {
            if (Token::Match(tok, ") throws %var% {"))
                Token::eraseTokens(tok, tok->tokAt(3));
            else if (tok->str() == "private")
                tok->str("private:");
            else if (tok->str() == "protected")
                tok->str("protected:");
            else if (tok->str() == "public")
                tok->str("public:");
        }
    }

    // Replace NULL with 0..
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (tok->str() == "NULL" || tok->str() == "__null" ||
            tok->str() == "'\\0'" || tok->str() == "'\\x0'")
        {
            tok->str("0");
        }
        else if (tok->isNumber() &&
                 MathLib::isInt(tok->str()) &&
                 MathLib::toLongNumber(tok->str()) == 0)
        {
            tok->str("0");
        }
    }


    // replace inline SQL with "asm()" (Oracle PRO*C). Ticket: #1959
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::simpleMatch(tok, "EXEC SQL"))
        {
            // delete all tokens until ";"
            const Token *end = tok;
            while (end && end->str() != ";")
                end = end->next();
            Token::eraseTokens(tok, end);

            if (tok)
            {
                // insert "asm ( ) ;"
                tok->str("asm");
                tok->insertToken("(");
                tok = tok->next();
                tok->insertToken(")");
            }
        }
    }

    if (!createLinks())
    {
        // Source has syntax errors, can't proceed
        return false;
    }

    // check for simple syntax errors..
    for (const Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::simpleMatch(tok, "> struct {") &&
            Token::simpleMatch(tok->tokAt(2)->link(), "} ;"))
        {
            syntaxError(tok);
            deallocateTokens();
            return false;
        }
    }

    // remove some unhandled macros in global scope
    removeMacrosInGlobalScope();

    // specify array size..
    arraySize();

    // simplify labels..
    labels();

    simplifyDoWhileAddBraces();
    simplifyIfAddBraces();

    // Combine "- %num%" ..
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "[([+-*/=,] - %num%") && tok->strAt(2)[0] != '-')
        {
            tok->next()->str(std::string("-") + tok->strAt(2));
            tok->next()->deleteNext();
        }

        if (Token::Match(tok, "return|case - %num%") && tok->strAt(2)[0] != '-')
        {
            tok->next()->str(std::string("-") + tok->strAt(2));
            tok->next()->deleteNext();
        }
    }

    // Combine tokens..
    for (Token *tok = _tokens; tok && tok->next(); tok = tok->next())
    {
        const char c1 = tok->str()[0];

        if (tok->str().length() == 1 && tok->next()->str().length() == 1)
        {
            const char c2 = tok->next()->str()[0];

            // combine equal tokens..
            if (c1 == c2 && (c1 == '<' || c1 == '|' || c1 == ':'))
            {
                tok->str(tok->str() + c2);
                tok->deleteNext();
                if (c1 == '<' && Token::simpleMatch(tok->next(), "="))
                {
                    tok->str("<<=");
                    tok->deleteNext();
                }
                continue;
            }

            // combine +-*/ and =
            else if (c2 == '=' && (strchr("+-*/%&|^=!<>", c1)))
            {
                tok->str(tok->str() + c2);
                tok->deleteNext();
                continue;
            }

            // replace "->" with "."
            else if (c1 == '-' && c2 == '>')
            {
                tok->str(".");
                tok->deleteNext();
                continue;
            }
        }

        else if (tok->str() == ">>" && tok->next()->str() == "=")
        {
            tok->str(">>=");
            tok->deleteNext();
        }

        else if ((c1 == 'p' || c1 == '_') && tok->next()->str() == ":")
        {
            if (tok->str() == "private" || tok->str() == "protected" || tok->str() == "public" || tok->str() == "__published")
            {
                tok->str(tok->str() + ":");
                tok->deleteNext();
                continue;
            }
        }
    }

    // ";a+=b;" => ";a=a+b;"
    simplifyCompoundAssignment();

    // check for more complicated syntax errors when using templates..
    if (!preprocessorCondition)
    {
        for (const Token *tok = _tokens; tok; tok = tok->next())
        {
            // skip executing scopes..
            if (Token::Match(tok, ") const| {") || Token::simpleMatch(tok, ", {"))
            {
                while (tok->str() != "{")
                    tok = tok->next();
                tok = tok->link();
                if (!tok)
                    break;
            }

            // skip executing scopes (ticket #1984)..
            if (Token::simpleMatch(tok, "; {"))
            {
                tok = tok->next()->link();
                if (!tok)
                    break;
            }

            // skip executing scopes (ticket #1985)..
            if (Token::simpleMatch(tok, "try {"))
            {
                tok = tok->next()->link();
                while (Token::simpleMatch(tok, "} catch ("))
                {
                    tok = tok->tokAt(2)->link();
                    if (Token::simpleMatch(tok, ") {"))
                        tok = tok->next()->link();
                }
                if (!tok)
                    break;
            }

            // not start of statement?
            if (tok->previous() && !Token::Match(tok, "[;{}]"))
                continue;

            // skip starting tokens.. ;;; typedef typename foo::bar::..
            while (Token::Match(tok, "[;{}]"))
                tok = tok->next();
            while (Token::Match(tok, "typedef|typename"))
                tok = tok->next();
            while (Token::Match(tok, "%type% ::"))
                tok = tok->tokAt(2);
            if (!tok)
                break;

            // template variable or type..
            if (Token::Match(tok, "%type% <"))
            {
                // these are used types..
                std::set<std::string> usedtypes;

                // parse this statement and see if the '<' and '>' are matching
                unsigned int level = 0;
                for (const Token *tok2 = tok; tok2 && !Token::Match(tok2, "[;{}]"); tok2 = tok2->next())
                {
                    if (tok2->str() == "(")
                        tok2 = tok2->link();
                    else if (tok2->str() == "<")
                    {
                        bool inclevel = false;
                        if (Token::simpleMatch(tok2->previous(), "operator <"))
                            ;
                        else if (level == 0)
                            inclevel = true;
                        else if (tok2->next()->isStandardType())
                            inclevel = true;
                        else if (Token::simpleMatch(tok2, "< typename"))
                            inclevel = true;
                        else if (Token::Match(tok2->tokAt(-2), "<|, %type% <") && usedtypes.find(tok2->strAt(-1)) != usedtypes.end())
                            inclevel = true;
                        else if (Token::Match(tok2, "< %type%") && usedtypes.find(tok2->strAt(1)) != usedtypes.end())
                            inclevel = true;
                        else if (Token::Match(tok2, "< %type%"))
                        {
                            // is the next token a type and not a variable/constant?
                            // assume it's a type if there comes another "<"
                            const Token *tok3 = tok2->next();
                            while (Token::Match(tok3, "%type% ::"))
                                tok3 = tok3->tokAt(2);
                            if (Token::Match(tok3, "%type% <"))
                                inclevel = true;
                        }

                        if (inclevel)
                        {
                            ++level;
                            if (Token::Match(tok2->tokAt(-2), "<|, %type% <"))
                                usedtypes.insert(tok2->strAt(-1));
                        }
                    }
                    else if (tok2->str() == ">")
                    {
                        if (level > 0)
                            --level;
                    }
                    else if (tok2->str() == ">>")
                    {
                        if (level > 0)
                            --level;
                        if (level > 0)
                            --level;
                    }
                }
                if (level > 0)
                {
                    syntaxError(tok);
                    deallocateTokens();
                    return false;
                }
            }
        }
    }


    // Remove "= default|delete" inside class|struct definitions
    // Todo: Remove it if it is used "externally" too.
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "struct|class %var% :|{"))
        {
            unsigned int indentlevel = 0;
            for (Token * tok2 = tok->tokAt(2); tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "{")
                    ++indentlevel;
                else if (tok2->str() == "}")
                {
                    if (indentlevel <= 1)
                        break;
                    --indentlevel;
                }
                else if (indentlevel == 1 && Token::Match(tok2, ") = delete|default ;"))
                {
                    Token * const end = tok2->tokAt(4);
                    tok2 = tok2->link()->previous();

                    // operator ==|>|<|..
                    if (Token::Match(tok2->previous(), "operator %any%"))
                        tok2 = tok2->previous();
                    else if (Token::simpleMatch(tok2->tokAt(-2), "operator [ ]"))
                        tok2 = tok2->tokAt(-2);
                    else if (Token::simpleMatch(tok2->tokAt(-2), "operator ( )"))
                        tok2 = tok2->tokAt(-2);
                    else if (Token::simpleMatch(tok2->tokAt(-3), "operator delete [ ]"))
                        tok2 = tok2->tokAt(-3);

                    while ((tok2->isName() && tok2->str().find(":") == std::string::npos) ||
                           Token::Match(tok2, "[&*~]"))
                        tok2 = tok2->previous();
                    if (Token::Match(tok2, "[;{}]") || tok2->isName())
                        Token::eraseTokens(tok2, end);
                    tok2 = end;
                }
            }
        }
    }

    // Remove __declspec()
    simplifyDeclspec();

    // remove calling conventions __cdecl, __stdcall..
    simplifyCallingConvention();

    // remove __attribute__((?))
    simplifyAttribute();

    // remove unnecessary member qualification..
    removeUnnecessaryQualification();

    // remove Microsoft MFC..
    simplifyMicrosoftMFC();

    // Remove Qt signals and slots
    simplifyQtSignalsSlots();

    // remove Borland stuff..
    simplifyBorland();

    // Remove "volatile", "inline", "register", and "restrict"
    simplifyKeyword();

    // Remove __builtin_expect, likely and unlikely
    simplifyBuiltinExpect();

    // #2449: syntax error: enum with typedef in it
    for (const Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "enum %var% {"))
        {
            for (const Token *tok2 = tok->tokAt(3); tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "typedef")
                {
                    syntaxError(tok2);
                    deallocateTokens();
                    return false;
                }
                else if (tok2->str() == "}")
                {
                    break;
                }
            }
        }
    }

    // typedef..
    simplifyTypedef();

    // catch bad typedef canonicalization
    if (!validate())
    {
        // Source has syntax errors, can't proceed
        return false;
    }

    // enum..
    simplifyEnum();

    // Remove __asm..
    simplifyAsm();

    // When the assembly code has been cleaned up, no @ is allowed
    for (const Token *tok = _tokens; tok; tok = tok->next())
    {
        if (tok->str() == "(")
            tok = tok->link();
        else if (tok->str()[0] == '@')
        {
            deallocateTokens();
            return false;
        }
    }

    // collapse compound standard types into a single token
    // unsigned long long int => long _isUnsigned=true,_isLong=true
    simplifyStdType();

    // simplify bit fields..
    simplifyBitfields();

    // Use "<" comparison instead of ">"
    simplifyComparisonOrder();

    /**
     * @todo simplify "for"
     * - move out start-statement "for (a;b;c);" => "{ a; for(;b;c); }"
     * - try to change "for" loop to a "while" loop instead
     */

    simplifyConst();

    // struct simplification "struct S {} s; => struct S { } ; S s ;
    simplifyStructDecl();

    // struct initialization (must be used before simplifyVarDecl)
    simplifyStructInit();

    // Change initialisation of variable to assignment
    simplifyInitVar();

    // Split up variable declarations.
    simplifyVarDecl();

    // f(x=g())   =>   x=g(); f(x)
    simplifyAssignmentInFunctionCall();

    simplifyVariableMultipleAssign();

    // Remove redundant parentheses
    simplifyRedundantParanthesis();

    // Handle templates..
    simplifyTemplates();

    // Simplify templates.. sometimes the "simplifyTemplates" fail and
    // then unsimplified function calls etc remain. These have the
    // "wrong" syntax. So this function will just fix so that the
    // syntax is corrected.
    simplifyTemplates2();

    // Simplify the operator "?:"
    simplifyConditionOperator();

    // remove exception specifications..
    removeExceptionSpecifications(_tokens);

    // Collapse operator name tokens into single token
    // operator = => operator=
    simplifyOperatorName();

    // simplify function pointers
    simplifyFunctionPointers();

    // "if (not p)" => "if (!p)"
    // "if (p and q)" => "if (p && q)"
    // "if (p or q)" => "if (p || q)"
    while (simplifyLogicalOperators()) { }

    // Change initialisation of variable to assignment
    simplifyInitVar();

    // Split up variable declarations.
    simplifyVarDecl();

    if (!preprocessorCondition)
    {
        setVarId();

        // Change initialisation of variable to assignment
        simplifyInitVar();
    }

    _tokens->assignProgressValues();

    removeRedundantSemicolons();

    return validate();
}
//---------------------------------------------------------------------------

/** Specify array size if it hasn't been given */

void Tokenizer::arraySize()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "%var% [ ] = { %str% }"))
        {
            tok->tokAt(4)->deleteThis();
            tok->tokAt(5)->deleteThis();
        }

        if (Token::Match(tok, "%var% [ ] = {"))
        {
            unsigned int sz = 1;
            const Token *tok2 = tok->tokAt(5);
            while (Token::Match(tok2, "%any% ,"))
            {
                if (tok2->isName())
                    break;
                sz++;
                tok2 = tok2->tokAt(2);
            }

            if (!tok2->isName() && Token::Match(tok2, "%any% } ;"))
                tok->next()->insertToken(MathLib::toString<unsigned int>(sz));
        }

        else if (Token::Match(tok, "%var% [ ] = %str% ;"))
        {
            std::size_t sz = tok->strAt(4).length() - 1;
            tok->next()->insertToken(MathLib::toString<unsigned int>((unsigned int)sz));
        }
    }
}

/** simplify labels in the code.. add an ";" */

void Tokenizer::labels()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, ") const| {"))
        {
            // Simplify labels in the executable scope..
            unsigned int indentlevel = 0;
            while (0 != (tok = tok->next()))
            {
                // indentations..
                if (tok->str() == "{")
                    ++indentlevel;
                else if (tok->str() == "}")
                {
                    if (indentlevel <= 1)
                        break;
                    --indentlevel;
                }

                // simplify label..
                if (Token::Match(tok, "[;{}] %var% : %var%"))
                {
                    if (!Token::Match(tok->next(), "public|protected|private"))
                        tok->tokAt(2)->insertToken(";");
                }
            }
        }
    }
}


/**
 * is the token pointing at a template parameters block
 * < int , 3 > => yes
 * \param tok start token that must point at "<"
 * \return number of parameters (invalid parameters => 0)
 */
static unsigned int templateParameters(const Token *tok)
{
    unsigned int numberOfParameters = 0;

    if (!tok)
        return 0;
    if (tok->str() != "<")
        return 0;
    tok = tok->next();

    while (tok)
    {
        ++numberOfParameters;

        // skip std::
        while (Token::Match(tok, "%var% ::"))
            tok = tok->tokAt(2);
        if (!tok)
            return 0;

        // num/type ..
        if (!tok->isNumber() && !tok->isName())
            return 0;
        tok = tok->next();

        // optional "*"
        if (tok->str() == "*")
            tok = tok->next();

        // ,/>
        if (tok->str() == ">")
            return numberOfParameters;
        if (tok->str() != ",")
            break;
        tok = tok->next();
    }
    return 0;
}


/**
 * Remove "template < ..." they can cause false positives because they are not expanded
 */
static void removeTemplates(Token *tok)
{
    for (; tok; tok = tok->next())
    {
        if (! Token::simpleMatch(tok, "template <"))
            continue;

        for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next())
        {
            if (tok2->str() == "{")
            {
                tok2 = tok2->link();
                tok2 = tok2 ? tok2->next() : 0;
                Token::eraseTokens(tok, tok2);
                tok->str(";");
                break;
            }
            if (tok2->str() == "(")
            {
                tok2 = tok2->link();
                if (!tok2)
                    break;
            }
            if (tok2->str() == ";")
            {
                Token::eraseTokens(tok, tok2->next());
                tok->str(";");
                break;
            }
        }
    }
}

std::set<std::string> Tokenizer::simplifyTemplatesExpandSpecialized()
{
    std::set<std::string> expandedtemplates;

    // Locate specialized templates..
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (tok->str() != "template")
            continue;
        if (!Token::simpleMatch(tok->next(), "< >"))
            continue;

        // what kind of template is this?
        Token *tok2 = tok->tokAt(3);
        while (tok2 && (tok2->isName() || tok2->str() == "*"))
            tok2 = tok2->next();

        if (!templateParameters(tok2))
            continue;

        // unknown template.. bail out
        if (!tok2->previous()->isName())
            continue;

        tok2 = tok2->previous();
        std::string s;
        {
            std::ostringstream ostr;
            const Token *tok3 = tok2;
            for (tok3 = tok2; tok3 && tok3->str() != ">"; tok3 = tok3->next())
            {
                if (tok3 != tok2)
                    ostr << " ";
                ostr << tok3->str();
            }
            if (!Token::simpleMatch(tok3, "> ("))
                continue;
            s = ostr.str();
        }

        // save search pattern..
        const std::string pattern(s + " > (");

        // remove spaces to create new name
        while (s.find(" ") != std::string::npos)
            s.erase(s.find(" "), 1);
        const std::string name(s + ">");
        expandedtemplates.insert(name);

        // Rename template..
        Token::eraseTokens(tok2, Token::findmatch(tok2, "("));
        tok2->str(name);

        // delete the "template < >"
        tok->deleteThis();
        tok->deleteThis();
        tok->deleteThis();

        // Use this special template in the code..
        while (0 != (tok2 = const_cast<Token *>(Token::findmatch(tok2, pattern.c_str()))))
        {
            Token::eraseTokens(tok2, Token::findmatch(tok2, "("));
            tok2->str(name);
        }
    }

    return expandedtemplates;
}

std::list<Token *> Tokenizer::simplifyTemplatesGetTemplateDeclarations()
{
    std::list<Token *> templates;
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::simpleMatch(tok, "template <"))
        {
            // set member variable, the code has templates.
            // this info is used by checks
            _codeWithTemplates = true;

            for (const Token *tok2 = tok; tok2; tok2 = tok2->next())
            {
                // Just a declaration => ignore this
                if (tok2->str() == ";")
                    break;

                // Implementation => add to "templates"
                if (tok2->str() == "{")
                {
                    templates.push_back(tok);
                    break;
                }
            }
        }
    }
    return templates;
}

std::list<Token *> Tokenizer::simplifyTemplatesGetTemplateInstantiations()
{
    std::list<Token *> used;

    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        // template definition.. skip it
        if (Token::simpleMatch(tok, "template <"))
        {
            // Goto the end of the template definition
            for (; tok; tok = tok->next())
            {
                // skip inner '(' .. ')' and '{' .. '}'
                if (tok->str() == "{" || tok->str() == "(")
                {
                    // skip inner tokens. goto ')' or '}'
                    tok = tok->link();

                    // this should be impossible. but break out anyway
                    if (!tok)
                        break;

                    // the end '}' for the template definition => break
                    if (tok->str() == "}")
                        break;
                }

                // the end ';' for the template definition
                else if (tok->str() == ";")
                {
                    break;
                }
            }
            if (!tok)
                break;
        }
        else if (Token::Match(tok->previous(), "[({};=] %type% <") ||
                 Token::Match(tok->tokAt(-2), "[,:] private|protected|public %var% <"))
        {
            if (templateParameters(tok->next()))
                used.push_back(tok);
        }
    }

    return used;
}


void Tokenizer::simplifyTemplatesUseDefaultArgumentValues(const std::list<Token *> &templates,
        const std::list<Token *> &instantiations)
{
    for (std::list<Token *>::const_iterator iter1 = templates.begin(); iter1 != templates.end(); ++iter1)
    {
        // template parameters with default value has syntax such as:
        //     x = y
        // this list will contain all the '=' tokens for such arguments
        std::list<Token *> eq;

        // parameter number. 1,2,3,..
        std::size_t templatepar = 1;

        // the template classname. This will be empty for template functions
        std::string classname;

        // Scan template declaration..
        for (Token *tok = *iter1; tok; tok = tok->next())
        {
            // end of template parameters?
            if (tok->str() == ">")
            {
                if (Token::Match(tok, "> class|struct %var%"))
                    classname = tok->strAt(2);
                break;
            }

            // next template parameter
            if (tok->str() == ",")
                ++templatepar;

            // default parameter value
            else if (tok->str() == "=")
                eq.push_back(tok);
        }
        if (eq.empty() || classname.empty())
            continue;

        // iterate through all template instantiations
        for (std::list<Token *>::const_iterator iter2 = instantiations.begin(); iter2 != instantiations.end(); ++iter2)
        {
            Token *tok = *iter2;

            if (!Token::Match(tok, (classname + " < %any%").c_str()))
                continue;

            // count the parameters..
            unsigned int usedpar = 1;
            for (tok = tok->tokAt(3); tok; tok = tok->tokAt(2))
            {
                if (tok->str() == ">")
                    break;

                if (tok->str() == ",")
                    ++usedpar;

                else
                    break;
            }
            if (tok && tok->str() == ">")
            {
                tok = tok->previous();
                std::list<Token *>::const_iterator it = eq.begin();
                for (std::size_t i = (templatepar - eq.size()); it != eq.end() && i < usedpar; ++i)
                    ++it;
                while (it != eq.end())
                {
                    tok->insertToken(",");
                    tok = tok->next();
                    const Token *from = (*it)->next();
                    std::stack<Token *> links;
                    while (from && (!links.empty() || (from->str() != "," && from->str() != ">")))
                    {
                        tok->insertToken(from->str());
                        tok = tok->next();
                        if (Token::Match(tok, "(|["))
                            links.push(tok);
                        else if (!links.empty() && Token::Match(tok, ")|]"))
                        {
                            Token::createMutualLinks(links.top(), tok);
                            links.pop();
                        }
                        from = from->next();
                    }
                    ++it;
                }
            }
        }

        for (std::list<Token *>::iterator it = eq.begin(); it != eq.end(); ++it)
        {
            (*it)->deleteThis();
            (*it)->deleteThis();
        }
    }
}

/**
 * Match template declaration/instantiation
 * @param instance template instantiation
 * @param name name of template
 * @param numberOfArguments number of template arguments
 * @param patternAfter pattern that must match the tokens after the ">"
 * @return match => true
 */
static bool simplifyTemplatesInstantiateMatch(const Token *instance, const std::string &name, unsigned int numberOfArguments, const char patternAfter[])
{
    if (!Token::simpleMatch(instance, (name + " <").c_str()))
        return false;

    if (numberOfArguments != templateParameters(instance->next()))
        return false;

    if (patternAfter)
    {
        const Token *tok = Token::findmatch(instance, ">");
        if (!tok || !Token::Match(tok->next(), patternAfter))
            return false;
    }

    // nothing mismatching was found..
    return true;
}

void Tokenizer::simplifyTemplatesInstantiate(const Token *tok,
        std::list<Token *> &used,
        std::set<std::string> &expandedtemplates)
{
    // this variable is not used at the moment. the intention was to
    // allow continous instantiations until all templates has been expanded
    bool done = false;

    std::vector<const Token *> type;
    for (tok = tok->tokAt(2); tok && tok->str() != ">"; tok = tok->next())
    {
        if (Token::Match(tok, "%var% ,|>"))
            type.push_back(tok);
    }
    // bail out if the end of the file was reached
    if (!tok)
        return;

    // get the position of the template name
    unsigned char namepos = 0;
    if (Token::Match(tok, "> class|struct %type% {|:"))
        namepos = 2;
    else if (Token::Match(tok, "> %type% *|&| %type% ("))
        namepos = 2;
    else if (Token::Match(tok, "> %type% %type% *|&| %type% ("))
        namepos = 3;
    else
    {
        // debug message that we bail out..
        if (_settings->debugwarnings)
        {
            std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
            ErrorLogger::ErrorMessage::FileLocation loc;
            loc.line = tok->linenr();
            loc.setfile(file(tok));
            locationList.push_back(loc);

            const ErrorLogger::ErrorMessage errmsg(locationList,
                                                   Severity::debug,
                                                   "simplifyTemplates: bailing out",
                                                   "debug");

            if (_errorLogger)
                _errorLogger->reportErr(errmsg);
            else
                Check::reportError(errmsg);
        }
        return;
    }
    if ((tok->tokAt(namepos)->str() == "*" || tok->tokAt(namepos)->str() == "&"))
        ++namepos;

    // name of template function/class..
    const std::string name(tok->strAt(namepos));

    const bool isfunc(tok->strAt(namepos + 1) == "(");

    // locate template usage..
    std::string::size_type sz1 = used.size();
    unsigned int recursiveCount = 0;

    for (std::list<Token *>::const_iterator iter2 = used.begin(); iter2 != used.end(); ++iter2)
    {
        // If the size of "used" has changed, simplify calculations
        if (sz1 != used.size())
        {
            sz1 = used.size();
            simplifyCalculations();
            recursiveCount++;
            if (recursiveCount > 100)
            {
                // bail out..
                break;
            }
        }

        Token * const tok2 = *iter2;
        if (tok2->str() != name)
            continue;

        if (Token::Match(tok2->previous(), "[;{}=]") &&
            !simplifyTemplatesInstantiateMatch(*iter2, name, type.size(), isfunc ? "(" : "*| %var%"))
            continue;

        // New type..
        std::vector<const Token *> types2;
        std::string s;
        std::string s1(name + " < ");
        for (const Token *tok3 = tok2->tokAt(2); tok3 && tok3->str() != ">"; tok3 = tok3->next())
        {
            if (!tok3->next())
            {
                s.clear();
                break;
            }
            s1 += tok3->str();
            s1 += " ";
            if (Token::Match(tok3->previous(), "[<,]"))
                types2.push_back(tok3);
            // add additional type information
            if (tok3->isUnsigned())
                s += "unsigned";
            else if (tok3->isSigned())
                s += "signed";
            if (tok3->isLong())
                s += "long";
            s += tok3->str();
        }
        s1 += ">";
        const std::string type2(s);

        if (type2.empty() || type.size() != types2.size())
        {
            if (_settings->debugwarnings)
            {
                std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
                ErrorLogger::ErrorMessage::FileLocation loc;
                loc.line = tok2->linenr();
                loc.setfile(file(tok2));
                locationList.push_back(loc);

                const ErrorLogger::ErrorMessage errmsg(locationList,
                                                       Severity::debug,
                                                       "Failed to instantiate template. The checking continues anyway.",
                                                       "debug");

                _errorLogger->reportErr(errmsg);
            }
            if (type2.empty())
                continue;
            break;
        }

        // New classname/funcname..
        const std::string name2(name + "<" + type2 + ">");

        if (expandedtemplates.find(name2) == expandedtemplates.end())
        {
            expandedtemplates.insert(name2);
            // Copy template..
            int _indentlevel = 0;
            int _parlevel = 0;
            for (const Token *tok3 = _tokens; tok3; tok3 = tok3->next())
            {
                if (tok3->str() == "{")
                    ++_indentlevel;
                else if (tok3->str() == "}")
                    --_indentlevel;
                else if (tok3->str() == "(")
                    ++_parlevel;
                else if (tok3->str() == ")")
                    --_parlevel;

                // Start of template..
                if (tok3 == tok)
                {
                    tok3 = tok3->next();
                }

                // member function implemented outside class definition
                else if (_indentlevel == 0 &&
                         _parlevel == 0 &&
                         simplifyTemplatesInstantiateMatch(tok3, name, type.size(), ":: ~| %var% ("))
                {
                    addtoken(name2.c_str(), tok3->linenr(), tok3->fileIndex());
                    while (tok3->str() != "::")
                        tok3 = tok3->next();
                }

                // not part of template.. go on to next token
                else
                    continue;

                int indentlevel = 0;
                std::stack<Token *> braces;     // holds "{" tokens
                std::stack<Token *> brackets;   // holds "(" tokens
                std::stack<Token *> brackets2;  // holds "[" tokens

                for (; tok3; tok3 = tok3->next())
                {
                    if (tok3->str() == "{")
                        ++indentlevel;

                    else if (tok3->str() == "}")
                    {
                        if (indentlevel <= 1 && brackets.empty() && brackets2.empty())
                        {
                            // there is a bug if indentlevel is 0
                            // the "}" token should only be added if indentlevel is 1 but I add it always intentionally
                            // if indentlevel ever becomes 0, cppcheck will write:
                            // ### Error: Invalid number of character {
                            addtoken("}", tok3->linenr(), tok3->fileIndex());
                            Token::createMutualLinks(braces.top(), _tokensBack);
                            braces.pop();
                            break;
                        }
                        --indentlevel;
                    }


                    if (tok3->isName())
                    {
                        // search for this token in the type vector
                        unsigned int itype = 0;
                        while (itype < type.size() && type[itype]->str() != tok3->str())
                            ++itype;

                        // replace type with given type..
                        if (itype < type.size())
                        {
                            for (const Token *typetok = types2[itype];
                                 typetok && !Token::Match(typetok, "[,>]");
                                 typetok = typetok->next())
                            {
                                addtoken(typetok, tok3->linenr(), tok3->fileIndex());
                            }
                            continue;
                        }
                    }

                    // replace name..
                    if (Token::Match(tok3, (name + " !!<").c_str()))
                    {
                        addtoken(name2.c_str(), tok3->linenr(), tok3->fileIndex());
                        continue;
                    }

                    // copy
                    addtoken(tok3, tok3->linenr(), tok3->fileIndex());
                    if (Token::Match(tok3, "%type% <"))
                    {
                        if (!Token::Match(tok3, (name + " <").c_str()))
                            done = false;
                        used.push_back(_tokensBack);
                    }

                    // link() newly tokens manually
         