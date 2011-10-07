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
    else if (strncmp(str, "_Bool", 5) == 0)
    {
        str2 << "bool";
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
            return _settings->sizeof_long_double;
        else if (type->str() == "long")
            return _settings->sizeof_long_long;
    }

    return it->second;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// InsertTokens - Copy and insert tokens
//---------------------------------------------------------------------------

void Tokenizer::insertTokens(Token *dest, const Token *src, unsigned int n)
{
    std::stack<Token *> link;

    while (n > 0)
    {
        dest->insertToken(src->str());
        dest = dest->next();

        // Set links
        if (Token::Match(dest, "(|[|{"))
            link.push(dest);
        else if (!link.empty() && Token::Match(dest, ")|]|}"))
        {
            Token::createMutualLinks(dest, link.top());
            link.pop();
        }

        dest->fileIndex(src->fileIndex());
        dest->linenr(src->linenr());
        dest->varId(src->varId());
        dest->isName(src->isName());
        dest->isNumber(src->isNumber());
        dest->isBoolean(src->isBoolean());
        dest->isUnsigned(src->isUnsigned());
        dest->isSigned(src->isSigned());
        dest->isPointerCompare(src->isPointerCompare());
        dest->isLong(src->isLong());
        dest->isUnused(src->isUnused());
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
        tok2->isName(tok->isName());
        tok2->isNumber(tok->isNumber());
        tok2->isBoolean(tok->isBoolean());
        tok2->isUnsigned(tok->isUnsigned());
        tok2->isSigned(tok->isSigned());
        tok2->isPointerCompare(tok->isPointerCompare());
        tok2->isLong(tok->isLong());
        tok2->isUnused(tok->isUnused());
        tok2->varId(tok->varId());

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
            else if (ch=='&' && code.peek() == '&')
            {
                if (!CurrentToken.empty())
                {
                    addtoken(CurrentToken.c_str(), lineno, FileIndex, true);
                    CurrentToken.clear();
                }

                // &&
                ch = (char)code.get();
                addtoken("&&", lineno, FileIndex, true);
                continue;
            }
            else if (ch==':' && CurrentToken.empty() && code.peek() == ' ')
            {
                // :
                addtoken(":", lineno, FileIndex, true);
                CurrentToken.clear();
                continue;
            }
            else if (ch==':' && CurrentToken.empty() && code.peek() == ':')
            {
                // ::
                ch = (char)code.get();
                addtoken("::", lineno, FileIndex, true);
                CurrentToken.clear();
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
    if (tok1 && !(_settings->isEnabled("style")))
        return;

    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    std::string tok2_str;
    if (tok1 && tok2)
    {
        ErrorLogger::ErrorMessage::FileLocation loc;
        loc.line = tok1->linenr();
        loc.setfile(file(tok1));
        locationList.push_back(loc);
        loc.line = tok2->linenr();
        loc.setfile(file(tok2));
        locationList.push_back(loc);
        tok2_str = tok2->str();
    }
    else
        tok2_str = "name";

    const ErrorLogger::ErrorMessage errmsg(locationList,
                                           Severity::style,
                                           std::string(type + " '" + tok2_str +
                                                   "' hides typedef with same name"),
                                           "variableHidingTypedef",
                                           false);

    if (_errorLogger)
        _errorLogger->reportErr(errmsg);
    else
        Check::reportError(errmsg);
}

void Tokenizer::duplicateDeclarationError(const Token *tok1, const Token *tok2, const std::string &type)
{
    if (tok1 && !(_settings->isEnabled("style")))
        return;

    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    std::string tok2_str;
    if (tok1 && tok2)
    {
        ErrorLogger::ErrorMessage::FileLocation loc;
        loc.line = tok1->linenr();
        loc.setfile(file(tok1));
        locationList.push_back(loc);
        loc.line = tok2->linenr();
        loc.setfile(file(tok2));
        locationList.push_back(loc);
        tok2_str = tok2->str();
    }
    else
        tok2_str = "name";

    const ErrorLogger::ErrorMessage errmsg(locationList,
                                           Severity::style,
                                           std::string(type + " '" + tok2_str +
                                                   "' forward declaration unnecessary, already declared"),
                                           "unnecessaryForwardDeclaration",
                                           false);

    if (_errorLogger)
        _errorLogger->reportErr(errmsg);
    else
        Check::reportError(errmsg);
}

// check if this statement is a duplicate definition
bool Tokenizer::duplicateTypedef(Token **tokPtr, const Token *name, const Token *typeDef, bool undefinedStruct)
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
            if (end)
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
                            else if (tok->next()->str() == "{")
                            {
                                if (!undefinedStruct)
                                    duplicateTypedefError(*tokPtr, name, "Struct");
                                return true;
                            }
                            else if (Token::Match(tok->next(), ")|*"))
                            {
                                return true;
                            }
                            else if (tok->next()->str() == name->str())
                            {
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
                                           "debug",
                                           false);

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

    if (tok1->next() && tok1->next()->str() == ";" && tok1 && tok1->previous()->str() == "}")
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

        /** @todo add support for union */
        bool undefinedStruct = false;
        if (Token::Match(tok, "typedef enum|struct %type% %type% ;") && tok->strAt(2) == tok->strAt(3))
        {
            if (tok->strAt(1) == "enum")
            {
                tok->deleteThis();
                tok->deleteThis();
                tok->deleteThis();
                tok->deleteThis();
                continue;
            }
            else
            {
                const std::string pattern("struct " + tok->strAt(2) + " {|:");
                const Token *tok2 = Token::findmatch(_tokens, pattern.c_str(), tok);
                if (!tok2)
                    undefinedStruct = true;
            }
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

            while (Token::Match(tok->tokAt(offset), "const|signed|unsigned|struct|enum %type%") ||
                   (tok->tokAt(offset + 1) && tok->tokAt(offset + 1)->isStandardType()))
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

        // check for invalid input
        if (!tok->tokAt(offset))
        {
            syntaxError(tok);
            return;
        }

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

                // function pointer
                else if (Token::Match(tok->tokAt(offset), "( * %var% ) ("))
                {
                    // name token wasn't a name, it was part of the type
                    typeEnd = typeEnd->next();
                    functionPtr = true;
                    funcStart = tok->tokAt(offset + 1);
                    funcEnd = tok->tokAt(offset + 1);
                    typeName = tok->tokAt(offset + 2);
                    argStart = tok->tokAt(offset + 4);
                    argEnd = tok->tokAt(offset + 4)->link();
                    tok = argEnd->next();
                }

                // function
                else if (Token::Match(tok->tokAt(offset)->link(), ") const| ;|,"))
                {
                    function = true;
                    if (tok->tokAt(offset)->link()->next()->str() == "const")
                    {
                        specStart = tok->tokAt(offset)->link()->next();
                        specEnd = specStart;
                    }
                    argStart = tok->tokAt(offset);
                    argEnd = tok->tokAt(offset)->link();
                    tok = argEnd->next();
                    if (specStart)
                        tok = tok->next();
                }

                // syntax error
                else
                {
                    syntaxError(tok);
                    return;
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
        //           typedef ... (( .... type )( ... ));
        //           typedef ... ( * ( .... type )( ... ));
        else if ((tok->tokAt(offset)->str() == "(" &&
                  Token::Match(tok->tokAt(offset)->link()->previous(), "%type% ) (") &&
                  Token::Match(tok->tokAt(offset)->link()->next()->link(), ") const|volatile|;")) ||
                 (Token::simpleMatch(tok->tokAt(offset), "( (") &&
                  Token::Match(tok->tokAt(offset + 1)->link()->previous(), "%type% ) (") &&
                  Token::Match(tok->tokAt(offset + 1)->link()->next()->link(), ") const|volatile| ) ;|,")) ||
                 (Token::simpleMatch(tok->tokAt(offset), "( * (") &&
                  Token::Match(tok->tokAt(offset + 2)->link()->previous(), "%type% ) (") &&
                  Token::Match(tok->tokAt(offset + 2)->link()->next()->link(), ") const|volatile| ) ;|,")))
        {
            if (tok->strAt(offset + 1) == "(")
                offset++;
            else if (Token::simpleMatch(tok->tokAt(offset), "( * ("))
            {
                pointers.push_back("*");
                offset += 2;
            }

            if (tok->tokAt(offset)->link()->strAt(-2) == "*")
                functionPtr = true;
            else
                function = true;
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
            if (tok->str() == ")")
                tok = tok->next();
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
        else if (Token::Match(tok->tokAt(offset), "( * ( * %type% ) (") &&
                 Token::Match(tok->tokAt(offset + 6)->link(), ") ) (") &&
                 Token::Match(tok->tokAt(offset + 6)->link()->tokAt(2)->link(), ") ;|,"))
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
                 Token::simpleMatch(tok->tokAt(offset + 3)->link(), ") ) (") &&
                 Token::Match(tok->tokAt(offset + 3)->link()->tokAt(2)->link(), ") ;|,"))
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
            bool globalScope = false;
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

                // check for entering a new namespace
                else if (Token::Match(tok2, "namespace %any% {"))
                {
                    if (classLevel < spaceInfo.size() &&
                        spaceInfo[classLevel].isNamespace &&
                        spaceInfo[classLevel].className == tok2->next()->str())
                    {
                        classLevel++;
                        pattern.clear();
                        for (std::size_t i = classLevel; i < spaceInfo.size(); i++)
                            pattern += (spaceInfo[i].className + " :: ");

                        pattern += typeName->str();
                    }
                    scope++;
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
                        if (Token::simpleMatch(tok2->previous(), "::"))
                        {
                            tok2->previous()->previous()->deleteNext();
                            globalScope = true;
                        }

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
                        else if (duplicateTypedef(&tok2, typeName, typeDef, undefinedStruct))
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
                    // can't simplify 'operator functionPtr ()' and 'functionPtr operator ... ()'
                    if (functionPtr && (tok2->previous()->str() == "operator" ||
                                        tok2->next()->str() == "operator"))
                    {
                        simplifyType = false;
                        tok2 = tok2->next();
                        break;
                    }

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
                    bool structRemoved = false;
                    if (isDerived && Token::Match(typeStart, "class|struct"))
                    {
                        if (typeStart->str() == "struct")
                            structRemoved = true;
                        typeStart = typeStart->next();
                    }

                    // start substituting at the typedef name by replacing it with the type
                    tok2->str(typeStart->str());

                    // restore qualification if it was removed
                    if (typeStart->str() == "struct" || structRemoved)
                    {
                        if (structRemoved)
                            tok2 = tok2->previous();

                        if (globalScope)
                        {
                            tok2->insertToken("::");
                            tok2 = tok2->next();
                        }

                        for (std::size_t i = classLevel; i < spaceInfo.size(); i++)
                        {
                            tok2->insertToken(spaceInfo[i].className);
                            tok2 = tok2->next();
                            tok2->insertToken("::");
                            tok2 = tok2->next();
                        }
                    }

                    // add remainder of type
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
                        if (!inTemplate && function && tok2->next() && tok2->next()->str() != "*")
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
                            if (!tok2->next())
                            {
                                syntaxError(tok2);
                                return; // can't recover so quit
                            }

                            if (!inCast && !inSizeof)
                                tok2 = tok2->next();

                            // reference to array?
                            if (tok2->str() == "&")
                            {
                                tok2 = tok2->previous();
                                tok2->insertToken("(");
                                Token *tok3 = tok2->next();

                                // handle missing variable name
                                if (tok2->strAt(3) == ")" || tok2->strAt(3) == ",")
                                    tok2 = tok2->tokAt(2);
                                else
                                    tok2 = tok2->tokAt(3);

                                tok2->insertToken(")");
                                tok2 = tok2->next();
                                Token::createMutualLinks(tok2, tok3);
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

void Tokenizer::simplifyMulAnd(void)
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "[;{}] *"))
        {
            //fix Ticket #2784
            if (Token::Match(tok->next(), "* & %any% ="))
            {
                tok->deleteNext(); //del *
                tok->deleteNext(); //del &
                continue;
            }
            if (Token::Match(tok->next(), "* ( & %any% ) ="))
            {
                tok->deleteNext(); //del *
                tok->deleteNext(); //del (
                tok->deleteNext(); //del &
                tok->next()->deleteNext(); //del )
                continue;
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

    // Fill the map _typeSize..
    _typeSize.clear();
    _typeSize["char"] = 1;
    _typeSize["bool"] = _settings->sizeof_bool;
    _typeSize["short"] = _settings->sizeof_short;
    _typeSize["int"] = _settings->sizeof_int;
    _typeSize["long"] = _settings->sizeof_long;
    _typeSize["float"] = _settings->sizeof_float;
    _typeSize["double"] = _settings->sizeof_double;
    _typeSize["size_t"] = _settings->sizeof_size_t;
    _typeSize["*"] = _settings->sizeof_pointer;

    _configuration = configuration;

    // The "_files" vector remembers what files have been tokenized..
    _files.push_back(Path::simplifyPath(FileName));

    createTokens(code);

    // replace __LINE__ macro with line number
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (tok->str() == "__LINE__")
            tok->str(MathLib::toString(tok->linenr()));
    }

    // token concatenation
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "%var%|%num% ## %var%|%num%"))
        {
            tok->str(tok->str() + tok->strAt(2));
            tok->deleteNext();
            tok->deleteNext();
            if (tok->previous())
                tok = tok->previous();
        }
    }

    // simplify '[;{}] * & %any% =' to '%any% ='
    simplifyMulAnd();

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
    if (isJavaOrCSharp())
    {
        const bool isJava(_files[0].find(".java") != std::string::npos);
        for (Token *tok = _tokens; tok; tok = tok->next())
        {
            if (isJava && Token::Match(tok, ") throws %var% {"))
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

    // specify array size..
    arraySize();

    // simplify labels..
    labels();

    simplifyDoWhileAddBraces();

    if (!simplifyIfAddBraces())
        return false;

    // Combine "- %num%" ..
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "?|:|,|(|[|=|return|case|%op% - %num%"))
        {
            tok->next()->str("-" + tok->strAt(2));
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

        else if ((c1 == 'p' || c1 == '_') && tok->next()->str() == ":" && tok->strAt(2) != ":")
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
                        else if (tok2->next() && tok2->next()->isStandardType())
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

    // remove some unhandled macros in global scope
    removeMacrosInGlobalScope();

    // remove calling conventions __cdecl, __stdcall..
    simplifyCallingConvention();

    // remove __attribute__((?))
    simplifyAttribute();

    // remove unnecessary member qualification..
    removeUnnecessaryQualification();

    // remove Microsoft MFC..
    simplifyMicrosoftMFC();

    // convert Microsoft memory functions
    simplifyMicrosoftMemoryFunctions();

    // convert Microsoft string functions
    simplifyMicrosoftStringFunctions();

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

    // convert Microsoft DEBUG_NEW macro to new
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (tok->str() == "DEBUG_NEW")
            tok->str("new");
    }

    // typedef..
    simplifyTypedef();

    // catch bad typedef canonicalization
    //
    // to reproduce bad typedef, download upx-ucl from:
    // http://packages.debian.org/sid/upx-ucl
    // analyse the file src/stub/src/i386-linux.elf.interp-main.c
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

    // convert platform dependent types to standard types
    // 32 bits: size_t -> unsigned long
    // 64 bits: size_t -> unsigned long long
    simplifyPlatformTypes();

    // collapse compound standard types into a single token
    // unsigned long long int => long _isUnsigned=true,_isLong=true
    simplifyStdType();

    // simplify bit fields..
    simplifyBitfields();

    // Use "<" comparison instead of ">"
    simplifyComparisonOrder();

    // Simplify '(p == 0)' to '(!p)'
    simplifyIfNot();
    simplifyIfNotNull();

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
    simplifyRedundantParenthesis();

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

    // Convert e.g. atol("0") into 0
    simplifyMathFunctions();

    // Convert + + into + and + - into -
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        while (tok->next())
        {
            if (tok->str() == "+")
            {
                if (tok->next()->str() == "+")
                {
                    tok->deleteNext();
                    continue;
                }
                else if (tok->next()->str() == "-")
                {
                    tok->str("-");
                    tok->deleteNext();
                    continue;
                }
            }
            else if (tok->str() == "-")
            {
                if (tok->next()->str() == "-")
                {
                    tok->str("+");
                    tok->deleteNext();
                    continue;
                }
                else if (tok->next()->str() == "+")
                {
                    tok->deleteNext();
                    continue;
                }
            }

            break;
        }
    }

    // 0[a] -> a[0]
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "%num% [ %var% ]"))
        {
            const std::string temp = tok->str();
            tok->str(tok->tokAt(2)->str());
            tok->tokAt(2)->str(temp);
        }
    }

    simplifyDeadCode();

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
                if (Token::Match(tok, "[;{}] %var% : (| *|&| %var%"))
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
            unsigned int level = 0;

            // Goto the end of the template definition
            for (; tok; tok = tok->next())
            {
                // skip '<' .. '>'
                if (tok->str() == "<")
                    ++level;
                else if (tok->str() == ">")
                {
                    if (level <= 1)
                        break;
                    --level;
                }

                // skip inner '(' .. ')' and '{' .. '}'
                else if (tok->str() == "{" || tok->str() == "(")
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
        else if (Token::Match(tok->previous(), "[({};=] %var% <") ||
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
                                                   "debug",
                                                   false);

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

        // #2648 - simple fix for sizeof used as template parameter
        // TODO: this is a bit hardcoded. make a bit more generic
        if (Token::Match(tok2, "%var% < sizeof ( %type% ) >") && tok2->tokAt(4)->isStandardType())
        {
            Token * const tok3 = tok2->next();
            const unsigned int sz = sizeOfType(tok3->tokAt(3));
            Token::eraseTokens(tok3, tok3->tokAt(5));
            tok3->insertToken(MathLib::toString<unsigned int>(sz));
        }

        if (Token::Match(tok2->previous(), "[;{}=]") &&
            !simplifyTemplatesInstantiateMatch(*iter2, name, type.size(), isfunc ? "(" : "*| %var%"))
            continue;

        // New type..
        std::vector<const Token *> types2;
        std::string s;
        std::string s1(name + " < ");
        for (const Token *tok3 = tok2->tokAt(2); tok3 && tok3->str() != ">"; tok3 = tok3->next())
        {
            // #2648 - unhandled parenthesis => bail out
            // #2721 - unhandled [ => bail out
            if (tok3->str() == "(" || tok3->str() == "[")
            {
                s.clear();
                break;
            }
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
                                                       "debug",
                                                       false);

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
                    if (tok3->str() == "{")
                    {
                        braces.push(_tokensBack);
                    }
                    else if (tok3->str() == "}")
                    {
                        assert(braces.empty() == false);
                        Token::createMutualLinks(braces.top(), _tokensBack);
                        braces.pop();
                    }
                    else if (tok3->str() == "(")
                    {
                        brackets.push(_tokensBack);
                    }
                    else if (tok3->str() == "[")
                    {
                        brackets2.push(_tokensBack);
                    }
                    else if (tok3->str() == ")")
                    {
                        assert(brackets.empty() == false);
                        Token::createMutualLinks(brackets.top(), _tokensBack);
                        brackets.pop();
                    }
                    else if (tok3->str() == "]")
                    {
                        assert(brackets2.empty() == false);
                        Token::createMutualLinks(brackets2.top(), _tokensBack);
                        brackets2.pop();
                    }

                }

                assert(braces.empty());
                assert(brackets.empty());
            }
        }

        // Replace all these template usages..
        std::list< std::pair<Token *, Token *> > removeTokens;
        for (Token *tok4 = tok2; tok4; tok4 = tok4->next())
        {
            if (Token::simpleMatch(tok4, s1.c_str()))
            {
                Token * tok5 = tok4->tokAt(2);
                unsigned int count = 0;
                const Token *typetok = (!types2.empty()) ? types2[0] : 0;
                while (tok5 && tok5->str() != ">")
                {
                    if (tok5->str() != ",")
                    {
                        if (!typetok ||
                            tok5->isUnsigned() != typetok->isUnsigned() ||
                            tok5->isSigned() != typetok->isSigned() ||
                            tok5->isLong() != typetok->isLong())
                        {
                            break;
                        }

                        typetok = typetok ? typetok->next() : 0;
                    }
                    else
                    {
                        ++count;
                        typetok = (count < types2.size()) ? types2[count] : 0;
                    }
                    tok5 = tok5->next();
                }

                // matching template usage => replace tokens..
                // Foo < int >  =>  Foo<int>
                if (tok5 && tok5->str() == ">" && count + 1U == types2.size())
                {
                    tok4->str(name2);
                    for (Token *tok6 = tok4->next(); tok6 != tok5; tok6 = tok6->next())
                    {
                        if (tok6->isName())
                            used.remove(tok6);
                    }
                    removeTokens.push_back(std::pair<Token*,Token*>(tok4, tok5->next()));
                }

                tok4 = tok5;
                if (!tok4)
                    break;
            }
        }
        while (!removeTokens.empty())
        {
            Token::eraseTokens(removeTokens.back().first, removeTokens.back().second);
            removeTokens.pop_back();
        }
    }
}

void Tokenizer::simplifyTemplates()
{
    std::set<std::string> expandedtemplates(simplifyTemplatesExpandSpecialized());

    // Locate templates..
    std::list<Token *> templates(simplifyTemplatesGetTemplateDeclarations());

    if (templates.empty())
    {
        removeTemplates(_tokens);
        return;
    }

    // There are templates..
    // Remove "typename" unless used in template arguments..
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (tok->str() == "typename")
            tok->deleteThis();

        if (Token::simpleMatch(tok, "template <"))
        {
            while (tok && tok->str() != ">")
                tok = tok->next();
            if (!tok)
                break;
        }
    }

    // Locate possible instantiations of templates..
    std::list<Token *> used(simplifyTemplatesGetTemplateInstantiations());

    // No template instantiations? Then remove all templates.
    if (used.empty())
    {
        removeTemplates(_tokens);
        return;
    }

    // Template arguments with default values
    simplifyTemplatesUseDefaultArgumentValues(templates, used);

    // expand templates
    bool done = false;
    //while (!done)
    {
        done = true;
        for (std::list<Token *>::reverse_iterator iter1 = templates.rbegin(); iter1 != templates.rend(); ++iter1)
        {
            simplifyTemplatesInstantiate(*iter1, used, expandedtemplates);
        }
    }

    removeTemplates(_tokens);
}
//---------------------------------------------------------------------------

void Tokenizer::simplifyTemplates2()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (tok->str() == "(")
            tok = tok->link();

        else if (Token::Match(tok, "; %type% <"))
        {
            const Token *tok2 = tok->tokAt(3);
            std::string type;
            while (Token::Match(tok2, "%type% ,") || Token::Match(tok2, "%num% ,"))
            {
                type += tok2->str() + ",";
                tok2 = tok2->tokAt(2);
            }
            if (Token::Match(tok2, "%type% > (") || Token::Match(tok2, "%num% > ("))
            {
                type += tok2->str();
                tok = tok->next();
                tok->str(tok->str() + "<" + type + ">");
                Token::eraseTokens(tok, tok2->tokAt(2));
            }
        }
    }
}
//---------------------------------------------------------------------------

std::string Tokenizer::getNameForFunctionParams(const Token *start)
{
    if (start->next() == start->link())
        return "";

    std::string result;
    bool findNextComma = false;
    for (const Token *tok = start->next(); tok && tok != start->link(); tok = tok->next())
    {
        if (findNextComma)
        {
            if (tok->str() == ",")
                findNextComma = false;

            continue;
        }

        result.append(tok->str() + ",");
        findNextComma = true;
    }

    return result;
}

void Tokenizer::setVarId()
{
    // Clear all variable ids
    for (Token *tok = _tokens; tok; tok = tok->next())
        tok->varId(0);

    // Set variable ids..
    _varId = 0;
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (tok != _tokens && !Token::Match(tok, "[;{}(,] %type%") && !Token::Match(tok, "[;{}(,] ::"))
            continue;

        // Ticket #3104 - "if (NOT x)"
        if (tok->str() == "(" && tok->next()->str() == "NOT")
            continue;

        if (_errorLogger)
            _errorLogger->reportProgress(_files[0], "Tokenize (set variable id)", tok->progressValue());

        // If pattern is "( %type% *|& %var% )" then check if it's a
        // variable declaration or a multiplication / mask
        if (Token::Match(tok, "( %type% *|& %var% [),]") && !tok->next()->isStandardType())
        {
            if (!Token::Match(tok->previous(), "%type%"))
                continue;
            if (tok->strAt(-1) == "return")
                continue;
            if (tok->link() && !Token::Match(tok->link()->tokAt(1), "const| {") &&
                !Token::Match(tok->link()->tokAt(1), ":"))
                continue;
        }

        if (Token::Match(tok, "[,;{}(] %type%") || Token::Match(tok, "[;{}(,] ::"))
        {
            // not function declaration?
            // TODO: Better checking
            if (Token::Match(tok->tokAt(-2), "= %var% ("))
            {
                continue;
            }
            if (tok->str() == "(" &&
                tok->previous() &&
                !tok->previous()->isName() &&
                tok->strAt(-2) != "operator")
                continue;
            tok = tok->next();
        }

        if (tok->str() == "new")
            continue;

        if (tok->str() == "throw")
            continue;

        if (tok->str() == "unsigned")
            tok = tok->next();

        if (Token::Match(tok, "class|struct %type% :|{|;"))
            continue;

        if (Token::Match(tok, "using namespace %type% ;"))
        {
            tok = tok->next();
            continue;
        }

        if (Token::Match(tok, "goto %any% ;"))
            continue;

        if (Token::Match(tok, "else|return|typedef|delete|sizeof"))
            continue;

        while (Token::Match(tok, "const|static|extern|public:|private:|protected:|;|mutable"))
            tok = tok->next();

        // skip global namespace prefix
        if (Token::Match(tok, "::"))
            tok = tok->next();

        while (Token::Match(tok, "%var% ::"))
            tok = tok->tokAt(2);

        // Skip template arguments..
        if (Token::Match(tok, "%type% <"))
        {
            int level = 1;
            bool again;
            Token *tok2 = tok->tokAt(2);

            do // Look for start of templates or template arguments
            {
                if (!tok2) // syntax error
                    return;

                again = false;

                if (tok2 && tok2->str() == "const")
                    tok2 = tok2->next();

                while (Token::Match(tok2, "%var% ::"))
                    tok2 = tok2->tokAt(2);

                if (Token::Match(tok2, "%type% <"))
                {
                    level++;
                    tok2 = tok2->tokAt(2);
                    again = true;
                }
                else if (Token::Match(tok2, "%type% *|&| ,"))
                {
                    tok2 = tok2->tokAt(2);
                    if (!tok2) // syntax error
                        return;
                    if (tok2->str() == ",")
                    {
                        tok2 = tok2->next();
                        if (!tok2) // syntax error
                            return;
                    }
                    again = true;
                }
                else if (level > 1 && (Token::Match(tok2, "%type% *|&| >") ||
                                       Token::Match(tok2, "%num% >")))
                {
                    --level;
                    while (tok2->str() != ">")
                        tok2 = tok2->next();
                    tok2 = tok2->next();
                    if (!tok2) // syntax error
                        return;
                    if (tok2->str() == ",")
                    {
                        tok2 = tok2->next();
                        if (!tok2) // syntax error
                            return;
                    }
                    if (level == 1 && tok2->str() == ">")
                        break;
                    again = true;
                }
                else
                {
                    while (tok2 && (tok2->isName() || tok2->isNumber() || tok2->str() == "*" || tok2->str() == "&" || tok2->str() == ","))
                        tok2 = tok2->next();
                    if (tok2 && tok2->str() == "(")
                    {
                        tok2 = tok2->link()->next();
                        if (tok2 && tok2->str() == "(")
                            tok2 = tok2->link()->next();
                        again = true;
                    }
                }
            }
            while (again);

            do // Look for end of templates
            {
                again = false;

                if (level == 1 && Token::Match(tok2, "> %var%"))
                    tok = tok2;
                else if (level > 1 && tok2 && tok2->str() == ">")
                {
                    level--;
                    if (level == 0)
                        tok = tok2;
                    else
                    {
                        tok2 = tok2->tokAt(1);
                        again = true;
                    }
                }
                else if (level == 1 && Token::Match(tok2, "> ::|*|& %var%"))
                    tok = tok2->next();
                else
                    continue;       // Not code that I understand / not a variable declaration
            }
            while (again);
        }

        // Determine name of declared variable..
        std::string varname;
        Token *tok2 = tok->tokAt(1);
        while (tok2)
        {
            if (tok2->isName())
                varname = tok2->str();
            else if (tok2->str() != "*" && tok2->str() != "&")
                break;
            tok2 = tok2->next();
        }

        // End of tokens reached..
        if (!tok2)
            break;

        if (varname == "operator" && Token::Match(tok2, "=|+|-|*|/|[| ]| ("))
            continue;

        if (varname == "new" && Token::Match(tok2->tokAt(-2), "operator new (|["))
            continue;

        // Is it a function?
        if (tok2->str() == "(")
        {
            // Search for function declaration, e.g. void f();
            if (Token::simpleMatch(tok2->next(), ") ;"))
                continue;

            // Search for function declaration, e.g. void f( int c );
            if (Token::Match(tok2->next(), "%num%") ||
                Token::Match(tok2->next(), "%bool%") ||
                tok2->next()->str()[0] == '"' ||
                tok2->next()->str()[0] == '\'' ||
                tok2->next()->varId() != 0)
            {
                // This is not a function
            }
            else
            {
                continue;
            }
        }

        // Don't set variable id for 'AAA a[0] = 0;' declaration (#2638)
        if (tok2->previous()->varId() && tok2->str() == "[")
        {
            const Token *tok3 = tok2;
            while (tok3 && tok3->str() == "[")
            {
                tok3 = tok3->link();
                tok3 = tok3 ? tok3->next() : NULL;
            }
            if (Token::Match(tok3, "= !!{"))
                continue;
        }

        // Variable declaration found => Set variable ids
        if (Token::Match(tok2, "[,();[=]") && !varname.empty())
        {
            // Are we in a class declaration?
            // Then start at the start of the class declaration..
            while (NULL != (tok2 = tok2->previous()))
            {
                if (tok2->str() == "}" || tok2->str() == ")")
                    tok2 = tok2->link();
                else if (tok2->str() == "(")
                    break;
                else if (tok2->str() == "{")
                {
                    while (NULL != (tok2 = tok2->previous()))
                    {
                        if (Token::Match(tok2, "[,;{})]"))
                        {
                            if (!Token::Match(tok2, ", public|protected|private"))
                                break;
                        }
                        if (Token::Match(tok2, "class|struct"))
                            break;
                    }
                    break;
                }
            }

            /** @todo better handling when classes in different scopes have the same name */
            std::string className;
            if (Token::Match(tok2, "class|struct %type% [:{]"))
                className = tok2->strAt(1);

            // Set start token
            if (Token::Match(tok2, "class|struct"))
            {
                while (tok2->str() != "{")
                    tok2 = tok2->next();
            }
            else
                tok2 = tok;

            ++_varId;
            int indentlevel = 0;
            int parlevel = 0;
            bool funcDeclaration = false;
            while (NULL != (tok2 = tok2->next()))
            {
                const char c = tok2->str()[0];
                if (c == varname[0])
                {
                    if (tok2->str() == varname)
                    {
                        const std::string &prev = tok2->previous()->str();

                        /** @todo better handling when classes in different scopes have the same name */
                        if (!className.empty() && Token::Match(tok2->tokAt(-3), ("!!:: " + className + " ::").c_str()))
                            tok2->varId(_varId);

                        else if (tok2->str() == varname && prev != "struct" && prev != "union" && prev != "::" && prev != "." && tok2->strAt(1) != "::")
                            tok2->varId(_varId);
                    }
                }
                else if (c == '{')
                    ++indentlevel;
                else if (c == '}')
                {
                    --indentlevel;
                    if (indentlevel < 0)
                        break;

                    // We have reached the end of a loop: "for( int i;;) {  }"
                    if (funcDeclaration && indentlevel <= 0)
                        break;
                }
                else if (c == '(')
                    ++parlevel;
                else if (c == ')')
                {
                    // Is this a function parameter or a variable declared in for example a for loop?
                    if (parlevel == 0 && indentlevel == 0 && Token::Match(tok2, ") const| {"))
                        funcDeclaration = true;
                    else
                        --parlevel;
                }
                else if (parlevel < 0 && c == ';')
                    break;
            }
        }
    }

    // Member functions and variables in this source
    std::list<Token *> allMemberFunctions;
    std::list<Token *> allMemberVars;
    {
        for (Token *tok2 = _tokens; tok2; tok2 = tok2->next())
        {
            if (Token::Match(tok2, "%var% :: %var%"))
            {
                if (Token::simpleMatch(tok2->tokAt(3), "("))
                    allMemberFunctions.push_back(tok2);
                else if (tok2->tokAt(2)->varId() != 0)
                    allMemberVars.push_back(tok2);
            }
        }
    }

    // class members..
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "class|struct %var% {|:"))
        {
            const std::string &classname(tok->next()->str());

            // What member variables are there in this class?
            std::map<std::string, unsigned int> varlist;
            {
                unsigned int indentlevel = 0;
                for (const Token *tok2 = tok; tok2; tok2 = tok2->next())
                {
                    // Indentation..
                    if (tok2->str() == "{")
                        ++indentlevel;
                    else if (tok2->str() == "}")
                    {
                        if (indentlevel <= 1)
                            break;
                        --indentlevel;
                    }

                    // skip parentheses..
                    else if (tok2->str() == "(")
                        tok2 = tok2->link();

                    // Found a member variable..
                    else if (indentlevel == 1 && tok2->varId() > 0)
                        varlist[tok2->str()] = tok2->varId();
                }
            }

            // Are there any member variables in this class?
            if (varlist.empty())
                continue;

            // Member variables
            for (std::list<Token *>::iterator func = allMemberVars.begin(); func != allMemberVars.end(); ++func)
            {
                if (!Token::simpleMatch(*func, classname.c_str()))
                    continue;

                Token *tok2 = *func;
                tok2 = tok2->tokAt(2);
                tok2->varId(varlist[tok2->str()]);
            }

            // Member functions for this class..
            std::list<Token *> funclist;
            {
                const std::string funcpattern(classname + " :: %var% (");
                for (std::list<Token *>::iterator func = allMemberFunctions.begin(); func != allMemberFunctions.end(); ++func)
                {
                    Token *tok2 = *func;

                    // Found a class function..
                    if (Token::Match(tok2, funcpattern.c_str()))
                    {
                        // Goto the end parenthesis..
                        tok2 = tok2->tokAt(3)->link();
                        if (!tok2)
                            break;

                        // If this is a function implementation.. add it to funclist
                        if (Token::Match(tok2, ") const|volatile| {"))
                            funclist.push_back(tok2);
                    }
                }
            }

            // Update the variable ids..
            // Parse each function..
            for (std::list<Token *>::iterator func = funclist.begin(); func != funclist.end(); ++func)
            {
                unsigned int indentlevel = 0;
                for (Token *tok2 = *func; tok2; tok2 = tok2->next())
                {
                    if (tok2->str() == "{")
                        ++indentlevel;
                    else if (tok2->str() == "}")
                    {
                        if (indentlevel <= 1)
                            break;
                        --indentlevel;
                    }
                    else if (indentlevel > 0 &&
                             tok2->varId() == 0 &&
                             !Token::simpleMatch(tok2->previous(), ".") &&
                             varlist.find(tok2->str()) != varlist.end())
                    {
                        tok2->varId(varlist[tok2->str()]);
                    }
                }
            }

        }
    }

    // Struct/Class members
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        // str.clear is a variable
        // str.clear() is a member function
        if (tok->varId() != 0 &&
            Token::Match(tok->next(), ". %var% !!(") &&
            tok->tokAt(2)->varId() == 0)
        {
            ++_varId;

            const std::string pattern(std::string(". ") + tok->strAt(2));
            for (Token *tok2 = tok; tok2; tok2 = tok2->next())
            {
                if (tok2->varId() == tok->varId())
                {
                    if (Token::Match(tok2->next(), pattern.c_str()))
                        tok2->tokAt(2)->varId(_varId);
                }
            }
        }
    }
}

bool Tokenizer::createLinks()
{
    std::list<const Token*> type;
    std::list<Token*> links;
    std::list<Token*> links2;
    std::list<Token*> links3;
    for (Token *token = _tokens; token; token = token->next())
    {
        if (token->link())
        {
            token->link(0);
        }

        if (token->str() == "{")
        {
            links.push_back(token);
            type.push_back(token);
        }
        else if (token->str() == "}")
        {
            if (links.empty())
            {
                // Error, { and } don't match.
                syntaxError(token, '{');
                return false;
            }
            if (type.back()->str() != "{")
            {
                syntaxError(type.back(), type.back()->str()[0]);
                return false;
            }
            type.pop_back();

            Token::createMutualLinks(links.back(), token);
            links.pop_back();
        }
        else if (token->str() == "(")
        {
            links2.push_back(token);
            type.push_back(token);
        }
        else if (token->str() == ")")
        {
            if (links2.empty())
            {
                // Error, ( and ) don't match.
                syntaxError(token, '(');
                return false;
            }
            if (type.back()->str() != "(")
            {
                syntaxError(type.back(), type.back()->str()[0]);
                return false;
            }
            type.pop_back();

            Token::createMutualLinks(links2.back(), token);
            links2.pop_back();
        }
        else if (token->str() == "[")
        {
            links3.push_back(token);
            type.push_back(token);
        }
        else if (token->str() == "]")
        {
            if (links3.empty())
            {
                // Error, [ and ] don't match.
                syntaxError(token, '[');
                return false;
            }
            if (type.back()->str() != "[")
            {
                syntaxError(type.back(), type.back()->str()[0]);
                return false;
            }
            type.pop_back();

            Token::createMutualLinks(links3.back(), token);
            links3.pop_back();
        }
    }

    if (!links.empty())
    {
        // Error, { and } don't match.
        syntaxError(links.back(), '{');
        return false;
    }

    if (!links2.empty())
    {
        // Error, ( and ) don't match.
        syntaxError(links2.back(), '(');
        return false;
    }

    if (!links3.empty())
    {
        // Error, [ and ] don't match.
        syntaxError(links3.back(), '[');
        return false;
    }

    return true;
}

void Tokenizer::simplifySizeof()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "class|struct %var%"))
        {
            // we assume that the size of structs and classes are always
            // 100 bytes.
            _typeSize[tok->strAt(1)] = 100;
        }
    }

    // Locate variable declarations and calculate the size
    std::map<unsigned int, std::string> sizeOfVar;
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (tok->varId() != 0 && sizeOfVar.find(tok->varId()) == sizeOfVar.end())
        {
            const unsigned int varId = tok->varId();
            if (Token::Match(tok->tokAt(-3), "[;{}(,] %type% * %var% [;,)]") ||
                Token::Match(tok->tokAt(-4), "[;{}(,] const %type% * %var% [;),]") ||
                Token::Match(tok->tokAt(-2), "[;{}(,] %type% %var% [;),]") ||
                Token::Match(tok->tokAt(-3), "[;{}(,] const %type% %var% [;),]"))
            {
                const unsigned int size = sizeOfType(tok->previous());
                if (size == 0)
                {
                    continue;
                }

                sizeOfVar[varId] = MathLib::toString<unsigned int>(size);
            }

            else if (Token::Match(tok->tokAt(-3), "[;{}(,] struct %type% %var% [;,)]"))
            {
                sizeOfVar[varId] = "100";
            }

            else if (Token::Match(tok->tokAt(-1), "%type% %var% [ %num% ] [;=]") ||
                     Token::Match(tok->tokAt(-2), "%type% * %var% [ %num% ] [;=]"))
            {
                const unsigned int size = sizeOfType(tok->tokAt(-1));
                if (size == 0)
                    continue;

                sizeOfVar[varId] = MathLib::toString<unsigned long>(size * static_cast<unsigned long>(MathLib::toLongNumber(tok->strAt(2))));
            }

            else if (Token::Match(tok->tokAt(-1), "%type% %var% [ %num% ] [,)]") ||
                     Token::Match(tok->tokAt(-2), "%type% * %var% [ %num% ] [,)]"))
            {
                Token tempTok(0);
                tempTok.str("*");
                sizeOfVar[varId] = MathLib::toString<unsigned long>(sizeOfType(&tempTok));
            }

            else if (Token::Match(tok->tokAt(-1), "%type% %var% [ ] = %str% ;"))
            {
                const unsigned int size = sizeOfType(tok->tokAt(4));
                if (size == 0)
                    continue;

                sizeOfVar[varId] = MathLib::toString<unsigned int>(size);
            }
        }
    }

    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (tok->str() != "sizeof")
            continue;

        if (!tok->next())
            break;

        if (Token::simpleMatch(tok->next(), "sizeof"))
            continue;

        if (Token::simpleMatch(tok->next(), ". . ."))
        {
            Token::eraseTokens(tok, tok->tokAt(4));
        }

        // sizeof 'x'
        if (tok->strAt(1)[0] == '\'')
        {
            tok->deleteThis();
            std::ostringstream sz;
            sz << sizeof 'x';
            tok->str(sz.str());
            continue;
        }

        // sizeof('x')
        if (Token::Match(tok, "sizeof ( %any% )") && tok->strAt(2)[0] == '\'')
        {
            tok->deleteThis();
            tok->deleteThis();
            tok->deleteNext();
            std::ostringstream sz;
            sz << sizeof 'x';
            tok->str(sz.str());
            continue;
        }

        // sizeof "text"
        if (Token::Match(tok->next(), "%str%"))
        {
            tok->deleteThis();
            std::ostringstream ostr;
            ostr << (Token::getStrLength(tok) + 1);
            tok->str(ostr.str());
            continue;
        }

        // sizeof ("text")
        if (Token::Match(tok->next(), "( %str% )"))
        {
            tok->deleteThis();
            tok->deleteThis();
            tok->deleteNext();
            std::ostringstream ostr;
            ostr << (Token::getStrLength(tok) + 1);
            tok->str(ostr.str());
            continue;
        }

        // sizeof * (...) -> sizeof(*...)
        if (Token::simpleMatch(tok->next(), "* (") && !Token::simpleMatch(tok->tokAt(2)->link(), ") ."))
        {
            tok->deleteNext();
            tok->next()->insertToken("*");
        }

        // sizeof a++ -> sizeof(a++)
        if (Token::Match(tok->next(), "++|-- %var% !!.") || Token::Match(tok->next(), "%var% ++|--"))
        {
            tok->insertToken("(");
            tok->tokAt(3)->insertToken(")");
            Token::createMutualLinks(tok->next(), tok->tokAt(4));
        }

        // sizeof 1 => sizeof ( 1 )
        if (tok->next()->isNumber())
        {
            Token *tok2 = tok->next();
            tok->insertToken("(");
            tok2->insertToken(")");
            Token::createMutualLinks(tok->next(), tok2->next());
        }

        // sizeof int -> sizeof( int )
        else if (tok->next()->str() != "(")
        {
            // Add parenthesis around the sizeof
            int parlevel = 0;
            for (Token *tempToken = tok->next(); tempToken; tempToken = tempToken->next())
            {
                if (tempToken->str() == "(")
                    ++parlevel;
                else if (tempToken->str() == ")")
                    --parlevel;
                if (Token::Match(tempToken, "%var%"))
                {
                    while (tempToken && tempToken->next() && tempToken->next()->str() == "[")
                    {
                        tempToken = tempToken->next()->link();
                    }
                    if (!tempToken || !tempToken->next())
                    {
                        break;
                    }

                    if (tempToken->next()->str() == ".")
                    {
                        // We are checking a class or struct, search next varname
                        tempToken = tempToken->tokAt(1);
                        continue;
                    }
                    else if (Token::simpleMatch(tempToken->next(), "- >"))
                    {
                        // We are checking a class or struct, search next varname
                        tempToken = tempToken->tokAt(2);
                        continue;
                    }
                    else if (Token::Match(tempToken->next(), "++|--"))
                    {
                        // We have variable++ or variable--, there should be
                        // nothing after this
                        tempToken = tempToken->tokAt(2);
                    }
                    else if (parlevel > 0 && Token::simpleMatch(tempToken->next(), ") ."))
                    {
                        --parlevel;
                        tempToken = tempToken->tokAt(2);
                        continue;
                    }

                    // Ok, we should be clean. Add ) after tempToken
                    tok->insertToken("(");
                    tempToken->insertToken(")");
                    Token::createMutualLinks(tok->next(), tempToken->next());
                    break;
                }
            }
        }

        // sizeof(type *) => sizeof(*)
        if (Token::Match(tok->next(), "( %type% * )"))
        {
            tok->next()->deleteNext();
        }

        if (Token::simpleMatch(tok->next(), "( * )"))
        {
            tok->str(MathLib::toString<unsigned long>(sizeOfType(tok->tokAt(2))));
            Token::eraseTokens(tok, tok->tokAt(4));
        }

        // sizeof( a )
        else if (Token::Match(tok->next(), "( %var% )") && tok->tokAt(2)->varId() != 0)
        {
            if (sizeOfVar.find(tok->tokAt(2)->varId()) != sizeOfVar.end())
            {
                tok->deleteThis();
                tok->deleteThis();
                tok->deleteNext();
                tok->str(sizeOfVar[tok->varId()]);
            }
            else
            {
                // don't try to replace size of variable if variable has
                // similar name with type (#329)
            }
        }

        else if (Token::Match(tok, "sizeof ( %type% )"))
        {
            unsigned int size = sizeOfType(tok->tokAt(2));
            if (size > 0)
            {
                tok->str(MathLib::toString<unsigned int>(size));
                Token::eraseTokens(tok, tok->tokAt(4));
            }
        }

        else if (Token::Match(tok, "sizeof ( * %var% )") || Token::Match(tok, "sizeof ( %var% [ %num% ] )"))
        {
            // Some default value..
            unsigned int sz = 0;

            unsigned int varid = tok->tokAt((tok->tokAt(2)->str() == "*") ? 3 : 2)->varId();
            if (varid != 0)
            {
                // Try to locate variable declaration..
                const Token *decltok = Token::findmatch(_tokens, "%varid%", varid);
                if (Token::Match(decltok->previous(), "%type% %var% ["))
                {
                    sz = sizeOfType(decltok->previous());
                }
                else if (Token::Match(decltok->previous(), "* %var% ["))
                {
                    sz = sizeOfType(decltok->previous());
                }
                else if (Token::Match(decltok->tokAt(-2), "%type% * %var%"))
                {
                    sz = sizeOfType(decltok->tokAt(-2));
                }
            }
            else if (tok->strAt(3) == "[" && tok->tokAt(2)->isStandardType())
            {
                sz = sizeOfType(tok->tokAt(2));
                if (sz == 0)
                    continue;
                sz = sz * static_cast<unsigned long>(MathLib::toLongNumber(tok->strAt(4)));
            }

            if (sz > 0)
            {
                tok->str(MathLib::toString<unsigned int>(sz));
                Token::eraseTokens(tok, tok->next()->link()->next());
            }
        }
    }

}

bool Tokenizer::simplifyTokenList()
{
    // clear the _functionList so it can't contain dead pointers
    delete _symbolDatabase;
    _symbolDatabase = NULL;

    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::simpleMatch(tok, "* const"))
            tok->deleteNext();
    }

    // simplify references
    simplifyReference();

    simplifyStd();

    simplifyGoto();

    simplifyDeadCode();

    // Combine wide strings
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        while (tok->str() == "L" && tok->next() && tok->next()->str()[0] == '"')
        {
            // Combine 'L "string"'
            tok->str(tok->next()->str());
            tok->deleteNext();
        }
    }

    // Combine strings
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (tok->str()[0] != '"')
            continue;

        tok->str(simplifyString(tok->str()));
        while (tok->next() && tok->next()->str()[0] == '"')
        {
            tok->next()->str(simplifyString(tok->next()->str()));

            // Two strings after each other, combine them
            tok->concatStr(tok->next()->str());
            tok->deleteNext();
        }
    }

    simplifySizeof();

    // replace strlen(str)
    simplifyKnownVariables();
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "strlen ( %str% )"))
        {
            std::ostringstream ostr;
            ostr << Token::getStrLength(tok->tokAt(2));
            tok->str(ostr.str());
            tok->deleteNext();
            tok->deleteNext();
            tok->deleteNext();
        }
    }

    // change array to pointer..
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "%type% %var% [ ] [,;=]"))
        {
            Token::eraseTokens(tok->next(), tok->tokAt(4));
            tok->insertToken("*");
        }
    }

    // Replace constants..
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "const %type% %var% = %num% ;"))
        {
            unsigned int varId = tok->tokAt(2)->varId();
            if (varId == 0)
            {
                tok = tok->tokAt(5);
                continue;
            }

            const std::string num = tok->strAt(4);
            int indent = 1;
            for (Token *tok2 = tok->tokAt(6); tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "{")
                {
                    ++indent;
                }
                else if (tok2->str() == "}")
                {
                    --indent;
                    if (indent == 0)
                        break;
                }

                // Compare constants, but don't touch members of other structures
                else if (tok2->varId() == varId)
                {
                    tok2->str(num);
                }
            }
        }
    }

    simplifyCasts();

    // Simplify simple calculations..
    simplifyCalculations();

    // Replace "*(str + num)" => "str[num]"
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (! strchr(";{}(=<>", tok->str()[0]))
            continue;

        Token *next = tok->next();
        if (! next)
            break;

        if (Token::Match(next, "* ( %var% + %num% )") ||
            Token::Match(next, "* ( %var% + %var% )"))
        {
            // var
            tok = tok->next();
            tok->str(tok->strAt(2));

            // [
            tok = tok->next();
            tok->str("[");

            // num
            tok = tok->next();
            tok->str(tok->strAt(2));

            // ]
            tok = tok->next();
            tok->str("]");

            tok->deleteNext();
            tok->deleteNext();

            Token::createMutualLinks(next->tokAt(1), next->tokAt(3));
        }
    }

    // simplify "x=realloc(y,0);" => "free(y); x=0;"..
    // and "x = realloc (0, n);" => "x = malloc(n);"
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "; %var% = realloc ( %var% , 0 ) ;"))
        {
            const std::string varname(tok->next()->str());
            const unsigned int varid(tok->next()->varId());

            // Delete the "%var% ="
            tok->deleteNext();
            tok->deleteNext();

            // Change function name "realloc" to "free"
            tok->next()->str("free");

            // delete the ", 0"
            Token::eraseTokens(tok->tokAt(3), tok->tokAt(6));

            // goto the ";"
            tok = tok->tokAt(5);

            // insert "var=0;"
            tok->insertToken(";");
            tok->insertToken("0");
            tok->insertToken("=");
            tok->insertToken(varname);
            tok->next()->varId(varid);
        }
        else if (Token::Match(tok, "; %var% = realloc ( 0 , %num% ) ;"))
        {
            const std::string varname(tok->next()->str());

            tok = tok->tokAt(3);
            // Change function name "realloc" to "malloc"
            tok->str("malloc");

            // delete "0 ,"
            tok->next()->deleteNext();
            tok->next()->deleteNext();
        }
    }

    // Change initialisation of variable to assignment
    simplifyInitVar();

    // Simplify variable declarations
    simplifyVarDecl();

    simplifyFunctionParameters();
    elseif();
    simplifyErrNoInWhile();
    simplifyIfAssign();
    simplifyRedundantParenthesis();
    simplifyIfNot();
    simplifyIfNotNull();
    simplifyIfSameInnerCondition();
    simplifyComparisonOrder();
    simplifyNestedStrcat();
    simplifyWhile0();
    simplifyFuncInWhile();

    simplifyIfAssign();    // could be affected by simplifyIfNot

    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "case %any% : %var%"))
            tok->tokAt(2)->insertToken(";");
        if (Token::Match(tok, "default : %var%"))
            tok->next()->insertToken(";");
    }

    // In case variable declarations have been updated...
    setVarId();

    bool modified = true;
    while (modified)
    {
        modified = false;
        modified |= simplifyConditions();
        modified |= simplifyFunctionReturn();
        modified |= simplifyKnownVariables();
        modified |= removeReduntantConditions();
        modified |= simplifyRedundantParenthesis();
        modified |= simplifyQuestionMark();
        modified |= simplifyCalculations();
    }

    // simplify redundant for
    removeRedundantFor();

    // Remove redundant parentheses in return..
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        while (Token::simpleMatch(tok, "return ("))
        {
            Token *tok2 = tok->next()->link();
            if (Token::simpleMatch(tok2, ") ;"))
            {
                tok->deleteNext();
                tok2->deleteThis();
            }
            else
            {
                break;
            }
        }
    }

    simplifyReturn();

    removeRedundantAssignment();

    simplifyComma();

    removeRedundantSemicolons();

    if (!validate())
        return false;

    _tokens->assignProgressValues();

    if (_settings->debug)
    {
        _tokens->printOut(0, _files);
    }

    if (_settings->debugwarnings)
    {
        printUnknownTypes();
    }

    return true;
}
//---------------------------------------------------------------------------

void Tokenizer::removeMacrosInGlobalScope()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (tok->str() == "(")
        {
            tok = tok->link();
            if (Token::Match(tok, ") %type% {") && tok->strAt(1) != "const")
                tok->deleteNext();
        }

        if (tok->str() == "{")
            tok = tok->link();
    }
}
//---------------------------------------------------------------------------

void Tokenizer::removeRedundantAssignment()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (tok->str() == "{")
            tok = tok->link();

        if (Token::Match(tok, ") const| {"))
        {
            // parse in this function..
            std::set<unsigned int> localvars;
            if (tok->next()->str() == "const")
                tok = tok->next();
            const Token * const end = tok->next()->link();
            for (Token *tok2 = tok->next(); tok2 && tok2 != end; tok2 = tok2->next())
            {
                // skip local class or struct
                if (Token::Match(tok2, "class|struct %type% {|:"))
                {
                    // skip to '{'
                    while (tok2 && tok2->str() != "{")
                        tok2 = tok2->next();

                    if (tok2)
                        tok2 = tok2->link(); // skip local class or struct
                    else
                        return;
                }
                else if (Token::Match(tok2, "[;{}] %type% * %var% ;") && tok2->strAt(1) != "return")
                {
                    tok2 = tok2->tokAt(3);
                    localvars.insert(tok2->varId());
                }
                else if (Token::Match(tok2, "[;{}] %type% %var% ;") && tok2->next()->isStandardType())
                {
                    tok2 = tok2->tokAt(2);
                    localvars.insert(tok2->varId());
                }
                else if (tok2->varId() &&
                         !Token::Match(tok2->previous(), "[;{}] %var% = %var% ;") &&
                         !Token::Match(tok2->previous(), "[;{}] %var% = %num% ;") &&
                         !(Token::Match(tok2->previous(), "[;{}] %var% = %any% ;") && tok2->strAt(2)[0] == '\''))
                {
                    localvars.erase(tok2->varId());
                }
            }
            localvars.erase(0);
            if (!localvars.empty())
            {
                for (Token *tok2 = tok->next(); tok2 && tok2 != end; tok2 = tok2->next())
                {
                    if (Token::Match(tok2, "[;{}] %type% %var% ;") && localvars.find(tok2->tokAt(2)->varId()) != localvars.end())
                    {
                        Token::eraseTokens(tok2, tok2->tokAt(3));
                    }
                    else if (Token::Match(tok2, "[;{}] %type% * %var% ;") && localvars.find(tok2->tokAt(3)->varId()) != localvars.end())
                    {
                        Token::eraseTokens(tok2, tok2->tokAt(4));
                    }
                    else if (Token::Match(tok2, "[;{}] %var% = %any% ;") && localvars.find(tok2->next()->varId()) != localvars.end())
                    {
                        Token::eraseTokens(tok2, tok2->tokAt(4));
                    }
                }
            }
        }
    }
}

void Tokenizer::simplifyDeadCode()
{
    unsigned int indentlevel = 0;
    unsigned int indentcase = 0;
    unsigned int indentret = 0;
    unsigned int indentswitch = 0;
    unsigned int indentlabel = 0;
    unsigned int roundbraces = 0;
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (tok->str() == "(")
            ++roundbraces;
        else if (tok->str() == ")")
        {
            if (!roundbraces)
                break;  //too many ending round parenthesis
            --roundbraces;
        }

        if (tok->str() == "{")
        {
            ++indentlevel;
            if (indentret)
            {
                unsigned int indentlevel1 = indentlevel;
                for (Token *tok2 = tok->next(); tok2; tok2 = tok2->next())
                {
                    if (tok2->str() == "{")
                        ++indentlevel1;
                    else if (tok2->str() == "}")
                    {
                        if (indentlevel1 == indentlevel)
                            break;
                        --indentlevel1;
                    }
                    else if (Token::Match(tok2, "%var% : ;") && !Token::Match(tok2, "case!default"))
                    {
                        indentlabel = indentlevel1;
                        break;
                    }
                }
                if (indentlevel > indentlabel)
                {
                    tok = tok->previous();
                    tok->deleteNext();
                }
            }
        }
        else if (tok->str() == "}")
        {
            if (!indentlevel)
                break;  //too many closing parenthesis
            if (indentret)
            {
                if (!indentswitch || indentlevel > indentcase)
                {
                    if (indentlevel > indentret && indentlevel > indentlabel)
                    {
                        tok = tok->previous();
                        tok->deleteNext();
                    }
                }
                else
                {
                    if (indentcase > indentret && indentlevel > indentlabel)
                    {
                        tok = tok->previous();
                        tok->deleteNext();
                    }
                }
            }
            if (indentlevel == indentret)
            {
                indentret = 0;
            }
            --indentlevel;
            if (indentlevel <= indentcase)
            {
                if (!indentswitch)
                {
                    indentcase = 0;
                }
                else
                {
                    --indentswitch;
                    indentcase = indentlevel-1;
                }
            }
        }
        else if (!indentret)
        {
            if (tok->str() == "switch")
            {
                if (!indentlevel)
                    break;

                // Don't care about unpreprocessed macros part of code
                if (roundbraces)
                    break;

                unsigned int switchroundbraces = 0;
                for (Token *tok2 = tok->next(); tok2; tok2 = tok2->next())
                {
                    if (tok2->str() == "(")
                        ++switchroundbraces;
                    else if (tok2->str() == ")")
                    {
                        if (!switchroundbraces)
                            break;  //too many closing parenthesis
                        --switchroundbraces;
                    }
                    if (tok2->str() == "{")
                    {
                        if (switchroundbraces)
                        {
                            tok = tok2->previous();
                            break;  //too many opening parenthesis
                        }
                        tok = tok2;
                        ++indentswitch;
                        break;
                    }
                    else if (tok2->str() == "}")
                        break;  //it's not expected, hence it's bad code
                }
                if (!indentswitch)
                    break;
                ++indentlevel;
                indentcase = indentlevel;
            }
            else if (indentswitch && Token::Match(tok, "case|default"))
            {
                if (indentlevel > indentcase)
                {
                    --indentlevel;
                }
                for (Token *tok2 = tok->next(); tok2; tok2 = tok2->next())
                {
                    if (Token::Match(tok2, ": ;| "))
                    {
                        if (indentlevel == indentcase)
                        {
                            ++indentlevel;
                        }
                        tok = tok2;
                        break;
                    }
                    else if (Token::Match(tok2, "[{}]"))
                        break;  //bad code
                }
            }
            else if (tok->str()=="return")
            {
                if (!indentlevel)
                    break;

                // Don't care about unpreprocessed macros part of code
                if (roundbraces)
                    continue;

                //catch the first ';' after the return
                unsigned int returnroundbraces = 0;
                for (Token *tok2 = tok->next(); tok2; tok2 = tok2->next())
                {
                    if (tok2->str() == "(")
                        ++returnroundbraces;
                    else if (tok2->str() == ")")
                    {
                        if (!returnroundbraces)
                            break;  //excessive closing parenthesis
                        --returnroundbraces;
                    }
                    else if (tok2->str() == ";")
                    {
                        if (returnroundbraces)
                            break;  //excessive opening parenthesis
                        indentret = indentlevel;
                        tok = tok2;
                        break;
                    }
                    else if (Token::Match(tok2, "[{}]"))
                        break;  //I think this is an error code...
                }
                if (!indentret)
                    break;
            }
        }
        else if (indentret) //there's already a "return;" declaration
        {
            if (!indentswitch || indentlevel > indentcase+1)
            {
                if (indentlevel >= indentret && (!Token::Match(tok, "%var% : ;") || Token::Match(tok, "case|default")))
                {
                    tok = tok->previous();
                    tok->deleteNext();
                }
                else
                {
                    indentret = 0;
                }
            }
            else
            {
                if (!Token::Match(tok, "%var% : ;") && !Token::Match(tok, "case|default"))
                {
                    tok = tok->previous();
                    tok->deleteNext();
                }
                else
                {
                    indentret = 0;
                    tok = tok->previous();
                }
            }
        }
    }
}


bool Tokenizer::removeReduntantConditions()
{
    // Return value for function. Set to true if there are any simplifications
    bool ret = false;

    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (tok->str() != "if")
            continue;

        if (!Token::Match(tok->tokAt(1), "( %bool% ) {"))
            continue;

        // Find matching else
        const Token *elseTag = 0;

        // Find the closing "}"
        elseTag = tok->tokAt(4)->link()->next();

        bool boolValue = false;
        if (tok->tokAt(2)->str() == "true")
            boolValue = true;

        // Handle if with else
        if (elseTag && elseTag->str() == "else")
        {
            if (Token::simpleMatch(elseTag->next(), "if ("))
            {
                // Handle "else if"
                if (boolValue == false)
                {
                    // Convert "if( false ) {aaa;} else if() {bbb;}" => "if() {bbb;}"
                    Token::eraseTokens(tok, elseTag->tokAt(2));
                    ret = true;
                }
                else
                {
                    // Keep first if, remove every else if and else after it
                    const Token *lastTagInIf = elseTag->tokAt(2);
                    while (lastTagInIf)
                    {
                        if (lastTagInIf->str() == "(")
                        {
                            lastTagInIf = lastTagInIf->link()->next();
                        }

                        lastTagInIf = lastTagInIf->link()->next();
                        if (!Token::simpleMatch(lastTagInIf, "else"))
                            break;

                        lastTagInIf = lastTagInIf->next();
                        if (lastTagInIf->str() == "if")
                            lastTagInIf = lastTagInIf->next();
                    }

                    Token::eraseTokens(elseTag->previous(), lastTagInIf);
                    ret = true;
                }
            }
            else
            {
                // Handle else
                if (boolValue == false)
                {
                    // Convert "if( false ) {aaa;} else {bbb;}" => "{bbb;}" or ";{bbb;}"
                    if (tok->previous())
                        tok = tok->previous();
                    else
                        tok->str(";");

                    Token::eraseTokens(tok, elseTag->tokAt(1));
                }
                else
                {
                    if (elseTag->tokAt(1)->str() == "{")
                    {
                        // Convert "if( true ) {aaa;} else {bbb;}" => "{aaa;}"
                        const Token *end = elseTag->tokAt(1)->link();

                        // Remove the "else { aaa; }"
                        Token::eraseTokens(elseTag->previous(), end->tokAt(1));
                    }

                    // Remove "if( true )"
                    if (tok->previous())
                        tok = tok->previous();
                    else
                        tok->str(";");

                    Token::eraseTokens(tok, tok->tokAt(5));
                }

                ret = true;
            }
        }

        // Handle if without else
        else
        {
            if (boolValue == false)
            {
                // Remove if and its content
                if (tok->previous())
                    tok = tok->previous();
                else
                    tok->str(";");

                Token::eraseTokens(tok, elseTag);
            }
            else
            {
                // convert "if( true ) {aaa;}" => "{aaa;}"
                if (tok->previous())
                    tok = tok->previous();
                else
                    tok->str(";");

                Token::eraseTokens(tok, tok->tokAt(5));
            }

            ret = true;
        }
    }

    return ret;
}

void Tokenizer::removeRedundantFor()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "[;{}] for ( %var% = %num% ; %var% < %num% ; ++| %var% ++| ) {"))
        {
            // Same variable name..
            const std::string varname(tok->tokAt(3)->str());
            const unsigned int varid(tok->tokAt(3)->varId());
            if (varname != tok->tokAt(7)->str())
                continue;
            const Token *vartok = tok->tokAt(11);
            if (vartok->str() == "++")
                vartok = vartok->next();
            if (varname != vartok->str())
                continue;

            // Check that the difference of the numeric values is 1
            const MathLib::bigint num1(MathLib::toLongNumber(tok->strAt(5)));
            const MathLib::bigint num2(MathLib::toLongNumber(tok->strAt(9)));
            if (num1 + 1 != num2)
                continue;

            // check how loop variable is used in loop..
            bool read = false;
            bool write = false;
            unsigned int indentlevel = 0;
            for (const Token *tok2 = tok->tokAt(2)->link(); tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "{")
                    ++indentlevel;
                else if (tok2->str() == "}")
                {
                    if (indentlevel <= 1)
                        break;
                    --indentlevel;
                }

                if (tok2->str() == varname)
                {
                    if (tok2->previous()->isArithmeticalOp() &&
                        tok2->next() &&
                        (tok2->next()->isArithmeticalOp() || tok2->next()->str() == ";"))
                    {
                        read = true;
                    }
                    else
                    {
                        read = write = true;
                        break;
                    }
                }
            }

            // Simplify loop if loop variable isn't written
            if (!write)
            {
                // remove "for ("
                tok->deleteNext();
                tok->deleteNext();

                // If loop variable is read then keep assignment before
                // loop body..
                if (read)
                {
                    // goto ";"
                    tok = tok->tokAt(4);
                }
                else
                {
                    // remove "x = 0 ;"
                    tok->deleteNext();
                    tok->deleteNext();
                    tok->deleteNext();
                    tok->deleteNext();
                }

                // remove "x < 1 ; x ++ )"
                tok->deleteNext();
                tok->deleteNext();
                tok->deleteNext();
                tok->deleteNext();
                tok->deleteNext();
                tok->deleteNext();
                tok->deleteNext();

                // Add assignment after the loop body so the loop variable
                // get the correct end value
                Token *tok2 = tok->next()->link();
                tok2->insertToken(";");
                tok2->insertToken(MathLib::toString(num2));
                tok2->insertToken("=");
                tok2->insertToken(varname);
                tok2->next()->varId(varid);
            }
        }
    }
}


void Tokenizer::removeRedundantSemicolons()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (tok->str() == "(")
        {
            tok = tok->link();
        }
        for (;;)
        {
            if (Token::simpleMatch(tok, "; ;"))
            {
                tok->deleteNext();
            }
            else if (Token::simpleMatch(tok, "; { ; }"))
            {
                tok->deleteNext();
                tok->deleteNext();
                tok->deleteNext();
            }
            else
            {
                break;
            }
        }
    }
}


bool Tokenizer::simplifyIfAddBraces()
{
    for (Token *tok = _tokens; tok; tok = tok ? tok->next() : NULL)
    {
        if (tok->str() == "(" && !Token::Match(tok->previous(), "[;{}]"))
        {
            tok = tok->link();
            continue;
        }

        if (Token::Match(tok, "if|for|while ("))
        {
            // don't add "{}" around ";" in "do {} while();" (#609)
            const Token *prev = tok->previous();
            if (Token::simpleMatch(prev, "} while") &&
                prev->link() &&
                prev->link()->previous() &&
                prev->link()->previous()->str() == "do")
            {
                continue;
            }

            // Goto the ending ')'
            tok = tok->next()->link();

            // ')' should be followed by '{'
            if (!tok || Token::simpleMatch(tok, ") {"))
                continue;
        }

        else if (tok->str() == "else")
        {
            // An else followed by an if or brace don't need to be processed further
            if (Token::Match(tok, "else if|{"))
                continue;
        }

        else
        {
            continue;
        }

        // If there is no code after the if(), abort
        if (!tok->next())
        {
            // This is a syntax error and we should call syntaxError() and return false but
            // many tokenizer tests are written with this syntax error so just ingore it.
            return true;
        }

        // insert open brace..
        tok->insertToken("{");
        tok = tok->next();
        Token *tempToken = tok;

        bool innerIf = Token::simpleMatch(tempToken->next(), "if");

        if (Token::simpleMatch(tempToken->next(), "do {"))
            tempToken = tempToken->tokAt(2)->link();

        // insert close brace..
        // In most cases it would work to just search for the next ';' and insert a closing brace after it.
        // But here are special cases..
        // * if (cond) for (;;) break;
        // * if (cond1) if (cond2) { }
        // * if (cond1) if (cond2) ; else ;
        while ((tempToken = tempToken->next()) != NULL)
        {
            if (tempToken->str() == "{")
            {
                if (Token::simpleMatch(tempToken->previous(),"else {"))
                {
                    if (innerIf)
                        tempToken = tempToken->link();
                    else
                        tempToken = tempToken->tokAt(-2);
                    break;
                }
                tempToken = tempToken->link();
                if (!tempToken || !tempToken->next())
                    break;
                if (tempToken->next()->isName() && tempToken->next()->str() != "else")
                    break;
                continue;
            }

            if (tempToken->str() == "(")
            {
                tempToken = tempToken->link();
                continue;
            }

            if (tempToken->str() == "}")
            {
                // insert closing brace before this token
                tempToken = tempToken->previous();
                break;
            }

            if (tempToken->str() == ";")
            {
                if (!innerIf)
                    break;

                if (Token::simpleMatch(tempToken, "; else if"))
                    ;
                else if (Token::simpleMatch(tempToken, "; else"))
                    innerIf = false;
                else
                    break;
            }
        }

        if (tempToken)
        {
            tempToken->insertToken("}");
            Token::createMutualLinks(tok, tempToken->next());
        }
        else
        {
            // Can't insert matching "}" so give up.  This is fatal because it
            // causes unbalanced braces.
            syntaxError(tok);
            return false;
        }
    }
    return true;
}

bool Tokenizer::simplifyDoWhileAddBracesHelper(Token *tok)
{
    if (Token::Match(tok->next(), "[),]"))
    {
        // fix for #988
        return false;
    }

    Token *tok1 = tok;  // token with "do"
    Token *tok2 = NULL; // token with "while"
    Token *tok3 = tok->next();

    // skip loop body
    bool result = false;
    while (tok3)
    {
        if (tok3->str() == "{")
        {
            // skip all tokens until "}"
            tok3 = tok3->link();
        }
        else if (tok3->str() == "while")
        {
            tok2 = tok3;
            break;
        }
        else if (Token::simpleMatch(tok3, "do {"))
        {
            // Skip do{}while inside the current "do"
            tok3 = tok3->next()->link();
            if (Token::simpleMatch(tok3->next(), "while"))
                tok3 = tok3->next();
        }
        else if (Token::Match(tok3, "do !!{") &&
                 !Token::Match(tok3->next(), "[),]"))
        {
            // Handle do-while inside the current "do"
            // first and return true to get the outer
            // "do" to be handled later.
            tok1 = tok3;
            result = true;
        }

        tok3 = tok3->next();
    }

    if (tok2)
    {
        // insert "{" after "do"
        tok1->insertToken("{");

        // insert "}" before "while"
        tok2->previous()->insertToken("}");

        Token::createMutualLinks(tok1->next(), tok2->previous());
    }
    else
        result = false;

    return result;
}

void Tokenizer::simplifyDoWhileAddBraces()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "do !!{"))
        {
            while (simplifyDoWhileAddBracesHelper(tok))
            {
                // Call until the function returns false to
                // handle do-while inside do-while

            }
        }
    }
}

void Tokenizer::simplifyCompoundAssignment()
{
    // Simplify compound assignments:
    // "a+=b" => "a = a + b"
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "[;{}] (") || Token::Match(tok, "[;{}:] *| (| %var%"))
        {
            if (tok->str() == ":")
            {
                if (tok->strAt(-2) != "case")
                    continue;
            }

            // backup current token..
            Token * const tok1 = tok;

            if (tok->strAt(1) == "*")
                tok = tok->next();

            if (tok->strAt(1) == "(")
            {
                tok = tok->next()->link()->next();
            }
            else
            {
                // variable..
                tok = tok->tokAt(2);
                while (Token::Match(tok, ". %var%") ||
                       Token::Match(tok, "[|("))
                {
                    if (tok->str() == ".")
                        tok = tok->tokAt(2);
                    else
                    {
                        // goto "]" or ")"
                        tok = tok->link();

                        // goto next token..
                        tok = tok ? tok->next() : 0;
                    }
                }
            }
            if (!tok)
                break;

            // Is current token at a compound assignment: +=|-=|.. ?
            const std::string &str = tok->str();
            std::string op;  // operator used in assignment
            if (str.size() == 2 && str[1] == '=' && str.find_first_of("+-*/%&|^")==0)
                op = str.substr(0, 1);
            else if (str=="<<=" || str==">>=")
                op = str.substr(0, 2);
            else
            {
                tok = tok1;
                continue;
            }

            // Remove the whole statement if it says: "+=0;", "-=0;", "*=1;" or "/=1;"
            if (Token::Match(tok, "+=|-= 0 ;") ||
                Token::Match(tok, "+=|-= '\\0' ;") ||
                Token::simpleMatch(tok, "|= 0 ;") ||
                Token::Match(tok, "*=|/= 1 ;"))
            {
                tok = tok1;
                while (tok->next()->str() != ";")
                    tok->deleteNext();
            }
            else
            {
                // Enclose the rhs in parantheses..
                if (!Token::Match(tok->next(), "%any% [;)]"))
                {
                    // Only enclose rhs in parantheses if there is some operator
                    bool someOperator = false;
                    for (Token *tok2 = tok->next(); tok2; tok2 = tok2->next())
                    {
                        if (tok2->str() == "(")
                            tok2 = tok2->link();

                        if (Token::Match(tok2->next(), "[;)]"))
                        {
                            if (someOperator)
                            {
                                tok->insertToken("(");
                                tok2->insertToken(")");
                                Token::createMutualLinks(tok->next(), tok2->next());
                            }
                            break;
                        }

                        someOperator |= bool(tok2->isArithmeticalOp() || (tok2->str() == "?"));
                    }
                }

                // simplify the compound assignment..
                tok->str("=");
                tok->insertToken(op);

                std::stack<Token *> tokend;
                for (const Token *tok2 = tok->previous(); tok2 && tok2 != tok1; tok2 = tok2->previous())
                {
                    tok->insertToken(tok2->str());
                    tok->next()->varId(tok2->varId());
                    if (Token::Match(tok->next(), "]|)"))
                        tokend.push(tok->next());
                    else if (Token::Match(tok->next(), "(|["))
                    {
                        Token::createMutualLinks(tok->next(), tokend.top());
                        tokend.pop();
                    }
                }
            }
        }
    }
}

void Tokenizer::simplifyConditionOperator()
{
    int parlevel = 0;
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (tok->str() == "(")
            ++parlevel;
        else if (tok->str() == ")")
            --parlevel;
        else if (parlevel == 0 && (Token::Match(tok, ";|{|} *| %any% = %any% ? %any% : %any% ;") ||
                                   Token::Match(tok, ";|{|} return %any% ? %any% : %any% ;")))
        {
            std::string var(tok->strAt(1));
            bool isPointer = false;
            bool isReturn = false;
            int offset = 0;
            if (tok->next()->str() == "*")
            {
                tok = tok->next();
                var += " " + tok->strAt(1);
                isPointer = true;
            }
            else if (tok->next()->str() == "return")
            {
                isReturn = true;
                offset = -1;
            }

            const std::string condition(tok->strAt(3 + offset));
            const std::string value1(tok->strAt(5 + offset));
            const std::string value2(tok->strAt(7 + offset));

            if (isPointer)
            {
                tok = tok->previous();
                Token::eraseTokens(tok, tok->tokAt(10));
            }
            else if (isReturn)
                Token::eraseTokens(tok, tok->tokAt(7));
            else
                Token::eraseTokens(tok, tok->tokAt(9));

            Token *starttok = 0;

            std::string str;
            if (isReturn)
                str = "if ( condition ) { return value1 ; } return value2 ;";
            else
                str = "if ( condition ) { var = value1 ; } else { var = value2 ; }";

            std::string::size_type pos1 = 0;
            while (pos1 != std::string::npos)
            {
                std::string::size_type pos2 = str.find(" ", pos1);
                if (pos2 == std::string::npos)
                {
                    tok->insertToken(str.substr(pos1).c_str());
                    pos1 = pos2;
                }
                else
                {
                    tok->insertToken(str.substr(pos1, pos2 - pos1).c_str());
                    pos1 = pos2 + 1;
                }
                tok = tok->next();

                // set links.
                if (tok->str() == "(" || tok->str() == "{")
                    starttok = tok;
                else if (starttok && (tok->str() == ")" || tok->str() == "}"))
                {
                    Token::createMutualLinks(starttok, tok);
                    starttok = 0;
                }
                else if (tok->str() == "condition")
                    tok->str(condition);
                else if (tok->str() == "var")
                    tok->str(var);
                else if (tok->str() == "value1")
                    tok->str(value1);
                else if (tok->str() == "value2")
                    tok->str(value2);
            }
        }
    }
}

bool Tokenizer::simplifyConditions()
{
    bool ret = false;

    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "! %num%") || Token::Match(tok, "! %bool%"))
        {
            if (tok->next()->str() == "0" || tok->next()->str() == "false")
                tok->str("true");
            else
                tok->str("false");

            tok->deleteNext();
            ret = true;
        }

        if (Token::simpleMatch(tok, "( true &&") ||
            Token::simpleMatch(tok, "&& true &&") ||
            Token::simpleMatch(tok->next(), "&& true )"))
        {
            Token::eraseTokens(tok, tok->tokAt(3));
            ret = true;
        }

        else if (Token::simpleMatch(tok, "( false ||") ||
                 Token::simpleMatch(tok, "|| false ||") ||
                 Token::simpleMatch(tok->next(), "|| false )"))
        {
            Token::eraseTokens(tok, tok->tokAt(3));
            ret = true;
        }

        else if (Token::simpleMatch(tok, "( true ||") ||
                 Token::simpleMatch(tok, "( false &&"))
        {
            Token::eraseTokens(tok->next(), tok->link());
            ret = true;
        }

        else if (Token::simpleMatch(tok, "|| true )") ||
                 Token::simpleMatch(tok, "&& false )"))
        {
            tok = tok->next();
            Token::eraseTokens(tok->next()->link(), tok);
            ret = true;
        }

        // Change numeric constant in condition to "true" or "false"
        if (Token::Match(tok, "if|while ( %num% )|%oror%|&&"))
        {
            tok->tokAt(2)->str((tok->tokAt(2)->str() != "0") ? "true" : "false");
            ret = true;
        }
        if (Token::Match(tok, "&&|%oror% %num% )|%oror%|&&"))
        {
            tok->next()->str((tok->next()->str() != "0") ? "true" : "false");
            ret = true;
        }

        // Reduce "(%num% == %num%)" => "(true)"/"(false)"
        if (Token::Match(tok, "&&|%oror%|(") &&
            (Token::Match(tok->tokAt(1), "%num% %any% %num%") ||
             Token::Match(tok->tokAt(1), "%bool% %any% %bool%")) &&
            Token::Match(tok->tokAt(4), "&&|%oror%|)|?"))
        {
            std::string cmp = tok->strAt(2);
            bool result = false;
            if (Token::Match(tok->tokAt(1), "%num%"))
            {
                // Compare numbers

                if (cmp == "==" || cmp == "!=")
                {
                    const std::string op1(tok->strAt(1));
                    const std::string op2(tok->strAt(3));

                    bool eq = false;
                    if (MathLib::isInt(op1) && MathLib::isInt(op2))
                        eq = (MathLib::toLongNumber(op1) == MathLib::toLongNumber(op2));
                    else
                        eq = (op1 == op2);

                    if (cmp == "==")
                        result = eq;
                    else
                        result = !eq;
                }
                else
                {
                    double op1 = MathLib::toDoubleNumber(tok->strAt(1));
                    double op2 = MathLib::toDoubleNumber(tok->strAt(3));
                    if (cmp == ">=")
                        result = (op1 >= op2);
                    else if (cmp == ">")
                        result = (op1 > op2);
                    else if (cmp == "<=")
                        result = (op1 <= op2);
                    else if (cmp == "<")
                        result = (op1 < op2);
                    else
                        cmp = "";
                }
            }
            else
            {
                // Compare boolean
                bool op1 = (tok->strAt(1) == std::string("true"));
                bool op2 = (tok->strAt(3) == std::string("true"));

                if (cmp == "==")
                    result = (op1 == op2);
                else if (cmp == "!=")
                    result = (op1 != op2);
                else if (cmp == ">=")
                    result = (op1 >= op2);
                else if (cmp == ">")
                    result = (op1 > op2);
                else if (cmp == "<=")
                    result = (op1 <= op2);
                else if (cmp == "<")
                    result = (op1 < op2);
                else
                    cmp = "";
            }

            if (! cmp.empty())
            {
                tok = tok->next();
                tok->deleteNext();
                tok->deleteNext();

                tok->str(result ? "true" : "false");
                ret = true;
            }
        }
    }

    return ret;
}

bool Tokenizer::simplifyQuestionMark()
{
    bool ret = false;
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (tok->str() != "?")
            continue;

        if (!tok->tokAt(-2))
            continue;

        if (!Token::Match(tok->tokAt(-2), "[=,(]"))
            continue;

        if (!Token::Match(tok->previous(), "%bool%") &&
            !Token::Match(tok->previous(), "%num%"))
            continue;

        // Find the ":" token..
        Token *semicolon = 0;
        {
            unsigned int parlevel = 0;
            for (Token *tok2 = tok; tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "(")
                    ++parlevel;
                else if (tok2->str() == ")")
                {
                    if (parlevel == 0)
                        break;
                    --parlevel;
                }
                else if (parlevel == 0 && tok2->str() == ":")
                {
                    semicolon = tok2;
                    break;
                }
            }
        }
        if (!semicolon || !semicolon->next())
            continue;

        if (tok->previous()->str() == "false" ||
            tok->previous()->str() == "0")
        {
            // Use code after semicolon, remove code before it.
            semicolon = semicolon->next();
            tok = tok->tokAt(-2);
            Token::eraseTokens(tok, semicolon);

            tok = tok->next();
            ret = true;
        }

        // The condition is true. Delete the operator after the ":"..
        else
        {
            const Token *end = 0;

            // check the operator after the :
            if (Token::simpleMatch(semicolon, ": ("))
            {
                end = semicolon->next()->link();
                if (!Token::Match(end, ") !!."))
                    continue;
            }

            // delete the condition token and the "?"
            tok = tok->tokAt(-2);
            Token::eraseTokens(tok, tok->tokAt(3));

            // delete operator after the :
            if (end)
            {
                Token::eraseTokens(semicolon->previous(), end->next());
                continue;
            }

            int ind = 0;
            for (const Token *endTok = semicolon; endTok; endTok = endTok->next())
            {
                if (endTok->str() == ";")
                {
                    Token::eraseTokens(semicolon->previous(), endTok);
                    ret = true;
                    break;
                }

                else if (Token::Match(endTok, "[({[]"))
                {
                    ++ind;
                }

                else if (Token::Match(endTok, "[)}]]"))
                {
                    --ind;
                    if (ind < 0)
                    {
                        Token::eraseTokens(semicolon->previous(), endTok);
                        ret = true;
                        break;
                    }
                }
            }
        }
    }

    return ret;
}

void Tokenizer::simplifyCasts()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        // #2897 : don't remove cast in such cases:
        // *((char *)a + 1) = 0;
        if (!tok->isName() && Token::simpleMatch(tok->next(), "* ("))
        {
            tok = tok->tokAt(2)->link();
            continue;
        }

        while (Token::Match(tok->next(), "( %type% *| ) *|&| %var%") ||
               Token::Match(tok->next(), "( %type% %type% *| ) *|&| %var%") ||
               (!tok->isName() && (Token::Match(tok->next(), "( %type% * ) (") ||
                                   Token::Match(tok->next(), "( %type% %type% * ) ("))))
        {
            if (tok->isName() && tok->str() != "return")
                break;

            if (Token::simpleMatch(tok->previous(), "operator"))
                break;

            // Remove cast..
            Token::eraseTokens(tok, tok->next()->link()->next());

            if (tok->str() == ")" && tok->link()->previous())
            {
                // If there was another cast before this, go back
                // there to check it also. e.g. "(int)(char)x"
                tok = tok->link()->previous();
            }
        }

        // Replace pointer casts of 0.. "(char *)0" => "0"
        while (Token::Match(tok->next(), "( %type% * ) 0") ||
               Token::Match(tok->next(), "( %type% %type% * ) 0"))
        {
            Token::eraseTokens(tok, tok->next()->link()->next());
            if (tok->str() == ")" && tok->link()->previous())
            {
                // If there was another cast before this, go back
                // there to check it also. e.g. "(char*)(char*)0"
                tok = tok->link()->previous();
            }
        }

        while (Token::Match(tok->next(), "dynamic_cast|reinterpret_cast|const_cast|static_cast <"))
        {
            Token *tok2 = tok->next();
            unsigned int level = 0;
            while (tok2)
            {
                if (tok2->str() == "<")
                    ++level;
                else if (tok2->str() == ">")
                {
                    --level;
                    if (level == 0)
                        break;
                }
                tok2 = tok2->next();
            }

            if (Token::simpleMatch(tok2, "> ("))
            {
                Token *closeBracket = tok2->next()->link();
                if (closeBracket)
                {
                    Token::eraseTokens(tok, tok2->tokAt(2));
                    closeBracket->deleteThis();
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }
}


void Tokenizer::simplifyFunctionParameters()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (tok->str() == "{" || tok->str() == "[" || tok->str() == "(")
        {
            tok = tok->link();
            if (!tok)
                break;
            continue;
        }

        // Find the function e.g. foo( x ) or foo( x, y )
        if (Token::Match(tok, "%var% ( %var% [,)]"))
        {
            // We have found old style function, now we need to change it

            // backup pointer to the '(' token
            Token * const tok1 = tok->next();

            // Get list of argument names
            std::map<std::string, Token*> argumentNames;
            bool bailOut = false;
            for (tok = tok->tokAt(2); tok; tok = tok->tokAt(2))
            {
                if (!Token::Match(tok, "%var% [,)]"))
                {
                    bailOut = true;
                    break;
                }

                if (argumentNames.find(tok->str()) != argumentNames.end())
                {
                    // Invalid code, two arguments with the same name.
                    // TODO, print error perhaps?
                    bailOut = true;
                    break;
                }

                argumentNames[tok->str()] = tok;
                if (tok->next()->str() == ")")
                {
                    tok = tok->tokAt(2);
                    break;
                }
            }

            if (bailOut)
            {
                tok = tok1->link();
                if (!tok)
                    return;
                continue;
            }

            Token *start = tok;
            while (tok && tok->str() != "{")
            {
                if (tok->str() == ";")
                {
                    tok = tok->previous();
                    // Move tokens from start to tok into the place of
                    // argumentNames[tok->str()] and remove the ";"

                    if (argumentNames.find(tok->str()) == argumentNames.end())
                    {
                        bailOut = true;
                        break;
                    }

                    // Remove the following ";"
                    Token *temp = tok->tokAt(2);
                    tok->deleteNext();

                    // Replace "x" with "int x" or similar
                    Token::replace(argumentNames[tok->str()], start, tok);
                    argumentNames.erase(tok->str());
                    tok = temp;
                    start = tok;
                }
                else
                {
                    tok = tok->next();
                }
            }

            if (Token::simpleMatch(tok, "{"))
                tok = tok->link();

            if (tok == NULL)
            {
                break;
            }

            if (bailOut)
            {
                continue;
            }
        }
    }
}


void Tokenizer:: simplifyFunctionPointers()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        // #2873 - dont simplify function pointer usage here:
        // (void)(xy(*p)(0));
        if (Token::simpleMatch(tok, ") ("))
        {
            tok = tok->next()->link();
            continue;
        }

        // check for function pointer cast
        if (Token::Match(tok, "( %type% *| *| ( * ) (") ||
            Token::Match(tok, "( %type% %type% *| *| ( * ) (") ||
            Token::Match(tok, "static_cast < %type% *| *| ( * ) (") ||
            Token::Match(tok, "static_cast < %type% %type% *| *| ( * ) ("))
        {
            Token *tok1 = tok;

            if (tok1->str() == "static_cast")
                tok1 = tok1->next();

            tok1 = tok1->next();

            if (Token::Match(tok1->next(), "%type%"))
                tok1 = tok1->next();

            while (tok1->next()->str() == "*")
                tok1 = tok1->next();

            // check that the cast ends
            if (!Token::Match(tok1->tokAt(4)->link(), ") )|>"))
                continue;

            // ok simplify this function pointer cast to an ordinary pointer cast
            tok1->deleteNext();
            tok1->next()->deleteNext();
            const Token *tok2 = tok1->tokAt(2)->link();
            Token::eraseTokens(tok1->next(), tok2 ? tok2->next() : 0);
            continue;
        }

        // check for start of statement
        else if (tok->previous() && !Token::Match(tok->previous(), "{|}|;|(|public:|protected:|private:"))
            continue;

        if (Token::Match(tok, "%type% *| *| ( * %var% ) ("))
            ;
        else if (Token::Match(tok, "%type% %type% *| *| ( * %var% ) ("))
            tok = tok->next();
        else
            continue;

        while (tok->next()->str() == "*")
            tok = tok->next();

        // check that the declaration ends
        if (!Token::Match(tok->tokAt(5)->link(), ") ;|,|)|=|["))
            continue;

        // ok simplify this function pointer to an ordinary pointer
        tok->deleteNext();
        tok->tokAt(2)->deleteNext();
        const Token *tok2 = tok->tokAt(3)->link();
        Token::eraseTokens(tok->tokAt(2), tok2 ? tok2->next() : 0);
    }
}


bool Tokenizer::simplifyFunctionReturn()
{
    bool ret = false;
    int indentlevel = 0;
    for (const Token *tok = tokens(); tok; tok = tok->next())
    {
        if (tok->str() == "{")
            ++indentlevel;

        else if (tok->str() == "}")
            --indentlevel;

        else if (indentlevel == 0 && Token::Match(tok, "%var% ( ) { return %num% ; }") && tok->str() != ")")
        {
            const std::string pattern("(|[|=|%op% " + tok->str() + " ( ) ;|]|)|%op%");
            for (Token *tok2 = _tokens; tok2; tok2 = tok2->next())
            {
                if (Token::Match(tok2, pattern.c_str()))
                {
                    tok2 = tok2->next();
                    tok2->str(tok->strAt(5));
                    tok2->deleteNext();
                    tok2->deleteNext();
                    ret = true;
                }
            }
        }
    }

    return ret;
}


static void incdec(std::string &value, const std::string &op)
{
    int ivalue = 0;
    std::istringstream istr(value.c_str());
    istr >> ivalue;
    if (op == "++")
        ++ivalue;
    else if (op == "--")
        --ivalue;
    std::ostringstream ostr;
    ostr << ivalue;
    value = ostr.str();
}



void Tokenizer::simplifyVarDecl()
{
    // Split up variable declarations..
    // "int a=4;" => "int a; a=4;"
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::simpleMatch(tok, "= {"))
        {
            tok = tok->next()->link();
            if (!tok)
                break;
        }

        if (tok->previous() && !Token::Match(tok->previous(), "{|}|;|)|public:|protected:|private:"))
            continue;

        Token *type0 = tok;
        if (!Token::Match(type0, "::| %type%"))
            continue;
        if (Token::Match(type0, "else|return|public:|protected:|private:"))
            continue;

        bool isconst = false;
        bool isstatic = false;
        Token *tok2 = type0;
        unsigned int typelen = 1;

        while (Token::Match(tok2, "%type% %type% *| *| %var%"))
        {
            if (tok2->str() == "const")
                isconst = true;

            else if (tok2->str() == "static")
                isstatic = true;

            tok2 = tok2->next();
            ++typelen;
        }

        // Don't split up const declaration..
        if (isconst && Token::Match(tok2, "%type% %var% ="))
            continue;

        // strange looking variable declaration => don't split up.
        if (Token::Match(tok2, "%type% *| %var% , %type% *| %var%"))
            continue;

        // check for qualification..
        if (Token::Match(tok2,  ":: %type%"))
        {
            typelen++;
            tok2 = tok2->next();
        }

        if (Token::Match(tok2, "%type% :: %type%"))
        {
            while (tok2 && Token::Match(tok2, "%type% ::"))
            {
                typelen += 2;
                tok2 = tok2->tokAt(2);
            }
        }

        if (Token::Match(tok2, "%type% *| %var% ,|="))
        {
            const bool isPointer = (tok2->next()->str() == "*");
            const Token *varName = tok2->tokAt((isPointer ? 2 : 1));
            Token *endDeclaration = varName->next();

            if (varName->str() != "operator")
            {
                tok2 = endDeclaration; // The ',' or '=' token

                if (isstatic && tok2->str() == "=")
                {
                    if (Token::Match(tok2->next(), "%num% ,"))
                        tok2 = tok2->tokAt(2);
                    else
                        tok2 = NULL;
                }
            }
            else
                tok2 = NULL;
        }

        else if (Token::Match(tok2, "%type% * * %var% ,|="))
        {
            if (tok2->tokAt(3)->str() != "operator")
                tok2 = tok2->tokAt(4);    // The ',' token
            else
                tok2 = NULL;
        }

        else if (Token::Match(tok2, "%type% * const %var% ,|="))
        {
            if (tok2->tokAt(3)->str() != "operator")
            {
                tok2 = tok2->tokAt(4);    // The ',' token
            }
            else
            {
                tok2 = NULL;
            }
        }

        else if (Token::Match(tok2, "%type% %var% [ %num% ] ,|=|[") ||
                 Token::Match(tok2, "%type% %var% [ %var% ] ,|=|["))
        {
            tok2 = tok2->tokAt(5);    // The ',' token
            while (Token::Match(tok2, "[ %num% ]") || Token::Match(tok2, "[ %var% ]"))
                tok2 = tok2->tokAt(3);
            if (!Token::Match(tok2, "=|,"))
            {
                tok2 = NULL;
            }

            if (tok2 && tok2->str() == "=")
            {
                while (tok2 && tok2->str() != ",")
                {
                    if (tok2->str() == "{")
                        tok2 = tok2->link();

                    tok2 = tok2->next();

                    if (tok2 && tok2->str() == ";")
                        tok2 = NULL;
                }
            }
        }

        else if (Token::Match(tok2, "%type% * %var% [ %num% ] ,") ||
                 Token::Match(tok2, "%type% * %var% [ %var% ] ,"))
        {
            tok2 = tok2->tokAt(6);    // The ',' token
        }

        else if (Token::Match(tok2, "%type% <"))
        {
            typelen += 2;
            tok2 = tok2->tokAt(2);
            size_t indentlevel = 1;

            for (Token *tok3 = tok2; tok3; tok3 = tok3->next())
            {
                ++typelen;

                if (tok3->str() == "<")
                {
                    ++indentlevel;
                }
                else if (tok3->str() == ">")
                {
                    --indentlevel;
                    if (indentlevel == 0)
                    {
                        tok2 = tok3->next();
                        break;
                    }
                }
                else if (tok3->str() == ";")
                {
                    break;
                }
            }

            if (!tok2) // syntax error
                break;

            if (Token::Match(tok2, ":: %type%"))
            {
                typelen += 2;
                tok2 = tok2->tokAt(2);
            }

            if (!tok2) // syntax error
                break;

            if (tok2->str() == "*")
            {
                tok2 = tok2->next();
            }

            if (Token::Match(tok2, "%var% ,|="))
            {
                tok2 = tok2->next();    // The ',' token
                typelen--;
            }
            else
            {
                tok2 = NULL;
                typelen = 0;
            }
        }
        else
        {
            tok2 = NULL;
            typelen = 0;
        }

        if (tok2)
        {
            if (tok2->str() == ",")
            {
                tok2->str(";");
                insertTokens(tok2, type0, typelen);
                std::stack<Token *> link1;
                std::stack<Token *> link2;
                while (((typelen--) > 0) && (0 != (tok2 = tok2->next())))
                {
                    if (tok2->str() == "(")
                        link1.push(tok2);
                    else if (tok2->str() == ")" && !link1.empty())
                    {
                        Token::createMutualLinks(tok2, link1.top());
                        link1.pop();
                    }

                    else if (tok2->str() == "[")
                        link2.push(tok2);
                    else if (tok2->str() == "]" && !link2.empty())
                    {
                        Token::createMutualLinks(tok2, link2.top());
                        link2.pop();
                    }
                }
            }

            else
            {
                Token *eq = tok2;

                unsigned int level = 0;
                while (tok2)
                {
                    if (Token::Match(tok2, "[{(]"))
                        tok2 = tok2->link();

                    else if (tok2->str() == "<")
                    {
                        if (tok2->previous()->isName() && !tok2->previous()->varId())
                            ++level;
                    }

                    else if (level > 0 && tok2->str() == ">")
                        --level;

                    else if (level == 0 && strchr(";,", tok2->str()[0]))
                    {
                        // "type var ="   =>   "type var; var ="
                        Token *VarTok = type0->tokAt((int)typelen);
                        while (Token::Match(VarTok, "*|&|const"))
                            VarTok = VarTok->next();
                        insertTokens(eq, VarTok, 2);
                        eq->str(";");

                        // "= x, "   =>   "= x; type "
                        if (tok2->str() == ",")
                        {
                            tok2->str(";");
                            insertTokens(tok2, type0, typelen);
                        }
                        break;
                    }

                    tok2 = tok2->next();
                }
            }
        }
    }
}

void Tokenizer::simplifyPlatformTypes()
{
    enum { isLongLong, isLong, isInt } type;

    /** @todo This assumes a flat address space. Not true for segmented address space (FAR *). */
    if (_settings->sizeof_size_t == 8)
        type = isLongLong;
    else if (_settings->sizeof_size_t == 4 && _settings->sizeof_long == 4)
        type = isLong;
    else if (_settings->sizeof_size_t == 4)
        type = isInt;
    else
        return;

    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::simpleMatch(tok, "std :: size_t|ssize_t|ptrdiff_t|intptr_t|uintptr_t"))
        {
            tok->deleteNext();
            tok->deleteThis();
        }
        else if (Token::simpleMatch(tok, ":: size_t|ssize_t|ptrdiff_t|intptr_t|uintptr_t"))
        {
            tok->deleteThis();
        }

        if (Token::Match(tok, "size_t|uintptr_t"))
        {
            tok->str("unsigned");

            switch (type)
            {
            case isLongLong:
                tok->insertToken("long");
                tok->insertToken("long");
                break;
            case isLong :
                tok->insertToken("long");
                break;
            case isInt:
                tok->insertToken("int");
                break;
            }
        }
        else if (Token::Match(tok, "ssize_t|ptrdiff_t|intptr_t"))
        {
            switch (type)
            {
            case isLongLong:
                tok->str("long");
                tok->insertToken("long");
                break;
            case isLong :
                tok->str("long");
                break;
            case isInt:
                tok->str("int");
                break;
            }
        }
    }

    if (_settings->platformType == Settings::Win32A ||
        _settings->platformType == Settings::Win32W ||
        _settings->platformType == Settings::Win64)
    {
        for (Token *tok = _tokens; tok; tok = tok->next())
        {
            if (Token::Match(tok, "BOOL|INT|INT32"))
                tok->str("int");
            else if (Token::Match(tok, "BOOLEAN|BYTE|UCHAR"))
            {
                tok->str("unsigned");
                tok->insertToken("char");
            }
            else if (tok->str() == "CHAR")
                tok->str("char");
            else if (Token::Match(tok, "DWORD|ULONG"))
            {
                tok->str("unsigned");
                tok->insertToken("long");
            }
            else if (Token::Match(tok, "DWORD_PTR|ULONG_PTR|SIZE_T"))
            {
                tok->str("unsigned");
                tok->insertToken("long");
                if (_settings->platformType == Settings::Win64)
                    tok->insertToken("long");
            }
            else if (tok->str() == "FLOAT")
                tok->str("float");
            else if (tok->str() == "HRESULT")
                tok->str("long");
            else if (tok->str() == "INT64")
            {
                tok->str("long");
                tok->insertToken("long");
            }
            else if (tok->str() == "LONG")
                tok->str("long");
            else if (tok->str() == "LONG_PTR")
            {
                tok->str("long");
                if (_settings->platformType == Settings::Win64)
                    tok->insertToken("long");
            }
            else if (Token::Match(tok, "LPBOOL|PBOOL"))
            {
                tok->str("int");
                tok->insertToken("*");
            }
            else if (Token::Match(tok, "LPBYTE|PBOOLEAN|PBYTE"))
            {
                tok->str("unsigned");
                tok->insertToken("*");
                tok->insertToken("char");
            }
            else if (Token::Match(tok, "LPCSTR|PCSTR"))
            {
                tok->str("const");
                tok->insertToken("*");
                tok->insertToken("char");
            }
            else if (tok->str() == "LPCVOID")
            {
                tok->str("const");
                tok->insertToken("*");
                tok->insertToken("void");
            }
            else if (tok->str() == "LPDWORD")
            {
                tok->str("unsigned");
                tok->insertToken("*");
                tok->insertToken("long");
            }
            else if (Token::Match(tok, "LPINT|PINT"))
            {
                tok->str("int");
                tok->insertToken("*");
            }
            else if (Token::Match(tok, "LPLONG|PLONG"))
            {
                tok->str("long");
                tok->insertToken("*");
            }
            else if (Token::Match(tok, "LPSTR|PSTR|PCHAR"))
            {
                tok->str("char");
                tok->insertToken("*");
            }
            else if (Token::Match(tok, "LPVOID|PVOID|HANDLE"))
            {
                tok->str("void");
                tok->insertToken("*");
            }
            else if (Token::Match(tok, "LPWORD|PWORD"))
            {
                tok->str("unsigned");
                tok->insertToken("*");
                tok->insertToken("short");
            }
            else if (tok->str() == "SHORT")
                tok->str("short");
            else if (tok->str() == "UINT")
            {
                tok->str("unsigned");
                tok->insertToken("int");
            }
            else if (tok->str() == "UINT_PTR")
            {
                tok->str("unsigned");
                if (_settings->platformType == Settings::Win64)
                {
                    tok->insertToken("long");
                    tok->insertToken("long");
                }
                else
                    tok->insertToken("long");
            }
            else if (Token::Match(tok, "USHORT|WORD|WCHAR|wchar_t"))
            {
                tok->str("unsigned");
                tok->insertToken("short");
            }
            else if (tok->str() == "VOID")
                tok->str("void");
            else if (tok->str() == "TCHAR")
            {
                if (_settings->platformType == Settings::Win32A)
                    tok->str("char");
                else
                {
                    tok->str("unsigned");
                    tok->insertToken("short");
                }
            }
            else if (Token::Match(tok, "PTSTR|LPTSTR"))
            {
                if (_settings->platformType == Settings::Win32A)
                {
                    tok->str("char");
                    tok->insertToken("*");
                }
                else
                {
                    tok->str("unsigned");
                    tok->insertToken("*");
                    tok->insertToken("short");
                }
            }
            else if (Token::Match(tok, "PCTSTR|LPCTSTR"))
            {
                if (_settings->platformType == Settings::Win32A)
                {
                    tok->str("const");
                    tok->insertToken("*");
                    tok->insertToken("char");
                }
                else
                {
                    tok->str("const");
                    tok->insertToken("*");
                    tok->insertToken("short");
                    tok->insertToken("unsigned");
                }
            }
        }
    }
}

void Tokenizer::simplifyStdType()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        // long unsigned => unsigned long
        if (Token::Match(tok, "char|short|int|long|__int8|__int16|__int32|__int64 unsigned|signed"))
        {
            std::string temp = tok->str();
            tok->str(tok->next()->str());
            tok->next()->str(temp);
        }

        if (!Token::Match(tok, "unsigned|signed|char|short|int|long|__int8|__int16|__int32|__int64"))
            continue;

        // check if signed or unsigned specified
        if (Token::Match(tok, "unsigned|signed"))
        {
            bool isUnsigned = tok->str() == "unsigned";

            // unsigned i => unsigned int i
            if (!Token::Match(tok->next(), "char|short|int|long|__int8|__int16|__int32|__int64"))
                tok->str("int");
            else
                tok->deleteThis();
            tok->isUnsigned(isUnsigned);
            tok->isSigned(!isUnsigned);
        }

        if (Token::simpleMatch(tok, "__int8"))
            tok->str("char");
        else if (Token::simpleMatch(tok, "__int16"))
            tok->str("short");
        else if (Token::simpleMatch(tok, "__int32"))
            tok->str("int");
        else if (Token::simpleMatch(tok, "__int64"))
        {
            tok->str("long");
            tok->isLong(true);
        }
        else if (Token::simpleMatch(tok, "long"))
        {
            if (Token::simpleMatch(tok->next(), "long"))
            {
                tok->isLong(true);
                tok->deleteNext();
            }

            if (Token::simpleMatch(tok->next(), "int"))
                tok->deleteNext();
            else if (Token::simpleMatch(tok->next(), "double"))
            {
                tok->str("double");
                tok->isLong(true);
                tok->deleteNext();
            }
        }
        else if (Token::simpleMatch(tok, "short"))
        {
            if (Token::simpleMatch(tok->next(), "int"))
                tok->deleteNext();
        }
    }
}

void Tokenizer::simplifyIfAssign()
{
    // See also simplifyFunctionAssign

    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (!Token::Match(tok->next(), "if|while ( !| (| %var% =") &&
            !Token::Match(tok->next(), "if|while ( !| (| %var% . %var% ="))
            continue;

        // simplifying a "while" condition ?
        const bool iswhile(tok->next()->str() == "while");

        // delete the "if"
        tok->deleteNext();

        // Remember if there is a "!" or not. And delete it if there are.
        const bool isNot(tok->tokAt(2)->str() == "!");
        if (isNot)
            tok->next()->deleteNext();

        // Delete parenthesis.. and remember how many there are with
        // their links.
        std::stack<Token *> braces;
        while (tok->next()->str() == "(")
        {
            braces.push(tok->next()->link());
            tok->deleteNext();
        }

        // Skip the "%var% = ..."
        Token *tok2;
        unsigned int indentlevel = 0;
        for (tok2 = tok->next(); tok2; tok2 = tok2->next())
        {
            if (tok2->str() == "(")
                ++indentlevel;
            else if (tok2->str() == ")")
            {
                if (indentlevel == 0)
                    break;
                --indentlevel;
            }
        }

        // Insert "; if|while ( .."
        tok2 = tok2->previous();
        if (Token::simpleMatch(tok->tokAt(2), "."))
        {
            tok2->insertToken(tok->strAt(3));
            tok2->insertToken(tok->strAt(2));
        }
        tok2->insertToken(tok->strAt(1));
        tok2->next()->varId(tok->tokAt(1)->varId());

        while (! braces.empty())
        {
            tok2->insertToken("(");
            Token::createMutualLinks(tok2->next(), braces.top());
            braces.pop();
        }

        if (isNot)
            tok2->next()->insertToken("!");
        tok2->insertToken(iswhile ? "while" : "if");
        tok2->insertToken(";");

        // If it's a while loop.. insert the assignment in the loop
        if (iswhile)
        {
            indentlevel = 0;
            Token *tok3 = tok2;
            for (tok3 = tok2; tok3; tok3 = tok3->next())
            {
                if (tok3->str() == "{")
                    ++indentlevel;
                else if (tok3->str() == "}")
                {
                    if (indentlevel <= 1)
                        break;
                    --indentlevel;
                }
            }

            if (tok3 && indentlevel == 1)
            {
                tok3 = tok3->previous();
                std::stack<Token *> braces2;

                for (tok2 = tok2->next(); tok2 && tok2 != tok; tok2 = tok2->previous())
                {
                    tok3->insertToken(tok2->str());

                    Token *newTok = tok3->next();
                    newTok->fileIndex(tok2->fileIndex());
                    newTok->linenr(tok2->linenr());

                    // link() newly tokens manually
                    if (Token::Match(newTok, "}|)|]"))
                    {
                        braces2.push(newTok);
                    }
                    else if (Token::Match(newTok, "{|(|["))
                    {
                        Token::createMutualLinks(newTok, braces2.top());
                        braces2.pop();
                    }
                }
            }
        }
    }
}


void Tokenizer::simplifyVariableMultipleAssign()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "%var% = %var% = %num% ;") ||
            Token::Match(tok, "%var% = %var% = %var% ;"))
        {
            // skip intermediate assignments
            Token *tok2 = tok->previous();
            while (tok2 &&
                   tok2->str() == "=" &&
                   Token::Match(tok2->previous(), "%var%"))
            {
                tok2 = tok2->tokAt(-2);
            }

            if (!tok2 || tok2->str() != ";")
            {
                continue;
            }

            Token *stopAt = tok->tokAt(2);
            const Token *valueTok = tok->tokAt(4);
            const std::string value(valueTok->str());
            tok2 = tok2->next();

            while (tok2 != stopAt)
            {
                tok2->next()->insertToken(";");
                tok2->next()->insertToken(value);
                tok2 = tok2->tokAt(4);
            }
        }
    }
}


void Tokenizer::simplifyIfNot()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "(|&&|%oror%"))
        {
            tok = tok->next();
            while (tok && tok->str() == "(")
                tok = tok->next();

            if (!tok)
                break;

            if (Token::Match(tok, "0|false == (") ||
                Token::Match(tok, "0|false == %var%"))
            {
                tok->deleteNext();
                tok->str("!");
            }

            else if (Token::Match(tok, "%var% == 0|false"))
            {
                tok->deleteNext();
                tok->next()->str(tok->str());
                tok->str("!");
            }

            else if (Token::Match(tok, "%var% .|:: %var% == 0|false"))
            {
                tok = tok->previous();
                tok->insertToken("!");
                tok = tok->tokAt(4);
                Token::eraseTokens(tok, tok->tokAt(3));
            }

            else if (Token::Match(tok, "* %var% == 0|false"))
            {
                tok = tok->previous();
                tok->insertToken("!");
                tok = tok->tokAt(3);
                Token::eraseTokens(tok, tok->tokAt(3));
            }
        }

        else if (tok->link() && Token::Match(tok, ") == 0|false"))
        {
            // if( foo(x) == 0 )
            if (Token::Match(tok->link()->tokAt(-2), "( %var%"))
            {
                Token::eraseTokens(tok, tok->tokAt(3));
                tok->link()->previous()->insertToken(tok->link()->previous()->str().c_str());
                tok->link()->previous()->previous()->str("!");
            }

            // if( (x) == 0 )
            else if (Token::simpleMatch(tok->link()->previous(), "("))
            {
                Token::eraseTokens(tok, tok->tokAt(3));
                tok->link()->insertToken("(");
                tok->link()->str("!");
                Token *temp = tok->link();
                Token::createMutualLinks(tok->link()->next(), tok);
                temp->link(0);
            }
        }
    }
}


void Tokenizer::simplifyIfNotNull()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        Token *deleteFrom = NULL;

        // Remove 'x = (x != 0)'
        if (Token::simpleMatch(tok, "= ("))
        {
            if (Token::Match(tok->tokAt(-2), "[;{}] %var%"))
            {
                const std::string varname(tok->previous()->str());

                if (Token::Match(tok->tokAt(2), (varname + " != 0 ) ;").c_str()) ||
                    Token::Match(tok->tokAt(2), ("0 != " + varname + " ) ;").c_str()))
                {
                    tok = tok->tokAt(-2);
                    Token::eraseTokens(tok, tok->tokAt(9));
                }
            }
            continue;
        }

        if (Token::Match(tok, "(|&&|%oror%"))
        {
            tok = tok->next();

            if (!tok)
                break;

            if (Token::simpleMatch(tok, "0 != (") ||
                Token::Match(tok, "0 != %var%"))
            {
                deleteFrom = tok->previous();
                if (tok->tokAt(2))
                    tok->tokAt(2)->isPointerCompare(true);
            }

            else if (Token::Match(tok, "%var% != 0"))
            {
                deleteFrom = tok;
                tok->isPointerCompare(true);
            }

            else if (Token::Match(tok, "%var% .|:: %var% != 0"))
            {
                tok = tok->tokAt(2);
                deleteFrom = tok;
                tok->isPointerCompare(true);
            }
        }

        else if (tok->link() && Token::simpleMatch(tok, ") != 0"))
        {
            deleteFrom = tok;
        }

        if (deleteFrom)
        {
            Token::eraseTokens(deleteFrom, deleteFrom->tokAt(3));
            tok = deleteFrom;
        }
    }
}


void Tokenizer::simplifyIfSameInnerCondition()
{
    // same inner condition
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "if ( %var% ) {"))
        {
            const unsigned int varid(tok->tokAt(2)->varId());
            if (!varid)
                continue;

            for (Token *tok2 = tok->tokAt(5); tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "{" || tok2->str() == "}")
                    break;
                if (Token::simpleMatch(tok2, "if ("))
                {
                    tok2 = tok2->tokAt(2);
                    if (Token::Match(tok2, "%varid% )", varid))
                        tok2->str("true");
                    else if (Token::Match(tok2, "! %varid% )", varid))
                        tok2->next()->varId(varid);
                    break;
                }
            }
        }
    }
}


bool Tokenizer::simplifyLogicalOperators()
{
    bool ret = false;

    // "if (not p)" => "if (!p)"
    // "if (p and q)" => "if (p && q)"
    // "if (p or q)" => "if (p || q)"
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "if|while ( not|compl %var%"))
        {
            tok->tokAt(2)->str(tok->strAt(2) == "not" ? "!" : "~");
            ret = true;
        }
        else if (Token::Match(tok, "&& not|compl %var%"))
        {
            tok->next()->str(tok->strAt(1) == "not" ? "!" : "~");
            ret = true;
        }
        else if (Token::Match(tok, "|| not|compl %var%"))
        {
            tok->next()->str(tok->strAt(1) == "not" ? "!" : "~");
            ret = true;
        }
        // "%var%|) and %var%|("
        else if (Token::Match(tok->previous(), "%any% %var% %any%"))
        {
            if (!Token::Match(tok, "and|or|bitand|bitor|xor|not_eq"))
                continue;

            const Token *tok2 = tok;
            while (0 != (tok2 = tok2->previous()))
            {
                if (tok2->str() == ")")
                    tok2 = tok2->link();
                else if (Token::Match(tok2, "(|;|{|}"))
                    break;
            }
            if (tok2 && Token::Match(tok2->previous(), "if|while ("))
            {
                if (tok->str() == "and")
                    tok->str("&&");
                else if (tok->str() == "or")
                    tok->str("||");
                else if (tok->str() == "bitand")
                    tok->str("&");
                else if (tok->str() == "bitor")
                    tok->str("|");
                else if (tok->str() == "xor")
                    tok->str("^");
                else if (tok->str() == "not_eq")
                    tok->str("!=");
                ret = true;
            }
        }
    }
    return ret;
}

// int i(0); => int i; i = 0;
// int i(0), j; => int i; i = 0; int j;
void Tokenizer::simplifyInitVar()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (!tok->isName() || (tok->previous() && !Token::Match(tok->previous(), "[;{}]")))
            continue;

        if (Token::Match(tok, "class|struct|union| %type% *| %var% ( &| %any% ) ;") ||
            Token::Match(tok, "%type% *| %var% ( %type% ("))
        {
            tok = initVar(tok);
        }
        else if (Token::Match(tok, "class|struct|union| %type% *| %var% ( &| %any% ) ,"))
        {
            Token *tok1 = tok;
            while (tok1->str() != ",")
                tok1 = tok1->next();
            tok1->str(";");
            Token *tok2 = tok;
            if (Token::Match(tok2, "class|struct|union"))
            {
                tok1->insertToken(tok2->str());
                tok1 = tok1->next();
                tok2 = tok2->next();
            }
            tok1->insertToken(tok2->str());
            tok1 = tok1->next();
            tok2 = tok2->next();
            if (tok2->str() == "*")
            {
                tok1->insertToken("*");
            }
            tok = initVar(tok);
        }
    }
}

Token * Tokenizer::initVar(Token * tok)
{
    // call constructor of class => no simplification
    if (Token::Match(tok, "class|struct|union"))
    {
        if (tok->tokAt(2)->str() != "*")
            return tok;

        tok = tok->next();
    }
    else if (!tok->isStandardType() && tok->tokAt(1)->str() != "*")
        return tok;

    // goto variable name..
    tok = tok->next();
    if (tok->str() == "*")
        tok = tok->next();

    // sizeof is not a variable name..
    if (tok->str() == "sizeof")
        return tok;

    // check initializer..
    if (tok->tokAt(2)->isStandardType() || tok->tokAt(2)->str() == "void")
        return tok;
    else if (!tok->tokAt(2)->isNumber() && !Token::Match(tok->tokAt(2), "%type% (") && tok->tokAt(2)->str() != "&" && tok->tokAt(2)->varId() == 0)
        return tok;

    // insert '; var ='
    tok->insertToken(";");
    tok->next()->insertToken(tok->str());
    tok->tokAt(2)->varId(tok->varId());
    tok = tok->tokAt(2);
    tok->insertToken("=");

    // goto '('..
    tok = tok->tokAt(2);

    // delete ')'
    tok->link()->deleteThis();

    // delete this
    tok->deleteThis();

    return tok;
}


bool Tokenizer::simplifyKnownVariables()
{
    // return value for function. Set to true if any simplifications are made
    bool ret = false;

    // constants..
    {
        std::map<unsigned int, std::string> constantValues;
        for (Token *tok = _tokens; tok; tok = tok->next())
        {
            if (tok->isName() && Token::Match(tok, "static| const| static| %type% const| %var% = %any% ;"))
            {
                bool isconst = false;
                for (const Token *tok2 = tok; tok2->str() != "="; tok2 = tok2->next())
                {
                    if (tok2->str() == "const")
                    {
                        isconst = true;
                        break;
                    }
                }
                if (!isconst)
                    continue;

                Token *tok1 = tok;

                // start of statement
                if (tok != _tokens && !Token::Match(tok->previous(),"[;{}]"))
                    continue;
                // skip "const" and "static"
                while (tok->str() == "const" || tok->str() == "static")
                    tok = tok->next();
                // pod type
                if (!tok->isStandardType())
                    continue;

                const Token * const vartok = (tok->strAt(1) == "const") ? tok->tokAt(2) : tok->next();
                const Token * const valuetok = vartok->tokAt(2);
                if (valuetok->isNumber() || Token::Match(valuetok, "%str% ;"))
                {
                    constantValues[vartok->varId()] = valuetok->str();

                    // remove statement
                    while (tok1->str() != ";")
                        tok1->deleteThis();
                    tok = tok1;
                }
            }

            else if (tok->varId() && constantValues.find(tok->varId()) != constantValues.end())
            {
                tok->str(constantValues[tok->varId()]);
            }
        }
    }

    // variable id for float/double variables
    std::set<unsigned int> floatvars;

    // auto variables..
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        // Search for a block of code
        if (! Token::Match(tok, ") const| {"))
            continue;

        // parse the block of code..
        int indentlevel = 0;
        Token *tok2 = tok;
        for (; tok2; tok2 = tok2->next())
        {
            if (Token::Match(tok2, "[;{}] float|double %var% ;"))
            {
                floatvars.insert(tok2->tokAt(2)->varId());
            }

            if (tok2->str() == "{")
                ++indentlevel;

            else if (tok2->str() == "}")
            {
                --indentlevel;
                if (indentlevel <= 0)
                    break;
            }

            else if (tok2->previous()->str() != "*" &&
                     (Token::Match(tok2, "%var% = %num% ;") ||
                      Token::Match(tok2, "%var% = %str% ;") ||
                      (Token::Match(tok2, "%var% = %any% ;") && tok2->strAt(2)[0] == '\'') ||
                      Token::Match(tok2, "%var% [ ] = %str% ;") ||
                      Token::Match(tok2, "%var% [ %num% ] = %str% ;") ||
                      Token::Match(tok2, "%var% = %bool% ;") ||
                      Token::Match(tok2, "%var% = %var% ;") ||
                      Token::Match(tok2, "%var% = & %var% ;") ||
                      Token::Match(tok2, "%var% = & %var% [ 0 ] ;")))
            {
                const unsigned int varid = tok2->varId();
                if (varid == 0)
                    continue;

                // skip loop variable
                if (Token::Match(tok2->tokAt(-2), "(|:: %type%"))
                {
                    const Token *tok3 = tok2->previous();
                    while (Token::Match(tok3->previous(), ":: %type%"))
                        tok3 = tok3->tokAt(-2);
                    if (Token::Match(tok3->tokAt(-2), "for ( %type%"))
                        continue;
                }

                // struct name..
                const std::string structname = Token::Match(tok2->tokAt(-3), "[;{}] %var% .") ?
                                               std::string(tok2->strAt(-2) + " .") :
                                               std::string("");

                if (tok2->str() == tok2->strAt(2))
                    continue;

                const Token * const valueToken = tok2->tokAt(2);

                std::string value;
                unsigned int valueVarId = 0;

                Token *tok3 = NULL;
                bool valueIsPointer = false;

                if (!simplifyKnownVariablesGetData(varid, &tok2, &tok3, value, valueVarId, valueIsPointer, floatvars.find(tok2->varId()) != floatvars.end()))
                    continue;

                ret |= simplifyKnownVariablesSimplify(&tok2, tok3, varid, structname, value, valueVarId, valueIsPointer, valueToken, indentlevel);
            }

            else if (Token::Match(tok2, "strcpy ( %var% , %str% ) ;"))
            {
                const unsigned int varid(tok2->tokAt(2)->varId());
                if (varid == 0)
                    continue;
                const std::string structname("");
                const Token * const valueToken = tok2->tokAt(4);
                std::string value(valueToken->str());
                const unsigned int valueVarId(0);
                const bool valueIsPointer(false);
                Token *tok3 = tok2;
                for (int i = 0; i < 6; ++i)
                    tok3 = tok3->next();
                ret |= simplifyKnownVariablesSimplify(&tok2, tok3, varid, structname, value, valueVarId, valueIsPointer, valueToken, indentlevel);
            }
        }

        if (tok2)
            tok = tok2->previous();
    }

    return ret;
}

bool Tokenizer::simplifyKnownVariablesGetData(unsigned int varid, Token **_tok2, Token **_tok3, std::string &value, unsigned int &valueVarId, bool &valueIsPointer, bool floatvar)
{
    Token *tok2 = *_tok2;
    Token *tok3 = *_tok3;

    if (Token::Match(tok2->tokAt(-2), "for ( %varid% = %num% ; %varid% <|<= %num% ; ++| %varid% ++| ) {", varid))
    {
        // is there a "break" in the for loop?
        bool hasbreak = false;
        unsigned int indentlevel4 = 0;   // indentlevel for tok4
        for (const Token *tok4 = tok2->previous()->link(); tok4; tok4 = tok4->next())
        {
            if (tok4->str() == "{")
                ++indentlevel4;
            else if (tok4->str() == "}")
            {
                if (indentlevel4 <= 1)
                    break;
                --indentlevel4;
            }
            else if (tok4->str() == "break")
            {
                hasbreak = true;
                break;
            }
        }
        if (hasbreak)
            return false;

        // no break => the value of the counter value is known after the for loop..
        const std::string compareop = tok2->strAt(5);
        if (compareop == "<")
        {
            value = tok2->strAt(6);
            valueVarId = tok2->tokAt(6)->varId();
        }
        else
            value = MathLib::toString(MathLib::toLongNumber(tok2->strAt(6)) + 1);

        // Skip for-body..
        tok3 = tok2->previous()->link()->next()->link()->next();
    }
    else
    {
        value = tok2->strAt(2);
        valueVarId = tok2->tokAt(2)->varId();
        if (Token::simpleMatch(tok2->next(), "["))
        {
            value = tok2->next()->link()->strAt(2);
            valueVarId = 0;
        }
        else if (value == "&")
        {
            value = tok2->strAt(3);
            valueVarId = tok2->tokAt(3)->varId();

            // *ptr = &var; *ptr = 5;
            // equals
            // var = 5; not *var = 5;
            if (tok2->strAt(4) == ";")
                valueIsPointer = true;
        }

        // float value should contain a "."
        else if (tok2->tokAt(2)->isNumber() &&
                 floatvar &&
                 value.find(".") == std::string::npos)
        {
            value += ".0";
        }

        if (Token::simpleMatch(tok2->next(), "= &"))
            tok2 = tok2->tokAt(3);

        tok3 = tok2->next();
    }
    *_tok2 = tok2;
    *_tok3 = tok3;
    return true;
}

bool Tokenizer::simplifyKnownVariablesSimplify(Token **tok2, Token *tok3, unsigned int varid, const std::string &structname, std::string &value, unsigned int valueVarId, bool valueIsPointer, const Token * const valueToken, int indentlevel)
{
    const bool pointeralias(valueToken->isName() || Token::Match(valueToken, "& %var% ["));

    bool ret = false;

    Token* bailOutFromLoop = 0;
    int indentlevel3 = indentlevel;
    bool ret3 = false;
    for (; tok3; tok3 = tok3->next())
    {
        if (tok3->str() == "{")
        {
            ++indentlevel3;
        }
        else if (tok3->str() == "}")
        {
            --indentlevel3;
            if (indentlevel3 < indentlevel)
            {
                if (Token::Match((*tok2)->tokAt(-7), "%type% * %var% ; %var% = & %var% ;") &&
                    (*tok2)->tokAt(-5)->str() == (*tok2)->tokAt(-3)->str())
                {
                    (*tok2) = (*tok2)->tokAt(-4);
                    Token::eraseTokens((*tok2), (*tok2)->tokAt(5));
                }
                break;
            }
        }

        // Stop if there is a pointer alias and a shadow variable is
        // declared in an inner scope (#3058)
        if (valueIsPointer && tok3->varId() > 0 &&
            tok3->previous() && (tok3->previous()->isName() || tok3->previous()->str() == "*") &&
            valueToken->str() == "&" &&
            valueToken->next() &&
            valueToken->next()->isName() &&
            tok3->str() == valueToken->next()->str() &&
            tok3->varId() > valueToken->next()->varId())
        {
            // more checking if this is a variable declaration
            bool decl = true;
            for (const Token *tok4 = tok3->previous(); tok4; tok4 = tok4->previous())
            {
                if (Token::Match(tok4, "[;{}]"))
                    break;

                else if (tok4->isName())
                {
                    if (tok4->varId() > 0)
                    {
                        decl = false;
                        break;
                    }
                }

                else if (!Token::Match(tok4, "[&*]"))
                {
                    decl = false;
                    break;
                }
            }
            if (decl)
                break;
        }

        // Stop if label is found
        if (Token::Match(tok3, "; %type% : ;"))
            break;

        // Stop if return or break is found ..
        if (tok3->str() == "break")
            break;
        if ((indentlevel3 > 1 || !Token::simpleMatch(Token::findmatch(tok3,";"), "; }")) && tok3->str() == "return")
            ret3 = true;
        if (ret3 && tok3->str() == ";")
            break;

        if (pointeralias && Token::Match(tok3, ("!!= " + value).c_str()))
            break;

        // Stop if do is found
        if (tok3->str() == "do")
            break;

        // Stop if unknown function call is seen
        // If the variable is a global or a member variable it might be
        // changed by the function call
        // TODO: don't bail out if the variable is a local variable,
        //       then it can't be changed by the function call.
        if (tok3->str() == ")" && tok3->link() &&
            Token::Match(tok3->link()->tokAt(-2), "[;{}] %var% (") &&
            !Token::Match(tok3->link()->previous(), "if|for|while|switch"))
            break;

        // Stop if something like 'while (--var)' is found
        if (tok3->str() == "for" || tok3->str() == "while" || tok3->str() == "do")
        {
            const Token *endpar = tok3->next()->link();
            if (Token::simpleMatch(endpar, ") {"))
                endpar = endpar->next()->link();
            bool bailout = false;
            for (const Token *tok4 = tok3; tok4 && tok4 != endpar; tok4 = tok4->next())
            {
                if (Token::Match(tok4, "++|-- %varid%", varid) ||
                    Token::Match(tok4, "%varid% ++|--|=", varid))
                {
                    bailout = true;
                    break;
                }
            }
            if (bailout)
                break;
        }

        if (bailOutFromLoop)
        {
            // This could be a loop, skip it, but only if it doesn't contain
            // the variable we are checking for. If it contains the variable
            // we will bail out.
            if (tok3->varId() == varid)
            {
                // Continue
                //tok2 = bailOutFromLoop;
                break;
            }
            else if (tok3 == bailOutFromLoop)
            {
                // We have skipped the loop
                bailOutFromLoop = 0;
                continue;
            }

            continue;
        }
        else if (tok3->str() == "{" && tok3->previous()->str() == ")")
        {
            // There is a possible loop after the assignment. Try to skip it.
            if (tok3->previous()->link() &&
                !Token::simpleMatch(tok3->previous()->link()->previous(), "if"))
                bailOutFromLoop = tok3->link();
            continue;
        }

        // Variable used in realloc (see Ticket #1649)
        if (Token::Match(tok3, "%var% = realloc ( %var% ,") &&
            tok3->varId() == varid &&
            tok3->tokAt(4)->varId() == varid)
        {
            tok3->tokAt(4)->str(value);
            ret = true;
        }

        // condition "(|&&|%OROR% %varid% )|&&|%OROR%
        if (!Token::Match(tok3->previous(), "( %var% )") &&
            Token::Match(tok3->previous(), "&&|(|%oror% %varid% &&|%oror%|)", varid))
        {
            tok3->str(value);
            ret = true;
        }

        // Variable is used somehow in a non-defined pattern => bail out
        if (tok3->varId() == varid)
        {
            // This is a really generic bailout so let's try to avoid this.
            // There might be lots of false negatives.
            if (_settings->debugwarnings)
            {
                // FIXME: Fix all the debug warnings for values and then
                // remove this bailout
                if (pointeralias)
                    break;

                // suppress debug-warning when calling member function
                if (Token::Match(tok3->next(), ". %var% ("))
                    break;

                // suppress debug-warning when assignment
                if (Token::simpleMatch(tok3->next(), "="))
                    break;

                // taking address of variable..
                if (Token::Match(tok3->tokAt(-2), "return|= & %var% ;"))
                    break;

                // parameter in function call..
                if (Token::Match(tok3->tokAt(-2), "%var% ( %var% ,|)") ||
                    Token::Match(tok3->previous(), ", %var% ,|)"))
                    break;

                // conditional increment
                if (Token::Match(tok3->tokAt(-3), ") { ++|--") ||
                    Token::Match(tok3->tokAt(-2), ") { %var% ++|--"))
                    break;

                std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
                ErrorLogger::ErrorMessage::FileLocation loc;
                loc.line = tok3->linenr();
                loc.setfile(file(tok3));
                locationList.push_back(loc);

                const ErrorLogger::ErrorMessage errmsg(locationList,
                                                       Severity::debug,
                                                       "simplifyKnownVariables: bailing out (variable="+tok3->str()+", value="+value+")",
                                                       "debug",
                                                       false);

                if (_errorLogger)
                    _errorLogger->reportErr(errmsg);
                else
                    Check::reportError(errmsg);
            }

            break;
        }

        // Using the variable in condition..
        if (Token::Match(tok3->previous(), ("if ( " + structname + " %varid% ==|!=|<|<=|>|>=|)").c_str(), varid) ||
            Token::Match(tok3, ("( " + structname + " %varid% ==|!=|<|<=|>|>=").c_str(), varid) ||
            Token::Match(tok3, ("!|==|!=|<|<=|>|>= " + structname + " %varid% ==|!=|<|<=|>|>=|)|;").c_str(), varid) ||
            Token::Match(tok3->previous(), "strlen|free ( %varid% )", varid))
        {
            if (value[0] == '\"' && tok3->strAt(-1) != "strlen")
            {
                // bail out if value is a string unless if it's just given
                // as parameter to strlen
                break;
            }
            if (!structname.empty())
            {
                tok3->deleteNext();
                tok3->deleteNext();
            }
            if (Token::Match(valueToken, "& %var% ;"))
            {
                tok3->insertToken("&");
                tok3 = tok3->next();
            }
            tok3 = tok3->next();
            tok3->str(value);
            tok3->varId(valueVarId);
            ret = true;
        }

        // Delete pointer alias
        if (pointeralias && tok3->str() == "delete" &&
            (Token::Match(tok3, "delete %varid% ;", varid) ||
             Token::Match(tok3, "delete [ ] %varid%", varid)))
        {
            tok3 = (tok3->strAt(1) == "[") ? tok3->tokAt(3) : tok3->next();
            tok3->str(value);
            tok3->varId(valueVarId);
            ret = true;
        }

        // Variable is used in function call..
        if (Token::Match(tok3, ("%var% ( " + structname + " %varid% ,").c_str(), varid))
        {
            const char * const functionName[] =
            {
                "memcmp","memcpy","memmove","memset",
                "strcmp","strcpy","strncpy","strdup"
            };
            for (unsigned int i = 0; i < (sizeof(functionName) / sizeof(*functionName)); ++i)
            {
                if (tok3->str() == functionName[i])
                {
                    Token *par1 = tok3->next()->next();
                    if (!structname.empty())
                    {
                        par1->deleteThis();
                        par1->deleteThis();
                    }
                    par1->str(value);
                    par1->varId(valueVarId);
                    break;
                }
            }
        }

        // Variable is used as 2nd parameter in function call..
        if (Token::Match(tok3, ("%var% ( %any% , " + structname + " %varid% ,|)").c_str(), varid))
        {
            const char * const functionName[] =
            {
                "memcmp","memcpy","memmove",
                "strcmp","strcpy","strncmp","strncpy"
            };
            for (unsigned int i = 0; i < (sizeof(functionName) / sizeof(*functionName)); ++i)
            {
                if (tok3->str() == functionName[i])
                {
                    Token *par = tok3->tokAt(4);
                    if (!structname.empty())
                    {
                        par->deleteThis();
                        par->deleteThis();
                    }
                    par->str(value);
                    par->varId(valueVarId);
                    break;
                }
            }
        }

        // array usage
        if (Token::Match(tok3, ("[(,] " + structname + " %varid% [|%op%").c_str(), varid))
        {
            if (!structname.empty())
            {
                tok3->deleteNext();
                tok3->deleteNext();
            }
            tok3 = tok3->next();
            tok3->str(value);
            tok3->varId(valueVarId);
            ret = true;
        }

        // Variable is used in calculation..
        if (((tok3->previous()->varId() > 0) && Token::Match(tok3, ("& " + structname + " %varid%").c_str(), varid)) ||
            Token::Match(tok3, ("[=+-*/%^|[] " + structname + " %varid% [=?+-*/%^|;])]").c_str(), varid) ||
            Token::Match(tok3, ("[(=+-*/%^|[] " + structname + " %varid% <<|>>").c_str(), varid) ||
            Token::Match(tok3, ("<<|>> " + structname + " %varid% %op%|;|]|)").c_str(), varid) ||
            Token::Match(tok3->previous(), ("[=+-*/%^|[] ( " + structname + " %varid% !!=").c_str(), varid))
        {
            if (value[0] == '\"')
                break;
            if (!structname.empty())
            {
                tok3->deleteNext();
                tok3->deleteNext();
            }
            tok3 = tok3->next();
            tok3->str(value);
            tok3->varId(valueVarId);
            if (tok3->previous()->str() == "*" && valueIsPointer)
            {
                tok3 = tok3->previous();
                tok3->deleteThis();
            }
            ret = true;
        }

        if (Token::simpleMatch(tok3, "= {"))
        {
            unsigned int indentlevel4 = 0;
            for (const Token *tok4 = tok3; tok4; tok4 = tok4->next())
            {
                if (tok4->str() == "{")
                    ++indentlevel4;
                else if (tok4->str() == "}")
                {
                    if (indentlevel4 <= 1)
                        break;
                    --indentlevel4;
                }
                if (Token::Match(tok4, "{|, %varid% ,|}", varid))
                {
                    tok4->next()->str(value);
                    tok4->next()->varId(valueVarId);
                    ret = true;
                }
            }
        }

        // Using the variable in for-condition..
        if (Token::simpleMatch(tok3, "for ("))
        {
            for (Token *tok4 = tok3->tokAt(2); tok4; tok4 = tok4->next())
            {
                if (tok4->str() == "(" || tok4->str() == ")")
                    break;

                // Replace variable used in condition..
                if (Token::Match(tok4, "; %var% <|<=|!= %var% ; ++| %var% ++| )"))
                {
                    const Token *inctok = tok4->tokAt(5);
                    if (inctok->str() == "++")
                        inctok = inctok->next();
                    if (inctok->varId() == varid)
                        break;

                    if (tok4->next()->varId() == varid)
                    {
                        tok4->next()->str(value);
                        tok4->next()->varId(valueVarId);
                        ret = true;
                    }
                    if (tok4->tokAt(3)->varId() == varid)
                    {
                        tok4->tokAt(3)->str(value);
                        tok4->tokAt(3)->varId(valueVarId);
                        ret = true;
                    }
                }
            }
        }

        if (indentlevel == indentlevel3 && Token::Match(tok3->next(), "%varid% ++|--", varid) && MathLib::isInt(value))
        {
            const std::string op(tok3->strAt(2));
            if (Token::Match(tok3, "[{};] %any% %any% ;"))
            {
                Token::eraseTokens(tok3, tok3->tokAt(3));
            }
            else
            {
                tok3 = tok3->next();
                tok3->str(value);
                tok3->varId(valueVarId);
                tok3->deleteNext();
            }
            incdec(value, op);
            if (!Token::simpleMatch((*tok2)->tokAt(-2), "for ("))
            {
                (*tok2)->tokAt(2)->str(value);
                (*tok2)->tokAt(2)->varId(valueVarId);
            }
            ret = true;
        }

        if (indentlevel == indentlevel3 && Token::Match(tok3->next(), "++|-- %varid%", varid) && MathLib::isInt(value) &&
            !Token::Match(tok3->tokAt(3), "[.[]"))
        {
            incdec(value, tok3->strAt(1));
            (*tok2)->tokAt(2)->str(value);
            (*tok2)->tokAt(2)->varId(valueVarId);
            if (Token::Match(tok3, "[;{}] %any% %any% ;"))
            {
                Token::eraseTokens(tok3, tok3->tokAt(3));
            }
            else
            {
                tok3->deleteNext();
                tok3->next()->str(value);
                tok3->next()->varId(valueVarId);
            }
            tok3 = tok3->next();
            ret = true;
        }

        // return variable..
        if (Token::Match(tok3, "return %varid% %any%", varid) &&
            (tok3->tokAt(2)->isExtendedOp() || tok3->strAt(2) == ";") &&
            value[0] != '\"')
        {
            tok3->next()->str(value);
            tok3->next()->varId(valueVarId);
        }

        else if (pointeralias && Token::Match(tok3, "return * %varid% ;", varid) && value[0] != '\"')
        {
            tok3->deleteNext();
            tok3->next()->str(value);
            tok3->next()->varId(valueVarId);
        }
    }
    return ret;
}


void Tokenizer::elseif()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (!Token::simpleMatch(tok, "else if"))
            continue;
        int indent = 0;
        for (Token *tok2 = tok; indent >= 0 && tok2; tok2 = tok2->next())
        {
            if (Token::Match(tok2, "(|{"))
                ++indent;
            else if (Token::Match(tok2, ")|}"))
                --indent;

            if (indent == 0 && Token::Match(tok2, "}|;"))
            {
                if (tok2->next() && tok2->next()->str() != "else")
                {
                    tok->insertToken("{");
                    tok2->insertToken("}");
                    Token::createMutualLinks(tok->next(), tok2->next());
                    break;
                }
            }
        }
    }
}


bool Tokenizer::simplifyRedundantParenthesis()
{
    bool ret = false;
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (tok->str() != "(")
            continue;

        // !!operator = ( x ) ;
        if (tok->strAt(-2) != "operator" &&
            tok->strAt(-1) == "=" &&
            tok->strAt(1) != "{" &&
            Token::simpleMatch(tok->link(), ") ;"))
        {
            tok->link()->deleteThis();
            tok->deleteThis();
            continue;
        }

        while (Token::simpleMatch(tok, "( (") &&
               tok->link()->previous() == tok->next()->link())
        {
            // We have "(( *something* ))", remove the inner
            // parenthesis
            tok->deleteNext();
            tok->link()->tokAt(-2)->deleteNext();
            ret = true;
        }

        while (Token::Match(tok->previous(), "[,;{}(] ( %var% (") &&
               tok->link()->previous() == tok->tokAt(2)->link())
        {
            // We have "( func ( *something* ))", remove the outer
            // parenthesis
            tok->link()->deleteThis();
            tok->deleteThis();
            ret = true;
        }

        while (Token::Match(tok->previous(), "[;{] ( delete %var% ) ;"))
        {
            // We have "( delete var )", remove the outer
            // parenthesis
            tok->tokAt(3)->deleteThis();
            tok->deleteThis();
            ret = true;
        }

        while (Token::Match(tok->previous(), "[;{] ( delete [ ] %var% ) ;"))
        {
            // We have "( delete [] var )", remove the outer
            // parenthesis
            tok->tokAt(5)->deleteThis();
            tok->deleteThis();
            ret = true;
        }

        if (!Token::simpleMatch(tok->tokAt(-2), "operator delete") &&
            Token::Match(tok->previous(), "delete|; (") &&
            (tok->strAt(-1) != "delete" || tok->next()->varId() > 0) &&
            Token::Match(tok->link(), ") ;|,"))
        {
            tok->link()->deleteThis();
            tok->deleteThis();
            ret = true;
        }

        if (Token::Match(tok->previous(), "[(!*;{}] ( %var% )") && tok->next()->varId() != 0)
        {
            // We have "( var )", remove the parenthesis
            tok->deleteThis();
            tok->deleteNext();
            ret = true;
            continue;
        }

        if (Token::Match(tok->previous(), "[(!] ( %var% . %var% )"))
        {
            // We have "( var . var )", remove the parenthesis
            tok->deleteThis();
            tok = tok->tokAt(2);
            tok->deleteNext();
            ret = true;
            continue;
        }

        if (Token::Match(tok, "( ( %bool% )") ||
            Token::Match(tok, "( ( %num% )"))
        {
            tok->tokAt(2)->deleteNext();
            tok->deleteNext();
            ret = true;
        }

        if (Token::simpleMatch(tok->previous(), ", (") &&
            Token::simpleMatch(tok->link(), ") ="))
        {
            tok->link()->deleteThis();
            tok->deleteThis();
            ret = true;
        }
    }
    return ret;
}

void Tokenizer::simplifyReference()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        // starting executable scope..
        if (Token::Match(tok, ") const| {"))
        {
            // replace references in this scope..
            if (tok->next()->str() != "{")
                tok = tok->next();
            Token * const end = tok->next()->link();
            for (Token *tok2 = tok; tok2 && tok2 != end; tok2 = tok2->next())
            {
                // found a reference..
                if (Token::Match(tok2, "[;{}] %type% & %var% (|= %var% )| ;"))
                {
                    const unsigned int ref_id = tok2->tokAt(3)->varId();
                    if (!ref_id)
                        continue;

                    // replace reference in the code..
                    for (Token *tok3 = tok2->tokAt(7); tok3 && tok3 != end; tok3 = tok3->next())
                    {
                        if (tok3->varId() == ref_id)
                        {
                            tok3->str(tok2->strAt(5));
                            tok3->varId(tok2->tokAt(5)->varId());
                        }
                    }

                    Token::eraseTokens(tok2, tok2->tokAt(7));
                }
            }
        }
    }
}

bool Tokenizer::simplifyCalculations()
{
    bool ret = false;
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        // Remove parentheses around variable..
        // keep parentheses here: dynamic_cast<Fred *>(p);
        // keep parentheses here: A operator * (int);
        // keep parentheses here: int ( * ( * f ) ( ... ) ) (int) ;
        // keep parentheses here: int ( * * ( * compilerHookVector ) (void) ) ( ) ;
        // keep parentheses here: operator new [] (size_t);
        // keep parentheses here: Functor()(a ... )
        // keep parentheses here: ) ( var ) ;
        if (Token::Match(tok->next(), "( %var% ) ;|)|,|]|%op%") &&
            !tok->isName() &&
            tok->str() != ">" &&
            tok->str() != "]" &&
            !Token::simpleMatch(tok->previous(), "operator") &&
            !Token::simpleMatch(tok->previous(), "* )") &&
            !Token::simpleMatch(tok->previous(), ") )") &&
            !Token::Match(tok->tokAt(-2), "* %var% )") &&
            !Token::Match(tok->tokAt(-2), "%type% ( ) ( %var%") &&
            !Token::Match(tok, ") ( %var% ) ;")
           )
        {
            tok->deleteNext();
            tok = tok->next();
            tok->deleteNext();
            ret = true;
        }

        if (tok->isNumber())
        {
            if (tok->str() == "0")
            {
                if (Token::Match(tok->previous(), "[+-|] 0"))
                {
                    tok = tok->previous();
                    if (Token::Match(tok->tokAt(-4), "[;{}] %var% = %var% [+-|] 0 ;") &&
                        tok->strAt(-3) == tok->strAt(-1))
                    {
                        tok = tok->previous()->previous()->previous();
                        tok->deleteThis();
                        tok->deleteThis();
                        tok->deleteThis();
                    }
                    tok->deleteThis();
                    tok->deleteThis();
                    ret = true;
                }
                else if (Token::Match(tok->previous(), "[=([,] 0 [+|]") ||
                         Token::Match(tok->previous(), "return|case 0 [+|]"))
                {
                    tok->deleteThis();
                    tok->deleteThis();
                    ret = true;
                }
                else if (Token::Match(tok->previous(), "[=[(,] 0 * %any% ,|]|)|;|=|%op%") ||
                         Token::Match(tok->previous(), "return|case 0 * %any% ,|:|;|=|%op%"))
                {
                    tok->deleteNext();
                    if (tok->next()->str() == "(")
                        Token::eraseTokens(tok, tok->next()->link());
                    tok->deleteNext();
                    ret = true;
                }
            }

            if (Token::simpleMatch(tok->previous(), "* 1") || Token::simpleMatch(tok, "1 *"))
            {
                if (tok->previous()->isOp())
                    tok = tok->previous();
                tok->deleteThis();
                tok->deleteThis();
                ret = true;
            }

            // Remove parentheses around number..
            if (Token::Match(tok->tokAt(-2), "%any% ( %num% )") && !tok->tokAt(-2)->isName() && tok->strAt(-2) != ">")
            {
                tok = tok->previous();
                tok->deleteThis();
                tok->deleteNext();
                ret = true;
            }

            if (Token::simpleMatch(tok->previous(), "( 0 ||") ||
                Token::simpleMatch(tok->previous(), "|| 0 )") ||
                Token::simpleMatch(tok->previous(), "( 0 |") ||
                Token::simpleMatch(tok->previous(), "| 0 )") ||
                Token::simpleMatch(tok->previous(), "( 1 &&") ||
                Token::simpleMatch(tok->previous(), "&& 1 )"))
            {
                if (tok->previous()->isOp())
                    tok = tok->previous();
                tok->deleteThis();
                tok->deleteThis();
                ret = true;
            }

            if (Token::Match(tok, "%num% ==|!=|<=|>=|<|> %num%") &&
                MathLib::isInt(tok->str()) &&
                MathLib::isInt(tok->tokAt(2)->str()))
            {
                if (Token::Match(tok->previous(), "(|&&|%oror%") && Token::Match(tok->tokAt(3), ")|&&|%oror%"))
                {
                    const MathLib::bigint op1(MathLib::toLongNumber(tok->str()));
                    const std::string &cmp(tok->next()->str());
                    const MathLib::bigint op2(MathLib::toLongNumber(tok->tokAt(2)->str()));

                    std::string result;

                    if (cmp == "==")
                        result = (op1 == op2) ? "1" : "0";
                    else if (cmp == "!=")
                        result = (op1 != op2) ? "1" : "0";
                    else if (cmp == "<=")
                        result = (op1 <= op2) ? "1" : "0";
                    else if (cmp == ">=")
                        result = (op1 >= op2) ? "1" : "0";
                    else if (cmp == "<")
                        result = (op1 < op2) ? "1" : "0";
                    else if (cmp == ">")
                        result = (op1 > op2) ? "1" : "0";

                    tok->str(result);
                    tok->deleteNext();
                    tok->deleteNext();
                    ret = true;
                }
            }

            if (Token::Match(tok->previous(), "[([,=] %num% <<|>> %num%"))
            {
                const MathLib::bigint op1(MathLib::toLongNumber(tok->str()));
                const MathLib::bigint op2(MathLib::toLongNumber(tok->tokAt(2)->str()));
                MathLib::bigint result;

                if (tok->next()->str() == "<<")
                    result = op1 << op2;
                else
                    result = op1 >> op2;

                std::ostringstream ss;
                ss << result;

                tok->str(ss.str());
                tok->deleteNext();
                tok->deleteNext();
            }
        }

        else if (tok->next() && tok->next()->isNumber())
        {
            // (1-2)
            while (Token::Match(tok, "[[,(=<>+-*|&^] %num% [+-*/] %num% ]|,|)|;|=|%op%") ||
                   Token::Match(tok, "<< %num% [+-*/] %num% ]|,|)|;|=|%op%") ||
                   Token::Match(tok, "[[,(=<>+-*|&^] %num% [+-*/] %num% <<|>>") ||
                   Token::Match(tok, "<< %num% [+-*/] %num% <<") ||
                   Token::Match(tok, "[(,[] %num% [|&^] %num% [];,);]") ||
                   Token::Match(tok, "(|%op% %num% [+-*/] %num% )|%op%") ||
                   Token::Match(tok,"return|case %num% [+-*/] %num% ;|,|=|:|%op%"))
            {
                tok = tok->next();

                // Don't simplify "%num% / 0"
                if (Token::simpleMatch(tok->next(), "/ 0"))
                    continue;

                // & | ^
                if (Token::Match(tok->next(), "[&|^]"))
                {
                    std::string result;
                    const std::string first(tok->str());
                    const std::string second(tok->strAt(2));
                    const char op = tok->next()->str()[0];
                    if (op == '&')
                        result = MathLib::toString<MathLib::bigint>(MathLib::toLongNumber(first) & MathLib::toLongNumber(second));
                    else if (op == '|')
                        result = MathLib::toString<MathLib::bigint>(MathLib::toLongNumber(first) | MathLib::toLongNumber(second));
                    else if (op == '^')
                        result = MathLib::toString<MathLib::bigint>(MathLib::toLongNumber(first) ^ MathLib::toLongNumber(second));

                    if (!result.empty())
                    {
                        ret = true;
                        tok->str(result);
                        Token::eraseTokens(tok, tok->tokAt(3));
                        continue;
                    }
                }

                // Division where result is a whole number
                if (Token::Match(tok->previous(), "* %num% /") &&
                    tok->str() == MathLib::multiply(tok->strAt(2), MathLib::divide(tok->str(), tok->strAt(2))))
                {
                }

                // + and - are calculated after * and /
                else if (Token::Match(tok->next(), "[+-/]"))
                {
                    if (Token::Match(tok->previous(), "[*/%]"))
                        continue;
                    if (Token::Match(tok->tokAt(3), "[*/%]"))
                        continue;
                }

                if (Token::Match(tok->previous(), "- %num% - %num%"))
                    tok->str(MathLib::add(tok->str(), tok->tokAt(2)->str()));
                else if (Token::Match(tok->previous(), "- %num% + %num%"))
                    tok->str(MathLib::subtract(tok->str(), tok->tokAt(2)->str()));
                else
                    tok->str(MathLib::calculate(tok->str(), tok->tokAt(2)->str(), tok->strAt(1)[0], this));

                Token::eraseTokens(tok, tok->tokAt(3));

                // evaluate "2 + 2 - 2 - 2"
                // as (((2 + 2) - 2) - 2) = 0
                // instead of ((2 + 2) - (2 - 2)) = 4
                if (Token::Match(tok->next(), "[+-*/]"))
                {
                    tok = tok->previous();
                    continue;
                }

                ret = true;
            }
        }
    }
    return ret;
}




void Tokenizer::simplifyGoto()
{
    std::list<Token *> gotos;
    unsigned int indentlevel = 0;
    Token *beginfunction = 0;
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (tok->str() == "{")
        {
            if (beginfunction == 0 && indentlevel == 0 && tok->link())
                tok = tok->link();
            else
                ++indentlevel;
        }

        else if (tok->str() == "}")
        {
            if (indentlevel == 0)
                break;  // break out - it seems the code is wrong
            --indentlevel;
            if (indentlevel == 0)
            {
                gotos.clear();
                beginfunction = 0;
            }
        }

        else if (indentlevel > 0 && tok->str() == "(")
        {
            tok = tok->link();
        }

        else if (indentlevel == 0 && Token::Match(tok, ") const| {"))
        {
            gotos.clear();
            beginfunction = tok;
        }

        else if (Token::Match(tok, "goto %var% ;"))
            gotos.push_back(tok);

        else if (indentlevel == 1 && Token::Match(tok->previous(), "[};] %var% :"))
        {
            // Is this label at the end..
            bool end = false;
            int level = 0;
            for (const Token *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "}")
                {
                    --level;
                    if (level < 0)
                    {
                        end = true;
                        break;
                    }
                }
                else if (tok2->str() == "{")
                {
                    ++level;
                }

                if (Token::Match(tok2, "%var% :") || tok2->str() == "goto")
                {
                    break;
                }
            }
            if (!end)
                continue;

            const std::string name(tok->str());

            tok->deleteThis();
            tok->deleteThis();
            if (Token::Match(tok, "; %any%"))
                tok->deleteThis();

            // This label is at the end of the function.. replace all matching goto statements..
            for (std::list<Token *>::iterator it = gotos.begin(); it != gotos.end(); ++it)
            {
                Token *token = *it;
                if (token->next()->str() == name)
                {
                    // Delete the "goto name;"
                    token = token->previous();
                    token->deleteNext();
                    token->deleteNext();
                    token->deleteNext();

                    // Insert the statements..
                    bool ret = false;   // is there return
                    bool ret2 = false;  // is there return in indentlevel 0
                    std::list<Token*> links;
                    std::list<Token*> links2;
                    std::list<Token*> links3;
                    int lev = 0;
                    for (const Token *tok2 = tok; tok2; tok2 = tok2->next())
                    {
                        if (tok2->str() == "}")
                        {
                            --lev;
                            if (lev < 0)
                                break;
                        }
                        if (tok2->str() == "{")
                        {
                            ++lev;
                        }
                        else if (tok2->str() == "return")
                        {
                            ret = true;
                            if (indentlevel == 1 && lev == 0)
                                ret2 = true;
                        }
                        token->insertToken(tok2->str().c_str());
                        token = token->next();
                        token->linenr(tok2->linenr());
                        token->varId(tok2->varId());
                        if (ret2 && tok2->str() == ";")
                        {
                            break;
                        }
                        if (token->str() == "(")
                        {
                            links.push_back(token);
                        }
                        else if (token->str() == ")")
                        {
                            if (links.empty())
                            {
                                // This should never happen at this point
                                syntaxError(token, ')');
                                return;
                            }

                            Token::createMutualLinks(links.back(), token);
                            links.pop_back();
                        }
                        else if (token->str() == "{")
                        {
                            links2.push_back(token);
                        }
                        else if (token->str() == "}")
                        {
                            if (links2.empty())
                            {
                                // This should never happen at this point
                                syntaxError(token, '}');
                                return;
                            }

                            Token::createMutualLinks(links2.back(), token);
                            links2.pop_back();
                        }
                        else if (token->str() == "[")
                        {
                            links3.push_back(token);
                        }
                        else if (token->str() == "]")
                        {
                            if (links3.empty())
                            {
                                // This should never happen at this point
                                syntaxError(token, ']');
                                return;
                            }

                            Token::createMutualLinks(links3.back(), token);
                            links3.pop_back();
                        }
                    }

                    if (!ret)
                    {
                        token->insertToken("return");
                        token = token->next();
                        token->insertToken(";");
                        token = token->next();
                    }
                }
            }

            // goto the end of the function
            while (tok)
            {
                if (tok->str() == "{")
                    tok = tok->link();
                else if (tok->str() == "}")
                    break;
                tok = tok->next();
            }
            if (!tok)
                break;
            gotos.clear();
            beginfunction = 0;
            indentlevel = 0;
            continue;
        }
    }
}

void Tokenizer::simplifyNestedStrcat()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (! Token::Match(tok, "[;{}] strcat ( strcat ("))
        {
            continue;
        }

        // find inner strcat call
        Token *tok2 = tok->tokAt(3);
        while (Token::simpleMatch(tok2, "strcat ( strcat"))
        {
            tok2 = tok2->tokAt(2);
        }

        // If we have this code:
        //   strcat(strcat(dst, foo), bar);
        // We move this part of code before all strcat() calls: strcat(dst, foo)
        // And place "dst" token where the code was.
        Token *prevTok = tok2->previous();

        // Move tokens to new place
        Token::move(tok2, tok2->next()->link(), tok);
        tok = tok2->next()->link();

        // Insert the "dst" token
        prevTok->insertToken(tok2->strAt(2));

        // Insert semicolon after the moved strcat()
        tok->insertToken(";");
    }

}

void Tokenizer::duplicateEnumError(const Token * tok1, const Token * tok2, const std::string & type)
{
    if (tok1 && !(_settings->isEnabled("style")))
        return;

    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    std::string tok2_str;
    if (tok1 && tok2)
    {
        ErrorLogger::ErrorMessage::FileLocation loc;
        loc.line = tok1->linenr();
        loc.setfile(file(tok1));
        locationList.push_back(loc);
        loc.line = tok2->linenr();
        loc.setfile(file(tok2));
        locationList.push_back(loc);
        tok2_str = tok2->str();
    }
    else
        tok2_str = "name";

    const ErrorLogger::ErrorMessage errmsg(locationList,
                                           Severity::style,
                                           std::string(type + " '" + tok2_str +
                                                   "' hides enumerator with same name"),
                                           "variableHidingEnum",
                                           false);

    if (_errorLogger)
        _errorLogger->reportErr(errmsg);
    else
        Check::reportError(errmsg);
}

// Check if this statement is a duplicate definition.  A duplicate
// definition will hide the enumerator within it's scope so just
// skip the entire scope of the duplicate.
bool Tokenizer::duplicateDefinition(Token ** tokPtr, const Token * name)
{
    // check for an end of definition
    const Token * tok = *tokPtr;
    if (tok && Token::Match(tok->next(), ";|,|[|=|)|>"))
    {
        const Token * end = tok->next();

        if (end->str() == "[")
        {
            end = end->link()->next();
        }
        else if (end->str() == ",")
        {
            // check for function argument
            if (Token::Match(tok->previous(), "(|,"))
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
        }
        else if (end->str() == ")")
        {
            // check for function argument
            if (tok->previous()->str() == ",")
                return false;
        }

        if (end)
        {
            if (Token::simpleMatch(end, ") {")) // function parameter ?
            {
                // make sure it's not a conditional
                if (Token::Match(end->link()->previous(), "if|for|while|switch"))
                    return false;

                // look backwards
                if (tok->previous()->str() == "enum" ||
                    (Token::Match(tok->previous(), "%type%") &&
                     tok->previous()->str() != "return") ||
                    Token::Match(tok->tokAt(-2), "%type% &|*"))
                {
                    duplicateEnumError(*tokPtr, name, "Function parameter");
                    // duplicate definition so skip entire function
                    *tokPtr = end->next()->link();
                    return true;
                }
            }
            else if (end->str() == ">") // template parameter ?
            {
                // look backwards
                if (tok->previous()->str() == "enum" ||
                    (Token::Match(tok->previous(), "%type%") &&
                     tok->previous()->str() != "return"))
                {
                    // duplicate definition so skip entire template
                    while (end && end->str() != "{")
                        end = end->next();
                    if (end)
                    {
                        duplicateEnumError(*tokPtr, name, "Template parameter");
                        *tokPtr = end->link();
                        return true;
                    }
                }
            }
            else
            {
                // look backwards
                if (Token::Match(tok->previous(), "enum|,") ||
                    (Token::Match(tok->previous(), "%type%") &&
                     tok->previous()->str() != "return"))
                {
                    duplicateEnumError(*tokPtr, name, "Variable");
                    return true;
                }
            }
        }
    }
    return false;
}

void Tokenizer::simplifyEnum()
{
    // Don't simplify enums in java files
    if (isJavaOrCSharp())
        return;

    std::string className;
    int classLevel = 0;
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "class|struct|namespace %any%") &&
            (!tok->previous() || (tok->previous() && tok->previous()->str() != "enum")))
        {
            className = tok->next()->str();
            classLevel = 0;
            continue;
        }
        else if (tok->str() == "}")
        {
            --classLevel;
            if (classLevel < 0)
                className = "";

            continue;
        }
        else if (tok->str() == "{")
        {
            ++classLevel;
            continue;
        }
        else if (Token::Match(tok, "enum class|struct| {|:") ||
                 Token::Match(tok, "enum class|struct| %type% {|:|;"))
        {
            Token *tok1;
            Token *start = tok;
            Token *end;
            Token *enumType = 0;
            Token *typeTokenStart = 0;
            Token *typeTokenEnd = 0;

            // check for C++0x enum class
            if (Token::Match(tok->next(), "class|struct"))
                tok->deleteNext();

            // check for name
            if (Token::Match(tok->next(), "%type%"))
            {
                enumType = tok->next();
                tok = tok->next();
            }

            // check for C++0x typed enumeration
            if (tok->next()->str() == ":")
            {
                tok = tok->next();

                if (!tok->next())
                {
                    syntaxError(tok);
                    return; // can't recover
                }

                typeTokenStart = tok->next();
                tok = tok->next();
                typeTokenEnd = typeTokenStart;

                while (typeTokenEnd->next() && (typeTokenEnd->next()->str() == "::" ||
                                                Token::Match(typeTokenEnd->next(), "%type%")))
                {
                    typeTokenEnd = typeTokenEnd->next();
                    tok = tok->next();
                }

                if (!tok->next())
                {
                    syntaxError(tok);
                    return; // can't recover
                }
            }

            // check for forward declaration
            if (tok->next()->str() == ";")
            {
                tok = tok->next();

                /** @todo start substitution check at forward declaration */
                // delete forward declaration
                while (start->next() != tok)
                    start->deleteThis();
                start->deleteThis();
                tok = start;
                continue;
            }
            else if (tok->next()->str() != "{")
            {
                syntaxError(tok->next());
                return;
            }

            tok1 = tok->next();
            end = tok1->link();
            tok1 = tok1->next();

            MathLib::bigint lastValue = -1;
            Token * lastEnumValueStart = 0;
            Token * lastEnumValueEnd = 0;

            // iterate over all enumerators between { and }
            // Give each enumerator the const value specified or if not specified, 1 + the
            // previous value or 0 if it is the first one.
            for (; tok1 && tok1 != end; tok1 = tok1->next())
            {
                Token * enumName = 0;
                Token * enumValue = 0;
                Token * enumValueStart = 0;
                Token * enumValueEnd = 0;

                if (tok1->str() == "(")
                {
                    tok1 = tok1->link();
                    continue;
                }

                if (Token::Match(tok1->previous(), ",|{ %type% ,|}"))
                {
                    // no value specified
                    enumName = tok1;
                    lastValue++;
                    tok1->insertToken("=");
                    tok1 = tok1->next();

                    if (lastEnumValueStart && lastEnumValueEnd)
                    {
                        // previous value was an expression
                        Token *valueStart = tok1;
                        tok1 = copyTokens(tok1, lastEnumValueStart, lastEnumValueEnd);

                        // value is previous expression + 1
                        tok1->insertToken("+");
                        tok1 = tok1->next();
                        tok1->insertToken(MathLib::toString<MathLib::bigint>(lastValue));
                        enumValue = 0;
                        enumValueStart = valueStart->next();
                        enumValueEnd = tok1->next();
                    }
                    else
                    {
                        // value is previous numeric value + 1
                        tok1->insertToken(MathLib::toString<MathLib::bigint>(lastValue));
                        enumValue = tok1->next();
                    }
                }
                else if (Token::Match(tok1->previous(), ",|{ %type% = %num% ,|}"))
                {
                    // value is specified numeric value
                    enumName = tok1;
                    lastValue = MathLib::toLongNumber(tok1->strAt(2));
                    enumValue = tok1->tokAt(2);
                    lastEnumValueStart = 0;
                    lastEnumValueEnd = 0;
                }
                else if (Token::Match(tok1->previous(), ",|{ %type% ="))
                {
                    // value is specified expression
                    enumName = tok1;
                    lastValue = 0;
                    tok1 = tok1->tokAt(2);
                    enumValueStart = tok1;
                    enumValueEnd = tok1;
                    int level = 0;
                    if (enumValueEnd->str() == "(" ||
                        enumValueEnd->str() == "[" ||
                        enumValueEnd->str() == "{")
                        level++;
                    while (enumValueEnd->next() &&
                           (!Token::Match(enumValueEnd->next(), "}|,") || level))
                    {
                        if (enumValueEnd->next()->str() == "(" ||
                            enumValueEnd->next()->str() == "[" ||
                            enumValueEnd->next()->str() == "{")
                            level++;
                        else if (enumValueEnd->next()->str() == ")" ||
                                 enumValueEnd->next()->str() == "]" ||
                                 enumValueEnd->next()->str() == "}")
                            level--;

                        enumValueEnd = enumValueEnd->next();
                    }
                    // remember this expression in case it needs to be incremented
                    lastEnumValueStart = enumValueStart;
                    lastEnumValueEnd = enumValueEnd;
                    // skip over expression
                    tok1 = enumValueEnd;
                }

                // find all uses of this enumerator and substitute it's value for it's name
                if (enumName && (enumValue || (enumValueStart && enumValueEnd)))
                {
                    const std::string pattern = className.empty() ?
                                                std::string("") :
                                                std::string(className + " :: " + enumName->str());
                    int level = 1;
                    bool inScope = true;

                    bool exitThisScope = false;
                    int exitScope = 0;
                    bool simplify = false;
                    bool hasClass = false;
                    const Token *endScope = 0;
                    for (Token *tok2 = tok1->next(); tok2; tok2 = tok2->next())
                    {
                        if (tok2->str() == "}")
                        {
                            --level;
                            if (level < 0)
                                inScope = false;

                            if (exitThisScope)
                            {
                                if (level < exitScope)
                                    exitThisScope = false;
                            }
                        }
                        else if (tok2->str() == "{")
                        {
                            // Is the same enum redefined?
                            const Token *begin = end->link();
                            if (tok2->fileIndex() == begin->fileIndex() &&
                                tok2->linenr() == begin->linenr() &&
                                Token::Match(begin->tokAt(-2), "enum %type% {") &&
                                Token::Match(tok2->tokAt(-2), "enum %type% {") &&
                                begin->strAt(-1) == tok2->strAt(-1))
                            {
                                // remove duplicate enum
                                Token * startToken = tok2->tokAt(-3);
                                tok2 = tok2->link()->next();
                                Token::eraseTokens(startToken, tok2);
                                if (!tok2)
                                    break;
                            }
                            else
                            {
                                // Not a duplicate enum..
                                ++level;
                            }
                            endScope = tok2->link();
                        }
                        else if (!pattern.empty() && Token::Match(tok2, pattern.c_str()))
                        {
                            simplify = true;
                            hasClass = true;
                        }
                        else if (inScope && !exitThisScope && tok2->str() == enumName->str())
                        {
                            if (Token::simpleMatch(tok2->previous(), "::") ||
                                Token::Match(tok2->next(), "::|["))
                            {
                                // Don't replace this enum if:
                                // * it's preceded or followed by "::"
                                // * it's followed by "["
                            }
                            else if (!duplicateDefinition(&tok2, enumName))
                            {
                                simplify = true;
                                hasClass = false;
                            }
                            else
                            {
                                // something with the same name.
                                exitScope = level;
                                if (endScope)
                                    tok2 = endScope->previous();
                            }
                        }

                        if (simplify)
                        {
                            if (enumValue)
                                tok2->str(enumValue->str());
                            else
                            {
                                tok2 = tok2->previous();
                                tok2->deleteNext();
                                tok2 = copyTokens(tok2, enumValueStart, enumValueEnd);
                            }

                            if (hasClass)
                            {
                                tok2->deleteNext();
                                tok2->deleteNext();
                            }

                            simplify = false;
                        }
                    }
                }
            }

            // check for a variable definition: enum {} x;
            if (end->next() && end->next()->str() != ";")
            {
                Token *tempTok = end;

                tempTok->insertToken(";");
                tempTok = tempTok->next();
                if (typeTokenStart == 0)
                    tempTok->insertToken("int");
                else
                {
                    Token *tempTok1 = typeTokenStart;

                    tempTok->insertToken(tempTok1->str());

                    while (tempTok1 != typeTokenEnd)
                    {
                        tempTok1 = tempTok1->next();

                        tempTok->insertToken(tempTok1->str());
                        tempTok = tempTok->next();
                    }
                }
            }

            if (enumType)
            {
                const std::string pattern(className.empty() ? "" : (className + " :: " + enumType->str()).c_str());

                // count { and } for tok2
                int level = 0;
                bool inScope = true;

                bool exitThisScope = false;
                int exitScope = 0;
                bool simplify = false;
                bool hasClass = false;
                for (Token *tok2 = end->next(); tok2; tok2 = tok2->next())
                {
                    if (tok2->str() == "}")
                    {
                        --level;
                        if (level < 0)
                            inScope = false;

                        if (exitThisScope)
                        {
                            if (level < exitScope)
                                exitThisScope = false;
                        }
                    }
                    else if (tok2->str() == "{")
                        ++level;
                    else if (!pattern.empty() && ((Token::simpleMatch(tok2, "enum") && Token::Match(tok2->next(), pattern.c_str())) || Token::Match(tok2, pattern.c_str())))
                    {
                        simplify = true;
                        hasClass = true;
                    }
                    else if (inScope && !exitThisScope && (tok2->str() == enumType->str() || (tok2->str() == "enum" && tok2->next() && tok2->next()->str() == enumType->str())))
                    {
                        if (Token::simpleMatch(tok2->previous(), "::"))
                        {
                            // Don't replace this enum if it's preceded by "::"
                        }
                        else if (tok2->next() &&
                                 (tok2->next()->isName() || tok2->next()->str() == "("))
                        {
                            simplify = true;
                            hasClass = false;
                        }
                    }

                    if (simplify)
                    {
                        if (tok2->str() == "enum")
                            tok2->deleteNext();
                        if (typeTokenStart == 0)
                            tok2->str("int");
                        else
                        {
                            Token *tok3 = typeTokenStart;

                            tok2->str(tok3->str());

                            while (tok3 != typeTokenEnd)
                            {
                                tok3 = tok3->next();

                                tok2->insertToken(tok3->str());
                                tok2 = tok2->next();
                            }
                        }

                        if (hasClass)
                        {
                            tok2->deleteNext();
                            tok2->deleteNext();
                        }

                        simplify = false;
                    }
                }
            }

            tok1 = start;
            while (tok1->next() && tok1->next() != end)
                tok1->deleteNext();
            tok1->deleteNext();
            if (start != _tokens)
            {
                tok1 = start->previous();
                tok1->deleteNext();
                tok = tok1;
            }
            else
            {
                _tokens->deleteThis();
                tok = _tokens;
            }
        }
    }
}

void Tokenizer::simplifyStd()
{
    std::set<std::string> f;
    f.insert("strcat");
    f.insert("strcpy");
    f.insert("strncat");
    f.insert("strncpy");
    f.insert("free");
    f.insert("malloc");
    f.insert("strdup");

    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (tok->str() != "std")
            continue;

        if (Token::Match(tok->previous(), "[(,{};] std :: %var% (") &&
            f.find(tok->strAt(2)) != f.end())
        {
            tok->deleteNext();
            tok->deleteThis();
        }
    }
}

//---------------------------------------------------------------------------
// Helper functions for handling the tokens list
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------

const Token *Tokenizer::getFunctionTokenByName(const char funcname[]) const
{
    getSymbolDatabase();

    std::list<Scope>::const_iterator scope;

    for (scope = _symbolDatabase->scopeList.begin(); scope != _symbolDatabase->scopeList.end(); ++scope)
    {
        if (scope->type == Scope::eFunction)
        {
            if (scope->classDef->str() == funcname)
                return scope->classDef;
        }
    }
    return NULL;
}


void Tokenizer::fillFunctionList()
{
    getSymbolDatabase();
}

//---------------------------------------------------------------------------

// Deallocate lists..
void Tokenizer::deallocateTokens()
{
    deleteTokens(_tokens);
    _tokens = 0;
    _tokensBack = 0;
    _files.clear();
}

void Tokenizer::deleteTokens(Token *tok)
{
    while (tok)
    {
        Token *next = tok->next();
        delete tok;
        tok = next;
    }
}

//---------------------------------------------------------------------------

const char *Tokenizer::getParameterName(const Token *ftok, unsigned int par)
{
    unsigned int _par = 1;
    for (; ftok; ftok = ftok->next())
    {
        if (ftok->str() == ")")
            break;
        if (ftok->str() == ",")
            ++_par;
        if (par == _par && Token::Match(ftok, "%var% [,)]"))
            return ftok->str().c_str();
    }
    return NULL;
}

//---------------------------------------------------------------------------

std::string Tokenizer::fileLine(const Token *tok) const
{
    std::ostringstream ostr;
    ostr << "[" << _files.at(tok->fileIndex()) << ":" << tok->linenr() << "]";
    return ostr.str();
}

std::string Tokenizer::file(const Token *tok) const
{
    return _files.at(tok->fileIndex());
}

//---------------------------------------------------------------------------

void Tokenizer::syntaxError(const Token *tok)
{
    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    if (tok)
    {
        ErrorLogger::ErrorMessage::FileLocation loc;
        loc.line = tok->linenr();
        loc.setfile(file(tok));
        locationList.push_back(loc);
    }

    const ErrorLogger::ErrorMessage errmsg(locationList,
                                           Severity::error,
                                           "syntax error",
                                           "syntaxError",
                                           false);

    if (_errorLogger)
        _errorLogger->reportErr(errmsg);
    else
        Check::reportError(errmsg);
}

void Tokenizer::syntaxError(const Token *tok, char c)
{
    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    if (tok)
    {
        ErrorLogger::ErrorMessage::FileLocation loc;
        loc.line = tok->linenr();
        loc.setfile(file(tok));
        locationList.push_back(loc);
    }

    const ErrorLogger::ErrorMessage errmsg(locationList,
                                           Severity::error,
                                           std::string("Invalid number of character (") +
                                           c +
                                           ") " +
                                           "when these macros are defined: '" +
                                           _configuration +
                                           "'.",
                                           "syntaxError",
                                           false);

    if (_errorLogger)
        _errorLogger->reportErr(errmsg);
    else
        Check::reportError(errmsg);
}

void Tokenizer::cppcheckError(const Token *tok) const
{
    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    if (tok)
    {
        ErrorLogger::ErrorMessage::FileLocation loc;
        loc.line = tok->linenr();
        loc.setfile(file(tok));
        locationList.push_back(loc);
    }

    const ErrorLogger::ErrorMessage errmsg(locationList,
                                           Severity::error,
                                           "Analysis failed. If the code is valid then please report this failure.",
                                           "cppcheckError",
                                           false);

    if (_errorLogger)
        _errorLogger->reportErr(errmsg);
    else
        Check::reportError(errmsg);
}


void Tokenizer::simplifyMathFunctions()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "atol ( %str% )"))
        {
            if (!MathLib::isInt(tok->tokAt(2)->strValue()))
            {
                // Ignore strings which we can't convert
                continue;
            }

            if (tok->previous() &&
                Token::simpleMatch(tok->previous()->previous(), "std ::"))
            {
                // Delete "std ::"
                tok = tok->previous()->previous();
                tok->deleteNext();
                tok->deleteThis();
            }

            // Delete atol(
            tok->deleteNext();
            tok->deleteThis();

            // Convert string into a number
            tok->str(MathLib::toString(MathLib::toLongNumber(tok->strValue())));

            // Delete remaining )
            tok->deleteNext();
        }
    }
}

void Tokenizer::simplifyComma()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::simpleMatch(tok, "for (") ||
            Token::Match(tok, "=|enum {"))
        {
            tok = tok->next()->link();
            if (!tok)
                break;

            continue;
        }

        if (tok->str() == "(")
        {
            tok = tok->link();
            continue;
        }

        // Skip unhandled template specifiers..
        if (Token::Match(tok, "%var% <"))
        {
            // Todo.. use the link instead.
            unsigned int comparelevel = 0;
            for (Token *tok2 = tok; tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "<")
                    ++comparelevel;
                else if (tok2->str() == ">")
                {
                    if (comparelevel <= 1)
                    {
                        tok = tok2;
                        break;
                    }
                    ++comparelevel;
                }
                else if (Token::Match(tok2, "[;{}]"))
                    break;
            }
        }

        // If token after the comma is a constant number, simplification is not required.
        if (tok->str() != "," || Token::Match(tok->next(), "%num%"))
            continue;

        // We must not accept just any keyword, e.g. accepting int
        // would cause function parameters to corrupt.
        if (Token::simpleMatch(tok->next(), "delete"))
        {
            // Handle "delete a, delete b;"
            tok->str(";");
        }

        if (tok->previous() && tok->previous()->previous())
        {
            if (Token::Match(tok->previous()->previous(), "delete %var% , %var% ;") &&
                tok->next()->varId() != 0)
            {
                // Handle "delete a, b;"
                tok->str(";");
                tok->insertToken("delete");
            }
            else
            {
                for (Token *tok2 = tok->previous(); tok2; tok2 = tok2->previous())
                {
                    if (tok2->str() == "=")
                    {
                        // Handle "a = 0, b = 0;"
                        tok->str(";");
                        break;
                    }
                    else if (Token::Match(tok2, "delete %var%") ||
                             Token::Match(tok2, "delete [ ] %var%"))
                    {
                        // Handle "delete a, a = 0;"
                        tok->str(";");
                        break;
                    }
                    else if (Token::Match(tok2, "[;,{}()]"))
                    {
                        break;
                    }
                }
            }
        }

        bool inReturn = false;
        Token *startFrom = NULL;    // next tokean after "; return"
        Token *endAt = NULL;        // first ";" token after "; return"

        // find "; return" pattern before comma
        for (Token *tok2 = tok; tok2; tok2 = tok2->previous())
        {
            if (Token::Match(tok2, "[;{}]"))
            {
                break;

            }
            else if (tok2->str() == "return" && Token::Match(tok2->previous(), "[;{}]"))
            {
                inReturn = true;
                startFrom = tok2->next();
                break;
            }
        }

        // find token where return ends and also count commas
        if (inReturn)
        {
            size_t commaCounter = 0;
            size_t indentlevel = 0;

            for (Token *tok2 = startFrom; tok2; tok2 = tok2->next())
            {
                if (tok2->str() == ";")
                {
                    endAt = tok2;
                    break;

                }
                else if (tok2->str() == "(")
                {
                    ++indentlevel;

                }
                else if (tok2->str() == ")")
                {
                    --indentlevel;

                }
                else if (tok2->str() == "," && indentlevel == 0)
                {
                    ++commaCounter;
                }
            }

            if (commaCounter)
            {
                indentlevel = 0;

                // change tokens:
                // "; return a ( ) , b ( ) , c ;"
                // to
                // "; return a ( ) ; b ( ) ; c ;"
                for (Token *tok2 = startFrom; tok2 != endAt; tok2 = tok2->next())
                {
                    if (tok2->str() == "(")
                    {
                        ++indentlevel;

                    }
                    else if (tok2->str() == ")")
                    {
                        --indentlevel;

                    }
                    else if (tok2->str() == "," && indentlevel == 0)
                    {
                        tok2->str(";");
                        --commaCounter;
                        if (commaCounter == 0)
                        {
                            tok2->insertToken("return");
                        }
                    }
                }

                // delete old "return"
                startFrom->previous()->deleteThis();
                startFrom = 0;   // give dead pointer a value

                tok = endAt;
                if (!tok)
                    return;
            }
        }

    }
}


void Tokenizer::removeExceptionSpecifications(Token *tok) const
{
    while (tok)
    {
        if (tok->str() == "{")
            tok = tok->link();

        else if (tok->str() == "}")
            break;

        else if (Token::Match(tok, ") const| throw ("))
        {
            if (tok->next()->str() == "const")
            {
                Token::eraseTokens(tok->next(), tok->tokAt(3)->link());
                tok = tok->next();
            }
            else
                Token::eraseTokens(tok, tok->tokAt(2)->link());
            tok->deleteNext();
        }

        else if (Token::Match(tok, "class|namespace|struct %type%"))
        {
            while (tok && !Token::Match(tok, "[;{=]"))
                tok = tok->next();
            if (tok && tok->str() == "{")
            {
                removeExceptionSpecifications(tok->next());
                tok = tok->link();
            }
        }

        tok = tok ? tok->next() : 0;
    }
}



bool Tokenizer::validate() const
{
    std::stack<const Token *> linktok;
    const Token *lastTok = 0;
    for (const Token *tok = tokens(); tok; tok = tok->next())
    {
        lastTok = tok;
        if (Token::Match(tok, "[{([]"))
        {
            if (tok->link() == 0)
            {
                cppcheckError(tok);
                return false;
            }

            linktok.push(tok);
            continue;
        }

        else if (Token::Match(tok, "[})]]"))
        {
            if (tok->link() == 0)
            {
                cppcheckError(tok);
                return false;
            }

            if (linktok.empty() == true)
            {
                cppcheckError(tok);
                return false;
            }

            if (tok->link() != linktok.top())
            {
                cppcheckError(tok);
                return false;
            }

            if (tok != tok->link()->link())
            {
                cppcheckError(tok);
                return false;
            }

            linktok.pop();
            continue;
        }

        if (tok->link() != 0)
        {
            cppcheckError(tok);
            return false;
        }
    }

    if (!linktok.empty())
    {
        cppcheckError(linktok.top());
        return false;
    }

    // Validate that the Tokenizer::_tokensBack is updated correctly during simplifications
    if (lastTok != _tokensBack)
    {
        cppcheckError(lastTok);
        return false;
    }

    return true;
}

std::string Tokenizer::simplifyString(const std::string &source)
{
    std::string str = source;

    // true when previous char is a \ .
    bool escaped = false;
    for (std::string::size_type i = 0; i + 2 < str.size(); i++)
    {
        if (!escaped)
        {
            if (str[i] == '\\')
                escaped = true;

            continue;
        }

        if (str[i] == 'x')
        {
            // Hex value
            if (str[i+1] == '0' && str[i+2] == '0')
                str.replace(i, 3, "0");
            else if (i > 0)
            {
                // We will replace all other character as 'a'
                // If that causes problems in the future, this can
                // be improved. But for now, this should be OK.
                unsigned int n = 1;
                while (n < 2 && std::isxdigit(str[i+1+n]))
                    ++n;
                --i;
                n += 2;
                str.replace(i, n, "a");
            }
        }
        else if (MathLib::isOctalDigit(str[i]))
        {
            if (MathLib::isOctalDigit(str[i+1]) &&
                MathLib::isOctalDigit(str[i+2]))
            {
                if (str[i+1] == '0' && str[i+2] == '0')
                    str.replace(i, 3, "0");
                else
                {
                    // We will replace all other character as 'a'
                    // If that causes problems in the future, this can
                    // be improved. But for now, this should be OK.
                    --i;
                    str.replace(i, 4, "a");
                }
            }
        }

        escaped = false;
    }

    return str;
}


void Tokenizer::simplifyStructInit()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "[;{}] struct| %type% %var% = { . %type% ="))
        {
            // Goto "." and check if the initializations have an expected format
            const Token *tok2 = tok;
            while (tok2->str() != ".")
                tok2 = tok2->next();
            while (tok2 && tok2->str() == ".")
            {
                if (Token::Match(tok2, ". %type% = %num% [,}]"))
                    tok2 = tok2->tokAt(4);
                else if (Token::Match(tok2, ". %type% = %var% [,}]"))
                    tok2 = tok2->tokAt(4);
                else if (Token::Match(tok2, ". %type% = & %var% [,}]"))
                    tok2 = tok2->tokAt(5);
                else
                    break;

                if (Token::simpleMatch(tok2, ", ."))
                    tok2 = tok2->next();
            }
            if (!Token::simpleMatch(tok2, "} ;"))
                continue;

            // Known expression format => Perform simplification
            Token *vartok = tok->tokAt(3);
            if (vartok->str() == "=")
                vartok = vartok->previous();
            vartok->next()->str(";");

            Token *tok3 = vartok->tokAt(2);
            tok3->link(0);
            while (Token::Match(tok3, "[{,] . %type% ="))
            {
                tok3->str(vartok->str());
                tok3->varId(vartok->varId());
                tok3 = tok3->tokAt(5);
                while (!Token::Match(tok3, "[,}]"))
                    tok3 = tok3->next();
                if (tok3->str() == "}")
                {
                    tok3->deleteThis();
                    break;
                }
                tok3->previous()->insertToken(";");
            }
        }
    }
}


void Tokenizer::simplifyComparisonOrder()
{
    // Use "<" comparison instead of ">"
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "[;(] %any% >|>= %any% [);]"))
        {
            if (!tok->next()->isName() && !tok->next()->isNumber())
                continue;
            const Token *operand2 = tok->tokAt(3);
            if (!operand2->isName() && !operand2->isNumber())
                continue;
            const std::string op1(tok->strAt(1));
            tok->next()->str(tok->strAt(3));
            tok->tokAt(3)->str(op1);
            if (tok->tokAt(2)->str() == ">")
                tok->tokAt(2)->str("<");
            else
                tok->tokAt(2)->str("<=");
        }
        else if (Token::Match(tok, "( %num% ==|!= %var% )"))
        {
            if (!tok->next()->isName() && !tok->next()->isNumber())
                continue;
            const std::string op1(tok->strAt(1));
            tok->next()->str(tok->strAt(3));
            tok->tokAt(3)->str(op1);
        }
    }
}

void Tokenizer::simplifyConst()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "[;{}(,] %type% const") &&
            tok->next()->str().find(":") == std::string::npos &&
            tok->next()->str() != "operator")
        {
            tok->tokAt(2)->str(tok->tokAt(1)->str());
            tok->tokAt(1)->str("const");
        }
    }
}

void Tokenizer::getErrorMessages(ErrorLogger *errorLogger, const Settings *settings)
{
    Tokenizer t(settings, errorLogger);
    t.syntaxError(0, ' ');
    t.cppcheckError(0);
    t.duplicateTypedefError(0, 0, "Variable");
    t.duplicateDeclarationError(0, 0, "Variable");
    t.duplicateEnumError(0, 0, "Variable");
    t.unnecessaryQualificationError(0, "type");
}

/** find pattern */
static bool findmatch(const Token *tok1, const Token *tok2, const char pattern[])
{
    for (const Token *tok = tok1; tok && tok != tok2; tok = tok->next())
    {
        if (Token::Match(tok, pattern))
            return true;
    }
    return false;
}

void Tokenizer::simplifyWhile0()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        // while (0)
        const bool while0(Token::Match(tok, "while ( 0|false )"));

        // for (0)
        const bool for0(Token::Match(tok, "for ( %var% = %num% ; %var% < %num% ;") &&
                        tok->strAt(2) == tok->strAt(6) &&
                        tok->strAt(4) == tok->strAt(8));

        if (!while0 && !for0)
            continue;

        if (while0 && Token::simpleMatch(tok->previous(), "}"))
        {
            // find "do"
            Token *tok2 = tok->previous()->link();
            tok2 = tok2 ? tok2->previous() : 0;
            if (tok2 && tok2->str() == "do" && !findmatch(tok2, tok, "continue|break"))
            {
                // delete "do {"
                tok2->deleteThis();
                tok2->deleteThis();

                // delete "} while ( 0 )"
                tok = tok->previous();
                tok->deleteNext();  // while
                tok->deleteNext();  // (
                tok->deleteNext();  // 0
                tok->deleteNext();  // )
                tok->deleteThis();  // }

                continue;
            }
        }

        // remove "while (0) { .. }"
        if (Token::simpleMatch(tok->next()->link(), ") {"))
        {
            const Token *end = tok->next()->link()->next()->link();
            if (!findmatch(tok, end, "continue|break"))
            {
                Token::eraseTokens(tok, end ? end->next() : 0);
                tok->deleteThis();  // delete "while"
            }
        }

    }
}

void Tokenizer::simplifyErrNoInWhile()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (tok->str() != "errno")
            continue;

        Token *endpar = 0;
        if (Token::Match(tok->previous(), "&& errno == EINTR ) { ;| }"))
            endpar = tok->tokAt(3);
        else if (Token::Match(tok->tokAt(-2), "&& ( errno == EINTR ) ) { ;| }"))
            endpar = tok->tokAt(4);
        else
            continue;

        if (Token::simpleMatch(endpar->link()->previous(), "while ("))
        {
            Token *tok1 = tok->previous();
            if (tok1->str() == "(")
                tok1 = tok1->previous();

            // erase "&& errno == EINTR"
            Token::eraseTokens(tok1->previous(), endpar);

            // tok is invalid.. move to endpar
            tok = endpar;

        }
    }
}


void Tokenizer::simplifyFuncInWhile()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (!Token::Match(tok, "while ( %var% ( %var% ) ) {"))
            continue;

        Token *func = tok->tokAt(2);
        Token *var = tok->tokAt(4);
        Token *end = tok->tokAt(7)->link();
        if (!end)
            break;

        tok->str("int");
        tok->insertToken("cppcheck:r");
        tok->tokAt(1)->insertToken("=");
        tok->tokAt(2)->insertToken(func->str());
        tok->tokAt(3)->insertToken("(");
        tok->tokAt(4)->insertToken(var->str());
        tok->tokAt(5)->varId(var->varId());
        tok->tokAt(5)->insertToken(")");
        tok->tokAt(6)->insertToken(";");
        tok->tokAt(7)->insertToken("while");
        tok->tokAt(9)->insertToken("cppcheck:r");
        Token::createMutualLinks(tok->tokAt(4), tok->tokAt(6));
        end->previous()->insertToken("cppcheck:r");
        end->previous()->insertToken("=");
        Token::move(func, func->tokAt(3), end->previous());
        end->previous()->insertToken(";");

        tok = end;
    }
}

void Tokenizer::simplifyStructDecl()
{
    // A counter that is used when giving unique names for anonymous structs.
    unsigned int count = 0;

    // Skip simplification of unions in class definition
    std::list<bool> skip; // true = in function, false = not in function
    skip.push_back(false);

    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        Token *restart;

        // check for start of scope and determine if it is in a function
        if (tok->str() == "{")
            skip.push_back(Token::Match(tok->previous(), "const|)"));

        // end of scope
        else if (tok->str() == "}" && !skip.empty())
            skip.pop_back();

        // check for named struct/union
        else if (Token::Match(tok, "class|struct|union %type% :|{"))
        {
            Token *isStatic = tok->previous() && tok->previous()->str() == "static" ? tok->previous() : NULL;
            Token *type = tok->next();
            Token *next = tok->tokAt(2);

            while (next && next->str() != "{")
                next = next->next();
            if (!next)
                continue;
            skip.push_back(false);
            tok = next->link();
            restart = next;

            // check for named type
            if (Token::Match(tok->next(), "*|&| %type% ,|;|["))
            {
                tok->insertToken(";");
                tok = tok->next();
                if (isStatic)
                {
                    isStatic->deleteThis();
                    tok->insertToken("static");
                    tok = tok->next();
                }
                tok->insertToken(type->str().c_str());
            }

            tok = restart;
        }

        // check for anonymous struct/union
        else if (Token::Match(tok, "struct|union {"))
        {
            bool inFunction = skip.back();
            skip.push_back(false);
            Token *tok1 = tok;

            restart = tok->next();
            tok = tok->next()->link();

            // check for named type
            if (Token::Match(tok->next(), "*|&| %type% ,|;|["))
            {
                std::string name;

                name = "Anonymous" + MathLib::toString<unsigned int>(count++);

                tok1->insertToken(name.c_str());

                tok->insertToken(";");
                tok = tok->next();
                tok->insertToken(name.c_str());
            }

            // unnamed anonymous struct/union so possibly remove it
            else if (tok->next() && tok->next()->str() == ";")
            {
                if (tok1->str() == "union" && inFunction)
                {
                    // Try to create references in the union..
                    Token *tok2 = tok1->tokAt(2);
                    while (tok2)
                    {
                        if (Token::Match(tok2, "%type% %var% ;"))
                            tok2 = tok2->tokAt(3);
                        else
                            break;
                    }
                    if (!Token::simpleMatch(tok2, "} ;"))
                        continue;
                    Token *vartok = 0;
                    tok2 = tok1->tokAt(2);
                    while (Token::Match(tok2, "%type% %var% ;"))
                    {
                        if (!vartok)
                        {
                            vartok = tok2->next();
                            tok2 = tok2->tokAt(3);
                        }
                        else
                        {
                            tok2->insertToken("&");
                            tok2 = tok2->tokAt(2);
                            tok2->insertToken(vartok->str());
                            tok2->next()->varId(vartok->varId());
                            tok2->insertToken("=");
                            tok2 = tok2->tokAt(4);
                        }
                    }
                }

                // don't remove unnamed anonymous unions from a class, struct or union
                if (!(tok1->str() == "union" && !inFunction))
                {
                    skip.pop_back();
                    tok1->deleteThis();
                    if (tok1->next() == tok)
                    {
                        tok1->deleteThis();
                        tok = tok1;
                    }
                    else
                        tok1->deleteThis();
                    restart = tok1->previous();
                    tok->deleteThis();
                    if (tok->next())
                        tok->deleteThis();
                }
            }

            if (!restart)
            {
                simplifyStructDecl();
                return;
            }
            else if (!restart->next())
                return;

            tok = restart;
        }
    }
}

void Tokenizer::simplifyCallingConvention()
{
    const char * pattern = "__cdecl|__stdcall|__fastcall|__thiscall|__clrcall|__syscall|__pascal|__fortran|__far|__near|WINAPI|APIENTRY|CALLBACK";
    while (Token::Match(_tokens, pattern))
    {
        _tokens->deleteThis();
    }
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        while (Token::Match(tok->next(), pattern))
        {
            tok->deleteNext();
        }
    }
}

void Tokenizer::simplifyDeclspec()
{
    while (Token::simpleMatch(_tokens, "__declspec (") && _tokens->next()->link() && _tokens->next()->link()->next())
    {
        Token::eraseTokens(_tokens, _tokens->next()->link()->next());
        _tokens->deleteThis();
    }
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::simpleMatch(tok, "__declspec (") && tok->next()->link() && tok->next()->link()->next())
        {
            Token::eraseTokens(tok, tok->next()->link()->next());
            tok->deleteThis();
        }
    }
}

void Tokenizer::simplifyAttribute()
{
    while (Token::simpleMatch(_tokens, "__attribute__ (") && _tokens->next()->link() && _tokens->next()->link()->next())
    {
        Token::eraseTokens(_tokens, _tokens->next()->link()->next());
        _tokens->deleteThis();
    }
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::simpleMatch(tok, "__attribute__ (") && tok->next()->link() && tok->next()->link()->next())
        {
            if (Token::simpleMatch(tok->tokAt(2), "( unused )"))
            {
                // check if after variable name
                if (Token::Match(tok->next()->link()->next(), ";|="))
                {
                    if (Token::Match(tok->previous(), "%type%"))
                        tok->previous()->isUnused(true);
                }

                // check if before variable name
                else if (Token::Match(tok->next()->link()->next(), "%type%"))
                    tok->next()->link()->next()->isUnused(true);
            }

            Token::eraseTokens(tok, tok->next()->link()->next());
            tok->deleteThis();
            tok = tok->previous();
        }
    }
}

// Remove "volatile", "inline", "register", and "restrict"
void Tokenizer::simplifyKeyword()
{
    const char pattern[] = "volatile|inline|__inline|__forceinline|register|restrict|__restrict|__restrict__";
    while (Token::Match(_tokens, pattern))
    {
        _tokens->deleteThis();
    }
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        while (Token::Match(tok->next(), pattern))
        {
            tok->deleteNext();
        }
    }
}

void Tokenizer::simplifyAssignmentInFunctionCall()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (tok->str() == "(")
            tok = tok->link();

        // Find 'foo(var='. Exclude 'assert(var=' to allow tests to check that assert(...) does not contain side-effects
        else if (Token::Match(tok, "[;{}] %var% ( %var% =") &&
                 Token::simpleMatch(tok->tokAt(2)->link(), ") ;") &&
                 tok->strAt(1) != "assert")
        {
            const std::string funcname(tok->strAt(1));
            const Token * const vartok = tok->tokAt(3);

            // Goto ',' or ')'..
            for (Token *tok2 = tok->tokAt(4); tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "(")
                    tok2 = tok2->link();
                else if (tok2->str() == ";")
                    break;
                else if (tok2->str() == ")" || tok2->str() == ",")
                {
                    tok2 = tok2->previous();

                    tok2->insertToken(vartok->str());
                    tok2->next()->varId(vartok->varId());

                    tok2->insertToken("(");
                    Token::createMutualLinks(tok2->next(), tok->tokAt(2)->link());

                    tok2->insertToken(funcname);
                    tok2->insertToken(";");

                    Token::eraseTokens(tok, vartok);
                    break;
                }
            }
        }
    }
}

// Remove __asm..
void Tokenizer::simplifyAsm()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok->next(), "__asm|_asm|asm {") &&
            tok->tokAt(2)->link() &&
            tok->tokAt(2)->link()->next())
        {
            Token::eraseTokens(tok, tok->tokAt(2)->link()->next());
        }

        else if (Token::Match(tok->next(), "asm|__asm|__asm__ volatile|__volatile__| ("))
        {
            // Goto "("
            Token *partok = tok->tokAt(2);
            if (partok->str() != "(")
                partok = partok->next();
            Token::eraseTokens(tok, partok->link() ? partok->link()->next() : NULL);
        }

        else if (Token::simpleMatch(tok->next(), "__asm"))
        {
            const Token *tok2 = tok->next();
            while (tok2 && (tok2->isNumber() || tok2->isName() || tok2->str() == ","))
                tok2 = tok2->next();
            if (tok2 && tok2->str() == ";")
                Token::eraseTokens(tok, tok2);
            else
                continue;
        }

        else
            continue;

        // insert "asm ( )"
        tok->insertToken(")");
        tok->insertToken("(");
        tok->insertToken("asm");

        Token::createMutualLinks(tok->tokAt(2), tok->tokAt(3));
    }
}

// Simplify bitfields
void Tokenizer::simplifyBitfields()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        Token *last = 0;

        if (Token::Match(tok, ";|{|}|public:|protected:|private: const| %type% %var% :") &&
            !Token::Match(tok->next(), "case|public|protected|private|class|struct") &&
            !Token::simpleMatch(tok->tokAt(2), "default :"))
        {
            int offset = 0;
            if (tok->next()->str() == "const")
                offset = 1;

            Token *tok1 = tok->tokAt(2 + offset);
            if (tok1 && tok1->tokAt(2) &&
                (tok1->tokAt(2)->isBoolean() || Token::Match(tok1->tokAt(2), "%num%") ||
                 !Token::Match(tok1->tokAt(2), "public|protected|private| %type% ::|<|,|{|;")))
            {
                while (tok1->next() && !Token::Match(tok1->next(), ";|,"))
                    tok1->deleteNext();

                last = tok1->next();
            }
        }
        else if (Token::Match(tok, ";|{|}|public:|protected:|private: const| %type% : %any% ;") &&
                 tok->next()->str() != "default")
        {
            int offset = 0;
            if (tok->next()->str() == "const")
                offset = 1;

            if (tok->strAt(3 + offset) != "{")
            {
                Token::eraseTokens(tok->tokAt(0), tok->tokAt(5 + offset));
                tok = tok->previous();
            }
        }

        if (last && last->str() == ",")
        {
            Token *tok1 = last;
            tok1->str(";");

            Token *tok2 = tok->next();
            tok1->insertToken(tok2->str());
            tok1 = tok1->next();
            tok1->isSigned(tok2->isSigned());
            tok1->isUnsigned(tok2->isUnsigned());
            tok1->isLong(tok2->isLong());
        }
    }
}


// Remove __builtin_expect(...), likely(...), and unlikely(...)
void Tokenizer::simplifyBuiltinExpect()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::simpleMatch(tok->next(), "__builtin_expect ("))
        {
            // Count parentheses for tok2
            unsigned int parlevel = 0;
            for (Token *tok2 = tok->next(); tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "(")
                    ++parlevel;
                else if (tok2->str() == ")")
                {
                    if (parlevel <= 1)
                        break;
                    --parlevel;
                }
                if (parlevel == 1 && tok2->str() == ",")
                {
                    if (Token::Match(tok2, ", %num% )"))
                    {
                        tok->deleteNext();
                        Token::eraseTokens(tok2->previous(), tok2->tokAt(2));
                    }
                    break;
                }
            }
        }
        else if (Token::Match(tok->next(), "likely|unlikely ("))
        {
            // remove closing ')'
            tok->tokAt(2)->link()->previous()->deleteNext();

            // remove "likely|unlikely ("
            tok->deleteNext();
            tok->deleteNext();
        }
    }
}


// Remove Microsoft MFC 'DECLARE_MESSAGE_MAP()'
void Tokenizer::simplifyMicrosoftMFC()
{
    // skip if not Windows
    if (!(_settings->platformType == Settings::Win32A ||
          _settings->platformType == Settings::Win32W ||
          _settings->platformType == Settings::Win64))
        return;

    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::simpleMatch(tok->next(), "DECLARE_MESSAGE_MAP ( )"))
        {
            tok->deleteNext();
            tok->deleteNext();
            tok->deleteNext();
        }
        else if (Token::Match(tok->next(), "DECLARE_DYNAMIC|DECLARE_DYNAMIC_CLASS|DECLARE_DYNCREATE ( %any% )"))
        {
            tok->deleteNext();
            tok->deleteNext();
            tok->deleteNext();
            tok->deleteNext();
        }
    }
}

void Tokenizer::simplifyMicrosoftMemoryFunctions()
{
    // skip if not Windows
    if (!(_settings->platformType == Settings::Win32A ||
          _settings->platformType == Settings::Win32W ||
          _settings->platformType == Settings::Win64))
        return;

    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::simpleMatch(tok, "CopyMemory ("))
        {
            tok->str("memcpy");
        }
        else if (Token::simpleMatch(tok, "MoveMemory ("))
        {
            tok->str("memmove");
        }
        else if (Token::simpleMatch(tok, "FillMemory ("))
        {
            // FillMemory(dst, len, val) -> memset(dst, val, len)
            tok->str("memset");

            // find first ','
            Token *tok1 = tok->tokAt(2);
            unsigned int level = 0;
            while (tok1)
            {
                if (tok1->str() == "(")
                    level++;
                else if (tok1->str() == ")")
                    level--;
                else if (level == 0 && tok1->str() == ",")
                    break;

                tok1 = tok1->next();
            }

            // find second ','
            if (tok1)
            {
                Token *tok2 = tok1->next();
                level = 0;
                while (tok2)
                {
                    if (tok2->str() == "(")
                        level++;
                    else if (tok2->str() == ")")
                        level--;
                    else if (level == 0 && tok2->str() == ",")
                        break;

                    tok2 = tok2->next();
                }

                // move second argument to third position
                if (tok2)
                {
                    Token::move(tok1, tok2->previous(), tok->next()->link()->previous());
                }
            }
        }
        else if (Token::simpleMatch(tok, "ZeroMemory ("))
        {
            // ZeroMemory(dst, len) -> memset(dst, 0, len)
            tok->str("memset");

            Token *tok1 = tok->tokAt(2);
            unsigned int level = 0;
            while (tok1)
            {
                if (tok1->str() == "(")
                    level++;
                else if (tok1->str() == ")")
                    level--;
                else if (level == 0 && tok1->str() == ",")
                    break;

                tok1 = tok1->next();
            }

            if (tok1)
            {
                tok1->insertToken("0");
                tok1 = tok1->next();
                tok1->insertToken(",");
            }
        }
    }
}

void Tokenizer::simplifyMicrosoftStringFunctions()
{
    // skip if not Windows
    if (_settings->platformType == Settings::Win32A)
    {
        for (Token *tok = _tokens; tok; tok = tok->next())
        {
            if (Token::simpleMatch(tok, "_topen ("))
            {
                tok->str("open");
            }
            else if (Token::simpleMatch(tok, "_tfopen ("))
            {
                tok->str("fopen");
            }
            else if (Token::simpleMatch(tok, "_tcscat ("))
            {
                tok->str("strcat");
            }
            else if (Token::simpleMatch(tok, "_tcschr ("))
            {
                tok->str("strchr");
            }
            else if (Token::simpleMatch(tok, "_tcscmp ("))
            {
                tok->str("strcmp");
            }
            else if (Token::simpleMatch(tok, "_tcsdup ("))
            {
                tok->str("strdup");
            }
            else if (Token::simpleMatch(tok, "_tcscpy ("))
            {
                tok->str("strcpy");
            }
            else if (Token::simpleMatch(tok, "_tcslen ("))
            {
                tok->str("strlen");
            }
            else if (Token::simpleMatch(tok, "_tcsncat ("))
            {
                tok->str("strncat");
            }
            else if (Token::simpleMatch(tok, "_tcsncpy ("))
            {
                tok->str("strncpy");
            }
            else if (Token::simpleMatch(tok, "_tcsnlen ("))
            {
                tok->str("strnlen");
            }
            else if (Token::simpleMatch(tok, "_tcsrchr ("))
            {
                tok->str("strrchr");
            }
            else if (Token::simpleMatch(tok, "_tcsstr ("))
            {
                tok->str("strstr");
            }
            else if (Token::simpleMatch(tok, "_tcstok ("))
            {
                tok->str("strtok");
            }
            else if (Token::simpleMatch(tok, "_tprintf ("))
            {
                tok->str("printf");
            }
            else if (Token::simpleMatch(tok, "_stprintf ("))
            {
                tok->str("sprintf");
            }
            else if (Token::simpleMatch(tok, "_sntprintf ("))
            {
                tok->str("snprintf");
            }
            else if (Token::simpleMatch(tok, "_tscanf ("))
            {
                tok->str("scanf");
            }
            else if (Token::simpleMatch(tok, "_stscanf ("))
            {
                tok->str("sscanf");
            }
            else if (Token::Match(tok, "_T ( %str% )"))
            {
                tok->deleteThis();
                tok->deleteThis();
                tok->deleteNext();
            }
            else if (Token::Match(tok, "_T ( %any% )") && tok->strAt(2)[0] == '\'')
            {
                tok->deleteThis();
                tok->deleteThis();
                tok->deleteNext();
            }
        }
    }
    else if (_settings->platformType == Settings::Win32W ||
             _settings->platformType == Settings::Win64)
    {
        for (Token *tok = _tokens; tok; tok = tok->next())
        {
            if (Token::simpleMatch(tok, "_tcscat ("))
            {
                tok->str("wcscat");
            }
            else if (Token::simpleMatch(tok, "_tcschr ("))
            {
                tok->str("wcschr");
            }
            else if (Token::simpleMatch(tok, "_tcscmp ("))
            {
                tok->str("wcscmp");
            }
            else if (Token::simpleMatch(tok, "_tcscpy ("))
            {
                tok->str("wcscpy");
            }
            else if (Token::simpleMatch(tok, "_tcsdup ("))
            {
                tok->str("wcsdup");
            }
            else if (Token::simpleMatch(tok, "_tcslen ("))
            {
                tok->str("wcslen");
            }
            else if (Token::simpleMatch(tok, "_tcsncat ("))
            {
                tok->str("wcsncat");
            }
            else if (Token::simpleMatch(tok, "_tcsncpy ("))
            {
                tok->str("wcsncpy");
            }
            else if (Token::simpleMatch(tok, "_tcsnlen ("))
            {
                tok->str("wcsnlen");
            }
            else if (Token::simpleMatch(tok, "_tcsrchr ("))
            {
                tok->str("wcsrchr");
            }
            else if (Token::simpleMatch(tok, "_tcsstr ("))
            {
                tok->str("wcsstr");
            }
            else if (Token::simpleMatch(tok, "_tcstok ("))
            {
                tok->str("wcstok");
            }
            else if (Token::simpleMatch(tok, "_tprintf ("))
            {
                tok->str("wprintf");
            }
            else if (Token::simpleMatch(tok, "_stprintf ("))
            {
                tok->str("swprintf");
            }
            else if (Token::simpleMatch(tok, "_sntprintf ("))
            {
                tok->str("snwprintf");
            }
            else if (Token::simpleMatch(tok, "_tscanf ("))
            {
                tok->str("wscanf");
            }
            else if (Token::simpleMatch(tok, "_stscanf ("))
            {
                tok->str("swscanf");
            }
            else if (Token::Match(tok, "_T ( %str% )"))
            {
                tok->deleteThis();
                tok->deleteThis();
                tok->deleteNext();
            }
            else if (Token::Match(tok, "_T ( %any% )") && tok->strAt(2)[0] == '\'')
            {
                tok->deleteThis();
                tok->deleteThis();
                tok->deleteNext();
            }
        }
    }
}

// Remove Borland code
void Tokenizer::simplifyBorland()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "( __closure * %var% )"))
        {
            tok->deleteNext();
        }
    }

    // I think that these classes are always declared at the outer scope
    // I save some time by ignoring inner classes.
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (tok->str() == "{")
        {
            tok = tok->link();
            if (!tok)
                break;
        }

        if (Token::Match(tok, "class %var% :|{"))
        {
            // count { and } for tok2
            unsigned int indentlevel = 0;
            for (Token *tok2 = tok; tok2; tok2 = tok2->next())
            {
                if (tok2->str() == "{")
                {
                    if (indentlevel == 0)
                        indentlevel = 1;
                    else
                        tok2 = tok2->link();
                }
                else if (tok2->str() == "}")
                {
                    break;
                }
                else if (tok2->str() == "__property" &&
                         Token::Match(tok2->previous(), ";|{|}|protected:|public:|__published:"))
                {
                    while (tok2->next() && !Token::Match(tok2, "{|;"))
                        tok2->deleteThis();
                    if (Token::simpleMatch(tok2, "{"))
                    {
                        Token::eraseTokens(tok2, tok2->link());
                        tok2->deleteThis();
                        tok2->deleteThis();

                        // insert "; __property ;"
                        tok2->previous()->insertToken(";");
                        tok2->previous()->insertToken("__property");
                        tok2->previous()->insertToken(";");
                    }
                }
            }
        }
    }
}

// Remove Qt signals and slots
void Tokenizer::simplifyQtSignalsSlots()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        // check for emit which can be outside of class
        if (Token::Match(tok, "emit|Q_EMIT %var% (") &&
            Token::simpleMatch(tok->tokAt(2)->link(), ") ;"))
        {
            tok->deleteThis();
        }
        else if (!Token::Match(tok, "class %var% :"))
            continue;

        if (tok->previous() && tok->previous()->str() == "enum")
        {
            tok = tok->tokAt(2);
            continue;
        }

        // count { and } for tok2
        unsigned int indentlevel = 0;
        for (Token *tok2 = tok; tok2; tok2 = tok2->next())
        {
            if (tok2->str() == "{")
            {
                indentlevel++;
                if (indentlevel == 1)
                    tok = tok2;
                else
                    tok2 = tok2->link();
            }
            else if (tok2->str() == "}")
            {
                indentlevel--;
                if (indentlevel == 0)
                    break;
            }

            if (Token::simpleMatch(tok2->next(), "Q_OBJECT"))
            {
                tok2->deleteNext();
            }
            else if (Token::Match(tok2->next(), "public|protected|private slots|Q_SLOTS :"))
            {
                tok2 = tok2->next();
                tok2->str(tok2->str() + ":");
                tok2->deleteNext();
                tok2->deleteNext();
                tok2 = tok2->previous();
            }
            else if (Token::Match(tok2->next(), "signals|Q_SIGNALS :"))
            {
                tok2 = tok2->next();
                tok2->str("protected:");
                tok2->deleteNext();
            }
            else if (Token::Match(tok2->next(), "emit|Q_EMIT %var% (") &&
                     Token::simpleMatch(tok2->tokAt(3)->link(), ") ;"))
            {
                tok2->deleteNext();
            }
        }
    }
}

const SymbolDatabase *Tokenizer::getSymbolDatabase() const
{
    if (!_symbolDatabase)
        _symbolDatabase = new SymbolDatabase(this, _settings, _errorLogger);

    return _symbolDatabase;
}

void Tokenizer::simplifyOperatorName()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (tok->str() == "operator")
        {
            // operator op
            std::string op;
            Token *par = tok->next();
            bool done = false;
            while (!done && par)
            {
                done = true;
                if (par && par->isName())
                {
                    op += par->str();
                    par = par->next();
                    done = false;
                }
                if (Token::Match(par, "=|.|%op%"))
                {
                    op += par->str();
                    par = par->next();
                    done = false;
                }
                if (Token::simpleMatch(par, "[ ]"))
                {
                    op += "[]";
                    par = par->next()->next();
                    done = false;
                }
                if (Token::Match(par, "( *| )"))
                {
                    // break out and simplify..
                    if (Token::Match(par, "( ) const| [=;{),]"))
                        break;

                    while (par->str() != ")")
                    {
                        op += par->str();
                        par = par->next();
                    }
                    op += ")";
                    par = par->next();
                    done = false;
                }
            }

            if (par && Token::Match(par->link(), ") const| [=;{),]"))
            {
                tok->str("operator" + op);
                Token::eraseTokens(tok,par);
            }
        }
    }
}

// remove unnecessary member qualification..
void Tokenizer::removeUnnecessaryQualification()
{
    std::vector<Space> classInfo;
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "class|struct|namespace %type% :|{") &&
            (!tok->previous() || (tok->previous() && tok->previous()->str() != "enum")))
        {
            Space info;
            info.isNamespace = tok->str() == "namespace";
            tok = tok->next();
            info.className = tok->str();
            tok = tok->next();
            while (tok && tok->str() != "{")
                tok = tok->next();
            if (!tok)
                return;
            info.classEnd = tok->link();
            classInfo.push_back(info);
        }
        else if (!classInfo.empty())
        {
            if (tok == classInfo.back().classEnd)
                classInfo.pop_back();
            else if (tok->str() == classInfo.back().className &&
                     !classInfo.back().isNamespace && tok->previous()->str() != ":" &&
                     (Token::Match(tok, "%type% :: ~| %type% (") ||
                      Token::Match(tok, "%type% :: operator")))
            {
                int offset = 3;
                if (tok->strAt(2) == "operator")
                {
                    const Token *tok1 = tok->tokAt(offset);

                    // check for operator ()
                    if (tok1->str() == "(")
                    {
                        tok1 = tok1->next();
                        offset++;
                    }

                    while (tok1 && tok1->str() != "(")
                    {
                        tok1 = tok1->next();
                        offset++;
                    }
                }
                else if (tok->strAt(2) == "~")
                    offset++;

                if (Token::Match(tok->tokAt(offset)->link(), ") const| {|;|:"))
                {
                    std::string qualification = tok->str() + "::";

                    // check for extra qualification
                    /** @todo this should be made more generic to handle more levels */
                    if (Token::Match(tok->tokAt(-2), "%type% ::"))
                    {
                        if (classInfo.size() >= 2)
                        {
                            if (classInfo.at(classInfo.size() - 2).className != tok->strAt(-2))
                                continue;
                            else
                                qualification = tok->strAt(-2) + "::" + qualification;
                        }
                        else
                            continue;
                    }

                    if (_settings && _settings->isEnabled("portability"))
                        unnecessaryQualificationError(tok, qualification);

                    tok->deleteThis();
                    tok->deleteThis();
                }
            }
        }
    }
}

void Tokenizer::unnecessaryQualificationError(const Token *tok, const std::string &qualification)
{
    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    if (tok)
    {
        ErrorLogger::ErrorMessage::FileLocation loc;
        loc.line = tok->linenr();
        loc.setfile(file(tok));
        locationList.push_back(loc);
    }

    const ErrorLogger::ErrorMessage errmsg(locationList,
                                           Severity::portability,
                                           "Extra qualification \'" + qualification + "\' unnecessary and considered an error by many compilers.",
                                           "unnecessaryQualification",
                                           false);

    if (_errorLogger)
        _errorLogger->reportErr(errmsg);
    else
        Check::reportError(errmsg);
}

void Tokenizer::simplifyReturn()
{
    for (Token *tok = _tokens; tok; tok = tok->next())
    {
        if (Token::Match(tok, "return strncat ( %any% , %any% , %any% ) ;"))
        {
            // Change to: strncat ( %any% , %any% , %any% ) ;
            tok->deleteNext();
            tok->str("strncat");

            // Change to: strncat ( %any% , %any% , %any% ) ; return %any% ;
            tok->tokAt(8)->insertToken("return");
            copyTokens(tok->tokAt(9), tok->tokAt(2), tok->tokAt(2));
            tok->tokAt(10)->insertToken(";");
        }
    }
}

void Tokenizer::printUnknownTypes()
{
    getSymbolDatabase();

    std::set<std::string> unknowns;

    for (size_t i = 1; i <= _varId; i++)
    {
        const Variable *var = _symbolDatabase->getVariableFromVarId(i);

        // is unknown record type?
        if (var && var->isClass() && !var->type())
        {
            std::string    name;

            // single token type?
            if (var->typeStartToken() == var->typeEndToken())
                name = var->typeStartToken()->str();

            // complcated type
            else
            {
                const Token *tok = var->typeStartToken();
                int level = 0;

                while (tok)
                {
                    // skip pointer and reference part of type
                    if (level == 0 && (tok->str() ==  "*" || tok->str() == "&"))
                        break;

                    name += tok->str();

                    if (Token::Match(tok, "struct|union"))
                        name += " ";

                    // pointers and referennces are OK in template
                    else if (tok->str() == "<")
                        level++;
                    else if (tok->str() == ">")
                        level--;

                    if (tok == var->typeEndToken())
                        break;

                    tok = tok->next();
                }
            }

            unknowns.insert(name);
        }
    }

    if (!unknowns.empty())
    {
        std::ostringstream ss;

        ss << unknowns.size() << " unknown types:" << std::endl;

        std::set<std::string>::const_iterator it;
        size_t count = 1;

        for (it = unknowns.begin(); it != unknowns.end(); ++it, ++count)
            ss << count << ": " << *it << std::endl;

        if (_errorLogger)
            _errorLogger->reportOut(ss.str());
    }
}
